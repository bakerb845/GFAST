#ifndef _gfast_tracebuffer_h__
#define _gfast_tracebuffer_h__ 1
#if defined WINNT || defined WIN32 || defined WIN64
#include <windows.h>
#include <limits.h>
#else
#include <linux/limits.h>
#endif
#include <stdbool.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#endif
#include <hdf5.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_struct.h"
#include "gfast_config.h"
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include "gfast_config.h"

/* Linked list node for hash_set */
struct gnsstrace_node {
    struct gnsstrace_node *next; /* next entry in chain */
    char *name;            /* defined name (NSCL) */
    int i;                 /* index into gnsstraceData_struct for this NSCL */
};

struct gnsstrace_hashmap_struct {
    struct gnsstrace_node **map; /* hash array [hashsize] */
    uint32_t hashsize;     /* hashsize for hashmap array */
};

struct gnsstrace_struct
{
    char netw[64];      /*!< Network name */
    char stnm[64];      /*!< Station name */
    char chan[64];      /*!< Channel name */
    char loc[64];       /*!< Location name */
    double *times;      /*!< Epochal times (UTC seconds) at each point data
                             point [npts] */
    int *data;          /*!< Data [npts] */
    int *chunkPtr;      /*!< Points to first index of continuous trace
                             [nchunks+1].  The first index should be zero and 
                             the last index should be npts. */
    double dt;          /*!< Sampling period (s) */
    int nchunks;        /*!< Number of chunks */
    int npts;           /*!< Number of points in times and data */
};

struct gnsstraceData_struct
{
    struct gnsstrace_struct *traces; /*!< Concatenated traces */
    int ntraces;                    /*!< Number of traces */
    struct gnsstrace_hashmap_struct *hashmap; /*!< Hashmap for trace NSCLs */
    bool linit;                     /*!< If true then the structure is 
                                         initialized. */
};

// Header information used to sort channel data before adding to gnsstraceData_struct
struct string_index {
  char nscl[15];
  char net[8];
  char sta[8];
  char cha[8];
  char loc[8];

  double time;
  int indx;
  int nsamps;
};

struct h5trace_struct
{
    char netw[64];        /*!< Network name for forming earthworm requests */
    char stnm[64];        /*!< Station name for forming earthworm requests */
    char chan[64];        /*!< Channel name for forming earthworm requests */
    char loc[64];         /*!< Location code for forming earthworm requests */
    char *groupName;      /*!< Full path to HDF5 data group (null terminated) */
    char *metaGroupName;  /*!< Full path to HDF5 metadata group name (null
                               terminated) */    
    //double *buffer1;      /*!< Dataset 1 [maxpts] - TODO - delete its in h5 */
    //double *buffer2;      /*!< Dataset 2 [maxpts] - TODO - delete its in h5 */
    double *data;         /*!< Data to be copied onto other structure [ncopy] */
    double t1;            /*!< Start time of data (UTC-seconds) */
    //double t1beg;         /*!< Epochal start time of buffer1 (UTC-seconds) - TODO - delete */
    //double t2beg;         /*!< Epochal start time of buffer2 (UTC-seconds) - TODO - delete */
    double slat;          /*!< Station latitude (degrees) */
    double slon;          /*!< Station longitude (degrees) */
    double selev;         /*!< Station elevation above sea level (m) */
    double dt;            /*!< Sampling period (seconds) */
    double gain;          /*!< Instrument gain */
    int idest;            /*!< Maps this trace back to the appropriate
                               seven-component data stream */
    int maxpts;           /*!< Max number of points in data buffers */
    int npts1;            /*!< Number of points in buffer 1 - TODO - delete */
    int npts2;            /*!< Number of points in buffer 2 - TODO - delete */
    int ncopy;            /*!< Number of points to copy from Earthworm traceBuffer
                               to HDF5 or number of points to copy from HDF5 to
                               GFAST data buffer */
    int traceNumber;      /*!< Trace number of H5 data block */
    int dtGroupNumber;    /*!< Sampling period group number */
};

struct h5traceBuffer_struct
{
    struct h5trace_struct *traces; /*!< HDF5 trace data structure */
    char **dtGroupName;            /*!< Name of sampling period groups
                                        [ndtGroups] */
    int *dtPtr;                    /*!< Maps from idt'th dtGroup to start index
                                        of traces [ndtGroups+1] */
    hid_t fileID;                  /*!< HDF5 file handle */
    int ndtGroups;                 /*!< Number of sampling period groups */
    int ntraces;                   /*!< Number of traces to collect */
    bool linit;                    /*!< True if the structure has been
                                        initialized */
};

#ifdef __cplusplus
extern "C"
{
#endif

///// Methods for hashing gnsstrace channels
/* Hashing function */
uint32_t traceBuffer_gnsstrace_hash(const char *s);
/* Calls hash() and returns index into array with given hashsize */
uint32_t traceBuffer_gnsstrace_make_hash(const char *s, uint32_t hashsize);
/* Add a value to the set */
struct gnsstrace_node *traceBuffer_gnsstrace_hashmap_add(struct gnsstrace_hashmap_struct *hashmap,
                                              const char *name,
                                              int index);
/* Remove a value from the set */
int traceBuffer_gnsstrace_hashmap_remove(struct gnsstrace_hashmap_struct *hashmap, const char *name);
/* Check if set contains a value */
struct gnsstrace_node *traceBuffer_gnsstrace_hashmap_contains(struct gnsstrace_hashmap_struct *hashmap,
                                                   const char *name);
/* Free hashmap node */
void traceBuffer_gnsstrace_free_node(struct gnsstrace_node *np);
/* Free hashmap table */
void traceBuffer_gnsstrace_free_hashmap(struct gnsstrace_hashmap_struct *hashmap);
/* Print the full set structure, for debugging */
void traceBuffer_gnsstrace_print_hashmap(struct gnsstrace_hashmap_struct *hashmap);
/* Use the un-modded hash value to determine whether there are any true collisions (debugging) */
int traceBuffer_gnsstrace_print_true_collisions(struct gnsstrace_hashmap_struct *hashmap);
///// End hashing functions

/* Frees memory on the gnsstraceData structure */
void traceBuffer_gnsstrace_freeGnsstraceData(struct gnsstraceData_struct *gnsstraceData);
/* Frees memory on the gnsstraceData trace structure */
void traceBuffer_gnsstrace_freeGnsstrace(const bool clearSNCL,
                                   struct gnsstrace_struct *trace);
/* Sets the gnsstraceData structure and desired SNCL's from the input gpsData */
int traceBuffer_gnsstrace_setGnsstraceDataFromGFAST(struct GFAST_data_struct *gpsData,
                                         struct gnsstraceData_struct *gnsstraceData);
int traceBuffer_gnsstrace_printGnsstraceData(struct gnsstraceData_struct *gnsstraceData);
/* Unpack messages and associated functions */
int traceBuffer_gnsstrace_myCompare2(const void *x, const void *y);
void traceBuffer_gnsstrace_printStringindex(struct string_index *d, int n);
void traceBuffer_gnsstrace_sort2(struct string_index *vals, int n);
int traceBuffer_gnsstrace_unpackTraceBuf2Messages(
    const int nRead,
    const char *msgs,
    struct gnsstraceData_struct *gnsstraceData);
int traceBuffer_gnsstrace_unpackGeojsonMessages(
    const int nRead,
    const char *msgs,
    const int max_payload_size,
    struct h5traceBuffer_struct *h5traceBuffer,
    struct gnsstraceData_struct *gnsstraceData);
/* Reads a chunk of data from a Data group */
double *traceBuffer_h5_readData(const hid_t groupID,
                                const int ntraces,
                                int *maxpts,
                                double *dt, double *ts1, double *ts2,
                                double *gain, int *ierr);
/* Sets data in h5 file */
int traceBuffer_h5_setData(const double currentTime,
                           struct gnsstraceData_struct gnsstraceData,
                           struct h5traceBuffer_struct h5traceBuffer);
/* Copies the trace buffer to the GFAST structure */
int traceBuffer_h5_copyTraceBufferToGFAST(
    struct h5traceBuffer_struct *traceBuffer,
    struct GFAST_data_struct *gps_data);
/* Initialize tracebuffer structure from GFAST */
int traceBuffer_h5_setTraceBufferFromGFAST(
    const double bufflen,
    struct GFAST_data_struct gps_data,
    struct h5traceBuffer_struct *traceBuffer);
/* Close the HDF5 file with the traces */
int traceBuffer_h5_finalize(struct h5traceBuffer_struct *h5trace);
/* Get data from the HDF5 file */
int traceBuffer_h5_getData(const double t1, const double t2,
                           struct h5traceBuffer_struct *h5traceBuffer);
/* Get a double array */
int traceBuffer_h5_getDoubleArray(const hid_t groupID,
                                  const int i1, const int i2, 
                                  const char *citem,
                                  const double traceNaN, 
                                  const int nwork,
                                  double *__restrict__ work);
/* Get scalar data from the HDF5 file */
int traceBuffer_h5_getScalars(const hid_t groupID,
                              const int intNaN,
                              const double doubleNaN,
                              int *maxpts,
                              double *dt, double *gain,
                              double *ts1, double *ts2);
/* Initialize the HDF5 tracebuffer structure */
int traceBuffer_h5_initialize(const int job,
                              const bool linMemory,
                              const char *h5dir,
                              const char *h5file,
                              struct h5traceBuffer_struct *h5traceBuffer);
/* Sets the HDF5 file name */
int traceBuffer_h5_setFileName(const char *h5dir,
                               const char *h5file,
                               char h5name[PATH_MAX]);
/* Set scalar data */
int traceBuffer_h5_setDoubleScalar(const hid_t groupID,
                                   const char *citem,
                                   const double scalar);
int traceBuffer_h5_setIntegerScalar(const hid_t groupID,
                                    const char *citem,
                                    const int scalar);

#define GFAST_traceBuffer_h5_copyTraceBufferToGFAST(...)       \
              traceBuffer_h5_copyTraceBufferToGFAST(__VA_ARGS__)
#define GFAST_traceBuffer_h5_finalize(...)       \
              traceBuffer_h5_finalize(__VA_ARGS__)
#define GFAST_traceBuffer_h5_getData(...)       \
              traceBuffer_h5_getData(__VA_ARGS__)
#define GFAST_traceBuffer_h5_getDoubleArray(...)       \
              traceBuffer_h5_getDoubleArray(__VA_ARGS__)
#define GFAST_traceBuffer_h5_getScalars(...)       \
              traceBuffer_h5_getScalars(__VA_ARGS__)
#define GFAST_traceBuffer_h5_initialize(...)       \
              traceBuffer_h5_initialize(__VA_ARGS__)
#define GFAST_traceBuffer_h5_setFileName(...)       \
              traceBuffer_h5_setFileName(__VA_ARGS__)
#define GFAST_traceBuffer_h5_setDoubleScalar(...)       \
              traceBuffer_h5_setDoubleScalar(__VA_ARGS__)
#define GFAST_traceBuffer_h5_setIntegerScalar(...)       \
              traceBuffer_h5_setIntegerScalar(__VA_ARGS__)
#define GFAST_traceBuffer_h5_setTraceBufferFromGFAST(...)       \
              traceBuffer_h5_setTraceBufferFromGFAST(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* _gfast_tracebuffer_h__ */
