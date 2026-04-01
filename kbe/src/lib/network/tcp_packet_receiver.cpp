// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "tcp_packet_receiver.h"
#ifndef CODE_INLINE
#include "tcp_packet_receiver.inl"
#endif

#include "network/address.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/event_poller.h"
#include "network/error_reporter.h"
#include "helper/debug_helper.h"
#include <openssl/err.h>

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
static ObjectPool<TCPPacketReceiver> _g_objPool("TCPPacketReceiver");
ObjectPool<TCPPacketReceiver>& TCPPacketReceiver::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
TCPPacketReceiver* TCPPacketReceiver::createPoolObject(const std::string& logPoint)
{
	return _g_objPool.createObject(logPoint);
}

//-------------------------------------------------------------------------------------
void TCPPacketReceiver::reclaimPoolObject(TCPPacketReceiver* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void TCPPacketReceiver::destroyObjPool()
{
	if (DebugHelper::isInit())
	{
		DEBUG_MSG(fmt::format("TCPPacketReceiver::destroyObjPool(): size {}.\n",
			_g_objPool.size()));
	}

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
TCPPacketReceiver::SmartPoolObjectPtr TCPPacketReceiver::createSmartPoolObj(const std::string& logPoint)
{
	return SmartPoolObjectPtr(new SmartPoolObject<TCPPacketReceiver>(ObjPool().createObject(logPoint), _g_objPool));
}

//-------------------------------------------------------------------------------------
TCPPacketReceiver::TCPPacketReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	PacketReceiver(endpoint, networkInterface)
{
}

//-------------------------------------------------------------------------------------
TCPPacketReceiver::~TCPPacketReceiver()
{
	//DEBUG_MSG("TCPPacketReceiver::~TCPPacketReceiver()\n");
}

//-------------------------------------------------------------------------------------
bool TCPPacketReceiver::processRecv(bool expectingPacket)
{
	Channel* pChannel = getChannel();
	KBE_ASSERT(pChannel != NULL);

	if(pChannel->condemn() > 0)
	{
		return false;
	}

	TCPPacket* pReceiveWindow = TCPPacket::createPoolObject(OBJECTPOOL_POINT);
	int len = pReceiveWindow->recvFromEndPoint(*pEndpoint_);

	if (len < 0)
	{
		TCPPacket::reclaimPoolObject(pReceiveWindow);

		PacketReceiver::RecvState rstate = this->checkSocketErrors(len, expectingPacket);

		if(rstate == PacketReceiver::RECV_STATE_INTERRUPT)
		{
			onGetError(pChannel, fmt::format("TCPPacketReceiver::processRecv(): error={}\n", kbe_lasterror()));
			return false;
		}

		return rstate == PacketReceiver::RECV_STATE_CONTINUE;
	}
	else if(len == 0) // ¿Í»§¶ËÕý³£ÍË³ö
	{
		TCPPacket::reclaimPoolObject(pReceiveWindow);
		onGetError(pChannel, "disconnected");
		return false;
	}
	
	Reason ret = this->processPacket(pChannel, pReceiveWindow);

	if(ret != REASON_SUCCESS)
		this->dispatcher().errorReporter().reportException(ret, pEndpoint_->addr());
	
	return true;
}

//-------------------------------------------------------------------------------------
void TCPPacketReceiver::onGetError(Channel* pChannel, const std::string& err)
{
	pChannel->condemn(err);
	pChannel->networkInterface().deregisterChannel(pChannel);
	pChannel->destroy();
	Network::Channel::reclaimPoolObject(pChannel);
}

//-------------------------------------------------------------------------------------
Reason TCPPacketReceiver::processFilteredPacket(Channel* pChannel, Packet * pPacket)
{
	// Èç¹ûÎªNone£¬ Ôò¿ÉÄÜÊÇ±»¹ýÂËÆ÷¹ýÂËµôÁË(¹ýÂËÆ÷ÕýÔÚ°´ÕÕ×Ô¼ºµÄ¹æÔò×é°ü½âÃÜ)
	if(pPacket)
	{
		pChannel->addReceiveWindow(pPacket);
	}

	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
PacketReceiver::RecvState TCPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	DWORD wsaErr = WSAGetLastError();
#endif //def _WIN32

	if (
#if KBE_PLATFORM == PLATFORM_WIN32
		wsaErr == WSAEWOULDBLOCK && !expectingPacket// send³ö´í´ó¸ÅÊÇ»º³åÇøÂúÁË, recv³ö´íÒÑ¾­ÎÞÊý¾Ý¿É¶ÁÁË
#else
		errno == EAGAIN && !expectingPacket			// recv»º³åÇøÒÑ¾­ÎÞÊý¾Ý¿É¶ÁÁË
#endif
		)
	{
		return RECV_STATE_BREAK;
	}

#if KBE_PLATFORM == PLATFORM_UNIX || KBE_PLATFORM == PLATFORM_APPLE
	if (errno == EAGAIN ||							// ÒÑ¾­ÎÞÊý¾Ý¿É¶ÁÁË
		errno == ECONNREFUSED ||					// Á¬½Ó±»·þÎñÆ÷¾Ü¾ø
		errno == EHOSTUNREACH)						// Ä¿µÄµØÖ·²»¿Éµ½´ï
	{
		this->dispatcher().errorReporter().reportException(
				REASON_NO_SUCH_PORT);

		return RECV_STATE_BREAK;
	}
#else
	/*
	´æÔÚµÄÁ¬½Ó±»Ô¶³ÌÖ÷»úÇ¿ÖÆ¹Ø±Õ¡£Í¨³£Ô­ÒòÎª£ºÔ¶³ÌÖ÷»úÉÏ¶ÔµÈ·½Ó¦ÓÃ³ÌÐòÍ»È»Í£Ö¹ÔËÐÐ£¬»òÔ¶³ÌÖ÷»úÖØÐÂÆô¶¯£¬
	»òÔ¶³ÌÖ÷»úÔÚÔ¶³Ì·½Ì×½Ó×ÖÉÏÊ¹ÓÃÁË¡°Ç¿ÖÆ¡±¹Ø±Õ£¨²Î¼ûsetsockopt(SO_LINGER)£©¡£
	ÁíÍâ£¬ÔÚÒ»¸ö»ò¶à¸ö²Ù×÷ÕýÔÚ½øÐÐÊ±£¬Èç¹ûÁ¬½ÓÒò¡°keep-alive¡±»î¶¯¼ì²âµ½Ò»¸öÊ§°Ü¶øÖÐ¶Ï£¬Ò²¿ÉÄÜµ¼ÖÂ´Ë´íÎó¡£
	´ËÊ±£¬ÕýÔÚ½øÐÐµÄ²Ù×÷ÒÔ´íÎóÂëWSAENETRESETÊ§°Ü·µ»Ø£¬ºóÐø²Ù×÷½«Ê§°Ü·µ»Ø´íÎóÂëWSAECONNRESET
	*/
	switch(wsaErr)
	{
	case WSAECONNRESET:
		WARNING_MSG(fmt::format("TCPPacketReceiver::processPendingEvents({}): "
			"Throwing REASON_GENERAL_NETWORK - WSAECONNRESET\n", (pEndpoint_ ? pEndpoint_->addr().c_str() : "")));
		return RECV_STATE_INTERRUPT;
	case WSAECONNABORTED:
		WARNING_MSG(fmt::format("TCPPacketReceiver::processPendingEvents({}): "
			"Throwing REASON_GENERAL_NETWORK - WSAECONNABORTED\n", (pEndpoint_ ? pEndpoint_->addr().c_str() : "")));
		return RECV_STATE_INTERRUPT;
	default:
		break;

	};

#endif // unix

#if KBE_PLATFORM == PLATFORM_WIN32
	if (wsaErr == 0
#else
	if (errno == 0
#endif
		&& pEndPoint()->isSSL())
	{
		long sslerr = ERR_get_error();
		if (sslerr > 0)
		{
			WARNING_MSG(fmt::format("TCPPacketReceiver::processPendingEvents({}): "
				"Throwing SSL - {}\n",
				(pEndpoint_ ? pEndpoint_->addr().c_str() : ""), ERR_error_string(sslerr, NULL)));

			return RECV_STATE_INTERRUPT;
		}
	}

#if KBE_PLATFORM == PLATFORM_WIN32
	WARNING_MSG(fmt::format("TCPPacketReceiver::processPendingEvents({}): "
				"Throwing REASON_GENERAL_NETWORK - {}\n",
				(pEndpoint_ ? pEndpoint_->addr().c_str() : ""), wsaErr));
#else
	WARNING_MSG(fmt::format("TCPPacketReceiver::processPendingEvents({}): "
				"Throwing REASON_GENERAL_NETWORK - {}\n",
				(pEndpoint_ ? pEndpoint_->addr().c_str() : ""), kbe_strerror()));
#endif

	//this->dispatcher().errorReporter().reportException(
	//		REASON_GENERAL_NETWORK);

	return RECV_STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------
}
}
