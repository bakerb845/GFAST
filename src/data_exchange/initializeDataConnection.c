#include "gfast_core.h"

void dataexchange_initializeDataConnection() {
#ifdef GFAST_USE_EW
    LOG_MSG("%s", "Initializing Earthworm");
#endif

#ifdef GFAST_ENABLE_GEOJSON
    #ifdef GFAST_USE_NATS
    LOG_MSG("%s", "Initializing NATS");
    data_exchange_nats_connect();
    #elif GFAST_USE_KAFKA
    LOG_MSG("%s", "Initializing Kafka");
    #else 
    LOG_ERRMSG("%s", "No data connections specified!");
    #endif
#endif

}