#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "gfast_eewUtils.h"
#include "gfast_core.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Driver for estimating earthquake magnitude from peak
 *        ground displacement
 *
 * @param[in] pgd_props  PGD inversion parameters
 * @param[in] SA_lat     event latitude (degrees) [-90,90]
 * @param[in] SA_lon     event longitude (degrees) [0,360]
 * @param[in] SA_dep     event depth (km)
 * @param[in] age_of_event time since event origin time (s)
 * @param[in] pgd_data   data structure holding the peak ground displacement
 *                       data, weights, and logical mask indicating site is
 *                       a candidate for inversion
 *
 * @param[out] pgd       results of the PGD estimation grid-search, 
 *                       the variance reduction at each depth in the grid
 *                       search, and the sites used in the inversion
 *
 * @result 0 indicates success
 *         1 indicates an error on the input pgd structure
 *         2 indicates error in input pgd_data structure
 *         3 indicates insufficient data for inversion
 *         4 indicates an error in computation
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */ 
int eewUtils_drivePGD(const struct GFAST_pgd_props_struct pgd_props,
                      const double SA_lat,
                      const double SA_lon,
                      const double SA_dep,
                      const double age_of_event,
                      struct GFAST_peakDisplacementData_struct pgd_data,
                      struct GFAST_pgdResults_struct *pgd)
{
    double *d, *srdist, *staAlt, *Uest, *utmRecvEasting, *utmRecvNorthing, *wts,
           iqrMin, *utmSrcEastings, *utmSrcNorthings, x1, x2, y1, y2;
    int i, indx, idep, ilat, ilon, ilatLon, iloc, ierr, j, k, l1, nlatlon, nloc, zone_loc;
    bool *luse, lnorthp;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = PGD_SUCCESS;
    d = NULL;
    wts = NULL;
    utmRecvNorthing = NULL;
    utmRecvEasting = NULL;
    staAlt = NULL;
    Uest = NULL;
    srdist = NULL;
    luse = NULL;
    // Verify the input data structure makes sense
    if (pgd_data.nsites < 1)
    {
        ierr = PGD_PD_DATA_ERROR;
        if (pgd_props.verbose > 1)
        {
            LOG_WARNMSG("%s", "No peak displacement data");
        }
        goto ERROR;
    }
    if (pgd_data.pd == NULL || pgd_data.wt == NULL ||
        pgd_data.lmask == NULL || pgd_data.lactive == NULL)
    {
        ierr = PGD_PD_DATA_ERROR;
        if (pgd_data.pd == NULL)
        {
            LOG_ERRMSG("%s", "pgd_data.pd is NULL");
        }
        if (pgd_data.wt == NULL)
        {
            LOG_ERRMSG("%s", "pgd_data.wt is NULL");
        }
        if (pgd_data.lactive == NULL)
        {
            LOG_ERRMSG("%s", "pgd_data.lactive is NULL");
        }
        if (pgd_data.lmask == NULL)
        {
            LOG_ERRMSG("%s", "pgd_data.lmask is NULL");
        }
        goto ERROR;
    }
    // Verify the output data structures 
    if (pgd->ndeps < 1)
    {
        LOG_ERRMSG("%s", "No depths in PGD gridsearch!");
        ierr = PGD_STRUCT_ERROR;
        goto ERROR;
    }
    if (pgd->mpgd == NULL || pgd->mpgd_vr == NULL ||
        pgd->srcDepths == NULL || pgd->UP == NULL ||
        pgd->UPinp == NULL || pgd->srdist == NULL ||
        pgd->lsiteUsed == NULL)
    {
        if (pgd->mpgd == NULL)
        {
            LOG_ERRMSG("%s", "pgd->mpgd is NULL");
        }
        if (pgd->mpgd_vr == NULL)
        {
            LOG_ERRMSG("%s", "pgd->mpgd_vr is NULL");
        }
        if (pgd->srcDepths == NULL)
        {
            LOG_ERRMSG("%s", "pgd->srcDepths is NULL");
        }
        if (pgd->UP == NULL)
        {
            LOG_ERRMSG("%s", "pgd->UP is NULL");
        }
        if (pgd->UPinp == NULL)
        {
            LOG_ERRMSG("%s", "pgd->UPinp is NULL");
        }
        if (pgd->srdist == NULL)
        {
            LOG_ERRMSG("%s", "pgd->srdist is NULL");
        }
        if (pgd->lsiteUsed == NULL)
        {
            LOG_ERRMSG("%s", "pgd->lsiteUsed is NULL");
        }
        ierr = PGD_STRUCT_ERROR;
        goto ERROR;
    }
    // Avoid a segfault
    if (pgd->nsites != pgd_data.nsites)
    {
        LOG_ERRMSG("nsites on pgd and pgd_data differs %d %d\n",
                   pgd->nsites, pgd_data.nsites);
        ierr = PGD_STRUCT_ERROR;
        goto ERROR;
    }
    // Warn in case hypocenter is outside of grid-search
    if (pgd_props.verbose > 1 &&
        (SA_dep < pgd->srcDepths[0] || SA_dep > pgd->srcDepths[pgd->ndeps-1]))
    {
        LOG_WARNMSG("%s", "Warning hypocenter isn't in grid search!");
    }
    // Null out results
    nloc = pgd->ndeps * pgd->nlats * pgd->nlons;
    pgd->opt_indx = -1;
    pgd->opt_dep = __FLT_MIN__;
    pgd->opt_lat = __FLT_MIN__;
    pgd->opt_lon = __FLT_MIN__;
    array_zeros64f_work(pgd->nsites, pgd->UPinp);
    array_zeros8l_work( pgd->nsites, pgd->lsiteUsed);
    array_zeros64f_work(nloc, pgd->mpgd);
    array_zeros64f_work(nloc, pgd->mpgd_sigma);
    array_zeros64f_work(nloc, pgd->mpgd_vr);
    array_zeros64f_work(nloc, pgd->dep_vr_pgd);
    array_zeros64f_work(nloc, pgd->iqr);
    array_zeros64f_work(pgd->nsites*nloc, pgd->UP);
    array_zeros64f_work(pgd->nsites*nloc, pgd->srdist);
    // Require there is a sufficient amount of data to invert
    luse = memory_calloc8l(pgd_data.nsites);
    l1 = 0;
    for (k=0; k<pgd_data.nsites; k++)
    {
        if (!pgd_data.lactive[k] || pgd_data.wt[k] <= 0.0){continue;}
        luse[k] = true;
        l1 = l1 + 1;
    }
    if (l1 < pgd_props.min_sites)
    {
        if (pgd_props.verbose > 1)
        {
            LOG_WARNMSG("Insufficient data to invert %d < %d\n",
                        l1, pgd_props.min_sites);
        }
        ierr = PGD_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Allocate space
    nlatlon = pgd->nlats * pgd->nlons;
    d               = memory_calloc64f(l1);
    utmRecvNorthing = memory_calloc64f(l1);
    utmRecvEasting  = memory_calloc64f(l1);
    staAlt          = memory_calloc64f(l1);
    wts             = memory_calloc64f(l1);
    Uest            = memory_calloc64f(l1*nloc);
    srdist          = memory_calloc64f(l1*nloc);
    utmSrcNorthings = memory_calloc64f(nlatlon);
    utmSrcEastings  = memory_calloc64f(nlatlon);
    // Get the source location
    zone_loc = pgd_props.utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;} // Estimate UTM zone from source lon
    for (ilon = 0; ilon < pgd->nlons; ilon++) {
        for (ilat = 0; ilat < pgd->nlats; ilat++) {
            ilatLon = ilon * pgd->nlats + ilat;
            core_coordtools_ll2utm(SA_lat + pgd->srcLats[ilat],
                                   SA_lon + pgd->srcLons[ilon],
                                   &y1, &x1,
                                   &lnorthp, &zone_loc);
            utmSrcNorthings[ilatLon] = y1; 
            utmSrcEastings[ilatLon] = x1;
        }
    }
    // Loop on the receivers, get distances, and data
    l1 = 0;
    for (k = 0; k < pgd_data.nsites; k++)
    {   
        if (!pgd_data.lactive[k] || pgd_data.wt[k] <= 0.0){continue;}
        // Get the recevier UTM
        core_coordtools_ll2utm(pgd_data.sta_lat[k],
                               pgd_data.sta_lon[k],
                               &y2, &x2,
                               &lnorthp, &zone_loc);
        // Copy information to data structures for grid search
        d[l1] = pgd_data.pd[k] * 100.0; // convert peak ground displacement to cm
        wts[l1] = pgd_data.wt[k];
        utmRecvNorthing[l1] = y2;
        utmRecvEasting[l1] = x2;
        staAlt[l1] = pgd_data.sta_alt[k];
        l1 = l1 + 1;
    } // Loop on data
    // Invert!
    if (pgd_props.verbose > 2)
    {   
        LOG_DEBUGMSG("Inverting for PGD with %d sites", l1);
    }   
    ierr = core_scaling_pgd_gridSearch(l1, pgd->ndeps, pgd->nlats, pgd->nlons,
                                       pgd_props.verbose,
                                       pgd_props.dist_tol,
                                       pgd_props.disp_def,
                                       utmSrcEastings,
                                       utmSrcNorthings,
                                       pgd->srcDepths,
                                       utmRecvEasting,
                                       utmRecvNorthing,
                                       staAlt,
                                       d,
                                       wts,
                                       srdist,
                                       pgd->mpgd,
                                       pgd->mpgd_vr,
                                       pgd->iqr,
                                       Uest);
    if (ierr != 0)
    {   
        if (pgd_props.verbose > 0)
        {
            LOG_ERRMSG("%s", "Error in PGD grid search!");
        }
        ierr = PGD_COMPUTE_ERROR;
    }
    // Extract observations
    k = 0;
    for (i=0; i<pgd->nsites; i++)
    {
        pgd->lsiteUsed[i] = luse[i];
        if (!luse[i]){continue;}
        pgd->UPinp[i] = d[k];
        k = k + 1;
    }
    { /*vk needed for more stringent c++ compiler*/
      enum isclError_enum isclerr = (enum isclError_enum)ierr;
      iqrMin = array_min64f(nloc, pgd->iqr, &isclerr);
    }
    // Extract the estimates and compute weighted objective function
    // Also add uncertainty estimate
    // First get last index value before lookup time is greater than age_of_event
    int i99;
    for (i99 = 0; (i99 < pgd_props.n99) && (pgd_props.t99[i99] <= age_of_event); i99++) {}
    i99 = (i99 <= 0) ? 0 : i99 - 1;
    // for (iloc = 0; iloc < nloc; iloc++)
    // {

    for (ilon = 0; ilon < pgd->nlons; ilon++)
    {
        for (ilat = 0; ilat < pgd->nlats; ilat++)
        {
            for (idep = 0; idep < pgd->ndeps; idep++)
            {
                // Get location in arrays
                indx = ilon * pgd->ndeps * pgd->nlats
                     + ilat * pgd->ndeps 
                     + idep;

                if (pgd_props.n99 == 0) {
                    pgd->mpgd_sigma[indx] = 0.5;
                } else {
                    pgd->mpgd_sigma[indx] = 0.5 * exp(pgd_props.m99[i99] - pgd->mpgd[indx]);
                }
                pgd->dep_vr_pgd[indx] = pgd->mpgd[indx] * iqrMin / pgd->iqr[indx];
                j = 0;
                for (i = 0; i < pgd->nsites; i++)
                {
                    pgd->UP[indx * pgd->nsites + i] = 0.0;
                    pgd->srdist[indx * pgd->nsites + i] = 0.0;
                    if (luse[i])
                    {
                        pgd->UP[indx * pgd->nsites + i] = Uest[indx * l1 + j];
                        pgd->srdist[indx * pgd->nsites + i] = srdist[indx * l1 + j];
                        j = j + 1;
                    }
                }

                // print results
                if (pgd_props.verbose > 2) {
                    LOG_DEBUGMSG("PGD results (%d) for %.4f, %.4f, %.1f: Mpgd=%.3f, Mpgd_sigma=%.3f, dep_vr_pgd=%.3f",
                        indx, pgd->srcLats[ilat] + SA_lat, pgd->srcLons[ilon]+ SA_lon, pgd->srcDepths[idep],
                        pgd->mpgd[indx], pgd->mpgd_sigma[indx], pgd->dep_vr_pgd[indx]);
                }
            }
        }
    }

    { /*vk needed for more stringent c++ compiler*/
      enum isclError_enum isclerr = (enum isclError_enum)ierr;
      pgd->opt_indx = array_argmin64f(nloc, pgd->dep_vr_pgd, &isclerr); 
    }
    // Unpack opt_indx for lat, lon, depth
    int rem;
    idep = pgd->opt_indx % pgd->ndeps;
    rem = (pgd->opt_indx - idep) / pgd->ndeps;
    ilat = rem % pgd->nlats;
    ilon = ((rem - ilat) / pgd->nlats) % pgd->nlons;

    pgd->opt_dep = pgd->srcDepths[idep];
    pgd->opt_lat = pgd->srcLats[ilat] + SA_lat;
    pgd->opt_lon = pgd->srcLons[ilon] + SA_lon;

ERROR:;
    memory_free64f(&d);
    memory_free64f(&utmRecvNorthing);
    memory_free64f(&utmRecvEasting);
    memory_free64f(&staAlt);
    memory_free64f(&wts);
    memory_free64f(&Uest);
    memory_free64f(&srdist);
    memory_free8l(&luse);
    return ierr;
}

