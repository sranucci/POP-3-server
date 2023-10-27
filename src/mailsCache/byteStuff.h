#ifndef BYTE_STUFF
#define BYTE_STUFF
#include <stdbool.h>

typedef struct charactersProcessor charactersProcessor;

charactersProcessor * initCharactersProcessor();
void freeCharactersProcessor(charactersProcessor * charactersProcessor);
int getNProcessedCharacters(charactersProcessor * charactersProcessor, char * buffer, int n);
int addCharactersToProcess(charactersProcessor * charactersProcessor, char * buffer, int n, bool eofReached);
int availableCharacters(charactersProcessor * charactersProcessor);
int availableSpace(charactersProcessor * charactersProcessor);
void resetCharactersProcessor(charactersProcessor * charactersProcessor);

#endif