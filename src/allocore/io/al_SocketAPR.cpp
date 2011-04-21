#include <cstring>
#include "allocore/io/al_Socket.hpp"
#include "allocore/system/al_Config.h"

#include "../private/al_ImplAPR.h"
#ifdef AL_LINUX
#include "apr-1.0/apr_network_io.h"
#else
#include "apr-1/apr_network_io.h"
#endif

#define PRINT_SOCKADDR(s)\
	printf("%s %s\n", s->hostname, s->servname);

namespace al{

struct Socket::Impl : public ImplAPR {

	Impl()
	:	ImplAPR(), mPort(0), mAddress(""), mSockAddr(0), mSock(0), mSender(false)
	{}

	Impl(unsigned int port, const char * address, al_sec timeout_, bool sender)
	:	ImplAPR(), mPort(port), mAddress(address), mSockAddr(0), mSock(0), mSender(sender)
	{
//		// opens the socket also:
//		timeout(timeout_);
		open(port, address, timeout_, sender);
	}

	void close(){
		if(opened()){
			check_apr(apr_socket_close(mSock));
			mSock=0;
		}
	}

	#define BAILONFAIL(func)\
		if(APR_SUCCESS != check_apr(func)){\
			printf("failed to create socket at %s:%i\n", address.c_str(), port);\
			close();\
			return false;\
		}

	bool open(unsigned int port, std::string address, al_sec timeout, bool sender){
		close();

		/* @see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html */

		mPort = port;
		mSender = sender;
		mAddress = address;

		BAILONFAIL(
			apr_sockaddr_info_get(&mSockAddr, mAddress[0] ? mAddress.c_str() : 0, APR_INET, mPort, 0, mPool)
		);

		BAILONFAIL(apr_socket_create(&mSock, mSockAddr->family, SOCK_DGRAM, APR_PROTO_UDP, mPool));

		if(mSock){
			// Assign address to socket. If TCP, establish new connection.
			if(mSender){	BAILONFAIL(apr_socket_connect(mSock, mSockAddr)); }
			else{			BAILONFAIL(apr_socket_bind(mSock, mSockAddr)); }

			// Set timeout behavior
			if(timeout == 0){
				// non-blocking:			APR_SO_NONBLOCK==1(on),  then timeout=0
				BAILONFAIL(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 1));
				BAILONFAIL(apr_socket_timeout_set(mSock, 0));
			}
			else if(timeout > 0){
				// blocking-with-timeout:	APR_SO_NONBLOCK==0(off), then timeout>0
				BAILONFAIL(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 0));
				BAILONFAIL(apr_socket_timeout_set(mSock, (apr_interval_time_t)(timeout * 1.0e6)));

			}
			else{
				// blocking-forever:		APR_SO_NONBLOCK==0(off), then timeout<0
				BAILONFAIL(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 0));
				BAILONFAIL(apr_socket_timeout_set(mSock, -1));
			}
			
			mTimeout = timeout;
		}

		return true;
	}

	// note that setting timeout will close and re-open the socket:
	void timeout(al_sec v){
		open(mPort, mAddress, v, mSender);
	}

//	bool open(unsigned int port, std::string address, bool sender){
//
//		close();
//
//		// TODO: check_apr results should jump to an err label and return an uninitialized socket!
//
//		/* @see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html */
//
//		mPort = port;
//		mSender = sender;
//		mAddress = address;
//		if (APR_SUCCESS == check_apr(apr_sockaddr_info_get(&mSockAddr, mAddress[0] ? mAddress.c_str() : 0, APR_INET, mPort, 0, mPool))) {
//			check_apr(apr_socket_create(&mSock, mSockAddr->family, SOCK_DGRAM, APR_PROTO_UDP, mPool));
//
//			if(mSock){
//				// Assign address to socket. If TCP, establish new connection.
//				if(mSender)	check_apr(apr_socket_connect(mSock, mSockAddr));
//				else		check_apr(apr_socket_bind(mSock, mSockAddr));
//				return true;
//			}
//		} else {
//			printf("failed to create socket at %s:%i\n", address.c_str(), port);
//		}
//		return false;
//	}
//	
//	// note that setting timeout will close and re-open the socket:
//	void timeout(al_sec v){
//		open(mPort, mAddress, mSender);
//		
//		if (opened()) {
//			if(v == 0){
//				// non-blocking:			APR_SO_NONBLOCK==1(on),  then timeout=0
//				check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 1));
//				check_apr(apr_socket_timeout_set(mSock, 0));
//			}
//			else if(v > 0){
//				// blocking-with-timeout:	APR_SO_NONBLOCK==0(off), then timeout>0
//				check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 0));
//				check_apr(apr_socket_timeout_set(mSock, (apr_interval_time_t)(v * 1.0e6)));
//
//			}
//			else{
//				// blocking-forever:		APR_SO_NONBLOCK==0(off), then timeout<0
//				check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 0));
//				check_apr(apr_socket_timeout_set(mSock, -1));
//			}
//		}
//		mTimeout = v;
//	}
	
	bool opened() const { return 0!=mSock; }

	unsigned int mPort;
	std::string mAddress;
	apr_sockaddr_t * mSockAddr;
	apr_socket_t * mSock;
	al_sec mTimeout;
	bool mSender;
};



Socket::Socket()
:	mImpl(0)
{
	mImpl = new Impl;
}

Socket::Socket(unsigned int port, const char * address, al_sec timeout, bool sender)
: mImpl(0)
{
	mImpl = new Impl(port, address, timeout, sender);
}

Socket::~Socket(){
	close();
	delete mImpl;
}

const std::string& Socket::address() const { return mImpl->mAddress; }

bool Socket::opened() const { return mImpl->opened(); }

unsigned int Socket::port() const { return mImpl->mPort; }

al_sec Socket::timeout() const { return mImpl->mTimeout; }


void Socket::close(){ mImpl->close(); }

bool Socket::open(unsigned int port, const char * address, al_sec timeout, bool sender){
	return mImpl->open(port, address, timeout, sender);
}



void Socket::timeout(al_sec v){ mImpl->timeout(v); }

size_t Socket::recv(char * buffer, size_t maxlen) {
	apr_size_t len = maxlen;
	apr_status_t r = apr_socket_recv(mImpl->mSock, buffer, &len);
	
	// only error check if not error# 35: Resource temporarily unavailable
	if(len){ check_apr(r); }
	return len;
}

size_t Socket::send(const char * buffer, size_t len) {
	apr_size_t size = len;
	if (mImpl->opened()) {
		//check_apr(apr_socket_send(mSock, buffer, &size));
		apr_socket_send(mImpl->mSock, buffer, &size);
	} else {
		size = 0;
	}
	return size;
}


std::string Socket::hostIP(){
	ImplAPR apr;
	char * addr;
	apr_sockaddr_t * sa;
	apr_sockaddr_info_get(&sa, hostName().c_str(), APR_INET, 8000, 0, apr.pool());
	while(sa) {
		apr_sockaddr_ip_get(&addr, sa);
		//printf("%s %s %s %d %d\n", addr, sa->hostname, sa->servname, sa->port, sa->family);
		sa = sa->next;
	}
	return addr;
}

std::string Socket::hostName(){
	char buf[APRMAXHOSTLEN+1];
	ImplAPR apr;
	check_apr(apr_gethostname(buf, sizeof(buf), apr.pool()));
	return buf;
}

} // al::
