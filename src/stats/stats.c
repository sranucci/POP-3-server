#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




static stats_singleton * createStatsInstance(){
    static stats_singleton statistics;
    memset(&statistics,0,sizeof(stats_singleton));
    return &statistics;
}




static stats_singleton * getStatsInstance(){
    static stats_singleton * singletonPtr = NULL;
    if ( singletonPtr == NULL ){
        singletonPtr = createStatsInstance();
    }
    return singletonPtr;
}



void addTransferedBytesToStats(int bytes){
    stats_singleton * stats = getStatsInstance();
    stats->bytesTransfered += bytes;
}

void addRecievedBytesToStats(int bytes){
    stats_singleton * stats = getStatsInstance();
    stats->bytesRecieved += bytes;
}

void addConcurrentConnectionToStats(){
    stats_singleton * stats = getStatsInstance();
    stats->concurrentConnections += 1;
    stats->historicConnections += 1;
}

void removeConcurrentConnectionFromStats(){
    stats_singleton * stats = getStatsInstance();
    stats->concurrentConnections -= 1;
}


uint64_t getHistoricConnectionsFromStats(){
    stats_singleton *stats = getStatsInstance();
    return stats->historicConnections;
}

uint64_t getConcurrentConnectionsFromStats(){
    stats_singleton * stats = getStatsInstance();
    return stats->concurrentConnections;
}

uint64_t getBytesTransferedFromStats(){
    stats_singleton * stats = getStatsInstance();
    return stats->bytesTransfered;
}

uint64_t getBytesRecievedFromStats(){
    stats_singleton * stats = getStatsInstance();
    return stats->bytesRecieved;
}