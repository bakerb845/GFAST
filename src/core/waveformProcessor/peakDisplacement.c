#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include "gfast_core.h"

/*!
 * @brief Computes the peak displacement for each GPS precise point position
 *        data stream with the additional requirement that the shear wave
 *        has passed through the site.
 *
 * @param[in] pgd_props     PGD properties, including utm_zone, window_vel, min_window_vel,
 *                          minimum_pgd_cm, maximum_pgd_cm, [une]_raw_sigma_threshold
 * @param[in] ev_lat        source hypocentral latitude (degrees) [-90,90]
 * @param[in] ev_lon        source hypocentral longitude (degrees) [0,360]
 * @param[in] ev_dep        source hypocentral depth (km) (this is positive
 *                          down from the free surface)
 * @param[in] ev_time       source origin time in seconds since epoch (UTC)
 * @param[in] gps_data      contains the most up-to-date precise point 
 *                          positions for each site 
 *
 * @param[in,out] pgd_data  on input holds a logical mask if a site is to
 *                          be ignored.
 *                          on output holds the peak ground displacement
 *                          at each site satisfying the S velocity window mask.
 *                          in this instance all sites with data are given
 *                          data weights of unity and all sites without
 *                          data are given data weights of zero.
 *
 * @param[out] ierr         0 indicates success
 *
 * @result the number of sites at which peak displacement was computed
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int core_waveformProcessor_peakDisplacement(
    const struct GFAST_pgd_props_struct *pgd_props,
    const double ev_lat,
    const double ev_lon,
    const double ev_dep,
    const double ev_time,
    struct GFAST_data_struct gps_data,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    int *ierr)
{
    double currentTime, distance, effectiveHypoDist, epoch,
           peakDisp, x1, x2, y1, y2, tmin, tmax, obsTime,
           uMaxUncertainty, nMaxUncertainty, eMaxUncertainty;
    int qMin, qRef, qPeak;
    int k, nsites, zone_loc, iRef, iPeak;
    //unused int i;
    bool lnorthp;
    bool lnorthp_event;
    bool l_use_observation;

    double s_arr_time;
    int nMaxLeader;

    // Values from properties file

    // If not -12345 then this is the desired UTM zone in which to compute source and receiver positions.
    // Otherwise, the UTM zone will be estimated from the source location
    const int utm_zone = pgd_props->utm_zone;
    // The shear wave velocity used in data windowing (km/s).
    // If the site/source distance is less than (current_time - ev_time)*svel_window
    // then the site will be excluded
    const double svel_window = pgd_props->window_vel;
    // The *minimum* shear wave velocity (km/s) used in data windowing
    const double min_svel_window = pgd_props->min_window_vel;
    // The *minimum* pgd value (cm) to pass on to inversion. Ignore others
    const double min_pgd_cm = pgd_props->minimum_pgd_cm;
    // The *maximum* pgd value (cm) to pass on to inversion. Ignore others
    const double max_pgd_cm = pgd_props->maximum_pgd_cm;
    // Threshold values for raw positional uncertainties (cm). If observed
    // peak sigma is greater than threshold, ignore the associated pd
    // observation. If sigma_threshold is < 0, allow any sigma.
    const double u_raw_sigma_threshold = pgd_props->u_raw_sigma_threshold;
    const double n_raw_sigma_threshold = pgd_props->n_raw_sigma_threshold;
    const double e_raw_sigma_threshold = pgd_props->e_raw_sigma_threshold;
    // Threshold value for Q channel. If the observed Q value for the reference
    // or peak displacement is <= this threshold, ignore the pd observation.
    // If q_value_threshold is < 0, allow any Q value
    const int q_value_threshold = pgd_props->q_value_threshold;

    LOG_MSG("Calculating PD, svel=(%.1f/%.1f), pgd_bounds=(%.1f/%.1f), sigma_thresh=(%.1f/%.1f/%.1f), q_thresh=%d",
        svel_window,
        min_svel_window,
        min_pgd_cm,
        max_pgd_cm,
        u_raw_sigma_threshold,
        n_raw_sigma_threshold,
        e_raw_sigma_threshold,
        q_value_threshold);

    //------------------------------------------------------------------------//
    //
    // Error handling
    *ierr = 0;
    nsites = 0;
    obsTime = 0.0;
    iRef = 0;
    iPeak = 0;
    uMaxUncertainty = 0.0;
    eMaxUncertainty = 0.0;
    nMaxUncertainty = 0.0;
    qMin = 0;
    qRef = 0;
    qPeak = 0;
    if (gps_data.stream_length != pgd_data->nsites)
    {
        LOG_ERRMSG("Inconsistent structure sizes %d %d",
                   gps_data.stream_length, pgd_data->nsites); 
        *ierr = 1;
        // For safety cut the inversion off at the knees
        if (pgd_data->nsites > 0)
        {
            for (k=0; k<pgd_data->nsites; k++)
            {
                pgd_data->pd[k] = 0.0;
                pgd_data->wt[k] = 0.0;
                pgd_data->lactive[k] = false;
            }
        }
        return nsites;
    }
    // Get the source location
    zone_loc = utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;} // Get UTM zone from source lat/lon
    GFAST_core_coordtools_ll2utm(ev_lat, ev_lon,
                                 &y1, &x1,
                                 &lnorthp_event, &zone_loc);
//LOG_MSG("peakDisp: utm_zone=%d ev_lat:%f ev_lon:%f x1:%f y1:%f\n",
         //utm_zone, ev_lat, ev_lon, x1, y1);
    // Loop on streams and if they satisfy the S wave mask get their PGD
    for (k=0; k<gps_data.stream_length; k++)
    {
        obsTime = 0.0;
        iRef = 0;
        iPeak = 0;
        uMaxUncertainty = 0.0;
        eMaxUncertainty = 0.0;
        nMaxUncertainty = 0.0;
        qMin = 0;
        qRef = 0;
        qPeak = 0;
        // Make sure I have the latest/greatest site location 
        pgd_data->sta_lat[k] = gps_data.data[k].sta_lat;
        pgd_data->sta_lon[k] = gps_data.data[k].sta_lon; 
        pgd_data->sta_alt[k] = gps_data.data[k].sta_alt;
        // Null out result
        pgd_data->pd_time[k] = obsTime; // Null out result
        pgd_data->pd[k] = 0.0; // Null out result
        pgd_data->wt[k] = 0.0; // Assume no weight
        pgd_data->lactive[k] = false; // Assume site is not active in inversion
        if (pgd_data->lmask[k]){continue;} // Not in inversion
        // Get the recevier UTM
        GFAST_core_coordtools_ll2utm(gps_data.data[k].sta_lat,
                                     gps_data.data[k].sta_lon,
                                     &y2, &x2,
                                     &lnorthp, &zone_loc);
        // Get the distance - remember source is + down and receiver is + up
        distance = sqrt( pow(x1 - x2, 2)
                       + pow(y1 - y2, 2)
                       + pow(ev_dep*1000.0 + gps_data.data[k].sta_alt, 2));
        distance = distance*1.e-3; // convert to km
        if (lnorthp != lnorthp_event) {
          /*
          LOG_MSG("Ignore: %s.%s.%s.%s lnorthp=%d != lnorthp_event=%d",
                  gps_data.data[k].stnm, gps_data.data[k].chan[0],
                  gps_data.data[k].netw, gps_data.data[k].loc,
                  lnorthp, lnorthp_event);
          */

          distance = 99999;  // Don't use this station
        }

        // Apply an S wave window mask to preclude likely outliers in
        // the ensuing PGD inversion 
        epoch = gps_data.data[k].tbuff[0]; //gps_data.data[k].epoch;
        currentTime = epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        effectiveHypoDist = (currentTime - ev_time)*svel_window;

        // MTH: I want to open up the props.processingTime so that it will still
        //      process a delayed PDL event, but I don't want to include stations
        //      that are >> from epicenter.  This seems to be a solution:
        if (effectiveHypoDist > 1000.){
          effectiveHypoDist = 1000.;    // Don't include stations > 1000 km 
        }

        // MTH: Right now gps_data[k].tbuff[0] has previously been set = ev_time
        //      So if for some reason n/e/ubuff[0] = nan at ev_time,
        //      then all subsequent PGD displacement measurements are fixed to nan.
        //      Let's calc where in the buff the S wave arrives and allow u0/n0/e0 
        //        up to this point
        s_arr_time = distance / svel_window - 2.0;

        nMaxLeader = (int)(ev_time + s_arr_time - gps_data.data[k].tbuff[0])/gps_data.data[k].dt;

        /*
         LOG_MSG("currentTime:%f epoch:%f effHypoDst:%.1f -vs- dist:%.1f %s.%s.%s.%s",
              currentTime, epoch, effectiveHypoDist, distance,
              gps_data.data[k].stnm, gps_data.data[k].chan[0],
              gps_data.data[k].netw, gps_data.data[k].loc);
        */

        if (distance < effectiveHypoDist)
        {
            // Compute the peak displacement max(norm(u + n + e, 2))
            tmin = distance / svel_window;
            tmax = distance / min_svel_window;
            //printf("Call __getPeakDisplacement dist:%.2f tmin:%.2f tmax:%.2f\n", distance, tmin, tmax);
            peakDisp = core_waveformProcessor_peakDisplacementHelper(
                gps_data.data[k].npts,
                gps_data.data[k].dt,
                ev_time,
                epoch,
                gps_data.data[k].ubuff,
                gps_data.data[k].nbuff,
                gps_data.data[k].ebuff,
                nMaxLeader,
                tmin,
                tmax,
                &obsTime,
                &iRef,
                &iPeak);

            /*
The Crowell et al. [2016] coefficients are
log10(PGD) = A + B*M + C*M*log10(distance)
A = -6.687
B = 1.500
C = -0.214
M 9 at 100km: -6.687 + 150 - 21.4*2 = 100 cm(?)
            */
/*
            float mag;
            float logPD;

            mag = 7.8;
            logPD = -6.687 + 1.5*mag - 0.214*mag*log10(distance);
            peakDisp = pow(10., logPD);
            peakDisp /= 100; // Pretty sure gfast wants peakDisp in meters
*/

            // At this point, use observation unless proven guilty
            l_use_observation = true; 
            // Do we have a real observation?
            if (isnan(peakDisp))
            {
                l_use_observation = false;
                LOG_MSG("currentTime:%f %s.%s.%s.%s Got peakDisp = nan ubuf=%f nbuf=%f ebuf=%f",
                        currentTime,
                        gps_data.data[k].stnm, gps_data.data[k].chan[0],
                        gps_data.data[k].netw, gps_data.data[k].loc,
                        gps_data.data[k].ubuff[gps_data.data[k].npts-1],
                        gps_data.data[k].nbuff[gps_data.data[k].npts-1],
                        gps_data.data[k].ebuff[gps_data.data[k].npts-1]);
            }
            else
            {
                // If there is a real observation, get the associated max uncertainties and q values
                if (!isnan(gps_data.data[k].usigmabuff[iRef]) && !isnan(gps_data.data[k].usigmabuff[iPeak])) {
                    uMaxUncertainty = fmax(gps_data.data[k].usigmabuff[iRef], gps_data.data[k].usigmabuff[iPeak]);
                }
                if (!isnan(gps_data.data[k].nsigmabuff[iRef]) && !isnan(gps_data.data[k].nsigmabuff[iPeak])) {
                    nMaxUncertainty = fmax(gps_data.data[k].nsigmabuff[iRef], gps_data.data[k].nsigmabuff[iPeak]);
                }
                if (!isnan(gps_data.data[k].esigmabuff[iRef]) && !isnan(gps_data.data[k].esigmabuff[iPeak])) {
                    eMaxUncertainty = fmax(gps_data.data[k].esigmabuff[iRef], gps_data.data[k].esigmabuff[iPeak]);
                }
                if (!isnan(gps_data.data[k].qbuff[iRef]) && !isnan(gps_data.data[k].qbuff[iPeak])) {
                    qRef = core_waveformProcessor_parseQChannelGoodness(gps_data.data[k].qbuff[iRef]);
                    qPeak = core_waveformProcessor_parseQChannelGoodness(gps_data.data[k].qbuff[iPeak]);
                    qMin = fmin(qRef, qPeak);
                }

                LOG_MSG("%s.%s.%s.%s peakDisp=%f dist=%.2f, peakSigmas=(%.4f,%.4f,%.4f), minQ=%d",
                        gps_data.data[k].stnm, gps_data.data[k].chan[0],
                        gps_data.data[k].netw, gps_data.data[k].loc,
                        peakDisp,
                        distance,
                        uMaxUncertainty,
                        nMaxUncertainty,
                        eMaxUncertainty,
                        qMin);
            }

            // Is the observation above the defined minimum?
            if (peakDisp * 100 <= min_pgd_cm)
            {
                l_use_observation = false;
                LOG_MSG("currentTime:%f %s.%s.%s.%s Ignoring pgd, %f cm <= min_pgd_cm %f",
                        currentTime,
                        gps_data.data[k].stnm, gps_data.data[k].chan[0],
                        gps_data.data[k].netw, gps_data.data[k].loc,
                        peakDisp * 100, min_pgd_cm);
            }

            // Is the observation below the defined maximum?
            if (peakDisp * 100 >= max_pgd_cm)
            {
                l_use_observation = false;
                LOG_MSG("currentTime:%f %s.%s.%s.%s Ignoring pgd, %f cm >= min_pgd_cm %f",
                        currentTime,
                        gps_data.data[k].stnm, gps_data.data[k].chan[0],
                        gps_data.data[k].netw, gps_data.data[k].loc,
                        peakDisp * 100, max_pgd_cm);
            }

            // Is the observation within the uncertainty bounds?
            // const double u_raw_sigma_threshold = 35; // cm
            // const double n_raw_sigma_threshold = 17; // cm
            // const double e_raw_sigma_threshold = 14; // cm
            if (((u_raw_sigma_threshold > 0) && (uMaxUncertainty * 100 >= u_raw_sigma_threshold)) ||
                ((n_raw_sigma_threshold > 0) && (nMaxUncertainty * 100 >= n_raw_sigma_threshold)) ||
                ((e_raw_sigma_threshold > 0) && (eMaxUncertainty * 100 >= e_raw_sigma_threshold))) 
            {
                l_use_observation = false;
                LOG_DEBUGMSG("CCC PeakDisp, %s.%s.%s.%s ignoring observation, ZNE sigmas: (%.1f,%.1f,%.1f), thresholds: (%.1f,%.1f,%.1f) cm",
                    gps_data.data[k].stnm, gps_data.data[k].chan[0],
                    gps_data.data[k].netw, gps_data.data[k].loc,
                    uMaxUncertainty * 100,
                    nMaxUncertainty * 100,
                    eMaxUncertainty * 100,
                    u_raw_sigma_threshold,
                    n_raw_sigma_threshold,
                    e_raw_sigma_threshold
                );
            }

            // Is the observation under the Q value threshold?
            if ((q_value_threshold >= 0) && (qMin < q_value_threshold)) {
                l_use_observation = false;
                LOG_DEBUGMSG("CCC PeakDisp, %s.%s.%s.%s ignoring observation, qRef/qPeak: (%d/%d), threshold: %d",
                    gps_data.data[k].stnm, gps_data.data[k].chan[0],
                    gps_data.data[k].netw, gps_data.data[k].loc,
                    qRef,
                    qPeak,
                    q_value_threshold
                );
            }

            // If it isn't a NaN and within the sanity bounds then retain it for processing
            if (l_use_observation) {
                pgd_data->pd_time[k] = obsTime; // epoch
                pgd_data->pd[k] = peakDisp; // meters
                pgd_data->wt[k] = 1.0;
                pgd_data->lactive[k] = true;
                nsites = nsites + 1;
            }
        } // End check on S-wave mask
    } // Loop on data streams
    return nsites;
}
