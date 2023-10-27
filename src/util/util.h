#ifndef UTIL_H_
#define UTIL_H_

#include <sys/socket.h>

int printSocketAddress(const struct sockaddr *address, char * addrBuffer);

// Determina si dos sockets son iguales (misma direccion y puerto)
int sockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);

#endif
