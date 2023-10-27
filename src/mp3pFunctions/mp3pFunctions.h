#ifndef MP3PPARSER
#define MP3PPARSER


#define VERSIONSIZE 128
#define AUTHORIZATION 512
#define UIDSIZE 256
#define COMMANDLEN 256
#define USERNAMESIZE 256
#define PASSWORDSIZE 256

typedef struct {
    char version[VERSIONSIZE];
    char authorization[AUTHORIZATION];
    char uniqueID[UIDSIZE];
    char username[USERNAMESIZE];
    char password[PASSWORDSIZE];
} mp3p_args_data;

typedef int (*command_strategy)(mp3p_args_data * data,char * dgramOutput);

typedef struct {
    mp3p_args_data headers;
    command_strategy commandFunction;
} mp3p_data;


int parseDatagram(char * dgram, int dgramLen,mp3p_data * dest);




#define DGRAMSUCCESS 0
#define DGRAMERROR -1
#endif