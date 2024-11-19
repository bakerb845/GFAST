#ifndef GFAST_CORE_H
#define GFAST_CORE_H 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "gfast_struct.h"
#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------------//
//                                  CMT                                       //
//----------------------------------------------------------------------------//
/* Decompose a moment tensor */
int core_cmt_decomposeMomentTensor(const int nmt,
                                   const double *__restrict__ M,
                                   double *__restrict__ DC_pct,
                                   double *__restrict__ Mw, 
                                   double *__restrict__ strike1,
                                   double *__restrict__ strike2,
                                   double *__restrict__ dip1,
                                   double *__restrict__ dip2,
                                   double *__restrict__ rake1,
                                   double *__restrict__ rake2);
/* CMT depth grid search */
int core_cmt_depthGridSearch(const int l1, const int ndeps,
                             const int verbose,
                             const bool deviatoric,
                             const double utmSrcEasting,
                             const double utmSrcNorthing,
                             const double *__restrict__ srcDepths,
                             const double *__restrict__ utmRecvEasting,
                             const double *__restrict__ utmRecvNorthing,
                             const double *__restrict__ staAlt,
                             const double *__restrict__ nObsOffset,
                             const double *__restrict__ eObsOffset,
                             const double *__restrict__ uObsOffset,
                             const double *__restrict__ nWts,
                             const double *__restrict__ eWts,
                             const double *__restrict__ uWts,
                             double *__restrict__ nEst,
                             double *__restrict__ eEst,
                             double *__restrict__ uEst,
                             double *__restrict__ mts);
/* Frees memory on the CMT structures */
void core_cmt_finalizeResults(struct GFAST_cmtResults_struct *cmt);
void core_cmt_finalizeOffsetData(struct GFAST_offsetData_struct *offset_data);
void core_cmt_finalize(struct GFAST_cmt_props_struct *cmt_props,
                       struct GFAST_offsetData_struct *offset_data,
                       struct GFAST_cmtResults_struct *cmt);
/* Full grid search */
int core_cmt_gridSearch(const int l1,
                        const int ndeps, const int nlats, const int nlons,
                        const int verbose,
                        const bool deviatoric,
                        const double *__restrict__ utmSrcEastings,
                        const double *__restrict__ utmSrcNorthings,
                        const double *__restrict__ srcDepths,
                        const double *__restrict__ utmRecvEasting,
                        const double *__restrict__ utmRecvNorthing,
                        const double *__restrict__ staAlt,
                        const double *__restrict__ nObsOffset,
                        const double *__restrict__ eObsOffset,
                        const double *__restrict__ uObsOffset,
                        const double *__restrict__ nWts,
                        const double *__restrict__ eWts,
                        const double *__restrict__ uWts,
                        double *__restrict__ nEst,
                        double *__restrict__ eEst,
                        double *__restrict__ uEst,
                        double *__restrict__ mts);
/* Initialize CMT data structures */
int core_cmt_initialize(struct GFAST_cmt_props_struct props,
                        struct GFAST_data_struct gps_data,
                        struct GFAST_cmtResults_struct *cmt,
                        struct GFAST_offsetData_struct *cmt_data);
/* Reads the initialization parameters from the ini file */
int core_cmt_readIni(const char *propfilename,
                    const char *group,
                    const int verbose, const int utm_zone,
                    struct GFAST_cmt_props_struct *cmt_props);
/* Set diagonal weight matrix in CMT inversion */
int core_cmt_setDiagonalWeightMatrix(const int n,
                                     const double *__restrict__ nWts,
                                     const double *__restrict__ eWts,
                                     const double *__restrict__ uWts,
                                     double *__restrict__ diagWt);
/* Set forward model in CMT inversion */
int core_cmt_setForwardModel(const int l1, const bool ldeviatoric,
                             const double *__restrict__ x1,
                             const double *__restrict__ y1,
                             const double *__restrict__ z1,
                             double *__restrict__ G);
/* Set RHS In CMT inversion */
int core_cmt_setRHS(const int n,
                    const double *__restrict__ nOffset,
                    const double *__restrict__ eOffset,
                    const double *__restrict__ uOffset,
                    double *__restrict__ U);
/* Weight the forward model by the diagonal weight matrix */
int core_cmt_weightForwardModel(const int mrows, const int ncols,
                                const double *__restrict__ diagWt,
                                const double *__restrict__ G,
                                double *__restrict__ diagWtG);
/* Weight the observations by the diagonal weight matrix */
int core_cmt_weightObservations(const int mrows,
                                const double *__restrict__ diagWt,
                                const double *__restrict__ b,
                                double *__restrict__ diagWb);

//----------------------------------------------------------------------------//
//                              coordtools                                    //
//----------------------------------------------------------------------------//
/* Convert lat/lon to UTM */
#ifdef _OPENMP
#pragma omp declare simd
#endif
void core_coordtools_ll2utm(const double lat_deg, const double lon_deg,
                            double *UTMNorthing, double *UTMEasting,
                            bool *lnorthp, int *zone);
/* Convert UTM to lat/lon */
#ifdef _OPENMP
#pragma omp declare simd
#endif
void core_coordtools_utm2ll(const int zone, const bool lnorthp,
                            const double UTMNorthing, const double UTMEasting,
                            double *lat_deg, double *lon_deg);
//----------------------------------------------------------------------------//
//                         GFAST gps data streams                             //
//----------------------------------------------------------------------------//
/* Frees memory on the GPS data buffer */
void core_data_finalize(struct GFAST_data_struct *gps_data);
/* Initializes the GPS metadata */
int core_data_initialize(struct GFAST_props_struct props,
                         struct GFAST_data_struct *gps_data);
/* Reads the metadata file */
int core_data_readMetaDataFile(const char *metaDataFile,
                               char **metaDataNetworks,
                               int n_networks,
                               struct GFAST_data_struct *gps_data);
/* Reads the site mask file */
int core_data_readSiteMaskFile(const char *siteMaskFile,
                               const int verbose,
                               struct GFAST_data_struct *gps_data);

//----------------------------------------------------------------------------//
//                          GFAST event handler                               //
//----------------------------------------------------------------------------//
/* Frees memory on an event structure */
void core_events_freeEvents(struct GFAST_activeEvents_struct *events);
// /* Convenience function to find min origin time in event list */
// double core_events_getMinOriginTime(struct GFAST_props_struct props,
//                                     struct GFAST_activeEvents_struct events,
//                                     bool *lnoEvents);
/* Adds a new event to the event list */
bool core_events_newEvent(struct GFAST_shakeAlert_struct SA, 
                          struct GFAST_activeEvents_struct *events,
                          struct GFAST_activeEvents_xml_status *xml_status);
bool core_events_syncXMLStatusWithEvents(struct GFAST_activeEvents_struct *events,
                                         struct GFAST_activeEvents_xml_status *xml_status);

/* Print the events in the event list */
void core_events_printEvents(struct GFAST_shakeAlert_struct SA);
// /* Remove a cancelled event from the events list */
// bool core_events_removeCancelledEvent(const char *evid,
//                                       const double currentTime,
//                                       const int verbose,
//                                       struct GFAST_shakeAlert_struct SA,
//                                       struct GFAST_activeEvents_struct *events);
/* Look through the events list and remove expired events */
int core_events_removeExpiredEvents(const double maxTime, 
                                    const double currenTime,
                                    const int verbose,
                                    struct GFAST_activeEvents_struct *events);
/* Remove an expired event from the events list */
bool core_events_removeExpiredEvent(const double maxtime,
                                    const double currentTime,
                                    const int verbose,
                                    struct GFAST_shakeAlert_struct SA,
                                    struct GFAST_activeEvents_struct *events);
// /* Potentially add the shakeAlert event to the event list */
// bool core_events_updateEvent(struct GFAST_shakeAlert_struct SA, 
//                              struct GFAST_activeEvents_struct *events,
//                              int *ierr);


//----------------------------------------------------------------------------//
//                              finite fault                                  //
//----------------------------------------------------------------------------//
/* Gridsearch on fault planes */
int core_ff_faultPlaneGridSearch(const int l1, const int l2, 
                                 const int nstr, const int ndip,
                                 const int nfp, const int verbose,
                                 const double *__restrict__ nObsOffset,
                                 const double *__restrict__ eObsOffset,
                                 const double *__restrict__ uObsOffset,
                                 const double *__restrict__ nWts,
                                 const double *__restrict__ eWts,
                                 const double *__restrict__ uWts,
                                 const double *__restrict__ utmRecvEasting,
                                 const double *__restrict__ utmRecvNorthing,
                                 const double *__restrict__ staAlt,
                                 const double *__restrict__ fault_xutm,
                                 const double *__restrict__ fault_yutm,
                                 const double *__restrict__ fault_alt,
                                 const double *__restrict__ length,
                                 const double *__restrict__ width,
                                 const double *__restrict__ strike,
                                 const double *__restrict__ dip,
                                 double *__restrict__ sslip,
                                 double *__restrict__ dslip,
                                 double *__restrict__ Mw, 
                                 double *__restrict__ vr, 
                                 double *__restrict__ NN, 
                                 double *__restrict__ EN, 
                                 double *__restrict__ UN, 
                                 double *__restrict__ sslip_unc,
                                 double *__restrict__ dslip_unc
                                 );
/* Frees finite fault structures */
void core_ff_finalizeFaultPlane(struct GFAST_faultPlane_struct *fp);
void core_ff_finalizeResults(struct GFAST_ffResults_struct *ff);
void core_ff_finalizeOffsetData(struct GFAST_offsetData_struct *offset_data);
void core_ff_finalize(struct GFAST_ff_props_struct *ff_props,
                      struct GFAST_offsetData_struct *ff_data,
                      struct GFAST_ffResults_struct *ff);
/* Initializes finite fault data structures */
int core_ff_initialize(struct GFAST_ff_props_struct props,
                       struct GFAST_data_struct gps_data,
                       struct GFAST_ffResults_struct *ff,
                       struct GFAST_offsetData_struct *ff_data);
/* Meshes the fault plane */
int core_ff_meshFaultPlane(const double ev_lat,
                           const double ev_lon,
                           const double ev_dep,
                           const double flen_pct,
                           const double fwid_pct,
                           const double M,
                           const double strikeF, const double dipF,
                           const int nstr, const int ndip,
                           const int utm_zone, const int verbose,
                           int *__restrict__ fault_ptr,
                           double *__restrict__ lat_vtx,
                           double *__restrict__ lon_vtx,
                           double *__restrict__ dep_vtx,
                           double *__restrict__ fault_xutm,
                           double *__restrict__ fault_yutm,
                           double *__restrict__ fault_alt,
                           double *__restrict__ strike,
                           double *__restrict__ dip,
                           double *__restrict__ length,
                           double *__restrict__ width);
/* Reads the finite fault parameters from the ini file */
int core_ff_readIni(const char *propfilename,
                    const char *group,
                    const int verbose, const int utm_zone,
                    const int cmtMinSites,
                    struct GFAST_ff_props_struct *ff_props);
/* Set the diagonal data weight matrix */
int core_ff_setDiagonalWeightMatrix(const int n,
                                    const double *__restrict__ nWts,
                                    const double *__restrict__ eWts,
                                    const double *__restrict__ uWts,
                                    double *__restrict__ diagWt);
/* Set the (unregularized) forward modeling matrix */
int core_ff_setForwardModel__okadagreenF(const int l1, const int l2, 
                                         const double *__restrict__ e,
                                         const double *__restrict__ n,
                                         const double *__restrict__ depth,
                                         const double *__restrict__ strike,
                                         const double *__restrict__ dip,
                                         const double *__restrict__ W,
                                         const double *__restrict__ L,
                                         double *__restrict__ G);
/* Set the regularization matrix for the finite fault inversion */
int core_ff_setRegularizer(const int l2, const int nstr,
                           const int ndip, const int nt, 
                           const double *__restrict__ width,
                           const double *__restrict__ length,
                           double *__restrict__ T);
/* Set the RHS */
int core_ff_setRHS(const int n,
                   const double *__restrict__ nOffset,
                   const double *__restrict__ eOffset,
                   const double *__restrict__ uOffset,
                   double *__restrict__ U);
/* Weight the forward modeling matrix */
int core_ff_weightForwardModel(const int mrows, const int ncols,
                               const double *__restrict__ diagWt,
                               const double *__restrict__ G,
                               double *__restrict__ diagWtG);
/* Weight the observations */
int core_ff_weightObservations(const int mrows,
                               const double *__restrict__ diagWt,
                               const double *__restrict__ b,
                               double *__restrict__ diagWb);

//----------------------------------------------------------------------------//
//                                 Logging                                    //
//----------------------------------------------------------------------------//

#ifdef ENABLE_PLOG
void init_plog();
void core_log_plog_V(const char *msg);
void core_log_plog_D(const char *msg);
void core_log_plog_I(const char *msg);
void core_log_plog_W(const char *msg);
void core_log_plog_E(const char *msg);
void core_log_plog_F(const char *msg);
void core_log_plog_N(const char *msg);
#endif

#ifndef ERRMSG
#define ERRMSG(msg, fmt, ...)                                                \
{ \
   memset(msg, 0, GFAST_MAXMSG_LEN*sizeof(char));                            \
   sprintf(msg, "[ERROR]: (%s:%s:line=%d) ", __FILE__, __func__, __LINE__ ); \
   do \
   {  \
     snprintf(&msg[strlen(msg)], GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   } while(0) \
};
#endif

#ifdef ENABLE_PLOG
#ifndef LOG_ERRMSG 
#define LOG_ERRMSG(fmt, ...) \
{ \
   char errmsg[GFAST_MAXMSG_LEN]; \
   memset(errmsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   snprintf(errmsg, GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   core_log_plog_E(errmsg); \
};
#endif
#else
#ifndef LOG_ERRMSG 
#define LOG_ERRMSG(fmt, ...) \
{ \
   char errmsg[GFAST_MAXMSG_LEN]; \
   memset(errmsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   sprintf(errmsg, "[ERROR]: (%s:%s:line=%d) ", __FILE__, __func__, __LINE__ );\
   do \
   {  \
     snprintf(&errmsg[strlen(errmsg)], GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   } while(0); \
   core_log_logErrorMessage(errmsg); \
};
#endif
#endif

#ifdef ENABLE_PLOG
#ifndef LOG_WARNMSG 
#define LOG_WARNMSG(fmt, ...) \
{ \
   char warnmsg[GFAST_MAXMSG_LEN]; \
   memset(warnmsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   snprintf(warnmsg, GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   core_log_plog_W(warnmsg); \
};
#endif
#else
#ifndef LOG_WARNMSG 
#define LOG_WARNMSG(fmt, ...) \
{ \
   char warnmsg[GFAST_MAXMSG_LEN]; \
   memset(warnmsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   sprintf(warnmsg, "[WARNING]: (%s:%s:line=%d) ", __FILE__, __func__, __LINE__ ); \
   do \
   {  \
     snprintf(&warnmsg[strlen(warnmsg)], GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   } while(0); \
   core_log_logWarningMessage(warnmsg); \
};
#endif
#endif

#ifdef ENABLE_PLOG
#ifndef LOG_INFOMSG 
#define LOG_INFOMSG(fmt, ...) \
{ \
   char infoMsg[GFAST_MAXMSG_LEN]; \
   memset(infoMsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   snprintf(infoMsg, GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   core_log_plog_I(infoMsg); \
};
#endif
#else
#ifndef LOG_INFOMSG 
#define LOG_INFOMSG(fmt, ...) \
{ \
   char infoMsg[GFAST_MAXMSG_LEN]; \
   memset(infoMsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   sprintf(infoMsg, "[INFO] %s: ", __func__); \
   do \
   {  \
     snprintf(&infoMsg[strlen(infoMsg)], GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   } while(0); \
   core_log_logInfoMessage(infoMsg); \
};
#endif
#endif

#ifdef ENABLE_PLOG
#ifndef LOG_DEBUGMSG 
#define LOG_DEBUGMSG(fmt, ...) \
{ \
   char debugMsg[GFAST_MAXMSG_LEN]; \
   memset(debugMsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   snprintf(debugMsg, GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   core_log_plog_D(debugMsg); \
};
#endif
#else
#ifndef LOG_DEBUGMSG 
#define LOG_DEBUGMSG(fmt, ...) \
{ \
   char debugMsg[GFAST_MAXMSG_LEN]; \
   memset(debugMsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   sprintf(debugMsg, "[DEBUG] %s: ", __func__); \
   do \
   {  \
     snprintf(&debugMsg[strlen(debugMsg)], GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   } while(0); \
   core_log_logDebugMessage(debugMsg); \
};
#endif
#endif

#ifdef ENABLE_PLOG
#ifndef LOG_MSG 
#define LOG_MSG(fmt, ...) \
{ \
   char debugMsg[GFAST_MAXMSG_LEN]; \
   memset(debugMsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   snprintf(debugMsg, GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   core_log_plog_I(debugMsg); \
};
#endif
#else
#ifndef LOG_MSG 
#define LOG_MSG(fmt, ...) \
{ \
   char debugMsg[GFAST_MAXMSG_LEN]; \
   memset(debugMsg, 0, GFAST_MAXMSG_LEN*sizeof(char)); \
   struct tm *gtime; \
   time_t now; \
   time(&now); \
   gtime = gmtime(&now); \
   sprintf(debugMsg, "%4d-%02d-%02d %2d:%02d:%02d [MTH] %s: ",   \
           gtime->tm_year+1900, gtime->tm_mon + 1, gtime->tm_mday,   \
           gtime->tm_hour % 24, gtime->tm_min, gtime->tm_sec, __func__); \
   do \
   {  \
     snprintf(&debugMsg[strlen(debugMsg)], GFAST_MAXMSG_LEN, fmt, __VA_ARGS__); \
   } while(0); \
   core_log_logMessage(debugMsg); \
};
#endif
#endif

int core_log_closeLogs(void);
int core_log_closeErrorLog(void);
int core_log_closeInfoLog(void);
int core_log_closeWarningLog(void);
int core_log_closeDebugLog(void);
int core_log_closeLog(void);

int core_log_createErrorLog(const char *fileName);
int core_log_closeLogs(void);
int core_log_closeErrorLog(void);
int core_log_closeInfoLog(void);
int core_log_closeWarningLog(void);
int core_log_closeDebugLog(void);
int core_log_closeLog(void);

int core_log_createErrorLog(const char *fileName);
int core_log_createInfoLog(const char *fileName);
int core_log_createWarningLog(const char *fileName);
int core_log_createDebugLog(const char *fileName);
int core_log_createLog(const char *fileName);

int core_log_openErrorLog(const char *fileName);
int core_log_openInfoLog(const char *fileName);
int core_log_openWarningLog(const char *fileName);
int core_log_openDebugLog(const char *fileName);
int core_log_openLog(const char *fileName);

void core_log_logErrorMessage(const char *msg);
void core_log_logWarningMessage(const char *msg);
void core_log_logDebugMessage(const char *msg);
void core_log_logInfoMessage(const char *msg);
void core_log_logMessage(const char *msg);
//----------------------------------------------------------------------------//
//                          Properties/initialization                         //
//----------------------------------------------------------------------------//
/* Finalize/free properties */
void core_properties_finalizePGDProperties(
   struct GFAST_pgd_props_struct *pgd_props);
void core_properties_finalizeCMTProperties(
   struct GFAST_cmt_props_struct *cmt_props);
void core_properties_finalizeFFProperties(
    struct GFAST_ff_props_struct *ff_props);
void core_properties_finalizeActiveMQProperties(
    struct GFAST_activeMQ_struct *activeMQ_props);
void core_properties_finalize(struct GFAST_props_struct *props);
/* Initialize the properties from an ini file */
int core_properties_initialize(const char *propfilename,
                               const enum opmode_type opmode,
                               struct GFAST_props_struct *props);
/* Print the properties */
void core_properties_print(struct GFAST_props_struct props);

//----------------------------------------------------------------------------//
//                              PGD scaling                                   //
//----------------------------------------------------------------------------//

/* Depth grid search in PGD scaling */
int core_scaling_pgd_depthGridSearch(const int l1, const int ndeps,
                                     const int verbose,
                                     const double dist_tol,
                                     const double disp_def,
                                     const double utmSrcEasting,
                                     const double utmSrcNorthing,
                                     const double *__restrict__ srcDepths,
                                     const double *__restrict__ utmRecvEasting,
                                     const double *__restrict__ utmRecvNorthing,
                                     const double *__restrict__ staAlt,
                                     const double *__restrict__ d,
                                     const double *__restrict__ wts,
                                     double *__restrict__ srdist,
                                     double *__restrict__ M,
                                     double *__restrict__ VR,
                                     double *__restrict__ iqt75_25,
                                     double *__restrict__ Uest);
/* Finalize the PGD data structures */
void core_scaling_pgd_finalizeData(
     struct GFAST_peakDisplacementData_struct *pgd_data);
void core_scaling_pgd_finalizeResults(
    struct GFAST_pgdResults_struct *pgd);
void core_scaling_pgd_finalize(
    struct GFAST_pgd_props_struct *pgd_props,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    struct GFAST_pgdResults_struct *pgd);
/* Initialize PGD scaling data structures */
int core_scaling_pgd_initialize(struct GFAST_pgd_props_struct pgd_props,
                                struct GFAST_data_struct gps_data,
                                struct GFAST_pgdResults_struct *pgd,
                                struct GFAST_peakDisplacementData_struct *pgd_data);
/* Drives full grid search */
int core_scaling_pgd_gridSearch(const int l1, const int ndeps,
                                const int nlats, const int nlons,
                                const int verbose,
                                const double dist_tol,
                                const double disp_def,
                                const double *__restrict__ utmSrcEastings,
                                const double *__restrict__ utmSrcNorthings,
                                const double *__restrict__ srcDepths,
                                const double *__restrict__ utmRecvEasting,
                                const double *__restrict__ utmRecvNorthing,
                                const double *__restrict__ staAlt,
                                const double *__restrict__ d,
                                const double *__restrict__ wts,
                                double *__restrict__ srdist,
                                double *__restrict__ M,
                                double *__restrict__ VR, 
                                double *__restrict__ iqr,
                                double *__restrict__ Uest);
/* Read ini file for PGD properties */
int core_scaling_pgd_readIni(const char *propfilename,
                             const char *group,
                             const int verbose, const int utm_zone,
                             struct GFAST_pgd_props_struct *pgd_props);
/* Set the diagonal data weighting matrix */
int core_scaling_pgd_setDiagonalWeightMatrix(const int l1, 
                                             const double *__restrict__ repi,
                                             const double *__restrict__ wts,
                                             double *__restrict__ W);
/* Set the forward modeling matrix */
int core_scaling_pgd_setForwardModel(const int n,
                                     const double B, const double C,
                                     const double *__restrict__ r,
                                     double *__restrict__ G);
/* Set the RHS */
int core_scaling_pgd_setRHS(const int n,
                            const double dist_tol,
                            const double dist_def,
                            const double A,
                            const double *__restrict__ d,
                            double *__restrict__ b);

/* MTH: Check dist/amp */
int core_scaling_pgd_sanityChecks(const int n,
                            const double dist_tol,
                            const double dist_def,
                            const double *__restrict__ repi,
                            const double *__restrict__ d);

/* Weight the PGD forward modeling operator */
int core_scaling_pgd_weightForwardModel(const int l1, 
                                        const double *__restrict__ W,
                                        const double *__restrict__ G,
                                        double *__restrict__ WG);
/* Weight the PGD RHS */
int core_scaling_pgd_weightObservations(const int l1, 
                                        const double *__restrict__ W,
                                        const double *__restrict__ b,
                                        double *__restrict__ Wb);
/* Reads the sigma-time-magnitude lookup table */
int core_scaling_readSigmaLookupFile(const char *sigmaLookupFile,
                                     struct GFAST_pgd_props_struct *pgd_props);
/* Reads the pgd thresholds */
int core_scaling_readPgdThresholdLookupFile(const char *pgdThresholdLookupFile,
                                            struct GFAST_pgd_props_struct *pgd_props);
/* Reads the raw sigma threshold file */
int core_scaling_readRawSigmaThresholdLookupFile(const char *rawSigmaThresholdLookupFile,
                                                 struct GFAST_pgd_props_struct *pgd_props);

//----------------------------------------------------------------------------//
//                            Waveform processor                              //
//----------------------------------------------------------------------------//
/* Parse the packed quality channel, return chi^2 value */
double core_waveformProcessor_parseQChannelChi2CWU(
    const double q_value);
/* Parse the packed quality channel, return mapped chi^2 value */
int core_waveformProcessor_parseQChannelChi2CWUmap(
    const double q_value);
/* Parse the packed quality channel, return goodness (0 or 1) */
int core_waveformProcessor_parseQChannelGoodness(
    const double q_value);
/* Compute the offset in north, east, and up */
int core_waveformProcessor_offset(const int utm_zone,
                                  const double svel_window,
                                  const double ev_lat,
                                  const double ev_lon,
                                  const double ev_dep,
                                  const double ev_time,
                                  struct GFAST_data_struct gps_data,
                                  struct GFAST_offsetData_struct *offset_data,
                                  int *ierr);
/* Compute the peak displacement from gps_data*/
int core_waveformProcessor_peakDisplacement(
    const struct GFAST_pgd_props_struct *pgd_props,
    const double ev_lat,
    const double ev_lon,
    const double ev_dep,
    const double ev_time,
    struct GFAST_data_struct gps_data,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    int *ierr);
/* Compute the peak displacement from buffers. Called by peakDisplacement */
double core_waveformProcessor_peakDisplacementHelper(
    const int npts,
    const double dt,
    const double ev_time,
    const double epoch,
    const double *__restrict__ ubuff,
    const double *__restrict__ nbuff,
    const double *__restrict__ ebuff,
    const int nMaxLeader,
    const double tmin,
    const double tmax,
    double *obsTime,
    int *iRef,
    int *iPeak);

#define GFAST_core_cmt_decomposeMomentTensor(...)       \
              core_cmt_decomposeMomentTensor(__VA_ARGS__)
#define GFAST_core_cmt_depthGridSearch(...)       \
              core_cmt_depthGridSearch(__VA_ARGS__)
#define GFAST_core_cmt_finalizeResults(...)       \
              core_cmt_finalizeResults(__VA_ARGS__)
#define GFAST_core_cmt_finalizeOffsetData(...)       \
              core_cmt_finalizeOffsetData(__VA_ARGS__)
#define GFAST_core_cmt_finalize(...)       \
              core_cmt_finalize(__VA_ARGS__)
#define GFAST_core_cmt_initialize(...)       \
              core_cmt_initialize(__VA_ARGS__)
#define GFAST_core_cmt_setDiagonalWeightMatrix(...)       \
              core_cmt_setDiagonalWeightMatrix(__VA_ARGS__)
#define GFAST_core_cmt_setForwardModel(...)       \
              core_cmt_setForwardModel(__VA_ARGS__)
#define GFAST_core_cmt_setRHS(...)       \
              core_cmt_setRHS(__VA_ARGS__)
#define GFAST_core_cmt_setForwardModel__deviatoric(...)       \
              core_cmt_setForwardModel__deviatoric(__VA_ARGS__)
#define GFAST_core_cmt_setRHS(...)       \
              core_cmt_setRHS(__VA_ARGS__)
#define GFAST_core_cmt_weightForwardModel(...)       \
              core_cmt_weightForwardModel(__VA_ARGS__)
#define GFAST_core_cmt_weightObservations(...)       \
              core_cmt_weightObservations(__VA_ARGS__)

#define GFAST_core_coordtools_ll2utm(...) \
              core_coordtools_ll2utm(__VA_ARGS__)
#define GFAST_core_coordtools_utm2ll(...) \
              core_coordtools_utm2ll(__VA_ARGS__)

#define GFAST_core_data_finalize(...)       \
              core_data_finalize(__VA_ARGS__)
#define GFAST_core_data_initialize(...)       \
              core_data_initialize(__VA_ARGS__)
#define GFAST_core_data_readMetaDataFile(...)       \
              core_data_readMetaDataFile(__VA_ARGS__)
#define GFAST_core_data_readSiteMaskFile(...)       \
              core_data_readSiteMaskFile(__VA_ARGS__)
              
#define GFAST_core_events_freeEvents(...)       \
              core_events_freeEvents(__VA_ARGS__)
#define GFAST_core_events_newEvent(...)       \
              core_events_newEvent(__VA_ARGS__)
#define GFAST_core_events_syncXMLStatusWithEvents(...)       \
              core_events_syncXMLStatusWithEvents(__VA_ARGS__)
#define GFAST_core_events_printEvents(...)       \
              core_events_printEvents(__VA_ARGS__)
#define GFAST_core_events_removeExpiredEvent(...)       \
              core_events_removeExpiredEvent(__VA_ARGS__)
// #define GFAST_core_events_getMinOriginTime(...)       
//               core_events_getMinOriginTime(__VA_ARGS__)
// #define GFAST_core_events_removeCancelledEvent(...)       
//               core_events_removeCancelledEvent(__VA_ARGS__)
// #define GFAST_core_events_updateEvent(...)       
//               core_events_updateEvent(__VA_ARGS__)

#define GFAST_core_ff_faultPlaneGridSearch(...)       \
              core_ff_faultPlaneGridSearch(__VA_ARGS__)
#define GFAST_core_ff_finalizeResults(...)       \
              core_ff_finalizeResults(__VA_ARGS__)
#define GFAST_core_ff_finalizeFaultPlane(...)       \
              core_ff_finalizeFaultPlane(__VA_ARGS__)
#define GFAST_core_ff_finalizeOffsetData(...)       \
              core_ff_finalizeOffsetData(__VA_ARGS__)
#define GFAST_core_ff_finalize(...)       \
              core_ff_finalize(__VA_ARGS__)
#define GFAST_core_ff_initialize(...)       \
              core_ff_initialize(__VA_ARGS__)
#define GFAST_core_scaling_pgd_finalize__props(...)       \
              core_scaling_pgd_finalize__props(__VA_ARGS__)
#define GFAST_core_scaling_pgd_finalizeData(...)       \
              core_scaling_pgd_finalizeData(__VA_ARGS__)
#define GFAST_core_scaling_pgd_finalizeResults(...)       \
              core_scaling_pgd_finalizeResults(__VA_ARGS__)
#define GFAST_core_scaling_pgd_finalize(...)       \
              core_scaling_pgd_finalize(__VA_ARGS__)
#define GFAST_core_ff_meshFaultPlane(...)       \
              core_ff_meshFaultPlane(__VA_ARGS__)
#define GFAST_core_ff_setDiagonalWeightMatrix(...)       \
              core_ff_setDiagonalWeightMatrix(__VA_ARGS__)
#define GFAST_core_ff_setForwardModel__okadagreenF(...)       \
              core_ff_setForwardModel__okadagreenF(__VA_ARGS__)
#define GFAST_core_ff_setRegularizer(...)       \
              core_ff_setRegularizer(__VA_ARGS__)
#define GFAST_core_ff_setRHS(...)       \
              core_ff_setRHS(__VA_ARGS__)
#define GFAST_core_ff_weightForwardModel(...)       \
              core_ff_weightForwardModel(__VA_ARGS__)
#define GFAST_core_ff_weightObservations(...)       \
              core_ff_weightObservations(__VA_ARGS__)

#define GFAST_core_properties_finalizePGDProperties(...)       \
              core_properties_finalizePGDProperties(__VA_ARGS__)
#define GFAST_core_properties_finalizeCMTProperties(...)       \
              core_properties_finalizeCMTProperties(__VA_ARGS__)
#define GFAST_core_properties_finalizeFFProperties(...)       \
              core_properties_finalizeFFProperties(__VA_ARGS__) 
#define GFAST_core_properties_finalizeActiveMQProperties(...)       \
              core_properties_finalizeActiveMQProperties(__VA_ARGS__)
#define GFAST_core_properties_finalize(...)       \
              core_properties_finalize(__VA_ARGS__)
#define GFAST_core_properties_initialize(...)       \
              core_properties_initialize(__VA_ARGS__)
#define GFAST_core_properties_print(...)       \
              core_properties_print(__VA_ARGS__)

#define GFAST_core_scaling_pgd_depthGridSearch(...)       \
              core_scaling_pgd_depthGridSearch(__VA_ARGS__)
#define GFAST_core_scaling_pgd_setDiagonalWeightMatrix(...)       \
              core_scaling_pgd_setDiagonalWeightMatrix(__VA_ARGS__)
#define GFAST_core_scaling_pgd_initialize(...)       \
              core_scaling_pgd_initialize(__VA_ARGS__)
#define GFAST_core_scaling_pgd_setForwardModel(...)       \
              core_scaling_pgd_setForwardModel(__VA_ARGS__)
#define GFAST_core_scaling_pgd_setRHS(...)       \
              core_scaling_pgd_setRHS(__VA_ARGS__)
#define GFAST_core_scaling_pgd_sanityChecks(...)       \
              core_scaling_pgd_sanityChecks(__VA_ARGS__)
#define GFAST_core_scaling_pgd_weightForwardModel(...)       \
              core_scaling_pgd_weightForwardModel(__VA_ARGS__)
#define GFAST_core_scaling_pgd_weightObservations(...)       \
              core_scaling_pgd_weightObservations(__VA_ARGS__)

#define GFAST_core_waveformProcessor_offset(...)       \
              core_waveformProcessor_offset(__VA_ARGS__)
#define GFAST_core_waveformProcessor_peakDisplacement(...)       \
              core_waveformProcessor_peakDisplacement(__VA_ARGS__)
#define GFAST_core_waveformProcessor_peakDisplacementHelper(...)       \
              core_waveformProcessor_peakDisplacementHelper(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* _gfast_core_h__ */
