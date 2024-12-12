#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_dataexchange.h"
#include "gfast_core.h"
/*!
 * @brief Disconnects from the ring and clears the earthworm ring structure.
 *
 * @param[in,out] ringInfo   On input contains the initialized earthworm ring
 *                           information.
 *                           On output contains the cleared earthworm ring
 *                           information.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker
 *
 * @copyright ISTI distributed under Apache 2.
 *
 */
int dataexchange_earthworm_finalize(struct ewRing_struct *ringInfo)
{
    if (!ringInfo->linit)
    {   
        LOG_ERRMSG("%s", "Error ewRing_struct was never initialized");
        return -1; 
    }
    if (ringInfo->getLogo != NULL && ringInfo->nlogo > 0)
    {
        free(ringInfo->getLogo);
    }
    tport_detach(&ringInfo->region);
    memset(ringInfo, 0, sizeof(struct ewRing_struct));
    return 0;
}
