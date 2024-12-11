#ifndef _gfast__h_
#define _gfast__h_ 1
#include <stdbool.h>
#include "gfast_config.h"
#ifdef  GFAST_USE_AMQ
#include "gfast_activeMQ.h"
#endif
#include "gfast_core.h"
#include "gfast_eewUtils.h"
#include "gfast_enum.h"
#include "gfast_hdf5.h"
#include "gfast_struct.h"
#include "gfast_traceBuffer.h"
#include "gfast_xml.h"
#include "gfast_dataexchange.h"

#ifdef __cplusplus
extern "C"
{
#endif
/* Buffer initialization */
int GFAST_buffer__getNumberOfStreams(struct GFAST_props_struct props);
int GFAST_buffer__setBufferSpace(struct GFAST_props_struct props,
                                struct GFAST_data_struct *gps_data);
int GFAST_buffer__setSitesAndLocations(struct GFAST_props_struct props,
                                       struct GFAST_data_struct *gps_data);
int GFAST_buffer__setSiteSamplingPeriod(struct GFAST_props_struct props,
                                        struct GFAST_data_struct *gps_data);
int GFAST_buffer__readDataFromSAC(int job,
                                  struct GFAST_props_struct props,
                                  struct GFAST_data_struct *gps_data);
void GFAST_buffer_print__samplingPeriod(struct GFAST_data_struct gps_data);
void GFAST_buffer_print__locations(struct GFAST_data_struct gps_data);
void GFAST_buffer__setInitialTime(double epoch0,
                                  struct GFAST_data_struct *gps_data);
/* Frees memory on pointers and data structures */
void GFAST_memory_freeWaveformData(struct GFAST_waveform3CData_struct *data);
void GFAST_memory_freeData(struct GFAST_data_struct *gps_data);
void GFAST_memory_freeProps(struct GFAST_props_struct *props);
void GFAST_memory_freeEvents(struct GFAST_activeEvents_struct *events);
void GFAST_memory_freePGDResults(struct GFAST_pgdResults_struct *pgd);
void GFAST_memory_freeCMTResults(struct GFAST_cmtResults_struct *cmt);
void GFAST_memory_freeFaultPlane(struct GFAST_faultPlane_struct *fp);
void GFAST_memory_freeFFResults(struct GFAST_ffResults_struct *ff);
void GFAST_memory_freePGDData(
     struct GFAST_peakDisplacementData_struct *pgd_data);
void GFAST_memory_freeOffsetData(struct GFAST_offsetData_struct *offset_data);
/* Initializes the GFAST parameters */
int GFAST_properties_initialize(const char *propfilename,
                                const enum opmode_type opmode,
                                struct GFAST_props_struct *props);
void GFAST_properties_print(struct GFAST_props_struct props);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef _gfast__h_ */
