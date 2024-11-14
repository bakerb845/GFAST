#ifndef GFAST_STRUCT_H
#define GFAST_STRUCT_H 1
#if defined WINNT || defined WIN32 || defined WIN64
#include <windows.h>
#include <limits.h>
#else
#include <linux/limits.h>
#endif
#include <stdbool.h>
#include "gfast_enum.h"
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include "gfast_config.h"


struct GFAST_pgd_props_struct
{
  bool do_pgd;          /*!< Should PGD be calculated? */
  double window_vel;    /*!< Velocity (km/s) used in determining if enough
			  data has arrived at a station in PGD
			  inversion. */
  double min_window_vel; /*!< min S vel to use for tmax=dist/min_s_vel PGD peak search*/
  double dist_tol;      /*!< Source-receiver distance tolerance (km). */
  double disp_def;      /*!< If the source receiver epicentral distance
			  is less than dist_tol this is the value assigned
			  to the PGD observation (cm). */
  double dLat;          /*!< Latitude perturbation (degrees) in epicentral
			  grid search. */
  double dLon;          /*!< Longitude perturbation (degrees) in epicentral
			  grid search. */
  int min_sites;        /*!< Minimum number of sites required to
			  proceed with PGD inversion. */
  int verbose;          /*!< Controls verbosity - errors will always
			  be output. <br> 
			  = 1 -> Output generic information. <br> 
			  = 2 -> Output generic information and
			  warnings. <br> 
			  = 3 -> Output generic information,
			  warnings, and debug information
			  and debug information. */
  int utm_zone;         /*!< UTM zone.  If this is -12345 then will 
			  extract the UTM zone from the event
			  origin. */
  int ngridSearch_lats; /*!< Number of latitudes in epicentral grid-search. */
  int ngridSearch_lons; /*!< Number of longitudes in epicentral grid-search.*/
  int ngridSearch_deps; /*!< Number of depths in PGD grid-search. */
  double t99[MAX_SIGMA_LOOKUP_VALUES];/*!< Time after origin (s) from lookup table [n99] */
  double m99[MAX_SIGMA_LOOKUP_VALUES];/*!< Magnitude from lookup table [n99] */
  int n99;              /*!< Number of t99, m99 values */
  int throttle_num_stations[MAX_THROTTLING_THRESHOLDS]; /*!< Threshold number of stations to send a message */
  double throttle_pgd_threshold[MAX_THROTTLING_THRESHOLDS]; /*!< Threshold pgd value (cm) */
  double throttle_time_threshold[MAX_THROTTLING_THRESHOLDS]; /*!< Threshold time value (s) */
  int n_throttle;             /*!< number of throttle criteria. num_stations, pgd_threshold,
                                time_threshold should be the same length */
  double u_raw_sigma_threshold; /*!< Maximum raw sigma (up) to allow in pd calculations (cm) */
  double n_raw_sigma_threshold; /*!< Maximum raw sigma (east) to allow in pd calculations (cm) */
  double e_raw_sigma_threshold; /*!< Maximum raw sigma (north) to allow in pd calculations (cm) */
  int q_value_threshold; /*!< Don't use pd observations for a station if q value is below threshold */
  double pgd_sigma_throttle; /*!< Maximum pgd magnitude uncertainty to allow in sending xml */
  double SA_mag_threshold;    /*!< Magnitude threshold above which to send xml messages */
  double minimum_pgd_cm; /*!< Minimum value to include a pgd value in inversion (cm) */
  double maximum_pgd_cm; /*!< Maximum value to include a pgd value in inversion (cm) */
  int max_assoc_stations; /*!< Maximum stations to add the 'assoc' tag to in xml */
  double change_threshold_mag; /*!< Minimum magnitude change threshold */
  double change_threshold_mag_uncer; /*!< Minimum magnitude uncertainty change threshold */
  double change_threshold_lat; /*!< Minimum latitude change threshold */
  double change_threshold_lon; /*!< Minimum longitude change threshold */
  double change_threshold_orig_time; /*!< Minimum origin time change threshold */
  int change_threshold_num_stations; /*!< Minimum number of stations change threshold */
};

struct GFAST_cmt_props_struct
{
  bool do_cmt;          /*!< Should CMT be calculated? */
  double window_vel;    /*!< Velocity (km/s) used in determining if S wave
			  has passed through the station in CMT
			  inversion. */
  double window_avg;    /*!< Amount of time (s) required after S wave
			  has passed through for averaging
			  the offset. */
  double dLat;          /*!< Latitude perturbation (degrees) in epicentral
			  grid search. */
  double dLon;          /*!< Longitude perturbation (degrees) in epicentral
			  grid search. */
  int min_sites;        /*!< Minimum number of sites required to
			  proceed with CMT inversion. */
  int verbose;          /*!< Controls verbosity - errors will always
			  be output. <br>
			  = 1 -> Output generic information. \n
			  = 2 -> Output generic information and
			  warnings. \n
			  = 3 -> Output generic information,
			  warnings, and debug information
			  and debug information. */
  int utm_zone;         /*!< UTM zone.  If this is -12345 then will 
			  extract the UTM zone from the event
			  origin. */
  int ngridSearch_lats; /*!< Number of latitudes in epicenter grid-search. */
  int ngridSearch_lons; /*!< Number of longitudes in epicenter grid-search. */
  int ngridSearch_deps; /*!< Number of depths in CMT grid-search. */
  bool ldeviatoric;     /*!< If true then the CMT inversion is 
			  constrained to purely deviatoric sources.
			  Otherwise, all 6 moment tensor terms
			  are inverted for. */
};

struct GFAST_ff_props_struct
{
  bool do_ff;          /*!< Should FF be calculated? */
  double window_vel;   /*!< Velocity (km/s) used in determining if S wave
			 has passed through the station in FF 
			 inversion. */
  double window_avg;   /*!< Amount of time (s) required after S wave
			 has passed through for averaging
			 the offset. */
  double flen_pct;     /*!< Fault length safety factor. */
  double fwid_pct;     /*!< Fault width safety factor. */
  int verbose;         /*!< Controls verbosity - errors will always
			 be output. \n
			 = 1 -> Output generic information. \n
			 = 2 -> Output generic information and
			 warnings. \n
			 = 3 -> Output generic information,
			 warnings, and debug information
			 and debug information. */
  int utm_zone;        /*!< UTM zone.  If this is -12345 then will 
			 extract the UTM zone from the event
			 origin. */
  int min_sites;       /*!< Minimum number of sites to proceed with
			 FF estimation. */
  int nstr;            /*!< Number of fault patches along strike. */
  int ndip;            /*!< Number of fault patches down dip. */
  int nfp;             /*!< Number of fault planes considered in
			 inversion (should be 2). */
};

struct GFAST_activeMQ_struct
{
  char user[512];             /*!< Username for ActiveMQ host computer. */
  char password[512];         /*!< Password to ActiveMQ host computer. */
  char originURL[512];        /*!< EEW ActiveMQ URL for reading event trigger messages */
  char originTopic[512];      /*!< This is the topic (ActiveMQ destination)
				that GFAST reads event trigger messages from*/
  char destinationURL[512];   /*!< EEW ActiveMQ URL for writing algorithm output to */
  char destinationTopic[512]; /*!< This is the topic (ActiveMQ destination)
				that GFAST will send messages to. */
  char hbTopic[512];          /*!< This is the topic (ActiveMQ destination)
				that GFAST will send heartbeats to. */
  int hbInterval;             /*!< interval between heartbeats in seconds */
  int msReconnect;            /*!< milliseconds to wait before reconnect
				(default is 500). */
  int maxAttempts;            /*!< Max number of attempts before declaring
				inability to connect to the host
				(default is 5). */
  int msWaitForMessage;       /*!< Milliseconds to wait for a message
				(default is 0). */
  unsigned int maxMessages;   /*!< Maximum messages in consumer FIFO buffer 
				(default is 5). */
  double msg_ttl;             /*!< Message time to live (seconds) (default is 0) */
};

struct GFAST_ew_struct
{
  char gpsRingName[256];      /*!< Name of earthworm ring with GPS data. */
  char moduleName[256];       /*!< Earthworm module name. */ 
};

struct GFAST_props_struct
{
  struct GFAST_pgd_props_struct pgd_props;  /*!< PGD properties structure. */
  struct GFAST_cmt_props_struct cmt_props;  /*!< CMT properties structure. */
  struct GFAST_ff_props_struct ff_props;    /*!< FF properties structure. */
  struct GFAST_activeMQ_struct activeMQ_props; /*!< ShakeAlert ActiveMQ properties */
  struct GFAST_ew_struct ew_props;          /*!< Earthworm properties. */
  char metaDataFile[PATH_MAX]; /*!< Contains the GPS metadata file 
				 which defines the sites, locations,
				 sampling periods, etc. to be used
				 by GFAST. */
  char *metaDataNetworks[16]; /*!< Networks that are allowed to be used */
  int n_networks; /*!< Number of networks to use. If 0, use any available */
  char SAeventsDir[PATH_MAX]; /*!< Location of dir containing SA events 
				SA.xml to trigger on */
  char SAoutputDir[PATH_MAX]; /*!< Location of dir where SA XML files will be written
				at output_interval_mins minutes */
  char siteMaskFile[PATH_MAX]; /*!< Contains a list of sites to mask from
				 the inversions.  If not specified then
				 all sites will be read. */
  char eewsfile[PATH_MAX];     /*!< In playback mode this XML decision
				 module style module has the event
				 hypocenter and origin time. */ 
  char obsdataDir[PATH_MAX];   /*!< Observed data file directory. */
  char obsdataFile[PATH_MAX];  /*!< Observed (archived) data file. */
  char h5ArchiveDir[PATH_MAX]; /*!< HDF5 archive directory. */ 
  char propfilename[PATH_MAX]; /*!< Name of GFAST properties file. */
  char anssNetwork[512];       /*!< ANSS network when writing quakeML
				 (e.g. UW, PM, etc.). */
  char anssDomain[512];        /*!< ANSS domain when writing quake ML
				 (e.g. anss.org, tsunami.gov, etc) */
  double dt_default;          /*!< Default sampling period (s) for GPS
				stations. */
  double bufflen;             /*!< The number of seconds to keep in the data
				buffers. */
  double processingTime;      /*!< Max processing time (s) after origin time
				which GFAST module will declare an event 
				done. This must be less than bufflen
				less than or equal to the synthetic
				runtime. */
  double eqDefaultDepth;      /*!< Default earthquake depth (km) to be applied
				to shakeAlert structure */
  double synthetic_runtime;   /*!< Simulation runtime (s) for offline mode. */
  double waitTime;            /*!< Number of seconds to wait before running
				another iteration of the realtime code. */
  int n_intervals;
  int output_interval_mins[16];   /*!< Intervals (mins) at which to output event solution XMLs */
  int utm_zone;               /*!< UTM zone.  If this is -12345 then will 
				extract the UTM zone from the event
				origin. */
  int verbose;                /*!< Controls verbosity - errors will always
				be output. \n
				= 1 -> Output generic information. \n
				= 2 -> Output generic information and
				warnings. \n
				= 3 -> Output generic information,
				warnings, and debug information
				and debug information. */
  bool lh5SummaryOnly;        /*!< If true then only the HDF5 summary
				will be written. */
  enum opmode_type opmode;    /*!< GFAST operation mode (realtime, 
				playback, offline). */
  enum dtinit_type dt_init;   /*!< Defines how to initialize GPS sampling
				period. */
  enum locinit_type loc_init; /*!< Defines how to initialize GPS locations. */
};


struct GFAST_faultPlane_struct
{
  double *lon_vtx;    /*!< For plotting this defines the longitude of each
			vertex (degrees) on each fault patch.  The ifp'th
			fault patch is accessed by 4*(idip*nstr + istr).
			The size is [4 x nstr x ndip] */
  double *lat_vtx;    /*!< For plotting this defines the latitude of each
			vertex (degrees) on each fault patch.  The ifp'th
			fault patch is accessed by 4*(idip*nstr + istr).
			The size is [4 x nstr x ndip] */
  double *dep_vtx;    /*!< For plotting this defines the depth of each
			vertex (km) on each fault patch.  The ifp'th
			fault patch is accessed by 4*(idip*nstr + istr).
			The size is [4 x nstr x ndip] */
  double *fault_xutm; /*!< Fault easting UTM (m).  The size is
			[nstr x ndip] with leading dimension nstr */
  double *fault_yutm; /*!< Fault northing UTM (m).  The size is
			[nstr x ndip] with leading dimension nstr */
  double *fault_alt;  /*!< Depth at fault patch (km) The size is 
			[nstr x ndip] with leading dimension nstr */
  double *strike;     /*!< Strike angles of fault patches (degrees)
			[nstr x ndip] with leading dimension nstr */
  double *dip;        /*!< Dip angles of fault patches (degrees).  The size
			is [nstr x ndip] with leading dimension nstr */
  double *length;     /*!< Length of fault patches (m).  The size is
			[nstr x ndip] with leading dimension nstr */
  double *width;      /*!< Width of fault patches (m).  The size is 
			[nstr x ndip] with leading dimension nstr */
  double *sslip;      /*!< Strike-slip along each fault patch.  The size
			is [nstr x ndip] with leading dimension nstr */
  double *dslip;      /*!< Dip-slip along each fault patch.  The size
			is [nstr x ndip] with leading dimension nstr */
  double *sslip_unc;  /*!< Uncertainty in strike-slip on each fault patch.
			The size is [nstr x ndip] with leading dimension
			nstr. */
  double *dslip_unc;  /*!< Uncertainty in dip slip on each fault patch.
			The size is [nstr x ndip] with leading dimension
			nstr. */
  double *EN;         /*!< Modeled east displacements [nsites_used] */
  double *NN;         /*!< Modeled north displacements [nsites_used] */
  double *UN;         /*!< Modeled vertical displacements [nsites_used] */
  double *Einp;       /*!< Observed input east displacements [nsites_used] */
  double *Ninp;       /*!< Observed input north displacements [nsites_used] */
  double *Uinp;       /*!< Observed input vertical displacements
			[nsites_used] */
  int *fault_ptr;     /*!< Maps from the ifp'th fault patch to the 
			start index of in (lon_vtx, lat_vtx, dep_vtx)
			[nstr*ndip + 1] */
  int maxobs;         /*!< Max number of allowable observations */
  int nsites_used;    /*!< Number of sites used in inversion */
  int nstr;           /*!< Number of fault patches along strike */
  int ndip;           /*!< Number of fault patches down dip */
};

struct GFAST_ffResults_struct
{
  struct GFAST_faultPlane_struct *fp;   /*!< Fault planes [nfp] */
  double *vr;                           /*!< Variance reduction on ifp'th
					  fault plane [nfp] */
  double *Mw;                           /*!< Moment magnitude on ifp'th
					  fault plane [nfp] */
  double *str;                          /*!< Strike of ifp'th fault
					  (degrees) [nfp] */
  double *dip;                          /*!< Dip on ifp'th fault plane 
					  (degrees) [nfp] */
  double SA_lat;                        /*!< Source latitude (degrees) [-90,90] */
  double SA_lon;                        /*!< Source longitude (degrees) [-180,360) */
  double SA_dep;                        /*!< Source depth (km) */
  double SA_mag;                        /*!< Source magnitude (Mw) */
  int preferred_fault_plane;            /*!< Preferred fault plane */
  int nfp;                              /*!< Number of fault planes */
};

struct GFAST_cmtResults_struct
{
  double *l2;        /*!< L_2 objective function at all depths [ndeps] */
  double *pct_dc;    /*!< Percent double couple at each depth [ndeps] */
  double *objfn;     /*!< Objective function at all depths = l2/pct_dc */
  double *mts;       /*!< Holds the NED moment tensor terms packed
		       \f$ \{m_{xx}, m_{yy}, m_{zz},
		       m_{xy}, m_{xz}, m_{yz} \} \f$.  [6*ndeps] */
  double *str1;      /*!< Strike (degrees) on first nodal plane for
		       all depths [ndeps]. \f$ \phi \in [0,360] \f$ */
  double *str2;      /*!< Strike (degrees) on second nodal plane for
		       all depths [ndeps]. \f$ \phi \in [0,360] \f$ */
  double *dip1;      /*!< Dip (degrees) on first nodal plane for
		       all depths [ndeps]. \f$ \delta \in [0,90] \f$ */
  double *dip2;      /*!< Dip (degrees) on second nodal plane for
		       all depths [ndeps]. \f$ \delta \in [0,90] \f$ */
  double *rak1;      /*!< Rake (degrees) on first nodal plane for
		       all depths [ndeps].
		       \f$ \lambda \in [-180,180] \f$ */
  double *rak2;      /*!< Rake (degrees) on second nodal plane for 
		       all depths [ndeps].
		       \f$ \lambda \in [-180,180] \f$ */
  double *Mw;        /*!< Moment magnitude for all depths [ndeps] */
  double *srcDepths; /*!< Source depths in moment tensor inversion grid
		       search (km) [ndeps] */
  double *EN;        /*!< Estimates on east component. The estimate
		       for the i'th site at the j'th depth is accessed by
		       j*sites+i [ndeps*nsites] */
  double *NN;        /*!< Estimates on north component.  The estimate
		       for the i'th site at the j'th depth is accessed by 
		       j*nsites+i [ndeps*nsites] */
  double *UN;        /*!< Estimates on vertical component.  The estimate
		       for the i'th site at the j'th depth is accessed by
		       j*nsites+i [ndeps*nsites] */
  double *Einp;      /*!< Observed input east displacements [nsites] */
  double *Ninp;      /*!< Observed input north displacements [nsites] */
  double *Uinp;      /*!< Observed input vertical displacements [nsites] */
  bool *lsiteUsed;   /*!< If true then the isite'th site from the
		       site list was used in the CMT estimation [nsite] */ 
  int opt_indx;      /*!< Optimal index in depth grid search [0, ndeps) */
  int nlats;         /*!< Number of latitudes in grid search */
  int nlons;         /*!< Number of longitudes in grid serach */
  int ndeps;         /*!< Number of depths in grid search */
  int nsites;        /*!< Should equal GFAST_data_struct's stream_length */
};

struct GFAST_pgdResults_struct
{
  double *mpgd;      /*!< PGD magnitude at id'th depth [ndeps] */
  double *mpgd_sigma;/*!< PGD magnitude uncertainty at id'th depth [ndeps] */
  double *mpgd_vr;   /*!< PGD variance reduction at id'th depth [ndeps] */
  double *dep_vr_pgd;/*!< PGD variance reduction at id'th depth 
		       normalized by the interquartile range [ndeps] */
  double *UP;        /*!< PGD estimates for each source depth
		       [nsites*ndeps] */
  double *UPinp;     /*!< PGD observations for each site [nsites] */
  double *srcLats;   /*!< Source latitudes in grid search (degrees) [nlats] */
  double *srcLons;   /*!< Source longitudes in grid search (degrees) [nlons] */
  double *srcDepths; /*!< PGD source depths in grid search (km) [ndeps] */
  double *srdist;    /*!< Source receiver distance (km) [ndeps*nsites] */
  double *iqr;       /*!< interquartile range (75 - 25) of the weighted
		       residuals at each depth [ndeps] */
  bool *lsiteUsed;   /*!< If true then the isite'th site from the 
		       site list was used in the PGD estimation [nsites] */
  int nlats;         /*!< Number of latitudes in grid search */
  int nlons;         /*!< Number of longitudes in grid serach */
  int ndeps;         /*!< Number of depths in PGD estimation */ 
  int nsites;        /*!< Should equal GFAST_data_struct's stream_length */
};

struct GFAST_peakDisplacementData_struct
{
  char **stnm;     /*!< Name of i'th site */
  double *pd;      /*!< Peak displacement (meters) observed at the i'th
		     site [nsites] */
  double *wt;      /*!< Data weight on the i'th peak displacement observation
		     [nsites] */
  double *sta_lat; /*!< Site latitude [-90,90] (degrees) */
  double *sta_lon; /*!< Site longitude [0,360] (degrees) */
  double *sta_alt; /*!< Site altitude (m) above sea-level */
  double *pd_time; /*!< Epoch time of peak displacement observation */
  bool *lmask;     /*!< If true then mask the i'th site in this inversion
		     [nsites] */
  bool *lactive;   /*!< If true then the i'th site has data from the waveform
		     processor and can be an active participant in the
		     inversion [nsites] */
  int nsites;      /*!< Number of sites (should be consistent with
		     buffered data) */
};

struct GFAST_offsetData_struct
{
  char **stnm;      /*!< Name of i'th site [nsites x 64] */
  double *ubuff;    /*!< Offset (meters) in up component observed at the
		      i'th site [nsites] */
  double *nbuff;    /*!< Offset (meters) in north component observed at the
		      i'th site [nsites] */
  double *ebuff;    /*!< Offset (meters) in east component observed at the
		      i'th site [nsites] */
  double *wtu;      /*!< Data weight on the i'th up offset 
		      observation [nsites] */
  double *wtn;      /*!< Data weight on the i'th north offset 
		      observation [nsites] */
  double *wte;      /*!< Data weight on the i'th east offset 
		      observation [nsites] */
  double *sta_lat; /*!< Site latitude [-90,90] (degrees) */
  double *sta_lon; /*!< Site longitude [0,360] (degrees) */
  double *sta_alt; /*!< Site altitude (m) above sea-level */
  bool *lmask;     /*!< If true then mask the i'th site in this inversion
		     [nsites] */
  bool *lactive;   /*!< If true then the i'th site has data from the waveform
		     processor and can be an active participant in the
		     inversion [nsites] */
  int nsites;       /*!< Number of sites */
};


struct GFAST_waveform3CData_struct
{
  char netw[64];    /*!< Network name */
  char stnm[64];    /*!< Station name */
  char chan[7][64]; /*!< Channel codes (Z, N, E, 3, 2, 1, Q) */
  char loc[64];     /*!< Location code */ 
  double *ubuff;    /*!< Up precise-point position buffer (meters).  If any
		      sample is not known it should be a NAN. [maxpts] */
  double *nbuff;    /*!< North precise-point position buffer (meters).  If any
		      sample is not known it should be a NAN. [maxpts] */
  double *ebuff;    /*!< East precise-point position buffer (meters).  If any
		      sample is not known it should be a NAN. */
  double *usigmabuff;    /*!< Up precise-point position buffer (meters).  If any
		      sample is not known it should be a NAN. [maxpts] */
  double *nsigmabuff;    /*!< North precise-point position buffer (meters).  If any
		      sample is not known it should be a NAN. [maxpts] */
  double *esigmabuff;    /*!< East precise-point position buffer (meters).  If any
		      sample is not known it should be a NAN. */
  double *qbuff;         /*!< Quality channel value. Multiple pieces of information are
          encoded here. */
  double *tbuff;    /*!< Epochal time buffer (s) [maxpts] */ 
  //double epoch;     /*!< Epoch time (seconds) corresponding to first sample 
  //                      of u, n, and e traces */
  double dt;        /*!< Sampling period (seconds). */
  double sta_lat;   /*!< Site latitude [-90,90] (degrees) */
  double sta_lon;   /*!< Site longitude [0,360] (degrees) */
  double sta_alt;   /*!< Site altitude (m) */
  double gain[7];   /*!< Instrument gain on all 3 channels + 3 channel sigmas + 
          quality channel */
  int maxpts;       /*!< Max number of poitns in buffer.  This is 
		      computed from the site sampling period and
		      the GFAST_parm_struct's bufflen */
  int npts;         /*!< Number of points in time series.  This cannot
		      exceed maxpts */
  bool lskip_pgd;   /*!< True  -> This site is ignored during the PGD
		      estimation.
		      False -> Will  attempt to use this site during
		      the PGD estimation. */
  bool lskip_cmt;   /*!< True  -> this site is ignored during the CMT 
		      inversion. 
		      False -> Will attempt to use this site during the
		      CMT inversion */
  bool lskip_ff;    /*!< True  -> this site is ignored during the finite
		      fault inversion
		      False -> Will attempt to use this during the 
		      finite fault inversion */
};

struct GFAST_data_struct
{
  struct GFAST_waveform3CData_struct *data;  /*!< Collocated data structure.
					       This is an array with
					       dimension [stream_length] */
  int stream_length;                       /*!< Number of streams. */
};

struct GFAST_shakeAlert_struct
{
  char eventid[128];  /*!< Event ID. */
  char orig_sys[12];  /*!< originating algorithm */
  int version;        /*!< message version */
  double lat;         /*!< Event latitude (degrees). */
  double lon;         /*!< Event longitude (degrees). */
  double dep;         /*!< Event depth (kilometers). */ 
  double mag;         /*!< Event magnitude. */
  double time;        /*!< Event epochal time (s). */
};

struct GFAST_activeEvents_struct
{
  struct GFAST_shakeAlert_struct *SA; /*!< Shake alert structure with 
					requisite event info.  This
					an array with dimension [nev]. */
  int nev;                            /*!< Number of events. */ 
  char pad1[4];
};

struct coreInfo_struct
{
  char id[128];                          /*!< Event ID */
  int version;                           /*!< Version number */
  double mag;                            /*!< Magnitude */
  bool lhaveMag;                         /*!< If true then mag is defined */
  enum alert_units_enum magUnits;        /*!< Magnitude units */
  bool lhaveMagUnits;                    /*!< If true then mag units are
					   defined */
  double magUncer;                       /*!< Magnitude uncertainty */
  bool lhaveMagUncer;                    /*!< If true then mag_uncer is
					   defined */
  enum alert_units_enum magUncerUnits;   /*!< Magnitude uncertainty units */
  bool lhaveMagUncerUnits;               /*!< If true then mag uncer units
					   is defined */
  double lat;                            /*!< Event latitude */
  bool lhaveLat;                         /*!< If true then the latitude
					   is defined */
  enum alert_units_enum latUnits;        /*!< Latitude units */
  bool lhaveLatUnits;                    /*!< If true then the lat units
					   are defined */
  double latUncer;                       /*!< Latitude uncertainty */
  bool lhaveLatUncer;                    /*!< If true then the latitude
					   uncertainty is defined */
  enum alert_units_enum latUncerUnits;   /*!< Latitude uncertainty units */
  bool lhaveLatUncerUnits;               /*!< If true then the latitude
					   uncertainty units are
					   defined */
  double lon;                            /*!< Event longitude */
  bool lhaveLon;                         /*!< If true then the longitude
					   is defined */
  enum alert_units_enum lonUnits;        /*!< Longitude units */
  bool lhaveLonUnits;                    /*!< If true then the longitude
					   units are defined */
  double lonUncer;                       /*!< Longitude uncertainty */
  bool lhaveLonUncer;                    /*!< If true then the longitude
					   uncertainty units are 
					   defined */
  enum alert_units_enum lonUncerUnits;   /*!< Longitude uncertainty units */
  bool lhaveLonUncerUnits;               /*!< If true then the longitude
					   uncertainty units are 
					   defined */
  double depth;                          /*!< Event depth */
  bool lhaveDepth;                       /*!< If true then the depth is
					   defined */
  enum alert_units_enum depthUnits;      /*!< Event depth units */
  bool lhaveDepthUnits;                  /*!< If true then the depth
					   units are defined */
  double depthUncer;                     /*!< Depth uncertainty */
  bool lhaveDepthUncer;                  /*!< If true then the depth
					   uncertainty is defined */
  enum alert_units_enum depthUncerUnits; /*!< Depth uncertainty units */
  bool lhaveDepthUncerUnits;             /*!< If true then the depth
					   uncertainty units are 
					   defined */
  double origTime;                       /*!< UTC origin time */
  bool lhaveOrigTime;                    /*!< If true then the origin
					   time is defined */
  enum alert_units_enum origTimeUnits;   /*!< Origin time units */
  bool lhaveOrigTimeUnits;               /*!< If true then the origin
					   time units are defined */
  double origTimeUncer;                  /*!< Origin time uncertainty */
  bool lhaveOrigTimeUncer;               /*!< If true then the origin time
					   uncertainty is defined */
  enum alert_units_enum
  origTimeUncerUnits;                /*!< Origin time uncertainty
				       units */
  bool lhaveOrigTimeUncerUnits;           /*!< If true then the origin time
					    uncertainty units are 
					    defined */
  double likelihood;  /*!< TODO: I have no idea what likelihood means */
  bool lhaveLikelihood; /*!< If true then the likelihood is defined */
  int numStations;   /*!< Number of stations used in event. */
};

struct GFAST_xml_output_status
{
  char eventid[128];                               /*!< Event ID. */
  int internal_version;                            /*!< GFAST internal version for this event ID. Always < 0 */
  int external_version;                            /*!< GFAST external version for this event ID. Always >= 0 */
  struct coreInfo_struct last_sent_core;           /*!< Core info of the last sent message */
  bool interval_complete[3][MAX_OUTPUT_INTERVALS]; /*!< intervals_complete[] for pgd + cmt + ff = 3 */
};

struct GFAST_activeEvents_xml_status
{
  struct GFAST_xml_output_status *SA_status;
  int nev;
  //char pad1[4];  /* MTH: seems to be for byte alignment but **check this** */
};

struct GFAST_xmlMessages_struct
{
  char **evids;  /*!< Event IDs. */
  char **cmtQML; /*!< CMT quakeML message. */
  char **ffXML;  /*!< Finite fault shakeAlert XML message. */
  char **pgdXML; /*!< PGD shakeAlert XML message. */
  int nmessages; /*!< Number of XML messages. */
  int mmessages; /*!< Max number of XML messages. */
};

#endif /* _gfast_struct_h__ */
