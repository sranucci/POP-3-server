#ifndef TCPSERVERUTIL_H_
#define TCPSERVERUTIL_H_
#define MAX_CONNECTIONS 500
#define NOT_ALLOCATED -1

#include <sys/select.h>

typedef struct {
    char* port;
    int userCount;
    char **users;
} args_data;


void freeArgs(args_data * args);
args_data * parseArgs(int argc, char ** argv);

// Create, bind, and listen a new TCP server socket
int setupTCPServerSocket(const char *service);
int setupUDPServerSocketIpv6(char * service);
int setupUDPServerSocketIpv4(char * service);


void acceptConnection(user_data* connectedUsers,int servSock);
void handleClients(fd_set *readFd, fd_set *writeFd, user_data *usersData);
void addClientsSocketsToSet(fd_set * readSet,fd_set* writeSet ,int * maxNumberFd, user_data * users);
void handleSelectActivityError();
void closeAllClients(user_data usersData[]);
void handleUdpRequest(int udpSocket);
#endif 
