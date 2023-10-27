#include "../parsers/commandParser.h"
#include "../logger/logger.h"
#include "clients.h"
#include <unistd.h> // close()
#include <string.h> // memset()
#include <sys/types.h> // send()
#include <sys/socket.h> // send()
#include <errno.h>
#include "../mailsCache/mailsCache.h"
#include "../users/users.h"

#include "../stats/stats.h"

#define NOT_ALLOCATED -1

void writeToClient(user_data * client){
    int toWrite = getBufferOccupiedSpace(&client->output_buff);
    if(toWrite!=0){
        char auxiliaryBuffer[toWrite];
        
        readDataFromBuffer(&client->output_buff, auxiliaryBuffer, toWrite);
        int bytesSent = send(client->socket, auxiliaryBuffer, toWrite, MSG_NOSIGNAL); //MSG_NOSIGNAL is to prevent errors if the client closes the connection while we are writing to him
        if ( bytesSent < 0 ){
            log(ERROR,"Could not send data to buffer %d",client->socket);
            closeClient(client);
            return;
        } 

        //we register how many bytes were send to the client in the statistics of our protocol
        addTransferedBytesToStats(bytesSent);
        
        if (bytesSent < toWrite){
            int bytesToWriteBack = toWrite - bytesSent;
            char * notSendPosition = auxiliaryBuffer + bytesSent; 
            writeDataToBuffer(&client->output_buff, notSendPosition , bytesToWriteBack );
        }
    }

    if(isBufferEmpty(&client->output_buff) && !availableCommands(client->command_list) && client->commandState == AVAILABLE)
        client->client_state = READING;
    return;
}


void readFromClient(user_data * client){
    char auxiliaryBuffer[MAXCOMMANDLENGTH+1];
    int bytesRead = recv(client->socket, auxiliaryBuffer, MAXCOMMANDLENGTH, 0);

    
    
    if ( bytesRead <= 0){
        //client closed connection, that position is released
        if ( bytesRead < 0){
            log(ERROR,"Error while doing recv for socket %d. Errno %d",client->socket, errno);
            return;
        }
        log(INFO, "The client in socket %d sent a EOF. Releasing his resources...", client->socket);
        closeClient(client);

        //we remove the concurrent connection from the statistics
        removeConcurrentConnectionFromStats();

        return;
    }
    auxiliaryBuffer[bytesRead] = 0; //null terminate



    //we add to statistics the amount of bytes read
    addRecievedBytesToStats(bytesRead);

    addData(client->command_list, auxiliaryBuffer);
    client->client_state = WRITING;
}

void initClient(user_data * client, int sockNum){
    client->socket = sockNum;
    client->session_state = AUTHENTICATION;
    client->client_state = WRITING;
    client->command_list = createList();
}

void closeClient(user_data * client){
    if(client->socket == NOT_ALLOCATED)
        return;

    //if the client was closed while we were executing a pop function, we free it
    if(client->commandState == PROCESSING){
        free(client->currentCommand);
    }

    toggleUserConnected(client->login_info.username, false);
    destroyList(client->command_list);
    freeCache(client->mailCache);
    close(client->socket);
    memset(client,0,sizeof(user_data)); //to mark it as unoccupied
    client->socket = NOT_ALLOCATED;
}

