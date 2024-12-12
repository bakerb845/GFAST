#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

/*!
 * @brief Initializes the data list which will be retained from a 
 *        ring read operation
 *
 * @param[in] ntraces   number of traces to create in gnsstraceData 
 * @param[in] nets      null terminated list of networks [ntraces]
 * @param[in] stats     null terminated list of stations [ntraces]
 * @param[in] chans     null terminated list of channels [ntraces]
 * @param[in] locs      null terminated list of location codes [ntraces]
 *
 * @param[out] gnsstraceData  on successful output contains the SNCL's to 
 *                      copy after each ring read
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
int traceBuffer_gnsstrace_setGnsstraceData(const int ntraces,
                                const char **nets,
                                const char **stats,
                                const char **chans,
                                const char **locs,
                                struct gnsstraceData_struct *gnsstraceData)
{
    int i;
    if (ntraces < 1 || nets == NULL || stats == NULL ||
        chans == NULL || locs == NULL)
    {
        if (ntraces < 1)
        {
            LOG_ERRMSG("%s", "No traces to initialize");
        }
        if (nets == NULL)
        {
            LOG_ERRMSG("%s", "Error must define network list");
        }
        if (stats == NULL)
        {
            LOG_ERRMSG("%s", "Error must define station list");
        }
        if (chans == NULL)
        {
            LOG_ERRMSG("%s", "Error must define channel list");
        }
        if (locs == NULL)
        {
            LOG_ERRMSG("%s", "Error must define location list");
        }
        return -1;
    }
    if (gnsstraceData->linit)
    {
        LOG_ERRMSG("%s", "Error gnsstraceData already initialized");
        return -1;
    }
    gnsstraceData->traces = (struct gnsstrace_struct *)
        calloc( (size_t) ntraces, sizeof(struct gnsstrace_struct));
    for (i=0; i<ntraces; i++)
    {
        strcpy(gnsstraceData->traces[i].netw, nets[i]);
        strcpy(gnsstraceData->traces[i].stnm, stats[i]);
        strcpy(gnsstraceData->traces[i].chan, chans[i]);
        strcpy(gnsstraceData->traces[i].loc,  locs[i]);
    }
    gnsstraceData->ntraces = ntraces;
    gnsstraceData->linit = true;
    return 0;
}
