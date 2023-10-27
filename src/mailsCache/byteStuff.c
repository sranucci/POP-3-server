#include "byteStuff.h"
#include "string.h"
#include <stdlib.h>
#include "../server/serverOptions.h"

typedef enum {
    NEWLINE,
    DOT,
    NORMAL,
    CR,
} read_states;

typedef struct charactersProcessor {
    char buffer[BUFFERSIZE];
    int idx;
    int len;
    bool eofReached;
    read_states state;
} charactersProcessor;

static void rebaseBuffer(charactersProcessor * charactersProcessor);
static int readFromBuffer(charactersProcessor * charactersProcessor);

charactersProcessor * initCharactersProcessor(){
    return calloc(1, sizeof(charactersProcessor));
}

void freeCharactersProcessor(charactersProcessor * charactersProcessor){
    free(charactersProcessor);
}

int getNProcessedCharacters(charactersProcessor * charactersProcessor, char * buffer, int n){
    if(charactersProcessor == NULL)
        return 0;
    // int available = availableCharacters(charactersProcessor);
    int toCopy = n;

    int copied;
    for (copied = 0; copied < toCopy; copied++)
    {
        int charRetrieved;
        if ((charRetrieved = readFromBuffer(charactersProcessor)) > 0)
            buffer[copied] = charRetrieved;
        else
            break;
    }
    

    rebaseBuffer(charactersProcessor);
    return copied;
}

int addCharactersToProcess(charactersProcessor * charactersProcessor, char * buffer, int n, bool eofReached){
    if(charactersProcessor == NULL)
        return 0;

    charactersProcessor->eofReached = eofReached;

    int freeSpace = availableSpace(charactersProcessor);
    int toAdd = freeSpace < n ? freeSpace : n;
    strncpy((charactersProcessor->buffer + charactersProcessor->len), buffer, toAdd);
    charactersProcessor->len += toAdd;
    return toAdd;
}

int availableCharacters(charactersProcessor * charactersProcessor){
    if(charactersProcessor == NULL)
        return 0;
    return charactersProcessor->len - charactersProcessor->idx;
}

int availableSpace(charactersProcessor * charactersProcessor){
    if(charactersProcessor == NULL)
        return 0;
    return BUFFERSIZE - availableCharacters(charactersProcessor);
}

void resetCharactersProcessor(charactersProcessor * charactersProcessor){
    charactersProcessor->idx = 0;
    charactersProcessor->len = 0;
    charactersProcessor->state = NEWLINE;
    charactersProcessor->eofReached = false;
}

static void rebaseBuffer(charactersProcessor * charactersProcessor){
    if ( charactersProcessor->len < charactersProcessor->idx){
        return;
    }

    int bytesToMove = availableCharacters(charactersProcessor);
    memmove(charactersProcessor->buffer, charactersProcessor->buffer + charactersProcessor->idx, bytesToMove );
    charactersProcessor->idx = 0;
    charactersProcessor->len = bytesToMove;
}

//returns amout of bytes read from buffer
static int readFromBuffer(charactersProcessor * charactersProcessor){
    //read but with state machine, EOF was not reached
    
    if ( charactersProcessor->idx >= charactersProcessor->len ){
        if (charactersProcessor->eofReached && charactersProcessor->state == DOT){
            charactersProcessor->state = NORMAL;
            return '.';
        }
        return 0;
    }

    int c = charactersProcessor->buffer[charactersProcessor->idx];

    if ( charactersProcessor->state == DOT && c == '\r'){
        charactersProcessor->state = NORMAL;
        return '.';
    }

    if ( c == '\r'){
        charactersProcessor->state = CR;
    } else if ( c == '\n'){
        charactersProcessor->state = NEWLINE;
    } else if ( c == '.' && charactersProcessor->state == NEWLINE){
        charactersProcessor->state = DOT;
    } else {
        charactersProcessor->state = NORMAL;
    }

    charactersProcessor->idx++;
    return c;
}