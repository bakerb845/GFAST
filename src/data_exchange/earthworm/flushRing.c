#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// #include "gfast_traceBuffer.h"
#include "gfast_dataexchange.h"
#include "gfast_core.h"
/*!
 * @brief Flushes the ring on ringInfo.
 *
 * @param[in] ringInfo    Structure with earthworm ring information to 
 *                        flush.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker
 *
 * @copyright ISTI distributed under Apache 2.
 *
 */
int dataexchange_earthworm_flushRing(struct ewRing_struct *ringInfo)
{
    MSG_LOGO gotLogo;
    char msg[MAX_TRACEBUF_SIZ];
    unsigned char sequenceNumber;
    long gotSize;
    int retval;
    //------------------------------------------------------------------------//
    //
    // Make sure user knows the ring was flushed
    if (ringInfo->linit == false)
    {
        LOG_ERRMSG("%s", "Error the ring was never initialized");
        return -1;
    }
    // Lift all the messages from the ring
    while (true)
    {
        retval = tport_copyfrom(&ringInfo->region, 
                                ringInfo->getLogo, ringInfo->nlogo,
                                &gotLogo, &gotSize, msg, MAX_TRACEBUF_SIZ,
                                &sequenceNumber);
        if (retval == GET_NONE){break;} // End of ring
        //dataexchange_earthworm_classifyGetRetval(retval);
    }
    if (ringInfo->msWait > 0){sleep_ew(ringInfo->msWait);}
    return 0;
}
