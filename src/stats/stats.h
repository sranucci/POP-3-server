#ifndef STATS
#define STATS

#include <stdint.h>

typedef struct {
    uint64_t historicConnections;
    uint64_t bytesTransfered;
    uint64_t bytesRecieved;
    uint64_t concurrentConnections;
} stats_singleton;


void addTransferedBytesToStats(int bytes);
void addConcurrentConnectionToStats();
void removeConcurrentConnectionFromStats();
void addRecievedBytesToStats(int bytes);

uint64_t getHistoricConnectionsFromStats();
uint64_t getConcurrentConnectionsFromStats();
uint64_t getBytesTransferedFromStats();
uint64_t getBytesRecievedFromStats();

#endif