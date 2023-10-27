#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h> //sprintf()

int 
printSocketAddress(const struct sockaddr *address, char *addrBuffer) {

	void *numericAddress; 

	in_port_t port;

	switch (address->sa_family) {
		case AF_INET:
			numericAddress = &((struct sockaddr_in *) address)->sin_addr;
			port = ntohs(((struct sockaddr_in *) address)->sin_port);
			break;
		case AF_INET6:
			numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
			port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
			break;
		default:
			strcpy(addrBuffer, "[unknown type]");    // Unhandled type
			return 0;
	}
	// Convert binary to printable address
	if (inet_ntop(address->sa_family, numericAddress, addrBuffer, INET6_ADDRSTRLEN) == NULL)
		strcpy(addrBuffer, "[invalid address]"); 
	else {
		if (port != 0)
			sprintf(addrBuffer + strlen(addrBuffer), ":%u", port);
	}
	return 1;
}