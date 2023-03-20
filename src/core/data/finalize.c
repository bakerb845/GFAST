#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Frees the GPS data structure
 *
 * @param[out] gps_data    GPS data structure to be freed/reset
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_data_finalize(struct GFAST_data_struct *gps_data)
{
    int k;
    if (gps_data->data != NULL)
    {
       for (k=0; k<gps_data->stream_length; k++)
       {
           memory_free64f(&gps_data->data[k].ubuff);
           memory_free64f(&gps_data->data[k].nbuff);
           memory_free64f(&gps_data->data[k].ebuff);
           memory_free64f(&gps_data->data[k].tbuff);
       }
       free(gps_data->data);
    }
    memset(gps_data, 0, sizeof(struct GFAST_data_struct)); 
    return;
}
