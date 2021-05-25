#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Reads the tracebuffer2 messages from the Earthworm ring specified
 *        on ringInfo.
 *
 * @param[in] messageBlock   Block allocator size.  Instead of reallocating
 *                           memory every loop iteration in the acquisition
 *                           one can allocate in a chunk of messages (e.g.
 *                           200 messages at a time i.e. messageBlock = 200).
 *                           If this number is too small there will be 
 *                           overhead in memory reallocation, if this number
 *                           is too big a lot of unnecessary space will be
 *                           allocated. 
 * @param[in] showWarnings   If true then the print warnings about having read
 *                           maximum number of messages.
 * @param[in] ringInfo       Earthworm ring reader structure.
 *
 * @param[out] nRead         Number of traceBuffer2 messages read.
 *
 * @param[out] ierr          0 Indicates success. <br>
 *                          -1 Indicates a terminate signal from the ring. <br>
 *                             the user should call traceBuffer_ewrr_finalize
 *                             and quit. <br>
 *                          -2 Indicates a read error on the ring. <br>
 *                          -3 Indicates the ringInfo structure was not
 *                             initalized. <br> 
 *                          -4 Indicates tracebuf2 type is unknown.
 *
 * @result An array of [nRead] traceBuffer2 messages read from the Earthworm
 *         ring.  The k'th message start index (for k=1,2,...,nRead) is given
 *         by: (k - 1)*MAX_TRACEBUF_SIZE [nRead*MAX_TRACEBUF_SIZE]
 *
 * @author Ben Baker
 *
 * @copyright ISTI distribted under Apache 2.
 *
 */
char *traceBuffer_ewrr_getMessagesFromRing(const int messageBlock,
                                           const bool showWarnings,
                                           struct ewRing_struct *ringInfo,
                                           int *nRead, int *ierr)
{
  MSG_LOGO gotLogo; 
  TRACE2_HEADER traceHeader;
  char *msg, *msgs, *msgWork;
  unsigned char sequenceNumber;
  long gotSize;
  int kdx, nblock, ncopy, nwork, retval, maxMessages, maxSpace;
  int debug = 0;
  //size_t nbytes; //, npcopy;
  //------------------------------------------------------------------------//
  //  
  // Make sure this is initialized
  *ierr = 0;
  *nRead = 0;
  msg = NULL;
  msgs = NULL;

  if (!ringInfo->linit)
    {
      LOG_ERRMSG("%s", "Error ringInfo not initialized");
      *ierr =-3;
      return msgs;
    }
  // Set some sane limits to avoid a segfault
  // arbitrarily set as 0.8 size of int
  maxSpace=0.8*INT_MAX;
  maxMessages=maxSpace/MAX_TRACEBUF_SIZ;

  if (messageBlock < 1)
    {
      LOG_ERRMSG("%s", "messageBlock allocator must be positive");
      *ierr =-4;
      return msgs;
    }
  // Set space
  memset(&gotLogo, 0, sizeof(MSG_LOGO));
  msgs = memory_calloc8c(MAX_TRACEBUF_SIZ*messageBlock);
  msg  = memory_calloc8c(MAX_TRACEBUF_SIZ);
  nblock = 1;
  // Unpack the ring
  while (true)
    {
      // May have a kill signal from the transport layer 
      retval = tport_getflag(&ringInfo->region);
      if (retval == TERMINATE) {
        LOG_ERRMSG("Receiving kill signal from ring %s", ringInfo->ewRingName);
        *ierr =-1;
        break;
      }
      // Copy from the memory
      retval = tport_copyfrom(&ringInfo->region, ringInfo->getLogo, ringInfo->nlogo,
                              &gotLogo, &gotSize, msg, MAX_TRACEBUF_SIZ, &sequenceNumber);
      // Classify my message
      retval = traceBuffer_ewrr_classifyGetRetval(retval);
      if (retval ==-2) {
        LOG_ERRMSG("%s", "An error was encountered getting message");
        *ierr =-2;
        break;
      }

      // MTH: This is where we should leave the ring:
      if (retval == GET_NONE){
        //printf("MTH: Break out of while loop!\n");
        break;
      }

      // Verify i want this message
      if (gotLogo.type == ringInfo->traceBuffer2Type)
      {
        // Get the header
        memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));
        *ierr = WaveMsg2MakeLocal(&traceHeader);
        if (*ierr < 0) {
          LOG_ERRMSG("%s", "Error flipping bytes");
          *ierr =-2;
          break;
        }

        if (debug){
          printf("getMsgs: %s.%s.%s.%s time:%f npts:%d\n",
          traceHeader.net, traceHeader.sta, traceHeader.chan, traceHeader.loc,
          traceHeader.starttime, traceHeader.nsamp);
        }

        kdx = *nRead*MAX_TRACEBUF_SIZ;
        memcpy(&msgs[kdx], msg, MAX_TRACEBUF_SIZ*sizeof(char));
        (*nRead)+=1;

        //check if sane allocation limits reached
        if (*nRead >= maxMessages) {
          LOG_MSG("XXgetMessagesFromRingXX: nRead=%d nblock=%d messageBlock=%d --> Single-call message limits reached.",
          *nRead, nblock, messageBlock);
          if (showWarnings) {
            LOG_WARNMSG("%s", "Single-call message limits reached");
          }
          break;
        }

        // Filled current allocation. Reallocate space if possible
        //vck should revisit this later and see if we can just use realloc()
        if (*nRead >= messageBlock*nblock)
        {
          LOG_MSG("XXgetMessagesFromRingXX: nRead=%d nblock=%d messageBlock=%d --> Reallocating msgs block",
                   *nRead, nblock, messageBlock);
          if (showWarnings) {
            LOG_WARNMSG("%s", "Reallocating msgs block");
          }
          ncopy = MAX_TRACEBUF_SIZ*(*nRead);

          //avoid exceeding limits
          if (maxSpace-ncopy < MAX_TRACEBUF_SIZ*messageBlock) {
            nwork = maxSpace;
          } else {
            nwork = ncopy + MAX_TRACEBUF_SIZ*messageBlock;
            nblock+=1;
          }
          // set workspace and copy old messages
          msgWork = memory_calloc8c(ncopy);
          memcpy(msgWork, msgs, (size_t) ncopy);
          // resize msgs
          memory_free8c(&msgs);
          msgs = memory_calloc8c(nwork);
          // copy back and free workspace
          memcpy(msgs, msgWork, (size_t) ncopy);
          memory_free8c(&msgWork);
        }
      }
    } // while true

  memory_free8c(&msg);

  if (ringInfo->msWait > 0){sleep_ew(ringInfo->msWait);}
  return msgs;
}
