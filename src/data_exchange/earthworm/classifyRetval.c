#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "transport.h"
// #include "gfast_traceBuffer.h"
#include "gfast_core.h"

/*!
 * @brief Classifies return value from an earthworm get transport activity.
 *
 * @param[in] retval    Earthworm return code to classify.
 *
 * @retval  1 -> The requested message was received. 
 * @retval  0 -> There are no more messages.
 * @retval -1 -> Messages were received but there may be a loss of
 *               information.
 * @retval -2 -> An error occurred and no messages were received.
 *
 * @author Ben Baker
 *
 * @copyright ISTI distribted under Apache 2.
 *
 */
int dataexchange_earthworm_classifyGetRetval(const int retval)
{
    char msg[128];
    // Got a requested message (modid, type, class)
    if (retval == GET_OK) {return 1;} 
    // No logos of requested messages in in memory 
    if (retval == GET_NONE) {return 0;} 
    memset(msg, 0, 128*sizeof(char)); 
    // Got a message but missed some
    if (retval == GET_MISS)
    {   
        sprintf(
            msg,
            "%s: GET_MISS: Some messages were missed",
            __func__);
        LOG_WARNMSG("%s", msg);
        return -1; 
    }
    // Got a message but ntrack_get was exceeded
    if (retval == GET_NOTRACK)
    {   
        sprintf(
            msg,
            "%s: GET_NOTRACK: Message exceeded NTRACK_GET",
            __func__);
        LOG_WARNMSG("%s", msg);
        return -1; 
    }   
    // Message requested exceeded my buffer
    if (retval == GET_TOOBIG)
    {
        sprintf(
            msg,
            "%s: GET_TOOBIG: Next message of requested logo(s) is too big",
            __func__);
        LOG_WARNMSG("%s", msg);
        return -2;
    }
    // Didn't check ring fast enough and missed a message
    if (retval == GET_MISS_LAPPED)
    {
        sprintf(
            msg,
            "%s: GET_MISS_LAPPED: Some messages were overwritten",
            __func__);
        LOG_WARNMSG("%s", msg);
        return -1;
    }
    // Message contains a gap
    if (retval == GET_MISS_SEQGAP)
    {
        sprintf(
            msg,
            "%s: GET_MISS_SEQGAP: A gap in messages was detected",
            __func__);
        LOG_WARNMSG("%s", msg);
        return -1;
    }
    // I don't know
    sprintf(
        msg,
        "%s: Could not classify return value",
        __func__);
    LOG_WARNMSG("%s", msg);
    return -2;
}

