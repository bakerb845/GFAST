#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

//============================================================================//
/*!
* @brief Sets the gnsstraceData structure and desired SNCL's from the input gpsData
*
* @param[in] gpsData      holds the GPS SNCL's GFAST is interested in
*
* @param[out] gnsstraceData     on output has space allocated and has a target
*                         list of SNCL's for message reading from the 
*                         earthworm data ring 
*
* @result 0 indicates success
*
* @author Ben Baker (ISTI)
*
*/
int traceBuffer_gnsstrace_setGnsstraceDataFromGFAST(struct GFAST_data_struct *gpsData,
                                         struct gnsstraceData_struct *gnsstraceData)
{
    const char *fcnm = __func__;
    char *nscl = NULL;
    int i, it, k;
    int debug = 0;
    if (gpsData->stream_length == 0) {
        LOG_ERRMSG("%s: Error no data to copy\n", fcnm);
        return -1;
    }
    if (gnsstraceData->linit) {
        LOG_ERRMSG("%s: Error gnsstraceData already set\n", fcnm);
        return -1;
    }

    // Init gnsstraceData_struct
    gnsstraceData->ntraces = 7 * gpsData->stream_length;
    gnsstraceData->traces = (struct gnsstrace_struct *)
        calloc( (size_t) gnsstraceData->ntraces, sizeof(struct gnsstrace_struct) );
    gnsstraceData->hashmap = (struct gnsstrace_hashmap_struct *)
        malloc(sizeof(struct gnsstrace_hashmap_struct));
    // Random prime, 80021 should have at most 4-5 collisions for a 20k nscl list
    const uint32_t hashsize = 80021;
    gnsstraceData->hashmap->hashsize = hashsize;
    gnsstraceData->hashmap->map = (struct gnsstrace_node **) calloc(hashsize, sizeof(struct gnsstrace_node *));

    // Copy channel names
    it = 0;
    for (k = 0; k < gpsData->stream_length; k++) {
        // Loop over each channel: Z, N, E, 3, 2, 1, Q
        for (i = 0; i < 7; i++) {
            strcpy(gnsstraceData->traces[it].netw, gpsData->data[k].netw);
            strcpy(gnsstraceData->traces[it].stnm, gpsData->data[k].stnm);
            strcpy(gnsstraceData->traces[it].chan, gpsData->data[k].chan[i]);
            strcpy(gnsstraceData->traces[it].loc,  gpsData->data[k].loc); 
            it = it + 1;
        }
    }
    if (debug) {
        LOG_DEBUGMSG("Printing %d traces in gnsstraceData", gnsstraceData->ntraces);
        for (i = 0; i < gnsstraceData->ntraces; i++) {
            LOG_DEBUGMSG("CCC %d: %s.%s.%s.%s", i,
                gnsstraceData->traces[i].netw,
                gnsstraceData->traces[i].stnm,
                gnsstraceData->traces[i].chan,
                gnsstraceData->traces[i].loc);
        }
        LOG_DEBUGMSG("%s", "Done printing traces in gnsstraceData");
    }

    // Now add trace names to hashmap
    for (i = 0; i < gnsstraceData->ntraces; i++) {
        nscl = (char *) malloc(31 * sizeof(char));
        sprintf(nscl, "%s.%s.%s.%s",
            gnsstraceData->traces[i].netw,
            gnsstraceData->traces[i].stnm,
            gnsstraceData->traces[i].chan,
            gnsstraceData->traces[i].loc);
        if (traceBuffer_gnsstrace_hashmap_add(gnsstraceData->hashmap, nscl, i) == NULL) {
            LOG_ERRMSG("%s: Couldn't add %s to hashmap!", fcnm, nscl);
            free(nscl);
            return -1;
        }
        free(nscl);
    }

    if (it != gnsstraceData->ntraces) {
        LOG_ERRMSG("%s: Lost count %d %d\n", fcnm, it, gnsstraceData->ntraces);
        return -1;
    }
    gnsstraceData->linit = true;
    return 0;
}
