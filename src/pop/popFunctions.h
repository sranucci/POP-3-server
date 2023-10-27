#ifndef POP_FUNCTIONS
#define POP_FUNCTIONS

#include "popStandards.h"
#include "../clients/clients.h"
#define MAX_SINGLE_LINE_RESPONSE 128 //max length of single line POP function responses (RFC1939 says 512 but we don't use that much)

typedef executionStatus (*command_handler)(char * arg1, char * arg2, user_data * user_data); //pop functions declaration

typedef struct command_with_state {
    char * commandStr;
    command_handler execute_command;
    pop_state pop_state_needed; //pop state needed to execute this command
} command_with_state;

#define TOTALCOMMANDS 10

command_with_state * getCommand(char * command_name);
int sendGreeting(user_data * user);

#endif