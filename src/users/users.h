#ifndef AVAILABLE_USERS
#define AVAILABLE_USERS

#define USERDATASIZE 512
#define MAXUSERS 500


#include <stdbool.h>

#define USERDATASIZE 512
#define MAXUSERS 500

typedef struct {
    char name[USERDATASIZE];
    char password[USERDATASIZE];
    bool userConnected;
} registered_users_data;

typedef struct user_node {
    registered_users_data data;
    struct user_node* next;
} user_node;

typedef struct {
    user_node* head;
    int size;
} user_linked_list_singleton;


void initializeUserSingleton(int initialUsers, char ** userData);
void toggleUserConnected(char * username, bool isConnected);
bool validPassword(char * username, char * password);
void removeAllUserNodes();
bool insertUserNode(char * name, char * password);
bool userIsConnected(char * username);
void deleteUserNode(char* name);
bool modifyUserPassword(char * username, char * newPassword);
int listUsers(char * dgram);


#endif