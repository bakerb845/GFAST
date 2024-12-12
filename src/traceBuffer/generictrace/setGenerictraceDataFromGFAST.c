#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

//============================================================================//
/*!
* @brief Sets the generictraceData structure and desired SNCL's from the input gpsData
*
* @param[in] gpsData      holds the GPS SNCL's GFAST is interested in
*
* @param[out] generictraceData     on output has space allocated and has a target
*                         list of SNCL's for message reading from the 
*                         earthworm data ring 
*
* @result 0 indicates success
*
* @author Ben Baker (ISTI)
*
*/
int traceBuffer_generictrace_setGenerictraceDataFromGFAST(struct GFAST_data_struct *gpsData,
                                         struct generictraceData_struct *generictraceData)
{
    const char *fcnm = __func__;
    char *nscl = NULL;
    int i, it, k;
    int debug = 0;
    if (gpsData->stream_length == 0) {
        LOG_ERRMSG("%s: Error no data to copy\n", fcnm);
        return -1;
    }
    if (generictraceData->linit) {
        LOG_ERRMSG("%s: Error generictraceData already set\n", fcnm);
        return -1;
    }

    // Init generictraceData_struct
    generictraceData->ntraces = 7 * gpsData->stream_length;
    generictraceData->traces = (struct generictrace_struct *)
        calloc( (size_t) generictraceData->ntraces, sizeof(struct generictrace_struct) );
    generictraceData->hashmap = (struct generictrace_hashmap_struct *)
        malloc(sizeof(struct generictrace_hashmap_struct));
    // Random prime, 80021 should have at most 4-5 collisions for a 20k nscl list
    const uint32_t hashsize = 80021;
    generictraceData->hashmap->hashsize = hashsize;
    generictraceData->hashmap->map = (struct generictrace_node **) calloc(hashsize, sizeof(struct generictrace_node *));

    // Copy channel names
    it = 0;
    for (k = 0; k < gpsData->stream_length; k++) {
        // Loop over each channel: Z, N, E, 3, 2, 1, Q
        for (i = 0; i < 7; i++) {
            strcpy(generictraceData->traces[it].netw, gpsData->data[k].netw);
            strcpy(generictraceData->traces[it].stnm, gpsData->data[k].stnm);
            strcpy(generictraceData->traces[it].chan, gpsData->data[k].chan[i]);
            strcpy(generictraceData->traces[it].loc,  gpsData->data[k].loc); 
            it = it + 1;
        }
    }
    if (debug) {
        LOG_DEBUGMSG("Printing %d traces in generictraceData", generictraceData->ntraces);
        for (i = 0; i < generictraceData->ntraces; i++) {
            LOG_DEBUGMSG("CCC %d: %s.%s.%s.%s", i,
                generictraceData->traces[i].netw,
                generictraceData->traces[i].stnm,
                generictraceData->traces[i].chan,
                generictraceData->traces[i].loc);
        }
        LOG_DEBUGMSG("%s", "Done printing traces in generictraceData");
    }

    // Now add trace names to hashmap
    for (i = 0; i < generictraceData->ntraces; i++) {
        nscl = (char *) malloc(31 * sizeof(char));
        sprintf(nscl, "%s.%s.%s.%s",
            generictraceData->traces[i].netw,
            generictraceData->traces[i].stnm,
            generictraceData->traces[i].chan,
            generictraceData->traces[i].loc);
        if (traceBuffer_generictrace_hashmap_add(generictraceData->hashmap, nscl, i) == NULL) {
            LOG_ERRMSG("%s: Couldn't add %s to hashmap!", fcnm, nscl);
            free(nscl);
            return -1;
        }
        free(nscl);
    }

    if (it != generictraceData->ntraces) {
        LOG_ERRMSG("%s: Lost count %d %d\n", fcnm, it, generictraceData->ntraces);
        return -1;
    }
    generictraceData->linit = true;
    return 0;
}
