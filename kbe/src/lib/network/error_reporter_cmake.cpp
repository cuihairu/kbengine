// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "network/error_reporter.h"

#ifndef CODE_INLINE
#include "network/error_reporter.inl"
#endif

#include <cstdarg>
#include <cstdio>
#include <sstream>

namespace KBEngine {
namespace Network {

const uint ErrorReporter::ERROR_REPORT_MIN_PERIOD_MS = 2000;
const uint ErrorReporter::ERROR_REPORT_COUNT_MAX_LIFETIME_MS = 10000;

ErrorReporter::ErrorReporter(EventDispatcher& dispatcher) :
  reportLimitTimerHandle_(),
  errorsAndCounts_()
{
  reportLimitTimerHandle_ = dispatcher.addTimer(ERROR_REPORT_MIN_PERIOD_MS * 1000, this);
}

ErrorReporter::~ErrorReporter()
{
  reportLimitTimerHandle_.cancel();
}

std::string ErrorReporter::addressErrorToString(
  const Address& address,
  const std::string& errorString)
{
  std::ostringstream out;
  out << address.c_str() << ": " << errorString;
  return out.str();
}

std::string ErrorReporter::addressErrorToString(
  const Address& address,
  const std::string& errorString,
  const ErrorReportAndCount& reportAndCount,
  const uint64& now)
{
  const int64 deltaStamps = now - reportAndCount.lastReportStamps;
  const double deltaMillis = 1000.0 * deltaStamps / stampsPerSecondD();

  char buf[512];
  std::snprintf(
    buf,
    sizeof(buf),
    "%u reports of '%s' in the last %.00fms",
    reportAndCount.count,
    addressErrorToString(address, errorString).c_str(),
    deltaMillis);

  return std::string(buf);
}

void ErrorReporter::reportError(const Address& address, const char* format, ...)
{
  char buf[1024];
  va_list va;
  va_start(va, format);
  std::vsnprintf(buf, sizeof(buf), format, va);
  va_end(va);
  buf[sizeof(buf) - 1] = '\0';

  this->addReport(address, std::string(buf));
}

void ErrorReporter::reportException(
  Reason reason,
  const Address& addr,
  const char* prefix,
  const char* suffix)
{
  NetworkException ne(reason, addr);
  this->reportException(ne, prefix, suffix);
}

void ErrorReporter::reportException(const NetworkException& ne, const char* prefix, const char* suffix)
{
  Address offender(0, 0);
  ne.getAddress(offender);

  if (prefix)
  {
    if (!suffix)
    {
      this->reportError(offender, "%s: Exception occurred: %s", prefix, reasonToString(ne.reason()));
    }
    else
    {
      this->reportError(
        offender,
        "%s: Exception occurred: %s %s",
        prefix,
        reasonToString(ne.reason()),
        suffix);
    }
  }
  else
  {
    if (!suffix)
    {
      this->reportError(offender, "Exception occurred: %s", reasonToString(ne.reason()));
    }
    else
    {
      this->reportError(
        offender,
        "Exception occurred: %s %s",
        reasonToString(ne.reason()),
        suffix);
    }
  }
}

void ErrorReporter::addReport(const Address& address, const std::string& errorString)
{
  const AddressAndErrorString addressError(address, errorString);
  ErrorsAndCounts::iterator searchIter = errorsAndCounts_.find(addressError);
  const uint64 now = timestamp();

  if (searchIter != errorsAndCounts_.end())
  {
    ErrorReportAndCount& reportAndCount = searchIter->second;
    reportAndCount.count++;

    const int64 millisSinceLastReport =
      1000 * (now - reportAndCount.lastReportStamps) / stampsPerSecond();

    reportAndCount.lastRaisedStamps = now;

    if (millisSinceLastReport >= ERROR_REPORT_MIN_PERIOD_MS)
    {
      KBE_ERROR_REPORTER_ERROR_MSG(
        fmt::format("{}\n", addressErrorToString(address, errorString, reportAndCount, now).c_str()));
      reportAndCount.count = 0;
      reportAndCount.lastReportStamps = now;
    }
  }
  else
  {
    KBE_ERROR_REPORTER_ERROR_MSG(fmt::format("{}\n", addressErrorToString(address, errorString).c_str()));
    ErrorReportAndCount reportAndCount = {now, now, 0};
    errorsAndCounts_[addressError] = reportAndCount;
  }
}

void ErrorReporter::reportPendingExceptions(bool reportBelowThreshold)
{
  const uint64 now = timestamp();
  ErrorsAndCounts::iterator staleIter = this->errorsAndCounts_.end();

  for (ErrorsAndCounts::iterator exceptionCountIter = this->errorsAndCounts_.begin();
       exceptionCountIter != this->errorsAndCounts_.end();
       ++exceptionCountIter)
  {
    if (staleIter != this->errorsAndCounts_.end())
    {
      this->errorsAndCounts_.erase(staleIter);
      staleIter = this->errorsAndCounts_.end();
    }

    const AddressAndErrorString& addressError = exceptionCountIter->first;
    ErrorReportAndCount& reportAndCount = exceptionCountIter->second;

    const int64 millisSinceLastReport =
      1000 * (now - reportAndCount.lastReportStamps) / stampsPerSecond();

    if (reportBelowThreshold || millisSinceLastReport >= ERROR_REPORT_MIN_PERIOD_MS)
    {
      if (reportAndCount.count)
      {
        KBE_ERROR_REPORTER_ERROR_MSG(
          fmt::format(
            "{}\n",
            addressErrorToString(addressError.first, addressError.second, reportAndCount, now).c_str()));
        reportAndCount.count = 0;
        reportAndCount.lastReportStamps = now;
      }
    }

    const uint64 sinceLastRaisedMillis =
      1000 * (now - reportAndCount.lastRaisedStamps) / stampsPerSecond();
    if (sinceLastRaisedMillis > ERROR_REPORT_COUNT_MAX_LIFETIME_MS)
    {
      staleIter = exceptionCountIter;
    }
  }

  if (staleIter != this->errorsAndCounts_.end())
  {
    this->errorsAndCounts_.erase(staleIter);
  }
}

void ErrorReporter::handleTimeout(TimerHandle, void*)
{
  this->reportPendingExceptions(false);
}

} // namespace Network
} // namespace KBEngine
