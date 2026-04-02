// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "sendmail_threadtasks.h"
#include "server/serverconfig.h"
#include "common/deadline.h"
#include "curl/curl.h"

#include <memory>
#include <mutex>

namespace
{
std::once_flag g_curlInitFlag;

bool ensureCurlInitialized()
{
	bool ok = true;
	std::call_once(g_curlInitFlag, [&ok]() {
		const CURLcode curlCode = curl_global_init(CURL_GLOBAL_ALL);
		ok = (curlCode == CURLE_OK);
	});

	return ok;
}

struct CurlSlistDeleter
{
	void operator()(curl_slist* list) const
	{
		if (list != nullptr)
		{
			curl_slist_free_all(list);
		}
	}
};

struct CurlMimeDeleter
{
	void operator()(curl_mime* mime) const
	{
		if (mime != nullptr)
		{
			curl_mime_free(mime);
		}
	}
};

std::string buildSmtpUrl(const KBEngine::EmailServerInfo& emailServerInfo)
{
	if (emailServerInfo.smtp_server.find("://") != std::string::npos)
	{
		return fmt::format("{}:{}", emailServerInfo.smtp_server, emailServerInfo.smtp_port);
	}

	const char* scheme = emailServerInfo.smtp_port == 465 ? "smtps" : "smtp";
	return fmt::format("{}://{}:{}", scheme, emailServerInfo.smtp_server, emailServerInfo.smtp_port);
}

std::string buildFromAddress(const KBEngine::EmailServerInfo& emailServerInfo)
{
	return KBEngine::strutil::kbe_trim(emailServerInfo.username);
}
}

namespace KBEngine{

//-------------------------------------------------------------------------------------
PreparedSendMailRequest SendEMailTask::prepareRequest(const EmailServerInfo& emailServerInfo)
{
	PreparedSendMailRequest request;
	request.smtpUrl = buildSmtpUrl(emailServerInfo);
	request.fromAddress = buildFromAddress(emailServerInfo);
	request.toAddress = strutil::kbe_trim(emailaddr_);
	request.subjectLine = subject();
	request.username = emailServerInfo.username;
	request.password = emailServerInfo.password;
	request.useSsl = emailServerInfo.smtp_port == 465;

	std::string mailmessage = message();

	KBEngine::strutil::kbe_replace(mailmessage, "${backlink}", fmt::format("http://{}:{}/{}_{}",
		cbaddr_,
		cbport_,
		getopkey(),
		code_));

	KBEngine::strutil::kbe_replace(mailmessage, "${username}", emailaddr_);
	KBEngine::strutil::kbe_replace(mailmessage, "${code}", code_);
	request.messageHtml = KBEngine::strutil::kbe_trim(mailmessage);
	request.mailFrom = fmt::format("<{}>", request.fromAddress);

	if (!request.username.empty())
	{
		if (emailServerInfo.smtp_auth == 1)
		{
			request.loginOptions = "AUTH=LOGIN";
		}
		else if (emailServerInfo.smtp_auth == 2)
		{
			request.loginOptions = "AUTH=PLAIN";
		}
	}

	return request;
}

//-------------------------------------------------------------------------------------
bool SendEMailTask::process()
{
	if (!ensureCurlInitialized())
	{
		ERROR_MSG(fmt::format("SendEMailTask::process: sendmail[{}] curl_global_init failed.\n", getopkey()));
		return false;
	}

	const PreparedSendMailRequest request = prepareRequest(g_kbeSrvConfig.emailServerInfo_);

	if (!request.isValid())
	{
		ERROR_MSG(fmt::format("SendEMailTask::process: sendmail[{}] invalid smtp config, "
			"url='{}', from='{}', to='{}'.\n", getopkey(), request.smtpUrl, request.fromAddress, request.toAddress));
		return false;
	}

	CURL* curl = curl_easy_init();
	if (curl == nullptr)
	{
		ERROR_MSG(fmt::format("SendEMailTask::process: sendmail[{}] curl_easy_init failed.\n", getopkey()));
		return false;
	}

	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curlHandle(curl, &curl_easy_cleanup);
	std::unique_ptr<curl_slist, CurlSlistDeleter> recipients(curl_slist_append(nullptr, request.toAddress.c_str()));
	if (!recipients)
	{
		ERROR_MSG(fmt::format("SendEMailTask::process: sendmail[{}] append rcpt failed.\n", getopkey()));
		return false;
	}

	curl_slist* headersRaw = nullptr;
	headersRaw = curl_slist_append(headersRaw, fmt::format("To: <{}>", request.toAddress).c_str());
	headersRaw = curl_slist_append(headersRaw, fmt::format("From: <{}>", request.fromAddress).c_str());
	headersRaw = curl_slist_append(headersRaw, fmt::format("Subject: {}", request.subjectLine).c_str());
	headersRaw = curl_slist_append(headersRaw, "Mime-Version: 1.0");
	if (!headersRaw)
	{
		ERROR_MSG(fmt::format("SendEMailTask::process: sendmail[{}] append headers failed.\n", getopkey()));
		return false;
	}

	std::unique_ptr<curl_slist, CurlSlistDeleter> headers(headersRaw);
	std::unique_ptr<curl_mime, CurlMimeDeleter> mime(curl_mime_init(curl));
	if (!mime)
	{
		ERROR_MSG(fmt::format("SendEMailTask::process: sendmail[{}] curl_mime_init failed.\n", getopkey()));
		return false;
	}

	curl_mimepart* part = curl_mime_addpart(mime.get());
	if (part == nullptr)
	{
		ERROR_MSG(fmt::format("SendEMailTask::process: sendmail[{}] curl_mime_addpart failed.\n", getopkey()));
		return false;
	}

	curl_mime_data(part, request.messageHtml.c_str(), CURL_ZERO_TERMINATED);
	curl_mime_type(part, "text/html");

	char errorBuffer[CURL_ERROR_SIZE] = {0};
	curl_easy_setopt(curl, CURLOPT_URL, request.smtpUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_USE_SSL, request.useSsl ? CURLUSESSL_ALL : CURLUSESSL_TRY);
	curl_easy_setopt(curl, CURLOPT_MAIL_FROM, request.mailFrom.c_str());
	curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients.get());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers.get());
	curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime.get());
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 10000L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 30000L);

	if (!request.username.empty())
	{
		curl_easy_setopt(curl, CURLOPT_USERNAME, request.username.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, request.password.c_str());

		if (!request.loginOptions.empty())
		{
			curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, request.loginOptions.c_str());
		}
	}

	const CURLcode curlCode = curl_easy_perform(curl);
	long responseCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

	if (curlCode != CURLE_OK)
	{
		const std::string curlError = errorBuffer[0] != '\0' ? errorBuffer : curl_easy_strerror(curlCode);
		ERROR_MSG(fmt::format("SendEMailTask::process: sendmail[{}] failed, curlCode={}, smtpCode={}, error={}\n",
			getopkey(), static_cast<int>(curlCode), responseCode, curlError));
		return false;
	}

	INFO_MSG(fmt::format("SendEMailTask::process: sendmail[{}] success, smtpCode={}\n", getopkey(), responseCode));
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState SendEMailTask::presentMainThread()
{
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
const char* SendActivateEMailTask::subject()
{
	return g_kbeSrvConfig.emailAtivationInfo_.subject.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendActivateEMailTask::message()
{
	return g_kbeSrvConfig.emailAtivationInfo_.message.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendResetPasswordEMailTask::subject()
{
	return g_kbeSrvConfig.emailResetPasswordInfo_.subject.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendResetPasswordEMailTask::message()
{
	return g_kbeSrvConfig.emailResetPasswordInfo_.message.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendBindEMailTask::subject()
{
	return g_kbeSrvConfig.emailBindInfo_.subject.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendBindEMailTask::message()
{
	return g_kbeSrvConfig.emailBindInfo_.message.c_str();
}

//-------------------------------------------------------------------------------------
}
