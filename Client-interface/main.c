#include "../src/logger/logger.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define TIMEOUT_SECONDS 1
#define MAX_RETRY 10
#define REQ_ID_LEN 32
#define MAX_DGRAM_SIZE 8096
#define INPSIZE 128

bool badAuthorization = false;
bool malformedAuthorization = false;
bool invalidVersion = false;

void printIntroduction();
int setupUDPClientSocket(const char* serverIP, int serverPort, void *structAddr, socklen_t *length);
int setupUDPClientSocketIPv6(const char* serverIP, int serverPort, void * structAddr, socklen_t * length);
void sendDatagram(int sockFd,char * command,struct sockaddr * serverAddr,size_t serverAddrLength);
void generateRandomString(char *output, size_t length);
int checkValidRequestId(char * requestId, char * dgramRecieved);

int main(int argc, char ** argv){

    //Receive the port to program
    if(argc < 4 || argc > 5 ){
        // log(ERROR,"Error: Invalid number of arguments\n");
        fprintf(stderr,"Usage: <IPv4|IPv6> <Server IP> <Server Port/Service> [optional: -BA|-MA|-WV]\n");
        exit(1);
    }

    if ( argc == 5){
        if ( strcmp(argv[4],"-BA") == 0){
            badAuthorization = true;
        } else if ( strcmp(argv[4],"-MA") == 0){
            malformedAuthorization = true;
        } else if ( strcmp(argv[4],"-WV") == 0){
            invalidVersion = true;
        }
    }

    if ( strcmp(argv[1],"IPv6") != 0 && strcmp(argv[1],"IPv4") != 0){
        fprintf(stderr,"INVALID IP VERSION, use IPv6 or IPv4\n");
        exit(1);
    }

    char * ipVersion = argv[1];
    char * server = argv[2];  
    char * servPortString = argv[3];

    int servPort = atoi(servPortString);
    if ( servPort <= 0){
        fprintf(stderr,"INVALID PORT FORMAT, PLEASE USE AN INT");
        exit(1);
    }


    errno = 0;

    char serverData[sizeof(struct sockaddr_in6)];//we reserve 28 bytes in memory, the struct for ipv6 is bigger than the one for ip4
    socklen_t addrLen;
    int sock;
    if ( strcmp(ipVersion,"IPv4") == 0){
        sock = setupUDPClientSocket(server, servPort, (void *) serverData,&addrLen);
    } else {
        sock = setupUDPClientSocketIPv6(server,servPort,(void *) serverData,&addrLen);
    }
    if (sock < 0)
        fprintf(stderr, "socket() failed: %s", strerror(errno));


    // client interface -------------------------------------

    printIntroduction();
    printf("> ");

    char input[INPSIZE];

    while (1){
        scanf("%[^\n]", input); // Read up to a new line and store in input

        // printf("%s\n", input);
        // printf("%d\n", strlen(input));
        char * message;
        if (strncmp(input, "BT", 2) == 0) {
            message = "Sending bytes transferred request: ...\n";
        }
        else if (strncmp(input, "BR", 2) == 0) {
            message = "Sending bytes received request: ...\n";
        }
        else if (strncmp(input, "CC", 2) == 0) {
            message = "Sending concurrent connections request: ...\n";
        }
        else if (strncmp(input, "HC", 2) == 0) {
            message = "Sending historic connections request: ...\n";
        }
        else if (strncmp(input, "AU", 2) == 0) {
            message = "Sending Adding user request....\n";
        }
        else if (strncmp(input, "MP", 2) == 0) {
            
            message = "Sending modifying password request...\n";

        }
        else if (strncmp(input, "DU", 2) == 0) {
            
            message = "Sending Delete User request.....\n";
        }
        else if (strncmp(input, "LU", 2) == 0) {
            message = "Listing users....\n";
        }
        else if (strncmp(input, "HELP", 4) == 0) {
            printIntroduction();
            getchar(); // Consume the newline character from the buffer
            printf("> ");
            continue;
        }
        else if (strncmp(input, "QUIT",4) == 0) {
            printf("Quitting...\n");
            return 0;
        }
        else {
            message = "Sending unknown command to server...\n";
        }

        puts(message);
        sendDatagram(sock,input,(struct sockaddr *) serverData,addrLen);
        getchar(); // Consume the newline character from the buffer
        printf("> ");
    }
}

int setupUDPClientSocket(const char* serverIP, int serverPort, void *structAddr, socklen_t *length) {
    int sockfd;

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    if (inet_pton(AF_INET,serverIP, &serverAddress.sin_addr) == 0) {
        fprintf(stderr, "Invalid server IP address\n");
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(serverAddress);
    memcpy(structAddr,&serverAddress,len);
    *length = len;


    return sockfd;
}

int setupUDPClientSocketIPv6(const char* serverIP, int serverPort, void * structAddr, socklen_t * length) {
    int sockfd;

    // Create a UDP socket
    sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin6_family = AF_INET6;
    serverAddress.sin6_port = htons(serverPort);
    if (inet_pton(AF_INET6, serverIP, &serverAddress.sin6_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address\n");
        exit(EXIT_FAILURE);
    }
    socklen_t len = sizeof(serverAddress);
    memcpy(structAddr,&serverAddress,len);
    *length = len;


    return sockfd;
}



void sendDatagram(int sockFd,char * command,struct sockaddr * serverAddr,size_t serverAddrLength){

    char * versionProtocol = "V1.0";
    if ( invalidVersion ){
        versionProtocol = "V2.0";
    }
    char * authorization = "C9h2iUZ4sWJY16fDl7Vg5RnH0vN8aQpX";
    if ( badAuthorization ){
        authorization = "C9h2iUZ4sWJY16fDl7Vg5RnH0vN8aQpI";
    } else if ( malformedAuthorization ){
        authorization = "C9h2iUZ4sWJY16fD";
    }

    char requestId[REQ_ID_LEN + 1];
    generateRandomString(requestId,REQ_ID_LEN);
    char buffer[MAX_DGRAM_SIZE + 1];
    sprintf(buffer,"MP3P %s\n%s\n%s\n%s",versionProtocol,requestId,authorization,command);
    int message_len = strlen(buffer);

    int retry = 0;
    int sent = 0;
    while (retry < MAX_RETRY) {
        ssize_t bytes_sent = sendto(sockFd, buffer, message_len, 0,
                                    (struct sockaddr *)serverAddr, serverAddrLength);

        if (bytes_sent < 0) {
            perror("Error sending message");
            exit(EXIT_FAILURE);
        }

        // we set the socket timeout
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;
        if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("Error setting receive timeout");
            exit(EXIT_FAILURE);
        }

        char response[MAX_DGRAM_SIZE + 1];
        ssize_t bytes_received = recv(sockFd, response, MAX_DGRAM_SIZE, 0);
        response[bytes_received] = 0;

        if (bytes_received < 0 ) {
            printf("Timeout occurred (attempt %d)\n", retry + 1);
            retry++;
        } else if (!checkValidRequestId(requestId,response)) {
            printf("A datagram was recieved but the id does not match, attempt %d\n",retry + 1);
            retry++;
        } else {
            printf("Response received: %s\n\n", response);
            sent = 1;
            break;
        }
    }

    if (!sent) {
        printf("Failed to receive response after %d attempts\n", MAX_RETRY);
    }
                             
}

void generateRandomString(char *output, size_t length) {
    static const char characters[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    size_t i;

    srand(time(NULL));

    for (i = 0; i < length; i++) {
        output[i] = characters[rand() % (sizeof(characters) - 1)];
    }

    output[length] = '\0';  
}

int checkValidRequestId(char * requestId, char * dgramRecieved){
    char * responseRequestId = strchr(dgramRecieved,'\n');

    responseRequestId++;
    for ( int i = 0; i < REQ_ID_LEN && responseRequestId[i] != '\n'; i++){
        if ( requestId[i] != responseRequestId[i] ){
            return false;
        }
    }

    return true;
}

void printIntroduction(){
    printf("Welcome! The commands available are:\n");
    printf("1.  BT                       : To see the bytes that were transfered\n");
    printf("2.  BR                       : To see the amount of bytes received \n");
    printf("3.  CC                       : To see the current connections\n");
    printf("4.  HC                       : To see the history connections\n");
    printf("5.  AU <USER>                : To add a user\n");
    printf("6.  MP <USER> <NEW PASSWORD> : To modify the password\n");
    printf("7.  DU <USER>                : To delete a user\n");
    printf("8. LU                        : To list users\n");
    printf("9. LC                        : To list MP3P capabilities\n");
    printf("10. HELP                     : To print options again\n");
    printf("11. QUIT                     : To quit\n");
}