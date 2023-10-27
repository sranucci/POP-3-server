#include "users.h"
#include <stdio.h>
#include <string.h>
#include "../logger/logger.h"
static bool isEmpty();

user_linked_list_singleton * createSingletonUserLinkedListInstance(){
    static user_linked_list_singleton instance;
    instance.head = NULL;
    instance.size = 0;
    return &instance;
}


static user_linked_list_singleton * getSingletonInstance(){
    static user_linked_list_singleton * instance = NULL;
    if (instance == NULL){
        instance = createSingletonUserLinkedListInstance();
    }
    return instance;
}

bool validPassword(char * username, char * password){
    user_linked_list_singleton * singletonPtr = getSingletonInstance();
    bool found = false;
    for ( user_node * userPtr = singletonPtr->head; userPtr != NULL && !found ; userPtr = userPtr->next){
        if (strcmp(userPtr->data.name,username) == 0 && strcmp(userPtr->data.password,password) == 0){
            found = true;
        }
    }
    return found;
}


// Function to check if the linked list is empty
static bool isEmpty() {
    user_linked_list_singleton * list = getSingletonInstance();
    return list->head == NULL;
}

static user_node * getUserNode(char * username) {
    user_linked_list_singleton * list = getSingletonInstance();
    user_node* currentNode = list->head;

    while (currentNode != NULL) {
        if (strcmp(currentNode->data.name, username) == 0) {
            return currentNode;
        }
        currentNode = currentNode->next;
    }

    return NULL;
}

bool userExists(char* name) {
    return getUserNode(name) != NULL;
}

bool userIsConnected(char * username){
    user_node * user = getUserNode(username);
    return (user != NULL) && (user->data.userConnected);
}

void toggleUserConnected(char * username, bool isConnected){
    user_node * user = getUserNode(username);
    if(user!=NULL)
        user->data.userConnected = isConnected;
}


bool modifyUserPassword(char * username, char * newPassword) {
    user_linked_list_singleton * list = getSingletonInstance();
    bool modified = false;
    for ( user_node * current = list->head;!modified && current != NULL; current = current->next){
        if ( strcmp(current->data.name,username) == 0 ){
            strcpy(current->data.password,newPassword);
            modified = true;
        }
    }
    return modified;
}

// Function to insert a node at the beginning of the linked list if the user does not exist
bool insertUserNode(char* name, char* password) {
    user_linked_list_singleton* list = getSingletonInstance();

    if (userExists(name)) {
        return false;
    }

    user_node* newNode = (user_node*)malloc(sizeof(user_node));
    strcpy(newNode->data.name, name);
    strcpy(newNode->data.password, password);
    newNode->data.userConnected = false;
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
    return true;
}


int listUsers(char * dgram){
    user_linked_list_singleton * list = getSingletonInstance();
    int bytesCopied = 0;
    for (user_node * userPtr = list->head; userPtr != NULL; userPtr = userPtr->next){
        bytesCopied += sprintf(dgram + bytesCopied,"%s %s\n",userPtr->data.name,userPtr->data.password);
    }
    return bytesCopied;
}


void deleteUserNode(char* name) {
    user_linked_list_singleton * list = getSingletonInstance();
    user_node* currentNode = list->head;
    user_node* prevNode = NULL;

    while (currentNode != NULL) {
        if (strcmp(currentNode->data.name, name) == 0) {
            if (prevNode == NULL) {
                // Node to be deleted is the head
                list->head = currentNode->next;
            } else {
                prevNode->next = currentNode->next;
            }

            free(currentNode);
            list->size--;
            return;
        }

        prevNode = currentNode;
        currentNode = currentNode->next;
    }

}

// Function to remove all nodes from the linked list
void removeAllUserNodes() {
    user_linked_list_singleton * list = getSingletonInstance();
    user_node* currentNode = list->head;
    user_node* nextNode = NULL;

    while (currentNode != NULL) {
        nextNode = currentNode->next;
        free(currentNode);
        currentNode = nextNode;
    }

    list->head = NULL;
    list->size = 0;
}


void initializeUserSingleton(int initialUsers, char ** names){
    char * delim = ":";
    for ( int i = 0; i < initialUsers ; i++){
        char * username = strtok(names[i],delim);
        char * password = strtok(NULL,delim);
        insertUserNode(username,password);
    }
}