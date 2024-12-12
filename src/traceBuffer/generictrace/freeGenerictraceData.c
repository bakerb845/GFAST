#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

/*!
 * @brief Releases memory on the generictraceData_struct data structure.
 *
 * @param[in,out] generictraceData   On input the initialized generictraceData structure. <br>
 *                          On output the structure has been cleared.
 * 
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
void traceBuffer_generictrace_freeGenerictraceData(struct generictraceData_struct *generictraceData)
{
    int i;
    if (!generictraceData->linit)
    {
        LOG_WARNMSG("%s", "Structure never set");
        return;
    }
    if (generictraceData->ntraces > 0 && generictraceData->traces != NULL)
    {
        for (i=0; i<generictraceData->ntraces; i++)
        {
             traceBuffer_generictrace_freeGenerictrace(true, &generictraceData->traces[i]);
        }
        free(generictraceData->traces);
    }
    traceBuffer_generictrace_free_hashmap(generictraceData->hashmap);
    memset(generictraceData, 0, sizeof(struct generictraceData_struct));
    return;
}
