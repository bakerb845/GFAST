#include <math.h>
#include <float.h>
#include "gfast_core.h"

#define PD_MAX_NAN -DBL_MAX
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

//============================================================================//
/*!
 * @brief Waveform processor to estimate the peak displacement observed
 *        on a 3 channel GPS stream where the peak displacement at any
 *        sample is Euclidean norm of its displacement.
 *
 * @param[in] npts             number of points in time series
 * @param[in] dt               sampling period (s) of GPS buffers
 * @param[in] ev_time          epochal UTC origin time (s)
 * @param[in] epoch            epochal UTC start time (s) of GPS traces
 * @param[in] ubuff            vertical position [npts]
 * @param[in] nbuff            north position [npts]
 * @param[in] ebuff            east position [npts]
 * @param[in] nMaxLeader       latest time index to start measurement in
 *                             case of nan's
 * @param[in] tmin             distance / svel_window (s)
 * @param[in] tmax             distance / min_svel_window (s)
 * 
 * @param[out] obsTime         epochal time of peak displacment (s)
 * @param[out] iRef            index of reference value
 * @param[out] iPeak           index of peak value
 * 
 *
 * @result the peak displacement observed on a trace.  this has the same
 *         units as ubuff, nbuff, and ebuff.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
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
    int *iPeak
    )
{
    double diffT, peakDisplacement_i, peakDisplacement, e0, n0, u0;
    int i, indx0, indx1;
    int ipeak = 0;
    int debug = 0;
    //------------------------------------------------------------------------//
    //
    // Set the initial position
    *obsTime = 0.0;
    *iRef = 0;
    *iPeak = 0;
    u0 = 0.0;
    n0 = 0.0;
    e0 = 0.0;
    diffT = ev_time - epoch;
    indx0 = MAX(0, (int) (diffT/dt + 0.5));
    indx0 = MIN(npts - 1, indx0);

    // Compute the offset
    u0 = ubuff[indx0];
    n0 = nbuff[indx0];
    e0 = ebuff[indx0];

    if (isnan(u0) || isnan(n0) || isnan(e0)) {
        for (i = indx0; i < nMaxLeader; i++) {
            if (!isnan(ubuff[i]) && !isnan(nbuff[i]) && !isnan(ebuff[i])) {
                indx0 = i;
                u0 = ubuff[indx0];
                n0 = nbuff[indx0];
                e0 = ebuff[indx0];
                //LOG_MSG("Search leader for t0:  nMax:%d indx0:%d u0:%f n0:%f e0:%f",
                        //nMaxLeader, indx0, u0, n0, e0);
                break;
            }
        }
    }

    indx1 = (int)(tmax/dt + 0.5);
    if (indx1 <= indx0) {
        // LOG_MSG("ERROR: indx0=%d >= indx1=%d (tmax=%f)", indx0, indx1, tmax);
        return (double) NAN;
    }
    if (indx1 > npts) {
        // LOG_MSG("tmin=%.1f tmax=%.1f npts=%d <= indx1=%d --> Set indx1=npts", tmin, tmax, npts, indx1);
        indx1 = npts;
    }

    // Prevent a problem
    if (isnan(u0) || isnan(n0) || isnan(e0))
    {
        // LOG_MSG("Returning NAN instead of calculating epoch:%f diffT=%f indx0=%d",
        //     epoch, diffT, indx0);
        return (double) NAN;
    }

    // Compute the maximum peak ground displacement 
    peakDisplacement = PD_MAX_NAN;
    if (debug) {
        LOG_MSG("Loop to find peakDisp: from i=indx0=%d to i<npts=%d, tmin=%f",
            indx0, npts, tmin);
    }
    for (i = indx0; i < indx1; i++)
    {
        // First calculate peak displacement
        peakDisplacement_i = PD_MAX_NAN;
        if (!isnan(ubuff[i]) && !isnan(nbuff[i]) && !isnan(ebuff[i]) )
        {
            peakDisplacement_i = sqrt( pow(ubuff[i] - u0, 2)
                                     + pow(nbuff[i] - n0, 2)
                                     + pow(ebuff[i] - e0, 2));
            if (debug){
                LOG_MSG("  i=%d peakDisplacement_i=%f ubuff:%f u0:%f nbuff:%f n0:%f ebuff:%f e0:%f",
                    i, peakDisplacement_i, ubuff[i], u0, nbuff[i], n0, ebuff[i], e0);
            }
        }
        if (peakDisplacement_i > peakDisplacement) {
            ipeak = i;
        }
        peakDisplacement = fmax(peakDisplacement_i, peakDisplacement);
    } // Loop on data points

    if (fabs(peakDisplacement - PD_MAX_NAN)/fabs(PD_MAX_NAN) < 1.e-10)
    {
         LOG_MSG("%s", "Returning NAN because peakDisp is ~ PD_MAX_NAN");
         peakDisplacement = (double) NAN;
    }
    if (!isnan(peakDisplacement)){
        *obsTime = epoch + dt * ipeak;
        *iRef = indx0;
        *iPeak = ipeak;
        // LOG_MSG("Got peak [%f] at ipeak:%d ubuff[i]=%f (u0=%f)  nbuff[i]=%f (n0=%f)  ebuff[i]=%f (e0=%f) ",
        //     peakDisplacement, ipeak, ubuff[ipeak], u0, nbuff[ipeak], n0, ebuff[ipeak], e0);
    }

    return peakDisplacement;
}