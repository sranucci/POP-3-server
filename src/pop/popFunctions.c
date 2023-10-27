#define GREETINGMESSAGE "+OK Pop3 Server Ready\r\n"
#define WRITE_ERROR -1
#define WRITE_SUCCESS 0


#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../logger/logger.h"
#include "popStandards.h"
#include "stdbool.h"
#include "../users/users.h"
#include "popFunctions.h"
#include "../mailsCache/mailsCache.h"

//----------------FUNCTION-PROTOTYPES--------------------------
executionStatus checkValidUsername(char * username, char * empty, user_data * data);
executionStatus checkValidPassword(char * password, char * empty, user_data * data);
executionStatus quit(char *, char *, user_data * user_data);
executionStatus noop(char *, char *, user_data * user_data);
executionStatus dele(char * toDelete, char *, user_data * user_data);
executionStatus rset(char *, char *, user_data * user_data);
executionStatus _stat(char *, char *, user_data * user_data);
executionStatus list(char *, char *, user_data * user_data);
executionStatus retr(char *, char *, user_data * user_data);
executionStatus capa(char *, char *, user_data * user_data);

//---------------- LIST-OF-COMMANDS----------------------------
#include "popFunctions.h"
#include "strings.h"
command_with_state validCommands[TOTALCOMMANDS] = {
    {"USER", checkValidUsername,    AUTHENTICATION},
    {"PASS", checkValidPassword,    AUTHENTICATION},
    {"STAT", _stat,                 TRANSACTION},
    {"LIST", list,                  TRANSACTION},
    {"RETR", retr,                  TRANSACTION},
    {"DELE", dele,                  TRANSACTION},
    {"NOOP", noop,                  TRANSACTION},
    {"RSET", rset,                  TRANSACTION},
    {"QUIT", quit,                  ANY},
    {"CAPA", capa,                  ANY},
};


//returns NULL if command not found or pointer to POP command if exists
command_with_state * getCommand(char * command_name){
    for ( int i = 0; i < TOTALCOMMANDS; i++){
        if ( strcasecmp(validCommands[i].commandStr, command_name) == 0 ){
            return &validCommands[i];
        }
    }
    return NULL;
}

int writeToOutputBuffer(char * buffer, user_data* data ) {
    int length = strlen(buffer);
    if(getBufferFreeSpace(&data->output_buff) >= length ){
        writeDataToBuffer(&data->output_buff, buffer, length);
        return WRITE_SUCCESS;
    }
    
    return WRITE_ERROR;
}


static void sendTerminationCRLF(user_data * user_data, bool addInitialCRLF){
    char * message;
    if(addInitialCRLF) //to use in RETR
        message = "\r\n.\r\n";
    else //to use in LIST
        message = ".\r\n";
    writeToOutputBuffer(message, user_data);
}



//------------------------USER FUNCTION------------------------------------------------------------------------

executionStatus checkValidUsername(char * username, char * empty, user_data * data){
    char * message = "+OK User received.\r\n";
    int len = strlen(message);
    if ( getBufferFreeSpace(&data->output_buff) >= len){
        writeDataToBuffer(&data->output_buff,message,len);
        strcpy(data->login_info.username,username);
        return FINISHED;
    } 
    return NOT_FINISHED;
}


executionStatus checkValidPassword(char * password, char * empty, user_data * data){
    char * message;
    if ( validPassword(data->login_info.username,password) ){
        if (userIsConnected(data->login_info.username)){
            message = "-ERR This user is already connected\r\n";
        } else {
            message = "+OK Welcome\r\n";
            toggleUserConnected(data->login_info.username, true);
            data->mailCache = initCache(data->login_info.username);
            data->session_state = TRANSACTION;
        }
    } else {
        message = "-ERR Authentication failed\r\n";
    }
    int len = strlen(message);
    if ( getBufferFreeSpace(&data->output_buff) >= len){
        writeDataToBuffer(&data->output_buff,message,len);
        return FINISHED;
    }
    return NOT_FINISHED;
}

//-------------------------LIST FUNCTIONS-----------------------------------------------------------------------

int sendGreeting(user_data * user){
    char * greetingMessage = GREETINGMESSAGE;
    writeDataToBuffer(&user->output_buff,greetingMessage,strlen(greetingMessage));
    return FINISHED;
}

executionStatus noop(char * unused, char * unused2, user_data * user_data){
    return writeToOutputBuffer("+OK\r\n", user_data);
}

executionStatus dele(char * toDelete, char * unused, user_data * user_data){
    int toDeleteNumber = atoi(toDelete);
    char * msg;
    executionStatus toReturn;
    if(markMailToDelete(user_data->mailCache, toDeleteNumber) == 0){
        toReturn = FINISHED;
        msg = "+OK message marked to delete\r\n";
    }
    else {
        toReturn = FAILED;
        msg = "-ERR That email doesn't seem to exist\r\n";
    }

    writeToOutputBuffer(msg, user_data);
    return toReturn;
}

executionStatus rset(char * unused, char * unused2, user_data * user_data){
    char * msg;
    executionStatus toReturn;
    if(resetToDelete(user_data->mailCache) == 0){
        msg = "+OK messages no longer marked to delete\r\n";
        toReturn = FINISHED;
    }
    else {
        msg = "-ERR problem while trying to execute rset\r\n";
        toReturn = FAILED;
    }

    writeToOutputBuffer(msg, user_data);
    return toReturn;
}

executionStatus quit(char * unused, char * unused2, user_data* user_data){
    char * msg;
    executionStatus toReturn;
    if(user_data->session_state == TRANSACTION){
        if(deleteMarkedMails(user_data->mailCache) == 0){
            msg = "+OK exiting\r\n";
            toReturn = FINISHED;
        }
        else {
            msg = "-ERR there was a problem deleting your emails\r\n";
            toReturn = FAILED;
        }
    } else {
        msg = "+OK exiting\r\n";
        toReturn = FINISHED;
    }
    writeToOutputBuffer(msg, user_data);
    user_data->session_state = UPDATE;
    
    return toReturn;
}

executionStatus _stat(char * unused, char * unused2, user_data * user_data){
    char msg[MAX_SINGLE_LINE_RESPONSE];
    snprintf(msg, MAX_SINGLE_LINE_RESPONSE, "+OK %d %lld\r\n", getAmountOfMails(user_data->mailCache), getSizeOfMails(user_data->mailCache));
    writeToOutputBuffer(msg, user_data);
    return FINISHED;
}

static executionStatus continueList(user_data * user_data, mailInfo * mailInfo){
    char buffer[MAX_SINGLE_LINE_RESPONSE];
    while(getBufferFreeSpace(&user_data->output_buff) > MAX_SINGLE_LINE_RESPONSE && hasNext(user_data->mailCache)){
        if(next(user_data->mailCache, mailInfo) >= 0){
            sprintf(buffer, "%d %ld\r\n", mailInfo->mailNo, mailInfo->sizeInBytes);
            writeToOutputBuffer(buffer, user_data);
        }
    }
    return hasNext(user_data->mailCache) ? NOT_FINISHED : FINISHED;
}

static executionStatus listWithNoArguments(user_data * user_data){
    char buffer[MAX_SINGLE_LINE_RESPONSE];
    mailInfo * mailInfo = malloc(sizeof(struct mailInfo));
    if (user_data->commandState == AVAILABLE){ //we aren't continuing a previous execution
        sprintf(buffer, "+OK There are %d messages available\r\n", getAmountOfMails(user_data->mailCache));
        writeToOutputBuffer(buffer, user_data);
        toBegin(user_data->mailCache);
    }

    executionStatus status = continueList(user_data, mailInfo);
    free(mailInfo);

    if (status == FINISHED){
        sendTerminationCRLF(user_data, false);
        return FINISHED;
    }
    else
        return NOT_FINISHED;
}

static executionStatus listWithArgument(user_data * user_data, char * mailNo){
    char buffer[MAX_SINGLE_LINE_RESPONSE];
    mailInfo * mailInfo = malloc(sizeof(struct mailInfo));
    executionStatus toReturn;
    if(getMailInfo(user_data->mailCache, atoi(mailNo), mailInfo) == FAILED){
            sprintf(buffer, "-ERR That message doesn't seem to exist\r\n");
            toReturn = FAILED;
        }
        else {
            sprintf(buffer, "+OK %d %ld\r\n", mailInfo->mailNo, mailInfo->sizeInBytes);
            toReturn = FINISHED;
        }

        writeToOutputBuffer(buffer, user_data);
        free(mailInfo);
        return toReturn;
}


executionStatus list(char * mailNo, char * unused, user_data * user_data){
    //calling list with an argument
    if(mailNo != NULL && mailNo[0] != 0){
        return listWithArgument(user_data, mailNo);
    } else { //calling list without an argument
        return listWithNoArguments(user_data);
    }
}

executionStatus continueRetr(user_data * user_data){
    int characters = getBufferFreeSpace(&user_data->output_buff);
    executionStatus retValue = NOT_FINISHED;

    //we do this so that the output buffer has at least MAX_SINGLE_LINE_RESPONSE to write a final string (like a \r\n.\r\n)
    if(characters > MAX_SINGLE_LINE_RESPONSE){
        char buffer[characters+1];
        retValue = getNCharsFromMail(user_data->mailCache, &characters, buffer);
        if(retValue != FAILED){
            buffer[characters] = 0;
            writeToOutputBuffer(buffer, user_data);
        }
    }
    return retValue;
}

executionStatus retr(char * mailNoString, char * unused, user_data * user_data){
    if(user_data->commandState == AVAILABLE){ //if we are executing a new function
        int mailNo = atoi(mailNoString);
        char * message;
        if(openMail(user_data->mailCache, mailNo) == FAILED){
            message = "-ERR That email doesn't seem to exist\r\n";
            writeToOutputBuffer(message, user_data);
            return FAILED;
        }
        message = "+OK message follows\r\n";
        writeToOutputBuffer(message, user_data);
    }
    
    executionStatus retValue = continueRetr(user_data);

    if(retValue == FINISHED){ //retr has finished executing
        sendTerminationCRLF(user_data, true);
        closeMail(user_data->mailCache);
    }
    
    return retValue;
}


executionStatus capa(char * unused1, char * unused2, user_data * user_data){
    if(writeToOutputBuffer("+OK Capability list follows\r\nUSER\r\nPIPELINING\r\n.\r\n", user_data) < 0){
        return NOT_FINISHED;
    }
    
    return FINISHED;
}