#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

/*!
 * @brief Releases memory on the gnsstraceData_struct data structure.
 *
 * @param[in,out] gnsstraceData   On input the initialized gnsstraceData structure. <br>
 *                          On output the structure has been cleared.
 * 
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
void traceBuffer_gnsstrace_freeGnsstraceData(struct gnsstraceData_struct *gnsstraceData)
{
    int i;
    if (!gnsstraceData->linit)
    {
        LOG_WARNMSG("%s", "Structure never set");
        return;
    }
    if (gnsstraceData->ntraces > 0 && gnsstraceData->traces != NULL)
    {
        for (i=0; i<gnsstraceData->ntraces; i++)
        {
             traceBuffer_gnsstrace_freeGnsstrace(true, &gnsstraceData->traces[i]);
        }
        free(gnsstraceData->traces);
    }
    traceBuffer_gnsstrace_free_hashmap(gnsstraceData->hashmap);
    memset(gnsstraceData, 0, sizeof(struct gnsstraceData_struct));
    return;
}
