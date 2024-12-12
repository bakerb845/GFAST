#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"


int traceBuffer_generictrace_printGenerictraceData(struct generictraceData_struct *generictraceData) {
    const char *fcnm = __func__;
    struct generictrace_node *node;
    char nscl[256];

    if (generictraceData->ntraces <= 0) {
        LOG_ERRMSG("%s: No data to print\n", fcnm);
        return -1;
    }

    int i, k;
    for (k = 0; k < generictraceData->ntraces; k++) {
        sprintf(nscl, "%s.%s.%s.%s", generictraceData->traces[k].netw, generictraceData->traces[k].stnm,
                generictraceData->traces[k].chan, generictraceData->traces[k].loc);
        node = traceBuffer_generictrace_hashmap_contains(generictraceData->hashmap, nscl);
        LOG_DEBUGMSG("%d: %s %f nchunks=%d npts=%d, node.i=%d",
            k, nscl,
            generictraceData->traces[k].dt, generictraceData->traces[k].nchunks, generictraceData->traces[k].npts,
            node->i);
        for (i = 0; i < generictraceData->traces[k].nchunks; i++) {
            LOG_DEBUGMSG("   chunk[%d]=%d",
                i, generictraceData->traces[k].chunkPtr[i]);
        }
        for (i = 0; i < generictraceData->traces[k].npts; i++) {
            LOG_DEBUGMSG("   data[%d]=%d, time[%d]=%f",
                i, generictraceData->traces[k].data[i], i, generictraceData->traces[k].times[i]);
        }
    }
    return 0;
}