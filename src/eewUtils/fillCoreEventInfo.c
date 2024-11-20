#include <math.h>
#include <string.h>
#include "gfast_struct.h"

/*!
 * @brief Fills a given coreInfo_struct with the appropriate information
 * @param[in] evid Event ID
 * @param[in] version Event version number
 * @param[in] SA_lat Event latitude
 * @param[in] SA_lon Event longitude
 * @param[in] SA_depth Event depth
 * @param[in] SA_mag Event magnitude
 * @param[in] SA_time Event origin time (UTC)
 * @param[in] num_stations Number of stations contributing
 * @param[out] core struct to fill with information
 * @return status code.
 */
int eewUtils_fillCoreEventInfo(
    const char *evid,
    const int version,
    const double SA_lat,
    const double SA_lon,
    const double SA_depth,
    const double SA_mag,
    const double SA_time,
    const int num_stations,
    struct coreInfo_struct *core)
{
    strcpy(core->id, evid);
    core->version = version;
    core->mag = SA_mag;
    core->lhaveMag = true;
    core->magUnits = MOMENT_MAGNITUDE;
    core->lhaveMagUnits = true;
    core->magUncer = 0.5;
    core->lhaveMagUncer = true;
    core->magUncerUnits = MOMENT_MAGNITUDE;
    core->lhaveMagUncerUnits = true;
    core->lat = SA_lat; 
    core->lhaveLat = true;
    core->latUnits = DEGREES;
    core->lhaveLatUnits = true;
    core->latUncer = (double) NAN;
    core->lhaveLatUncer = true;
    core->latUncerUnits = DEGREES;
    core->lhaveLatUncerUnits = true;
    // GFAST would call lon -120 as 240 by default. Change this to be
    // consistent with ShakeAlert seismic algorithms
    core->lon = (SA_lon > 180) ? SA_lon - 360: SA_lon;
    core->lhaveLon = true;
    core->lonUnits = DEGREES;
    core->lhaveLonUnits = true;
    core->lonUncer = (double) NAN;
    core->lhaveLonUncer = true;
    core->lonUncerUnits = DEGREES;
    core->lhaveLonUncerUnits = true;
    core->depth = SA_depth;
    core->lhaveDepth = true;
    core->depthUnits = KILOMETERS;
    core->lhaveDepthUnits = true;
    core->depthUncer = (double) NAN;
    core->lhaveDepthUncer = true;
    core->depthUncerUnits = KILOMETERS;
    core->lhaveDepthUncerUnits = true;
    core->origTime = SA_time;
    core->lhaveOrigTime = true;
    core->origTimeUnits = UTC;
    core->lhaveOrigTimeUnits = true;
    core->origTimeUncer = (double) NAN;
    core->lhaveOrigTimeUncer = true;
    core->origTimeUncerUnits = SECONDS;
    core->lhaveOrigTimeUncerUnits = true;
    core->likelihood = 0.8;
    core->lhaveLikelihood = true;
    core->numStations = num_stations;
    return 0;
}