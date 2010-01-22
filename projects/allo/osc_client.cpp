/* allocore */
#include "system/al_mainloop.h"
#include "system/al_time.h"
#include "types/al_pq.h"
#include "types/al_tube.h"
#include "types/al_types.h"

/* Apache Portable Runtime */
#include "apr_general.h"
#include "apr_errno.h"
#include "apr_pools.h"
#include "apr_network_io.h"

/* oscpack */
#include "protocol/oscpack/osc/OscReceivedElements.h"

#include "stdlib.h"

/* APR utility */
static apr_status_t check_apr(apr_status_t err) {
	char errstr[1024];
	if (err != APR_SUCCESS) {
		apr_strerror(err, errstr, 1024);
		fprintf(stderr, "apr error: %s\n", errstr);
	}
	return err;
}

/* OSC packet parser */
void osc_parsemessage(const osc::ReceivedMessage & p) {
	printf("address %s tags %s args %d\n", p.AddressPattern(), p.TypeTags(), p.ArgumentCount());
	// etc.
}

void osc_parsebundle(const osc::ReceivedBundle & p) {
	for(osc::ReceivedBundle::const_iterator i=p.ElementsBegin(); i != p.ElementsEnd(); ++i) {
		if(i->IsBundle()) {
			osc_parsebundle(osc::ReceivedBundle(*i));
		} else {
			osc_parsemessage(osc::ReceivedMessage(*i));
        }
	}
}

void osc_parse(const char * buf, size_t size) {
	osc::ReceivedPacket p(buf, size);
	if(p.IsBundle()) {
		osc_parsebundle(osc::ReceivedBundle(p));
	} else {
		osc_parsemessage(osc::ReceivedMessage(p));
	}
}

int main (int argc, char * argv[]) {


	apr_status_t err;
	apr_pool_t * pool;
	
	// init APR:
	check_apr(apr_initialize());
	atexit(apr_terminate);	// ensure apr tear-down
	
	// create mempool:
	check_apr(apr_pool_initialize());
	check_apr(apr_pool_create(&pool, NULL));
	apr_allocator_max_free_set(apr_pool_allocator_get(pool), 1024);

	/* @see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html */
	// create socket:
	apr_sockaddr_t * sa;
	apr_socket_t * sock;
	apr_port_t port = 7007;
	check_apr(apr_sockaddr_info_get(&sa, NULL, APR_INET, port, 0, pool));
	// for TCP, use SOCK_STREAM and APR_PROTO_TCP instead
	check_apr(apr_socket_create(&sock, sa->family, SOCK_DGRAM, APR_PROTO_UDP, pool));
	// bind socket to address:
	check_apr(apr_socket_bind(sock, sa));
	
	check_apr(apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1));
    //apr_socket_timeout_set(s, DEF_SOCK_TIMEOUT);
	
	//check_apr(apr_socket_listen(sock, SOMAXCONN));
	
//	// handle connections from remote clients:
//	apr_socket_t * remote; /* accepted socket */
//	check_apr(apr_socket_accept(&remote, sock, pool));

	// receive data:
	for (int i=0; i<1000; i++) {
		printf("______________\n");
		apr_size_t len;
		char data[len];
		do {
			len = 4096;
			apr_socket_recv(sock, data, &len);
			if (len) osc_parse(data, len); 
		} while (len > 0);
		
		al_sleep(1.);
	}
	
	
	// program end:
	apr_pool_destroy(pool);
	return 0;
}