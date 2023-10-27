#ifndef CLIENTS_H
#define CLIENTS_H
#include "../pop/popStandards.h"
#include "../buffer/circularBuffer.h"
#include "../mailsCache/mailsCache.h"
#define ACCEPT_FAILURE -1

typedef enum command_execute_state{
    AVAILABLE, //can execute new command
    PROCESSING, //the current command hasn't finished yet
} command_execute_state;

typedef enum client_state{
    READING, //the server is reading from the client
    WRITING, //the server is writing to the client
} client_state;

typedef struct login_info{
    char username[MAXARGSIZE+1];
} login_info;

typedef struct user_data{
    struct command_list * command_list;
    buffer output_buff;
    pop_state session_state;
    client_state client_state;
    int socket;
    login_info login_info;
    void *currentCommand; //command currently executing
    command_execute_state commandState; //command execution status (tells if you can execute a new command)
    mailCache * mailCache;
} user_data;

typedef struct registered_users{
    char * username;
    char * password;
} registered_users;

void writeToClient(user_data * client);
void readFromClient(user_data * client);
void initClient(user_data * client, int sockNum);
void closeClient(user_data * usersData);

#endif