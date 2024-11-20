#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_core.h"
#include "iscl/time/time.h"

/*!
 * @brief PGD scaling full grid search (lat, lon, depth) driver.
 *
 * @param[in] l1               Number of sites.
 * @param[in] ndeps            Number of source depths.
 * @param[in] nlats            Number of latitudes in grid search.
 * @param[in] nlons            Number of longitudes in grid search.
 * @param[in] verbose          Controls verbosity (< 2 is quiet).
 * @param[in] dist_tol         Displacement tolerance (cm).  If the displacement
 *                             is less than dist_tol it will be set to dist_tol.
 * @param[in] disp_def         Displacement default (cm) if d < dist_tol.
 * @param[in] utmSrcNorthings  Source UTM northing positions (m).  This is an 
 *                             array of dimension [nlat*nlon] with leading
 *                             dimension nlats.
 * @param[in] utmSrcEastings   Source UTM easting positions (m).  This is an 
 *                             array of dimension [nlat*nlon] with leading
 *                             dimension nlats..
 * @param[in] srcDepths        Source depth in grid search (km).  This is
 *                             an array of dimension [ndeps].
 * @param[in] utmRecvNorthing  Receiver UTM northing position (m).  This is
 *                             an array of dimension [l1].
 * @param[in] utmRecvEasting   Receiver UTM easting position (m).  This is
 *                             an array of dimension [l1].
 * @param[in] staAlt           Station elevation (m).  This is an array of
 *                             dimension [l1].
 * @param[in] d                Site peak ground displacements (cm).  This is
 *                             an array of dimension [l1].
 * @param[in] wts              Data weights on each observation.  Nominally,
 *                             this is an array of dimension [l1].  However,
 *                             if NULL or if each weight is the same then
 *                             this array will be ignored.
 *
 * @param[out] srdist          Source receiver distance between the
 *                             idep'th source and k'th receiver.  This is 
 *                             an array of length [l1*ndeps*nlat*nlon].
 *                             The i'th site at the idep'th depth is given by
 *                             ilon*nlats*ndeps*l1 + ilat*ndeps*l1 + idep*l1 + i
 * @param[out] M               Magnitude at each depth.  This is an array of
 *                             length [ndeps*nlat*nlon].
 *                             The (ilat,ilon,idep)'th is given by 
 *                             ilon*nlats*ndeps + ilat*ndeps + idep.
 * @param[out] VR              Variance reduction (percentage) at each
 *                             depth.  This is an array of length
 *                             [ndeps*nlat*nlon].
 *                             The (ilat,ilon,idep)'th is given by 
 *                             ilon*nlats*ndeps + ilat*ndeps + idep.
 * @param[out] iqr             The interquartile range computed from the
 *                             difference of the 75th percentile of the 
 *                             weighted residuals and the 25th percentile
 *                             of the weighted residuals at each depth [ndeps].
 *                             The (ilat,ilon,idepth)idep'th is given by 
 *                             ilon*nlats*ndeps + ilat*ndeps + idep.
 * @param[out] Uest            The PGD estimate peak ground displacements.  This
 *                             is an array of dimension [nlons*nlats*ndeps*l1].
 *                             The i'th estimate at the idep'th depth is given by
 *                             ilon*nlats*ndeps*l1 + ilat*ndeps*l1 + idep*l1 + i.
 *
 * @result 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
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
                                double *__restrict__ Uest)
{
    int ierr, ierr1, ilat, ilatLon, ilon;
    // Error check
    if (l1 < 1 || ndeps < 1 || nlats < 1 || nlons < 1)
    {
        if (l1 < 1)
        {
            LOG_ERRMSG("Error invalid number of input stations: %d", l1);
        }
        if (ndeps < 1)
        {
            LOG_ERRMSG("Error invalid number of source depths: %d", ndeps);
        }
        if (nlons < 1)
        {
            LOG_ERRMSG("Error invalid number of lons (easting): %d", nlons);
        }
        if (nlats < 1)
        {
            LOG_ERRMSG("Error invalid number of lats (northings): %d", nlats);
        }
        return -1;
    }
    if (utmSrcEastings == NULL || utmSrcNorthings == NULL ||
        srcDepths == NULL || utmRecvEasting == NULL ||
        utmRecvNorthing == NULL || staAlt == NULL ||
        d == NULL || M == NULL || VR == NULL || Uest == NULL || srdist == NULL)
    {
        if (utmSrcEastings == NULL)
        {
            LOG_ERRMSG("%s", "utmSrcEastings is NULL!");
        }
        if (utmSrcNorthings == NULL)
        {
            LOG_ERRMSG("%s", "utmSrcNorthings is NULL!");
        }
        if (srcDepths == NULL){LOG_ERRMSG("%s", "srcDepths is NULL!");}
        if (utmRecvEasting == NULL)
        {
            LOG_ERRMSG("%s", "utmRecvEasting is NULL!");
        }
        if (utmRecvNorthing == NULL)
        {
            LOG_ERRMSG("%s", "utmRecvNorthing is NULL!");
        }
        if (staAlt == NULL){LOG_ERRMSG("%s", "staAlt is NULL");}
        if (d == NULL){LOG_ERRMSG("%s", "d is NULL");}
        if (M == NULL){LOG_ERRMSG("%s", "M is NULL");}
        if (VR == NULL){LOG_ERRMSG("%s", "VR is NULL");}
        if (Uest == NULL){LOG_ERRMSG("%s", "Uest is NULL");}
        if (srdist == NULL){LOG_ERRMSG("%s", "srdist is NULL");}
        return -1;
    }
    // Loop on the longitudes (eastings)
    ierr = 0;
#ifdef PARALLEL_PGD
    #pragma omp parallel for collapse(2) \
     private(ilatLon, ilat, ilon) \
     reduction(+:ierr), shared(srdist, M, VR, iqr, Uest) \
     default(none)
#endif
    for (ilon = 0; ilon < nlons; ilon++)
    {
        // Scan through the latitudes (northings)
        for (ilat = 0; ilat < nlats; ilat++)
        {
            ilatLon = ilon * nlats + ilat;
            ierr1 = core_scaling_pgd_depthGridSearch(l1, ndeps,
                                                     verbose,
                                                     dist_tol,
                                                     disp_def,
                                                     utmSrcEastings[ilatLon],
                                                     utmSrcNorthings[ilatLon],
                                                     srcDepths,
                                                     utmRecvEasting,
                                                     utmRecvNorthing,
                                                     staAlt,
                                                     d,
                                                     wts,
                                                     &srdist[ilatLon*ndeps*l1],
                                                     &M[ilatLon*ndeps],
                                                     &VR[ilatLon*ndeps],
                                                     &iqr[ilatLon*ndeps],
                                                     &Uest[ilatLon*ndeps*l1]);
            if (ierr1 != 0)
            {
                LOG_ERRMSG("Error calling depthGridSearch %d %d", ilat, ilon);
                ierr = ierr + 1;
            }
        } // loop on latitudes 
    } // loop on longitudes
    // Check if i encountered an error
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error in gridsearch");
        return -2;
    }
    return 0;
}
