#include "mp3pFunctions.h"
#include <string.h>
#include <stdbool.h>
#include "../stats/stats.h"
#include <stdio.h>
#include <ctype.h>
#include "../users/users.h"
#include "../logger/logger.h"
#define MAXDATAGRAMLENGTH 8096
#define VERSION "V1.0"
#define AUTHKEY "C9h2iUZ4sWJY16fDl7Vg5RnH0vN8aQpX"


typedef enum {
    START,
    READM,
    READP,
    READ3,
    READSECONDP,
    READVERSIONV,
    READVERSIONNUM,
    READVERSIONDOT,
    READVERSIONSEPARATOR,
    READVERSIONSUBNUM,
    READFIRSTNEWLINE,
    READSECONDNEWLINE,
    READTHIRDNEWLINE,
    READCLIENTDGRAMID,
    READCLIENTPASSWORD,
    READSEPARATOR,
    READNUMBER,
    READDOT,
    READVERSION,
    READNEWLINE,
    READPASS,
    READCOMMAND,
    READB,
    READBT,
    READH,
    READC,
    READL,
    READM_FOR_MP,
    READMP,
    READHC,
    READA,
    READAU,
    READCC,
    READBR,
    READLU,
    READLC,
    READ_D,
    READDU,
    READONEARGSEPARATOR,
    READFIRSTARGSEPARATOR,
    READSECONDARGSEPARATOR,
    READINGUSER,
    READINGONLYUSER,
    READINGPASSWORD,
    DGRAM_LC_COMMAND,
    DGRAM_MP_COMMAND,
    DGRAM_BT_COMMAND,
    DGRAM_BR_COMMAND,
    DGRAM_HC_COMMAND,
    DGRAM_CC_COMMAND,
    DGRAM_LU_COMMAND,
    DGRAM_DU_COMMAND,
    DGRAM_AU_COMMAND,
    //---- bad states------
    DGRAM_INVALID_COMMAND,
    DGRAMDISCARD,
    DGRAMBADAUTH,
    INVALID_VERSION,
    INVALID_AUTHKEY,
} mp3p_states;

/*
ORDEN:
header
identifDgram
clave
contenido
*/


#define SERVERSTATSMESSAGE "MP3P V1.0 200\n%s\n%lu"
#define SERVERSTATUSMESSAGE "MP3P V1.0 %d\n%s\n"
#define OK 200
#define UNAUTHORIZED 100
#define WRONG_PROTOCOL_VERSION 101
#define RESOURCE_NOT_FOUND 102
#define BADAUTHORIZATIONFORMATION 103
#define INVALID_COMMAND_FORMATION 104


#define IDLENGTH 32


static inline int errorDatagramMessage(char * dgramOutput,mp3p_args_data * data, int error){
    return sprintf(dgramOutput,SERVERSTATUSMESSAGE,error,data->uniqueID);
}

static inline int okDatagramMessage(char * dgramOutput,mp3p_args_data* data){
    return sprintf(dgramOutput,SERVERSTATUSMESSAGE,OK,data->uniqueID);
}


//error type 100, unauthorized
static int unauthorizedStrategy(mp3p_args_data *data, char * dgramOutput){
    return errorDatagramMessage(dgramOutput,data,UNAUTHORIZED);
}
//error type 101, wrong protocol version
static int versionMismatchStrategy(mp3p_args_data * data, char * dgramOutput){
    return errorDatagramMessage(dgramOutput,data,WRONG_PROTOCOL_VERSION);
}

static int outputStatisticsMessage(char * dgramOutput, mp3p_args_data * data ,uint64_t numberData){
    return sprintf(dgramOutput,SERVERSTATSMESSAGE,data->uniqueID,numberData);
}

static int bytesTransferedStrategy(mp3p_args_data * args, char * dgramOutput){
    uint64_t transferedBytes = getBytesTransferedFromStats();
    return outputStatisticsMessage(dgramOutput,args,transferedBytes);
}

static int bytesRecievedStrategy(mp3p_args_data * args, char * dgramOutput){
    uint64_t transferedBytes = getBytesRecievedFromStats();
    return outputStatisticsMessage(dgramOutput,args,transferedBytes);
}


static int historicConnectionsStrategy(mp3p_args_data * args,char * dgramOutput){
    uint64_t historicConnections = getHistoricConnectionsFromStats();
    return outputStatisticsMessage(dgramOutput,args,historicConnections);
}


static int concurrentConnectionsStrategy(mp3p_args_data * args, char * dgramOutput){
    uint64_t concurrentConnections = getConcurrentConnectionsFromStats();
    return outputStatisticsMessage(dgramOutput,args,concurrentConnections);
}



static int deleteUserStrategy(mp3p_args_data * args, char * dgramOutput){
    deleteUserNode(args->username);
    log(INFO,"Deleted user %s from user list",args->username);
    return okDatagramMessage(dgramOutput,args);
}

static int modifyUserPasswordStrategy(mp3p_args_data * args, char * dgramOutput){
    bool modified = modifyUserPassword(args->username,args->password);
    if ( modified ){
        log(INFO,"user password %s has been modified to %s\n",args->username,args->password);
        return okDatagramMessage(dgramOutput,args);
    }
    return errorDatagramMessage(dgramOutput,args,RESOURCE_NOT_FOUND);
}

static int addUserStrategy(mp3p_args_data * args, char * dgramOutput){
    if ( !insertUserNode(args->username,args->password) ) {
        //el usuario ya existia

        return modifyUserPasswordStrategy(args,dgramOutput);
    }
    log(INFO,"User with username %s and password %s has been added",args->username,args->password);
    return okDatagramMessage(dgramOutput,args);
}

static int listUsersStrategy(mp3p_args_data * args, char * dgramOutput){
    int length = okDatagramMessage(dgramOutput,args);
    int bytesCopied = listUsers(dgramOutput + length);
    return length + bytesCopied;
}

static int badAuthorizationStrategy(mp3p_args_data * args, char * dgramOutput){
    return errorDatagramMessage(dgramOutput,args,BADAUTHORIZATIONFORMATION);
}


static int invalidCommandStrategy(mp3p_args_data * args, char * dgramOutput){
    return errorDatagramMessage(dgramOutput,args,INVALID_COMMAND_FORMATION);
}

static int listCapabilitiesStrategy(mp3p_args_data * args ,char * dgramOutput){
    
    int length = okDatagramMessage(dgramOutput,args);
    int bytesCopied = sprintf(dgramOutput + length,"MP3P V1.0\nBT\nBR\nCC\nHC\nDU\nAU\nMP\nLU\nLC\n");
    return length + bytesCopied;
}


static mp3p_states parseMp3pCharacter(char c, mp3p_states prevState, int * length,int * authorizationLength){

    static mp3p_states onSuccessCommandState;

    if ( prevState == START){
        if (c == 'M')
            return READM;
        return DGRAMDISCARD;
    }

    if (prevState == READM){
        if ( c == 'P')
            return READP;
        return DGRAMDISCARD;
    }

    if (prevState == READP){
        if ( c == '3')
            return READ3;
        return DGRAMDISCARD;
    }

    if (prevState == READ3){
        if ( c == 'P'){
            return READSECONDP;
        }
        return DGRAMDISCARD;
    }

    if (prevState == READSECONDP){
        if (c == ' '){
            return READVERSIONSEPARATOR;
        }
        return DGRAMDISCARD;
    }

    if (prevState == READVERSIONSEPARATOR){
        if ( c == 'V')
            return READVERSIONV;
        return DGRAMDISCARD;
    }

    if (prevState == READVERSIONV){
        if ( c >= '0' && c <= '9'){
            return READVERSIONNUM;
        }
        return DGRAMDISCARD;
    }

    if (prevState == READVERSIONNUM){
        if ( c >= '0' && c <= '9'){
            return READVERSIONNUM;
        } else if ( c == '.'){
            return READVERSIONDOT;
        }
        return DGRAMDISCARD;
    }

    if (prevState == READVERSIONDOT){
        if ( c >= '0' && c <= '9'){
            return READVERSIONSUBNUM;
        }
        return DGRAMDISCARD;
    }

    if (prevState == READVERSIONSUBNUM){
        if ( c == '\n'){
            return READFIRSTNEWLINE;
        }
        return DGRAMDISCARD;
    }

    if (prevState == READFIRSTNEWLINE){
        if (isalnum(c)){
            *length += 1;
            return READCLIENTDGRAMID;
        }
        return DGRAMDISCARD;
    }

    if (prevState == READCLIENTDGRAMID){
        if (c == '\n' && *length == IDLENGTH){
            return READSECONDNEWLINE;
        }
        if ( *length < IDLENGTH && isalnum(c)){
            *length += 1;
            return READCLIENTDGRAMID;
        }
        return DGRAMDISCARD;
    }


    if (prevState == READSECONDNEWLINE){
        if (isalnum(c)){
            *authorizationLength += 1;
            return READCLIENTPASSWORD;
        }
        return DGRAMBADAUTH;
    }

    if (prevState == READCLIENTPASSWORD){
        if (c == '\n' && *authorizationLength == IDLENGTH)
            return READTHIRDNEWLINE;
        if (*authorizationLength < IDLENGTH && isalnum(c)){
            *authorizationLength += 1;
            return READCLIENTPASSWORD;
        }
        return DGRAMBADAUTH;
    }

    if (prevState == READTHIRDNEWLINE){
        if (c == 'B'){
            return READB;
        } else if (c == 'H'){
            return READH;
        } else if (c == 'C'){
            return READC;
        } else if (c == 'L'){
            return READL;
        } else if (c == 'M'){
            return READM_FOR_MP;
        } else if ( c == 'D'){
            return READ_D;
        } else if (c == 'A'){
            return READA;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READA){
        if (c == 'U'){
            onSuccessCommandState = DGRAM_AU_COMMAND;
            return READAU;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READ_D){
        if ( c == 'U'){
            onSuccessCommandState = DGRAM_DU_COMMAND;
            return READDU;
        }
        return DGRAM_INVALID_COMMAND;
    }


    if (prevState == READM_FOR_MP){
        if (c == 'P'){
            onSuccessCommandState = DGRAM_MP_COMMAND;
            return READMP;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READL){
        if (c == 'U'){
            return READLU;
        } else if (c == 'C'){
            return READLC;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if ( prevState == READC){
        if (c == 'C'){
            return READCC;
        } 
        return DGRAM_INVALID_COMMAND;
    }

    if ( prevState == READH){
        if (c == 'C'){
            return READHC;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READB){
        if (c == 'R'){
            return READBR;
        }
        if (c == 'T'){
            return READBT;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READBT){
        if ( c == '\0'){
            return DGRAM_BT_COMMAND;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if ( prevState == READBR){
        if (c == '\0'){
            return DGRAM_BR_COMMAND;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if ( prevState == READHC){
        if (c == '\0'){
            return DGRAM_HC_COMMAND;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READCC){
        if (c == '\0'){
            return DGRAM_CC_COMMAND;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READLC){
        if (c == '\0'){
            return DGRAM_LC_COMMAND;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READLU){
        if (c == '\0'){
            return DGRAM_LU_COMMAND;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READDU){
        if ( c == ' '){
            return READONEARGSEPARATOR;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READMP || prevState == READAU){
        if ( c == ' '){
            return READFIRSTARGSEPARATOR;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READONEARGSEPARATOR){
        if ( c != ' ' && c != '\n' && c != 0){
            return READINGONLYUSER;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READFIRSTARGSEPARATOR){
        if (c != ' ' && c != '\n' && c != 0){
            return READINGUSER;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READINGONLYUSER){
        if (c != ' ' && c != '\n' && c != 0){
            return READINGONLYUSER;
        } else if (c == 0){
            return onSuccessCommandState;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READINGUSER){
        if ( c == ' '){
            return READSECONDARGSEPARATOR;
        }
        if ( c != '\n' && c != 0){
            return READINGUSER;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READSECONDARGSEPARATOR){
        if ( c != ' ' && c != '\n' && c != 0){
            return READINGPASSWORD;
        }
        return DGRAM_INVALID_COMMAND;
    }

    if (prevState == READINGPASSWORD){
        if ( c != '\n' && c != ' ' && c != 0){
            return READINGPASSWORD;
        } else if (c == 0){
            return onSuccessCommandState;
        }
        return DGRAM_INVALID_COMMAND;
    }


    return DGRAMDISCARD;

}


static inline bool checkVersion(char * version){
    return strcmp(version,VERSION) == 0;
}

static inline bool checkAuthentication(char * auth){
    return strcmp(auth,AUTHKEY) == 0;
}

//returns end of line, more efficient
char * copyVersion(char * dgram, char * dest){
    char * versionPtr = strchr(dgram,'V');
    while (*versionPtr != '\n')
    {
        *dest++ = *versionPtr++;
    }

    *dest = 0;

    return versionPtr;
    
}

char * copyLine(char * line, char * dest){
    while ( *line != '\n'){
        *dest++ = *line++;
    }
    *dest = 0;
    return line;
}


static void obtainUsername(char * dest,char * line){
    char * usernamePosition = strchr(line,' ');
    usernamePosition++;
    while ( *usernamePosition != ' ' && *usernamePosition != 0)
    {
        *dest++ = *usernamePosition++;
    }

    *dest = 0;
}

static void obtainPassword(char * dest, char * line){
    char * passwordPosition = strrchr(line,' ');
    passwordPosition++;
    while ( *passwordPosition != 0 && *passwordPosition != ' '){
        *dest++ = *passwordPosition++;
    }

    *dest = 0;
}


void copyDgramData(char * dgram, mp3p_data * dest,mp3p_states commandState){
    char * currentDgramPosition = copyVersion(dgram,dest->headers.version);
    //we skip the newline
    currentDgramPosition++;
    // we copy the packet id
    currentDgramPosition = copyLine(currentDgramPosition,dest->headers.uniqueID);
    //we skip newline
    currentDgramPosition++;
    //we copy the password
    currentDgramPosition = copyLine(currentDgramPosition,dest->headers.authorization);
    currentDgramPosition++;//we skip the newline

    if (commandState == DGRAM_MP_COMMAND || commandState == DGRAM_AU_COMMAND){
        obtainUsername(dest->headers.username,currentDgramPosition);
        obtainPassword(dest->headers.password,currentDgramPosition);
    } else if (commandState == DGRAM_DU_COMMAND){
        obtainUsername(dest->headers.username,currentDgramPosition);
    }



}



int parseDatagram(char * dgram, int dgramLen,mp3p_data * dest){
    mp3p_states currentState = START;
    int idLength = 0;
    int authorizationLength = 0;
    int i;
    for ( i = 0; i < dgramLen && i < MAXDATAGRAMLENGTH; i++){
        currentState = parseMp3pCharacter(dgram[i],currentState,&idLength,&authorizationLength);
        if ( currentState == DGRAM_INVALID_COMMAND || currentState == DGRAMDISCARD || currentState == DGRAMBADAUTH ){
            break;
        } 
    }

    if (i > MAXDATAGRAMLENGTH || currentState == DGRAMDISCARD){
        return DGRAMERROR;
    }

    //datagram is well-formed, we can copy the data
    copyDgramData(dgram,dest,currentState);

    if ( currentState != DGRAM_INVALID_COMMAND && currentState != DGRAMBADAUTH){

        if (!checkVersion(dest->headers.version)){
            currentState = INVALID_VERSION;
        } else if (!checkAuthentication(dest->headers.authorization)){
            currentState = INVALID_AUTHKEY;
        }
    }

    switch (currentState)
    {
    case DGRAM_BT_COMMAND:
        dest->commandFunction = bytesTransferedStrategy;
        break;
    case DGRAM_BR_COMMAND:
        dest->commandFunction = bytesRecievedStrategy;
        break;
    case DGRAM_HC_COMMAND:
        dest->commandFunction = historicConnectionsStrategy;
        break;
    case DGRAM_CC_COMMAND:
        dest->commandFunction = concurrentConnectionsStrategy;
        break;
    case DGRAM_LU_COMMAND:
        dest->commandFunction = listUsersStrategy;
        break;
    case DGRAM_MP_COMMAND:
        dest->commandFunction = modifyUserPasswordStrategy;
        break;
    case DGRAM_DU_COMMAND:
        dest->commandFunction = deleteUserStrategy;
        break;
    case DGRAM_AU_COMMAND:
        dest->commandFunction = addUserStrategy;
        break;
    case DGRAM_LC_COMMAND:
        dest->commandFunction = listCapabilitiesStrategy;
        break;
    case DGRAMBADAUTH:
        dest->commandFunction = badAuthorizationStrategy;
        break;
    case DGRAM_INVALID_COMMAND:
        dest->commandFunction = invalidCommandStrategy;
        break;
    case INVALID_AUTHKEY:
        dest->commandFunction = unauthorizedStrategy;
        break;
    case INVALID_VERSION:
        dest->commandFunction = versionMismatchStrategy;
        break;  
    default:
        break;
    }

    return DGRAMSUCCESS;
}


