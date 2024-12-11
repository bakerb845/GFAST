#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"


int traceBuffer_ewrr_printTB2Data(struct tb2Data_struct *tb2Data) {
    const char *fcnm = __func__;
    struct tb2_node *node;
    char nscl[256];

    if (tb2Data->ntraces <= 0) {
        LOG_ERRMSG("%s: No data to print\n", fcnm);
        return -1;
    }

    int i, k;
    for (k = 0; k < tb2Data->ntraces; k++) {
        sprintf(nscl, "%s.%s.%s.%s", tb2Data->traces[k].netw, tb2Data->traces[k].stnm,
                tb2Data->traces[k].chan, tb2Data->traces[k].loc);
        node = traceBuffer_ewrr_hashmap_contains(tb2Data->hashmap, nscl);
        LOG_DEBUGMSG("%d: %s %f nchunks=%d npts=%d, node.i=%d",
            k, nscl,
            tb2Data->traces[k].dt, tb2Data->traces[k].nchunks, tb2Data->traces[k].npts,
            node->i);
        for (i = 0; i < tb2Data->traces[k].nchunks; i++) {
            LOG_DEBUGMSG("   chunk[%d]=%d",
                i, tb2Data->traces[k].chunkPtr[i]);
        }
        for (i = 0; i < tb2Data->traces[k].npts; i++) {
            LOG_DEBUGMSG("   data[%d]=%d, time[%d]=%f",
                i, tb2Data->traces[k].data[i], i, tb2Data->traces[k].times[i]);
        }
    }
    return 0;
}