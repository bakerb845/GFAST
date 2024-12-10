#include "gfast_core.h"

void dataexchange_initializeDataConnection() {
#ifdef ENABLE_EARTHWORM
    LOG_MSG("%s", "Initializing Earthworm");
#elif ENABLE_GEOJSON

    #ifdef ENABLE_NATS
        LOG_MSG("%s", "Initializing NATS");
    #elif ENABLE_KAFKA
        LOG_MSG("%s", "Initializing Kafka");
    #else 
        LOG_ERRMSG("%s", "No data connections specified!");
    #endif

#else
    LOG_ERRMSG("%s", "No data connections specified!");
#endif

}