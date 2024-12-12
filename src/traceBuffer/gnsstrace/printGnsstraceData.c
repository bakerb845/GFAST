#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"


int traceBuffer_gnsstrace_printGnsstraceData(struct gnsstraceData_struct *gnsstraceData) {
    const char *fcnm = __func__;
    struct gnsstrace_node *node;
    char nscl[256];

    if (gnsstraceData->ntraces <= 0) {
        LOG_ERRMSG("%s: No data to print\n", fcnm);
        return -1;
    }

    int i, k;
    for (k = 0; k < gnsstraceData->ntraces; k++) {
        sprintf(nscl, "%s.%s.%s.%s", gnsstraceData->traces[k].netw, gnsstraceData->traces[k].stnm,
                gnsstraceData->traces[k].chan, gnsstraceData->traces[k].loc);
        node = traceBuffer_gnsstrace_hashmap_contains(gnsstraceData->hashmap, nscl);
        LOG_DEBUGMSG("%d: %s %f nchunks=%d npts=%d, node.i=%d",
            k, nscl,
            gnsstraceData->traces[k].dt, gnsstraceData->traces[k].nchunks, gnsstraceData->traces[k].npts,
            node->i);
        for (i = 0; i < gnsstraceData->traces[k].nchunks; i++) {
            LOG_DEBUGMSG("   chunk[%d]=%d",
                i, gnsstraceData->traces[k].chunkPtr[i]);
        }
        for (i = 0; i < gnsstraceData->traces[k].npts; i++) {
            LOG_DEBUGMSG("   data[%d]=%d, time[%d]=%f",
                i, gnsstraceData->traces[k].data[i], i, gnsstraceData->traces[k].times[i]);
        }
    }
    return 0;
}