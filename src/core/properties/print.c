#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/os/os.h"

/*!
 * @brief Prints the GFAST properties to the debug file.  Note, there is 
 *        no verbosity check.
 *
 * @param[in] props    GFAST properties structure to write to the debug file
 *
 * @author Ben Baker, ISTI
 *
 */
void core_properties_print(struct GFAST_props_struct props)
{
    const char *lspace = "    \0";
    int i;
    LOG_DEBUGMSG("%s", "GFAST properties");
    if (props.opmode == OFFLINE)
    {
        LOG_DEBUGMSG("%s GFAST site metadata file %s", lspace,
                     props.metaDataFile);  
        LOG_DEBUGMSG("%s GFAST is operating in offline mode", lspace);
        LOG_DEBUGMSG("%s GFAST default sampling period is %f (s)", lspace,
                     props.dt_default);
        if (props.dt_init == INIT_DT_FROM_FILE)
        {
            LOG_DEBUGMSG("%s GFAST will get sampling period from file %s",
                         lspace, props.metaDataFile); 
        }
        else if (props.dt_init == INIT_DT_FROM_DEFAULT)
        {
            LOG_DEBUGMSG("%s GFAST will set GPS sampling periods to %f (s)",
                         lspace, props.dt_default); 
        }
        else if (props.dt_init == INIT_DT_FROM_SAC)
        {
            LOG_DEBUGMSG("%s GFAST will set GPS sampling period from SAC files",
                         lspace);
        }
        if (props.loc_init == INIT_LOCS_FROM_FILE)
        {
            LOG_DEBUGMSG("%s GFAST will initialize locations from file %s",
                         lspace, props.metaDataFile);
        }
        else if (props.loc_init == INIT_LOCS_FROM_TRACEBUF)
        {
            LOG_DEBUGMSG("%s GFAST will initialize locations from tracebuf",
                         lspace);
        }
        else if (props.loc_init == INIT_LOCS_FROM_SAC)
        {
            LOG_DEBUGMSG("%s GFAST will initialize locations from SAC files",
                         lspace);
        }
        LOG_DEBUGMSG("%s GFAST simulation time (s) %f",
                     lspace, props.synthetic_runtime);
        LOG_DEBUGMSG("%s GFAST observed data directory: %s",
                   lspace, props.obsdataDir);
        LOG_DEBUGMSG("%s GFAST observed data file: %s",
                   lspace, props.obsdataFile);
    }
    else if (props.opmode == PLAYBACK)
    {
        LOG_DEBUGMSG("%s GFAST is operating in playback mode", lspace);
    }
    else if (props.opmode == REAL_TIME_EEW)
    {
        LOG_DEBUGMSG("%s GFAST is operating in EEW real-time mode", lspace);
        LOG_DEBUGMSG("%s GFAST time between iterations is %f (s)", lspace,
                     props.waitTime);
        LOG_DEBUGMSG("%s GFAST site position file %s", lspace,
                     props.metaDataFile);
        if (props.n_networks == 0)
        {
            LOG_DEBUGMSG("%s GFAST will use any networks available from the position file", 
                         lspace);
        }
        else
        {
            int nbuffer = 128;
            char buffer_network[nbuffer];
            int cx = 0;
            for (i = 0; i < props.n_networks; i++) {
                if (i == props.n_networks - 1) {
                    cx += snprintf(buffer_network + cx, nbuffer - cx, "%s",
                                    props.metaDataNetworks[i]);
                } else {
                    cx += snprintf(buffer_network + cx, nbuffer - cx, "%s, ",
                                    props.metaDataNetworks[i]);
                }
            }
            LOG_DEBUGMSG("%s GFAST will only use %d networks from the position file: %s", 
                         lspace, props.n_networks, buffer_network);
        }
        LOG_DEBUGMSG("%s GFAST default sampling period is %f (s)", lspace,
                   props.dt_default);
        if (props.opmode == REAL_TIME_EEW)
        {
            LOG_DEBUGMSG("%s GFAST trigger origin url: %s", lspace,
                         props.activeMQ_props.originURL);
            LOG_DEBUGMSG("%s GFAST origin topic: %s",
                         lspace, props.activeMQ_props.originTopic);
            LOG_DEBUGMSG("%s GFAST results destination url: %s", lspace,
                         props.activeMQ_props.destinationURL);
            LOG_DEBUGMSG("%s GFAST destination topic: %s",
                         lspace, props.activeMQ_props.destinationTopic);
            LOG_DEBUGMSG("%s GFAST heartbeat topic: %s",
                         lspace, props.activeMQ_props.hbTopic);
            LOG_DEBUGMSG("%s heartbeat interval (seconds) %d", lspace,
                         props.activeMQ_props.hbInterval);
            if (props.activeMQ_props.maxAttempts > 0)
            {
                LOG_DEBUGMSG("%s Milliseconds before reconnect %d",
                             lspace, props.activeMQ_props.msReconnect); 
                LOG_DEBUGMSG("%s Max number of attempts to connect %d",
                             lspace, props.activeMQ_props.maxAttempts);
            }
            LOG_DEBUGMSG("%s Will wait %d milliseconds for a message",
                         lspace, props.activeMQ_props.msWaitForMessage);
        }
    }
    LOG_DEBUGMSG("%s GFAST buffer length is %f seconds", lspace, props.bufflen);
    if (props.utm_zone ==-12345)
    {
        LOG_DEBUGMSG("%s GFAST will get UTM zone from hypocenters", lspace);
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will set UTM zone to %d", lspace,
                   props.utm_zone);
    }
    LOG_DEBUGMSG("%s GFAST verbosity level is %d", lspace, props.verbose);
    if (os_path_isfile(props.siteMaskFile))
    {
        LOG_DEBUGMSG("%s GFAST will use site maskfile: %s", lspace,
                     props.siteMaskFile);
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will not mask any sites", lspace);
    } 
    LOG_DEBUGMSG("%s GFAST HDF5 archive dir: %s", lspace, props.h5ArchiveDir);
    LOG_DEBUGMSG("%s GFAST will finish processing an event after %f (s)",
               lspace, props.processingTime);
    LOG_DEBUGMSG("%s GFAST will use a default earthquake depth of %f",
               lspace, props.eqDefaultDepth);
    if (props.lh5SummaryOnly)
    {
        LOG_DEBUGMSG("%s GFAST will only write an HDF5 summary", lspace);
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will save all steps to HDF5", lspace);
    }
    //--------------------------------pgd-------------------------------------//
    if (props.pgd_props.do_pgd) 
    {
        LOG_DEBUGMSG("%s GFAST will calculate PGD", lspace);
        LOG_DEBUGMSG("%s GFAST PGD source receiver distance tolerance %f (km)",
                lspace, props.pgd_props.dist_tol);
        LOG_DEBUGMSG("%s GFAST PGD default distance %f (cm)",
                lspace, props.pgd_props.disp_def);
        LOG_DEBUGMSG("%s GFAST Number of PGD grid search latitudes %d",
                lspace, props.pgd_props.ngridSearch_lats);
        LOG_DEBUGMSG("%s GFAST Number of PGD grid search longitudes %d",
                lspace, props.pgd_props.ngridSearch_lons);
        LOG_DEBUGMSG("%s GFAST Number of PGD grid search depths is %d",
                lspace, props.pgd_props.ngridSearch_deps);
        LOG_DEBUGMSG("%s GFAST PGD data selection velocity is %f (km/s)",
                lspace, props.pgd_props.window_vel);
        LOG_DEBUGMSG("%s GFAST Number of sites required to compute PGD is %d",
                lspace, props.pgd_props.min_sites);
        if (props.pgd_props.ngridSearch_lats > 1)
        {
            LOG_DEBUGMSG("%s GFAST PGD latitude grid spacing %f",
                    lspace, props.pgd_props.dLat); 
        }
        if (props.pgd_props.ngridSearch_lons > 1)
        {
            LOG_DEBUGMSG("%s GFAST PGD longitude grid spacing %f",
                    lspace, props.pgd_props.dLon);
        }
        if (props.pgd_props.n99 > 0)
        {
            LOG_DEBUGMSG("%s GFAST PGD sigma lookup: %d values",
                lspace, props.pgd_props.n99);
            for (i = 0; i < props.pgd_props.n99; i++) {
                LOG_DEBUGMSG("%s GFAST PGD sigma lookup: %5.1f %5.2f",
                    lspace, props.pgd_props.t99[i], props.pgd_props.m99[i]);
            }
        }
        else
        {
            LOG_DEBUGMSG("%s GFAST PGD sigma lookup: 0 values, using default",
                lspace);
        }
        if (props.pgd_props.n_throttle > 0)
        {
            LOG_DEBUGMSG("%s GFAST PGD threshold lookup: %d values",
                lspace, props.pgd_props.n_throttle);
            for (i = 0; i < props.pgd_props.n_throttle; i++) {
                LOG_DEBUGMSG("%s GFAST PGD threshold lookup: %5.1f %5.2f %d", lspace,
                    props.pgd_props.throttle_time_threshold[i],
                    props.pgd_props.throttle_pgd_threshold[i],
                    props.pgd_props.throttle_num_stations[i]);
            }
        }
        else
        {
            LOG_DEBUGMSG("%s GFAST PGD threshold lookup: 0 values, using default",
                lspace);
        }
        if (props.pgd_props.u_raw_sigma_threshold >= 0) {
            LOG_DEBUGMSG("%s Will ignore pd values if U raw sigma exceeds %f cm",
                lspace, props.pgd_props.u_raw_sigma_threshold);
        } else {
            LOG_DEBUGMSG("%s Will allow any U raw sigma values for pd calculations", lspace);
        }
        if (props.pgd_props.n_raw_sigma_threshold >= 0) {
            LOG_DEBUGMSG("%s Will ignore pd values if N raw sigma exceeds %f cm",
                lspace, props.pgd_props.n_raw_sigma_threshold);
        } else {
            LOG_DEBUGMSG("%s Will allow any N raw sigma values for pd calculations", lspace);
        }
        if (props.pgd_props.e_raw_sigma_threshold >= 0) {
            LOG_DEBUGMSG("%s Will ignore pd values if E raw sigma exceeds %f cm",
                lspace, props.pgd_props.e_raw_sigma_threshold);
        } else {
            LOG_DEBUGMSG("%s Will allow any E raw sigma values in pd calculations", lspace);
        }
        if (props.pgd_props.q_value_threshold >= 0) {
            LOG_DEBUGMSG("%s Will ignore pd values if Q value < %d",
                lspace, props.pgd_props.q_value_threshold);
        } else {
            LOG_DEBUGMSG("%s Will allow any Q values in pd calculations", lspace);
        }
        LOG_DEBUGMSG("%s Will throttle messages below SA mag %f",
                    lspace, props.pgd_props.SA_mag_threshold);
        LOG_DEBUGMSG("%s Will throttle messages above pgd mag sigma %f",
                    lspace, props.pgd_props.pgd_sigma_throttle);

        LOG_DEBUGMSG("%s GFAST Minimum PGD value to include in inversion (cm): %f",
                lspace, props.pgd_props.minimum_pgd_cm);
        LOG_DEBUGMSG("%s GFAST Maximum PGD value to include in inversion (cm): %f",
                lspace, props.pgd_props.maximum_pgd_cm);
        LOG_DEBUGMSG("%s GFAST Maximum stations to include assoc tag: %d",
                lspace, props.pgd_props.max_assoc_stations);

        // Change thresholds to prevent sending the same (or very similar) solution
        bool change_thresholds_not_set = true;
        if (props.pgd_props.change_threshold_mag >= 0) {
            change_thresholds_not_set = false;
            LOG_DEBUGMSG("%s Change threshold, magnitude: %f",
                lspace, props.pgd_props.change_threshold_mag);
        }
        if (props.pgd_props.change_threshold_mag_uncer >= 0) {
            change_thresholds_not_set = false;
            LOG_DEBUGMSG("%s Change threshold, magnitude uncertainty: %f",
                lspace, props.pgd_props.change_threshold_mag_uncer);
        }
        if (props.pgd_props.change_threshold_lat >= 0) {
            change_thresholds_not_set = false;
            LOG_DEBUGMSG("%s Change threshold, latitude: %f",
                lspace, props.pgd_props.change_threshold_lat);
        }
        if (props.pgd_props.change_threshold_lon >= 0) {
            change_thresholds_not_set = false;
            LOG_DEBUGMSG("%s Change threshold, longitude: %f",
                lspace, props.pgd_props.change_threshold_lon);
        }
        if (props.pgd_props.change_threshold_orig_time >= 0) {
            change_thresholds_not_set = false;
            LOG_DEBUGMSG("%s Change threshold, origin time: %f (s)",
                lspace, props.pgd_props.change_threshold_orig_time);
        }
        if (props.pgd_props.change_threshold_num_stations >= 0) {
            change_thresholds_not_set = false;
            LOG_DEBUGMSG("%s Change threshold, number of stations: %d",
                lspace, props.pgd_props.change_threshold_num_stations);
        }
        if (change_thresholds_not_set) {
            LOG_DEBUGMSG("%s Change thresholds not set!", lspace);
        }
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will not calculate PGD", lspace);
    }
    
    //--------------------------------cmt-------------------------------------//
    if (props.cmt_props.do_cmt) 
    {
        LOG_DEBUGMSG("%s GFAST will calculate CMT", lspace);
        LOG_DEBUGMSG("%s GFAST Number of latitudes in CMT grid search %d",
                lspace, props.cmt_props.ngridSearch_lats);
        LOG_DEBUGMSG("%s GFAST Number of longitudes in CMT grid search %d",
                lspace, props.cmt_props.ngridSearch_lons);
        LOG_DEBUGMSG("%s GFAST Number of depths in CMT grid search  %d",
                lspace, props.cmt_props.ngridSearch_deps);
        LOG_DEBUGMSG("%s GFAST CMT data selection velocity is %f (km/s)",
                lspace, props.cmt_props.window_vel);
        LOG_DEBUGMSG("%s GFAST CMT data averaging window length %f (s)",
                lspace, props.cmt_props.window_avg);
        LOG_DEBUGMSG("%s GFAST Number of sites required to compute CMT is %d",
                lspace, props.cmt_props.min_sites); 
        if (props.cmt_props.ldeviatoric)
        {
            LOG_DEBUGMSG("%s GFAST will apply deviatoric constraint to CMT",
                    lspace);
        }
        else
        {
            LOG_DEBUGMSG("%s GFAST will invert for all 6 MT terms", lspace);
        }
        if (props.cmt_props.ngridSearch_lats > 1)
        {   
            LOG_DEBUGMSG("%s GFAST CMT latitude grid spacing %f",
                    lspace, props.cmt_props.dLat); 
        }   
        if (props.cmt_props.ngridSearch_lons > 1)
        {   
            LOG_DEBUGMSG("%s GFAST CMT longitude grid spacing %f",
                    lspace, props.cmt_props.dLon);
        }
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will not calculate CMT", lspace);
    }
    //--------------------------------ff--------------------------------------//
    if (props.ff_props.do_ff) 
    {
        LOG_DEBUGMSG("%s GFAST will calculate FF", lspace);
        LOG_DEBUGMSG("%s GFAST will use %d fault patches along strike",
                    lspace, props.ff_props.nstr);
        LOG_DEBUGMSG("%s GFAST will use %d fault patches down-dip",
                    lspace, props.ff_props.ndip);
        LOG_DEBUGMSG("%s GFAST Number of sites required to compute FF is %d",
                    lspace, props.ff_props.min_sites);
        LOG_DEBUGMSG("%s GFAST finite fault data selection velocity is %f (km/s)",
                    lspace, props.ff_props.window_vel);
        LOG_DEBUGMSG("%s GFAST finite fault data averaging window length %f (s)",
                    lspace, props.ff_props.window_avg);
        LOG_DEBUGMSG("%s GFAST fault length safety factor %.2f pct",
                    lspace, props.ff_props.flen_pct);
        LOG_DEBUGMSG("%s GFAST fault width safety factor %.2f pct",
                    lspace, props.ff_props.fwid_pct); 
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will not calculate FF", lspace);
    }
    return;
}
