#ifdef WINNT
#include <window.h>
#include <winuser.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_dataexchange.h"
#include "gfast_core.h"


/*!
 * @brief Initializes tracebuf2 ring reader.
 *
 * @param[in] ewRing     Null terminated Earthworm data ring name.
 * @param[in] msWait     Number of milliseconds to wait after reading
 *                       Earthworm ring.  This will be attached to the
 *                       ringInfo structure.
 *
 * @param[out] ringInfo  Requisite information for reading the tracebuf2's
 *                       off the ewRing Earthworm ring.
 *                         
 * @result 0 indicates success.
 *
 * @author Ben Baker
 *
 * @copyright ISTI distribted under Apache 2.
 *
 */
int dataexchange_earthworm_initialize(const char *ewRing,
                                const int msWait,
                                struct ewRing_struct *ringInfo)
{
    // Check the inputs
    if (ringInfo == NULL)
    {
        LOG_ERRMSG("%s", "ewRing_struct cannot be NULL");
        return -1;
    }
    memset(ringInfo, 0, sizeof(struct ewRing_struct));
    strcpy(ringInfo->ewRingName, ewRing);
    if (ewRing == NULL)
    {
        LOG_ERRMSG("%s", "Error ewRing cannot be NULL");
        return -1;
    }
    if (strlen(ewRing) == 0)
    {
        LOG_ERRMSG("%s", "Error ewRing must be specified");
        return -1;
    }
    ringInfo->msWait = 0;
    if (msWait > 0){ringInfo->msWait = (unsigned int) msWait;}
    // Attach to the ring
    ringInfo->ringKey = GetKey(ringInfo->ewRingName);
    if (ringInfo->ringKey ==-1)
    {
        LOG_ERRMSG("Invalid ring %s", ewRing);
        return -1;
    }
    // Look up installation ID 
    if (GetLocalInst(&ringInfo->instLocalID) != 0)
    {
        LOG_ERRMSG("%s", "Error getting local installation id!");
        return -1;
    }
    if (GetInst("INST_WILDCARD", &ringInfo->instWildcardID ) != 0)
    {
        LOG_ERRMSG("%s", "Error getting INST_WILDCARD!");
        return -1;
    } 
    // Look up the tracebuffer2 type 
    if (GetType("TYPE_TRACEBUF2", &ringInfo->traceBuffer2Type) != 0)
    {
        LOG_ERRMSG("%s", "Error getting TYPE_TRACEBUF2!");
        return -1;
    }
    // Look up the heartbeat type 
    if (GetType("TYPE_HEARTBEAT", &ringInfo->heartBeatType) != 0)
    {   
        LOG_ERRMSG("%s", "Error getting TYPE_HEARTBEAT!");
        return -1; 
    }
    // Look up the error type
    if (GetType("TYPE_ERROR", &ringInfo->errorType) != 0)
    {
        LOG_ERRMSG("%s", "Error getting TYPE_ERROR!");
        return -1;
    }
    if (GetModId( "MOD_WILDCARD", &ringInfo->modWildcardID) != 0 )
    {
        LOG_ERRMSG("%s", "MOD_WILDCARD Missing from earthworm(_global).d");
        return -1;
    }
    // Hook the getLogo's up for reading tracebuffer2s
    ringInfo->nlogo = 1;
    ringInfo->getLogo = (MSG_LOGO *) calloc((size_t) ringInfo->nlogo,
                                            sizeof(MSG_LOGO));
    ringInfo->getLogo[0].type = ringInfo->traceBuffer2Type;
    // Attach to the ring
    tport_attach(&ringInfo->region, ringInfo->ringKey);
    ringInfo->linit = true;
    return 0;
}

/*
int GetEwh( EWH *Ewh )
{
   if ( GetLocalInst( &Ewh->MyInstId ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting MyInstId.\n" );
      return -1;
   }
   if ( GetInst( "INST_WILDCARD", &Ewh->GetThisInstId ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting GetThisInstId.\n" );
      return -2;
   }
   if ( GetModId( "MOD_WILDCARD", &Ewh->GetThisModId ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting GetThisModId.\n" );
      return -3;
   }
   if ( GetType( "TYPE_HEARTBEAT", &Ewh->TypeHeartBeat ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting TypeHeartbeat.\n" );
      return -4;
   }
   if ( GetType( "TYPE_ERROR", &Ewh->TypeError ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting TypeError.\n" );
      return -5;
   }
   if ( GetType( "TYPE_TRACEBUF2", &Ewh->TypeWaveform ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting TYPE_TRACEBUF2.\n" );
      return -6;
   }
   if ( GetType( "TYPE_HYPOTWC", &Ewh->TypeHypoTWC ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting TYPE_HYPOTWC.\n" );
      return -7;
   }
   return 0;
}
*/
