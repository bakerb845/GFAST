#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_events.h"
/*!
 * @brief Frees memory on the active events structure
 *
 * @param[inout] events     active event list with data to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void events_freeEvents(struct GFAST_activeEvents_struct *events)
{
    if (events->nev > 0)
    {
        free(events->SA);
    }
    memset(events, 0, sizeof(struct GFAST_activeEvents_struct));
    return;
}
