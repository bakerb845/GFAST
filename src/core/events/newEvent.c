#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast_core.h"
#include <math.h>
/*!
 * @brief Adds a new event to the working events list.
 *
 * @param[in] SA             This is the shakeAlert event whose existence is
 *                           checked for in events struture.  If it does not
 *                           exist in events then it is added. 
 * 
 * @param[in,out] events     On input holds the current active list of events. <br>
 *                           On output holds the new event in SA provided that 
 *                           SA does not already exist.
 * @param[in,out] xml_status On input holds the current active list of GFAST events
 *                           and version numbers. On output holds the new event
 *
 * @result If true then the new event in SA has been added to the events list.
 *
 * @author Ben Baker, ISTI
 *
 */
bool core_events_newEvent(struct GFAST_shakeAlert_struct SA,
                          struct GFAST_activeEvents_struct *events,
                          struct GFAST_activeEvents_xml_status *xml_status)
{
  struct GFAST_activeEvents_struct SAtemp;

  struct GFAST_xml_output_status output_status;
  struct GFAST_activeEvents_xml_status Xtemp;

  memset(&output_status, 0, sizeof( struct GFAST_xml_output_status));

  int iev, nev0, imodified;
  bool lnewEvent = true;
  bool lupdateEvent = false;

  // New event -> copy and update
  nev0 = events->nev;

  for (iev=0; iev<nev0; iev++)
    {
      if (strcasecmp(SA.eventid, events->SA[iev].eventid) == 0)
        {
	  lnewEvent = false;
	  if (fabs(SA.lat  - events->SA[iev].lat)  > 1.e-10 ||
	      fabs(SA.lon  - events->SA[iev].lon)  > 1.e-10 ||
	      fabs(SA.dep  - events->SA[iev].dep)  > 1.e-10 ||
	      fabs(SA.mag  - events->SA[iev].mag)  > 1.e-10 ||
	      fabs(SA.time - events->SA[iev].time) > 1.e-5)
	    {
	      lupdateEvent = true;
	      imodified = iev;
	      LOG_MSG("MTH: SA.eventid=%s xml has changed NEW OT:%.3f lat:%6.3f lon:%6.3f dep:%.2f mag:%.2f",
		      SA.eventid, SA.time, SA.lat, SA.lon, SA.dep, SA.mag);
	      LOG_MSG("MTH: SA.eventid=%s xml has changed OLD OT:%.3f lat:%6.3f lon:%6.3f dep:%.2f mag:%.2f",
		      events->SA[iev].eventid, events->SA[iev].time, events->SA[iev].lat, 
		      events->SA[iev].lon, events->SA[iev].dep, events->SA[iev].mag);
	    }
	  break;
        }
    }

  if (lnewEvent)
    {
      // Copy new event into workspace
      SAtemp.nev = nev0 + 1;
      SAtemp.SA = (struct GFAST_shakeAlert_struct *)
	calloc((size_t) SAtemp.nev,
	       sizeof(struct GFAST_shakeAlert_struct));

      //LOG_MSG("MTH: nev0=%d --> Copy new event into workspace", nev0);
      for (iev=0; iev<nev0; iev++)
        {
	  memcpy(&SAtemp.SA[iev], &events->SA[iev], sizeof(struct GFAST_shakeAlert_struct));
        }
      //LOG_MSG("MTH: nev0=%d --> Copy SA to SAtemp.SA", nev0);
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

      //LOG_MSG("MTH: nev0=%d --> Copy new event into workspace DONE", nev0);
      // MTH: Same thing for xml output status structs
      // Create a status output_status from incoming SA eventid
      strcpy(output_status.eventid, SA.eventid);
      output_status.internal_version = 0;  /*will decrement to -1 on first xml update*/
      output_status.external_version = -1;  /*will increment to 0 on first xml update*/
      memset(&output_status.last_sent_core, 0, sizeof(struct coreInfo_struct));

      // Append this output_status to the incoming xml_status list of records
      Xtemp.nev = nev0 + 1;
      Xtemp.SA_status = (struct GFAST_xml_output_status *) calloc((size_t) Xtemp.nev, sizeof(struct GFAST_xml_output_status));
      for (iev=0; iev<nev0; iev++){
	memcpy(&Xtemp.SA_status[iev], &xml_status->SA_status[iev], sizeof(struct GFAST_xml_output_status));
      }
      memcpy(&Xtemp.SA_status[nev0], &output_status, sizeof(struct GFAST_xml_output_status));
      // Free xml_status and copy Xtemp back in
      if (xml_status->nev > 0) {
	free(xml_status->SA_status);
	//memset(xml_status, 0, sizeof(struct GFAST_activeEvents_xml_status));
      }
      /*
        else {  // xml_status is empty
	memset(xml_status, 0, sizeof(struct GFAST_activeEvents_xml_status));
	xml_status->SA_status = (struct GFAST_xml_output_status *) calloc((size_t) 1, sizeof(struct GFAST_xml_output_status));
        }
      */
      xml_status->nev = Xtemp.nev;
      xml_status->SA_status = (struct GFAST_xml_output_status *) calloc((size_t) xml_status->nev,
									sizeof(struct GFAST_xml_output_status));

      //LOG_MSG("MTH: SA.eventid=%s --> Copy Xtemp back to xml_status nev=%d", SA.eventid, xml_status->nev);
      // Copy new list back to xml_status:
      for (iev=0; iev<xml_status->nev; iev++){
	memcpy(&xml_status->SA_status[iev], &Xtemp.SA_status[iev], sizeof(struct GFAST_xml_output_status));
      }

      // Free Xtemp
      free(Xtemp.SA_status);
    }
  else {
    if (lupdateEvent){
      //LOG_MSG("MTH: SA.eventid=%s xml has changed --> update events->SA[%d]", SA.eventid, imodified);
      memcpy(&events->SA[imodified], &SA, sizeof(struct GFAST_shakeAlert_struct));
    }
  }
  return lnewEvent;
}
