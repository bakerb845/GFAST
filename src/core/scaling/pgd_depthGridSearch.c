#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include <omp.h>
#include "gfast_core.h"
#include "iscl/linalg/linalg.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"
#include "iscl/statistics/statistics.h"

/*!
 * @brief Computes the predicted mangitude using PGD and Pd from
 *        the geodetic data by solving the overdetermined system:
 *         \f[ 
 *            W \left [B + C \log_{10}(r) \right ] \textbf{m}
 *          = W \left \{ \log_{10}(d) - A \right \}
 *         \f] 
 *        for the the magnitude m.  Here r is the hypocentral distance,
 *        d is the displacement, A, B, and C are defined in the routine. 
 *        The weighting matrix is a diagonal matrix defined by: 
 *          \f[ W = diag
 *                  \left[
 *                     e^{-\frac{\Delta^2}{8 \min(\Delta^2) }}
 *                  \right ] \f]
 *        where \f$ \Delta \f$ is the epicentral distance.
 *
 * @param[in] l1               number of sites
 * @param[in] ndeps            number of source depths
 * @param[in] verbose          controls verbosity (< 2 is quiet)
 * @param[in] dist_tol         displacement tolerance (cm).  if the displacment
 *                             is less than dist_tol it would be set to dist_tol
 * @param[in] disp_def         displacement default (cm) if d < dist_tol
 * @param[in] utmSrcNorthing   source UTM northing position (m)
 * @param[in] utmSrcEasting    source UTM easting position (m)
 * @param[in] srcDepths        source depth in grid search (km) [ndeps]
 * @param[in] utmRecvNorthing  receiver UTM northing position (m) [l1]
 * @param[in] utmRecvEasting   receiver UTM easting position (m) [l1]
 * @param[in] staAlt           station elevation (m) [l1]
 * @param[in] d                site peak ground displacements (cm) [l1]
 * @param[in] wts              data weights on each observation [l1].
 *                             if NULL or if each weight is the same then
 *                             this array will be ignored.
 *
 * @param[out] M               magnitude at each depth [ndeps]
 * @param[out] VR              variance reduction (percentage) at each
 *                             depth [ndeps]
 * @param[out] iqr75_25        the interquartile range computed from the
 *                             difference of the 75th percentile of the 
 *                             weighted residuals and the 25th percentile
 *                             of the weighted residuals at each depth [ndeps]
 * @param[out] Uest            the PGD estimate peak ground displacements.
 *                             the estimate for the i'th site at the
 *                             idep'th depth is access by idep*l1 + i [l1*ndeps]
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
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
                                     double *__restrict__ M,
                                     double *__restrict__ VR,
                                     double *__restrict__ iqr75_25,
                                     double *__restrict__ Uest)
{
    const char *fcnm = "core_scaling_pgd_depthGridSearch\0";
    double *b, *G, *r, *repi, *UP, *W, *Wb, *WG, *wres, pct[2], M1[1],
           est, srcDepth, xden, xnum;
    int i, idep, ierr, ierr1;
    const double A = -6.687;
    const double B = 1.500;
    const double C = -0.214;
    const double q[2] = {25.0, 75.0}; 
    const int nq = 2;
    //const double A = -4.434; /* TODO remove with approval */
    //const double B = 1.047;  /* TODO remove with approval */
    //const double C = -0.138; /* TODO remove with approval */
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    repi = NULL;
    r = NULL;
    wres = NULL;
    b = NULL;
    G = NULL;
    UP = NULL;
    W  = NULL;
    Wb = NULL;
    WG = NULL;
    // Error check
    if (l1 < 1)
    {
        log_errorF("%s: Error invalid number of input stations: %d\n",
                   fcnm, l1);
        ierr = 1;
        goto ERROR;
    }
    if (ndeps < 1)
    {
        log_errorF("%s: Error invalid number of source depths: %d\n",
                   fcnm, ndeps);
        ierr = 1;
        goto ERROR;
    }
    if (srcDepths == NULL || utmRecvEasting == NULL || 
        utmRecvNorthing == NULL || staAlt == NULL || 
        d == NULL || M == NULL || VR == NULL || Uest == NULL)
    {
        if (srcDepths == NULL){log_errorF("%s: srcDepths is NULL!\n", fcnm);}
        if (utmRecvEasting == NULL)
        {
            log_errorF("%s: utmRecvEasting is NULL!\n", fcnm);
        }
        if (utmRecvNorthing == NULL)
        {
            log_errorF("%s: utmRecvNorthing is NULL!\n", fcnm);
        }
        if (staAlt == NULL){log_errorF("%s: staAlt is NULL\n", fcnm);}
        if (d == NULL){log_errorF("%s: d is NULL\n", fcnm);}
        if (M == NULL){log_errorF("%s: M is NULL\n", fcnm);}
        if (VR == NULL){log_errorF("%s: VR is NULL\n", fcnm);}
        if (Uest == NULL){log_errorF("%s: Uest is NULL\n", fcnm);}
        ierr = 1;
        goto ERROR;
    }
    // Initialize result to nothing
    #pragma omp simd
    for (idep=0; idep<ndeps; idep++)
    {
        M[idep] = 0.0;
        VR[idep] = 0.0;
    }
    #pragma omp simd
    for (i=0; i<l1*ndeps; i++)
    {
        Uest[i] = 0.0;
    }
    // Set space
    G    = ISCL_memory_calloc__double(l1*1);
    b    = ISCL_memory_calloc__double(l1);
    r    = ISCL_memory_calloc__double(l1);
    W    = ISCL_memory_calloc__double(l1);
    WG   = ISCL_memory_calloc__double(l1*1);
    Wb   = ISCL_memory_calloc__double(l1);
    UP   = ISCL_memory_calloc__double(l1);
    repi = ISCL_memory_calloc__double(l1);
    wres = ISCL_memory_calloc__double(l1);
    // Compute the epicentral distances (km)
    #pragma omp simd
    for (i=0; i<l1; i++)
    {
        repi[i] = sqrt( pow(utmSrcEasting  - utmRecvEasting[i], 2)
                      + pow(utmSrcNorthing - utmRecvNorthing[i], 2) );
        repi[i] = repi[i]*1.e-3; // m -> km
    }
    // Set the RHS log10(d) - A
    ierr = GFAST_core_scaling_pgd_setRHS(l1,
                                         dist_tol, disp_def,
                                         A, d,
                                         b);
    if (ierr != 0)
    {
        log_errorF("%s: Error creating RHS\n", fcnm);
        goto ERROR;
    }
    // Compute the diagonal data weights
    ierr = GFAST_core_scaling_pgd_setDiagonalWeightMatrix(l1,
                                                          repi,
                                                          wts,
                                                          W);
    if (ierr != 0)
    {
        log_errorF("%s: Error setting diagonal weight matrix\n", fcnm);
        goto ERROR;
    }
    // Weight the observations
    ierr = GFAST_core_scaling_pgd_weightObservations(l1,
                                                     W,
                                                     b,
                                                     Wb);
    if (ierr < 0)
    {
        log_errorF("%s: Error weighting observations\n", fcnm);
        goto ERROR;
    }
    // Loop on depths
#ifdef PARALLEL_PGD
    #pragma omp parallel for \
     firstprivate(A, B, C, G, r, UP, verbose, WG, wres) \
     private(i, idep, ierr1, est, M1, pct, srcDepth, xden, xnum) \
     shared(b, d, iqr75_25, fcnm, M, repi, \
            srcDepths, staAlt, utmRecvEasting, utmRecvNorthing, \
            Uest, VR, W, Wb) \
     reduction(+:ierr) default(none)
#endif
    for (idep=0; idep<ndeps; idep++)
    {
        // Compute radial distance
        srcDepth = srcDepths[idep];
        #pragma omp simd aligned(r:CACHE_LINE_SIZE)
        for (i=0; i<l1; i++)
        {
            r[i] = sqrt( pow(utmSrcEasting  - utmRecvEasting[i], 2)
                       + pow(utmSrcNorthing - utmRecvNorthing[i], 2)
                       + pow(srcDepth*1000.0 - staAlt[i], 2) )*1.e-3;
        }
        // Generate the over-determined system: [B + C*log10(r)]*m = RHS 
        ierr1 = GFAST_core_scaling_pgd_setForwardModel(l1,
                                                       B, C, r,
                                                       G);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error creating G matrix\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Weight the forward modeling matrix
        ierr1 = GFAST_core_scaling_pgd_weightForwardModel(l1, 
                                                          W,
                                                          G,
                                                          WG);
        if (ierr1 < 0)
        {
            log_errorF("%s: Error weighting forward modeling matrix\n", fcnm);
            ierr = ierr + 1;
        }
        // Solve the weighted least squares problem (M = lstsq(W*G,W*b)[0])
        ierr1 = __linalg_lstsq__qr(LAPACK_COL_MAJOR,
                                   l1, 1, 1, false, WG, Wb,
                                   M1, NULL);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error solving the least-squares problem\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute the estimates: UP = G*M
        cblas_dgemv(CblasColMajor, CblasNoTrans,
                    l1, 1, 1.0, G, l1, M1, 1, 0.0, UP, 1);
        // Compute the variance reduction
        xnum = 0.0;
        xden = 0.0;
        #pragma omp simd reduction(+:xnum, xden)
        for (i=0; i<l1; i++)
        {
            est = pow(10.0, UP[i] + A);
            Uest[idep*l1+i] = est;
            wres[i] = W[i]*(d[i] - est);
            xnum = xnum + sqrt(wres[i]*wres[i]);
            xden = xden + sqrt(pow(W[i]*d[i], 2));
        }
        // Compute the interquartile range which will later be used as a penalty
        ierr1 = __statistics_percentile__double(l1, wres, nq, q,
                                                STATS_PERCENTILE_LINEAR, pct);
        iqr75_25[idep] = pct[1] - pct[0];
        if (ierr1 != 0)
        {   
            log_errorF("%s: Error computing interquartile range\n", fcnm);
            iqr75_25[idep] = 1.0;
        }
        // Copy results
        M[idep] = M1[0];
        VR[idep] = (1.0 - xnum/xden)*100.0;
    } // Loop on depths
    if (ierr != 0)
    {
        log_errorF("%s: Errors detected during grid search\n", fcnm);
        ierr = 1;
    }
ERROR:; // An error was encountered
    // Free space
    ISCL_memory_free__double(&repi);
    ISCL_memory_free__double(&r);
    ISCL_memory_free__double(&wres);
    ISCL_memory_free__double(&b);
    ISCL_memory_free__double(&G);
    ISCL_memory_free__double(&UP);
    ISCL_memory_free__double(&W);
    ISCL_memory_free__double(&Wb);
    ISCL_memory_free__double(&WG);
    return ierr;
}
