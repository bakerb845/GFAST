#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
/*!
 * @brief Removes an event ID from the active event list in the case that is
 *        was cancelled. 
 *
 * @param[in] evid         Event ID to remove from events list.
 * @param[in] currentTime  Current epochal time (UTC seconds).
 * @param[in] SA           shakeAlert event information to possibly remove.
 * @param[in] verbose      Controls verbosity.  Less than 1 is quiet.
 *
 * @param[in,out] events   On input contains all events. <br> 
 *                         On output the event list sans the SA event.
 *
 * @result If true then the event SA was removed from the events list.
 *
 * @author Ben Baker (ISTI)
 *
 */
bool core_events_removeCancelledEvent(const char *evid,
                                      const double currentTime,
                                      const int verbose,
                                      struct GFAST_shakeAlert_struct SA,
                                      struct GFAST_activeEvents_struct *events)
{
    struct GFAST_activeEvents_struct SAtemp;
    int iev, jev, nev0, pop_indx;
    bool lpopped;
    lpopped = false;
    if (events == NULL)
    {
        if (verbose > 0)
        {
            LOG_WARNMSG("%s", "Warning no events in list");
        }
        return lpopped;         
    }
    // Find the event and see if I can remove it
    pop_indx =-2;
    nev0 = events->nev;
    for (iev=0; iev<nev0; iev++)
    {
        if (strcasecmp(SA.eventid, events->SA[iev].eventid) == 0)
        {
            pop_indx =-1;
            if (strcasecmp(evid, events->SA[iev].eventid) == 0)
            {
                if (verbose > 0)
                {
                    LOG_INFOMSG("Removing %s %f from event list at %f",
                                SA.eventid, SA.time, currentTime);
                }
                pop_indx = iev;
                break;
            }
        }
    }
    // I couldn't even find the event in the event list
    if (pop_indx ==-2)
    {
        if (verbose > 0)
        {
            LOG_WARNMSG("Strangely cannot find this event %s",
                        SA.eventid);
        }
        return lpopped;
    }
    // Remove the event?
    if (pop_indx >-1)
    {
        // Only event - good bye
        if (nev0 == 1)
        {
            core_events_freeEvents(events);
            events->nev = 0;
            return lpopped;
        }
        // Copy old events into workspace ignoring event at pop_indx 
        SAtemp.nev = nev0 - 1;
        SAtemp.SA = (struct GFAST_shakeAlert_struct *)
                    calloc((size_t) SAtemp.nev,
                           sizeof(struct GFAST_shakeAlert_struct));
        jev = 0;
        for (iev=0; iev<nev0; iev++)
        {
            if (iev == pop_indx){continue;}
            memcpy(&SAtemp.SA[jev], &events->SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
            jev = jev + 1;
        }
        memcpy(&SAtemp.SA[nev0], &SA, sizeof(struct GFAST_shakeAlert_struct));
        // Resize events
        core_events_freeEvents(events);
        events->nev = SAtemp.nev;
        events->SA = (struct GFAST_shakeAlert_struct *)
                     calloc((size_t) events->nev,
                            sizeof(struct GFAST_shakeAlert_struct));
        // Copy old events back
        for (iev=0; iev<events->nev; iev++)
        {
            memcpy(&events->SA[iev], &SAtemp.SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
        }
        // Free SAtemp
        core_events_freeEvents(&SAtemp);
    }
    return lpopped;
}
