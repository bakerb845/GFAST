#include <math.h>
#include <stdbool.h>
#include "gfast_struct.h"
#include "gfast_core.h"


/*
 * Return true if the change thresholds are met or exceeded.
 * For each change threshold, the threshold value must be >= 0 to be evaluated
 * In other words, if the threshold is negative, it won't be used
 * If any single threshold is met or exceeded, return true
 */
bool eewUtils_changeThresholdsExceeded(
    const struct GFAST_props_struct *props,
    const struct coreInfo_struct *core,
    const struct coreInfo_struct *last_sent_core)
{
    // If all of the thresholds are unset, just ignore the whole check and return true.
    if ((props->pgd_props.change_threshold_mag < 0) &&
        (props->pgd_props.change_threshold_mag_uncer < 0) &&
        (props->pgd_props.change_threshold_lat < 0) &&
        (props->pgd_props.change_threshold_lon < 0) &&
        (props->pgd_props.change_threshold_orig_time < 0) &&
        (props->pgd_props.change_threshold_num_stations < 0))
    {
        LOG_DEBUGMSG("%s: All change thresholds unset, not checking!", __func__);
        return true;
    }

    // Check magnitude
    if ((props->pgd_props.change_threshold_mag >= 0) &&
        (fabs(core->mag - last_sent_core->mag) >= props->pgd_props.change_threshold_mag))
    {
        LOG_DEBUGMSG("%s: Returning true for mag: %f >= %f",
                     __func__,
                     fabs(core->mag - last_sent_core->mag),
                     props->pgd_props.change_threshold_mag);
        return true;
    }
    // Check magnitude uncertainty
    if ((props->pgd_props.change_threshold_mag_uncer >= 0) &&
        (fabs(core->magUncer - last_sent_core->magUncer) >= props->pgd_props.change_threshold_mag_uncer))
    {
        LOG_DEBUGMSG("%s: Returning true for mag uncer: %f >= %f",
                     __func__,
                     fabs(core->magUncer - last_sent_core->magUncer),
                     props->pgd_props.change_threshold_mag_uncer);
        return true;
    }
    // Check latitude
    if ((props->pgd_props.change_threshold_lat >= 0) &&
        (fabs(core->lat - last_sent_core->lat) >= props->pgd_props.change_threshold_lat))
    {
        LOG_DEBUGMSG("%s: Returning true for lat: %f >= %f",
                     __func__,
                     fabs(core->lat - last_sent_core->lat),
                     props->pgd_props.change_threshold_lat);
        return true;
    }
    // Check longitude
    if ((props->pgd_props.change_threshold_lon >= 0) &&
        (fabs(core->lon - last_sent_core->lon) >= props->pgd_props.change_threshold_lon))
    {
        LOG_DEBUGMSG("%s: Returning true for lon: %f >= %f",
                     __func__,
                     fabs(core->lon - last_sent_core->lon),
                     props->pgd_props.change_threshold_lon);
        return true;
    }
    // Check origin time
    if ((props->pgd_props.change_threshold_orig_time >= 0) &&
        (fabs(core->origTime - last_sent_core->origTime) >= props->pgd_props.change_threshold_orig_time))
    {
        LOG_DEBUGMSG("%s: Returning true for ot: %f >= %f",
                     __func__,
                     fabs(core->origTime - last_sent_core->origTime),
                     props->pgd_props.change_threshold_orig_time);
        return true;
    }
    // Check number of stations
    if ((props->pgd_props.change_threshold_num_stations >= 0) &&
        (abs(core->numStations - last_sent_core->numStations) >= props->pgd_props.change_threshold_num_stations))
    {
        LOG_DEBUGMSG("%s: Returning true for num stations: %d >= %d",
                     __func__,
                     abs(core->numStations - last_sent_core->numStations),
                     props->pgd_props.change_threshold_num_stations);
        return true;
    }

    // If none of the thresholds are exceeded, return false
    LOG_DEBUGMSG("%s: Returning false, no thresholds met", __func__);
    return false;
}

/*
 * Return true if this message should not be sent (false if it should be sent)
 */
bool eewUtils_sendXMLFilter(
    const struct GFAST_props_struct *props,
    const struct GFAST_shakeAlert_struct *SA,
    const struct GFAST_pgdResults_struct *pgd,
    const struct GFAST_peakDisplacementData_struct *pgd_data,
    const struct coreInfo_struct *core,
    const struct coreInfo_struct *last_sent_core,
    const double age_of_event) 
{

    // Conditions can be met or exceeded.
    // Exceeded is used if a value being more than a threshold means no throttling
    // Met is used if a value being less than a threshold means no throttling 
    bool pgd_exceeded = false;
    bool mag_exceeded = false;
    bool mag_sigma_met = false;
    bool change_thresholds_exceeded = false;

    // Determine if pgd threshold is exceeded n times
    int num_pgd_exceeded = 0, i, i_throttle;

    // Find the correct throttle criteria based on the time after origin
    i_throttle = -1;
    for (i = 0; i < props->pgd_props.n_throttle; i++) {
        if (props->pgd_props.throttle_time_threshold[i] > age_of_event) break;
        i_throttle++;
    }
    i_throttle = (i_throttle < 0) ? 0: i_throttle;
    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: For age_of_event %.2f, using i_throttle=%d, thresholds for time=%.1f, pgd=%.1f, nsta=%d",
            __func__,
            age_of_event,
            i_throttle,
            props->pgd_props.throttle_time_threshold[i_throttle],
            props->pgd_props.throttle_pgd_threshold[i_throttle],
            props->pgd_props.throttle_num_stations[i_throttle])
    }

    // Assumes pgd->nsites = pgd_data->nsites and indices correspond
    if (pgd->nsites != pgd_data->nsites) {
        LOG_ERRMSG("%s: nsites don't match for pgd, pgd_data! %d, %d\n", 
            __func__, pgd->nsites, pgd_data->nsites);
        return false;
    }
    for (i = 0; i < pgd->nsites; i++) {
        // skip site if it wasn't used
        if (!pgd->lsiteUsed[i]) { continue; }
        // pd is in meters, so convert to cm before comparing to threshold
        if (pgd_data->pd[i] * 100. >= props->pgd_props.throttle_pgd_threshold[i_throttle]) {
            num_pgd_exceeded++;
        }
    }

    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: PGD threshold of %.1f cm exceeded at %d stations (threshold num: %d)",
            __func__, props->pgd_props.throttle_pgd_threshold[i_throttle], num_pgd_exceeded,
            props->pgd_props.throttle_num_stations[i_throttle]);
    }
    if (num_pgd_exceeded >= props->pgd_props.throttle_num_stations[i_throttle]) {
        LOG_MSG("%s: PGD threshold of %.1f cm exceeded at %d stations (threshold num: %d)",
            __func__, props->pgd_props.throttle_pgd_threshold[i_throttle], num_pgd_exceeded,
            props->pgd_props.throttle_num_stations[i_throttle]);
        pgd_exceeded = true;
    }

    // Determine if SA mag threshold is exceeded
    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: SA mag: %f, threshold mag: %f",
            __func__, SA->mag, props->pgd_props.SA_mag_threshold);
    }
    if (SA->mag >= props->pgd_props.SA_mag_threshold) {
        mag_exceeded = true;
        LOG_MSG("%s: SA magnitude exceeded! SA mag: %f, threshold mag: %f",
            __func__, SA->mag, props->pgd_props.SA_mag_threshold);
    }

    // Determine if pgd magnitude sigma threshold is met
    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: PGD mag sigma: %f, threshold mag sigma: %f, PGD mag: %f",
            __func__, core->magUncer, props->pgd_props.pgd_sigma_throttle, core->mag);
    }
    if (core->magUncer <= props->pgd_props.pgd_sigma_throttle) {
        mag_sigma_met = true;
        LOG_MSG("%s: PGD mag sigma met! PGD mag sigma: %f, threshold mag sigma: %f, PGD mag: %f",
            __func__, core->magUncer, props->pgd_props.pgd_sigma_throttle, core->mag);
    }

    // Check change thresholds exceeded from the last sent message (first one automatically counts)
    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: PGD change thresholds -> dmag:%f, dmagU:%f, dlat:%f, dlon:%f, dot:%f, dnsta:%d",
            __func__,
            core->mag - last_sent_core->mag,
            core->magUncer - last_sent_core->magUncer,
            core->lat - last_sent_core->lat,
            core->lon - last_sent_core->lon,
            core->origTime - last_sent_core->origTime,
            core->numStations - last_sent_core->numStations
            );
    }
    if (eewUtils_changeThresholdsExceeded(props, core, last_sent_core)) {
        change_thresholds_exceeded = true;
        LOG_MSG("%s: PGD change thresholds exceeded! dmag:%f, dmagU:%f, dlat:%f, dlon:%f, dot:%f, dnsta:%d",
            __func__,
            core->mag - last_sent_core->mag,
            core->magUncer - last_sent_core->magUncer,
            core->lat - last_sent_core->lat,
            core->lon - last_sent_core->lon,
            core->origTime - last_sent_core->origTime,
            core->numStations - last_sent_core->numStations
            );
    }

    if (pgd_exceeded && mag_exceeded && mag_sigma_met && change_thresholds_exceeded) {
        LOG_MSG("%s: Message not throttled (%s v%d)! pgd_exceeded: %d, mag_exceeded: %d, mag_sigma_met: %d, change_thresholds_exceeded: %d",
            __func__, core->id, core->version, pgd_exceeded, mag_exceeded, mag_sigma_met, change_thresholds_exceeded);
        return false;
    }

    LOG_MSG("%s: Message throttled (%s v%d)! pgd_exceeded: %d, mag_exceeded: %d, mag_sigma_met: %d, change_thresholds_exceeded: %d",
        __func__, core->id, core->version, pgd_exceeded, mag_exceeded, mag_sigma_met, change_thresholds_exceeded);
    return true;
}