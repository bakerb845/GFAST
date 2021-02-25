#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "gfast_core.h"
#ifdef GFAST_USE_INTEL
 #ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Weverything"
 #endif
 #include <mkl_lapacke.h>
 #include <mkl_cblas.h>
 #ifdef __clang__
  #pragma clang diagnostic pop
 #endif
#else
#include <lapacke.h>
#include <cblas.h>
#endif
#include "iscl/array/array.h"
#include "iscl/linalg/linalg.h"
#include "iscl/time/time.h"
#include "iscl/memory/memory.h"

/*!
 * @brief This performs the grid-search finite-fault slip inversion
 *        for the given fault planes.
 *
 * @note The fault plane arrays (e.g. fault_xutm, fault_yutm, fault_alt,
 *       length, width, strike, dip, sslip, dslip, sslip_unc, dslip_unc)
 *       are packed such that the istr'th, idip'th element on the ifp'th fault
 *       plane is given ifp*l2 + idip*nstr + istr by where l2 = nstr*ndip.
 *       Similarly, the estimate displacement in the NN, EN, and UN
 *       for the i'th site for the ifp'th fault plane is given by 
 *       ifp*l1 + i where l1 is the number of sites in the inversion. 
 *
 * @param[in] l1               number of sites in the inversion. 
 * @param[in] l2               leading dimension for the fault plane
 *                             information.  this must be nstr*ndip.
 * @param[in] nstr             number of elements along strike in each fault
 *                             plane.
 * @param[in] ndip             number of elements down dip in each fault plane.
 * @param[in] nfp              number of fault planes in grid-search
 * @param[in] verbose          controls verbosity (0 will only report on 
 *                             errors)
 * @param[in] nObsOffset       the observed offset (m) in the north component
 *                             for the i'th site [l1]
 * @param[in] eObsOffset       the observed offset (m) in the east component
 *                             for the i'th site [l1]
 * @param[in] uObsOffset       the observed offset (m) in the vertical
 *                             component for the i'th site [l1]
 * @param[in] nWts             data weight on the i'th north observation [l1]
 * @param[in] eWts             data weight on the i'th east observation [l1]
 * @param[in] uWts             data weight on the i'th vertical observation [l1]
 * @param[in] utmRecvEasting   the UTM easting location (m) of the i'th
 *                             site [l1] 
 * @param[in] utmRecvNorthing  the UTM northing location (m) of the i'th
 *                             site [l1]
 * @param[in] staAlt           altitude (m) of i'th site above sea-level [l1]
 * @param[in] fault_xutm       the UTM x (east) location (m) of the if'th
 *                             fault patch on the ifp'th fault plane [l2*nfp].
 * @param[in] fault_yutm       the UTM y (north) location (m) of the if'th
 *                             fault patch on the ifp'th fault plane [l2*nfp].
 * @param[in] fault_alt        depth (km) of the if'th fault patch on the
 *                             ifp'th fault plane [l2*nfp].
 * @param[in] length           the length (m) of the if'th fault patch on the
 *                             ifp'th fault plane [l2*nfp].
 * @param[in] width            the width (m) of the if'th fault patch on the
 *                             ifp'th fault plane [l2*nfp].
 * @param[in] strike           the strike angle (degrees) of the if'th fault
 *                             patch on the ifp'th fault plane [l2*nfp].
 * @param[in] dip              the dip angle (degrees) of the if'th fault
 *                             patch on the ifp'th fault plane [l2*nfp].
 *
 * @param[out] sslip           slip along strike (m) on the if'th fault patch
 *                             on the ifp'th fault plane [l2*nfp]. 
 * @param[out] dslip           slip down dip (m) on the if'th fault patch
 *                             of the ifp'th fault plane [l2*nfp]. 
 * @param[out] Mw              moment magnitude for the event on the ifp'th
 *                             fault plane [nfp].
 * @param[out] vr              the variance reduction for the linearized slip
 *                             inversion on the ifp'th fault plane [nfp]
 * @param[out] NN              the estimated displacement in the north component
 *                             (m) for the i'th site on the ifp'th fault plane
 *                             [l1*nfp].
 * @param[out] EN              the estimated displacement in the east component
 *                             (m) for the i'th site on the ifp'th fault plane
 *                             [11*nfp].
 * @param[out] UN              the estimated displacement in the the vertical
 *                             component (m) for the i'th site on the ifp'th
 *                             fault plane [l1*nfp].
 *
 * @param[in,out] sslip_unc    If not NULL then this is the uncertainty (m)
 *                             estimated from the diagonal of the model
 *                             covariance matrix for the slip along strike on
 *                             the if'th fault patch on the ifp'th fault plane.
 *                             Note, that it must still be multiplied by a 
 *                             confidence level and a standard deviation.
 *                             If sslip_unc is requested then it must be an 
 *                             array of dimension of [l2*nfp].
 * @param[in,out] dslip_unc    If not NULL then this is the uncertainty (m)
 *                             estimated from the diagonal of the model
 *                             covariance matrix for the slip down dip on
 *                             the on the if'th fault patch on the ifp'th
 *                             fault plane.  Note, that it must still be
 *                             multiplied by a confidence level and a 
 *                             standard deviation.  If dslip_unc is requested
 *                             then it must be an array of dimension [l2*nfp].
 *  
 * @result 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
int core_ff_faultPlaneGridSearch(const int l1, const int l2,
                                 const int nstr, const int ndip,
                                 const int nfp, const int verbose,
                                 const double *__restrict__ nObsOffset,
                                 const double *__restrict__ eObsOffset,
                                 const double *__restrict__ uObsOffset,
                                 const double *__restrict__ nWts,
                                 const double *__restrict__ eWts,
                                 const double *__restrict__ uWts,
                                 const double *__restrict__ utmRecvEasting,
                                 const double *__restrict__ utmRecvNorthing,
                                 const double *__restrict__ staAlt,
                                 const double *__restrict__ fault_xutm,
                                 const double *__restrict__ fault_yutm,
                                 const double *__restrict__ fault_alt,
                                 const double *__restrict__ length,
                                 const double *__restrict__ width,
                                 const double *__restrict__ strike,
                                 const double *__restrict__ dip,
                                 double *__restrict__ sslip,
                                 double *__restrict__ dslip,
                                 double *__restrict__ Mw,
                                 double *__restrict__ vr,
                                 double *__restrict__ NN,
                                 double *__restrict__ EN,
                                 double *__restrict__ UN,
                                 double *__restrict__ sslip_unc,
                                 double *__restrict__ dslip_unc
                                 )
{
    double *diagWt, *G, *G2, *R, *S, *T, *UD, *UP, *WUD, *xrs, *yrs, *zrs,
           asum, ds_unc, lampred, len0, ss_unc, st, M0, res, wid0, xden, xnum;
    int i, ierr, ierr1, if_off, ifp, ij, io_off, j,
        mrowsG, mrowsG2, mrowsT, ncolsG, ncolsG2, ncolsT, ng, ng2, nt;
    bool lrmtx, lsslip_unc, ldslip_unc;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    xrs = NULL;
    yrs = NULL;
    zrs = NULL;
    G2 = NULL;
    UD = NULL;
    UP = NULL;
    T  = NULL;
    S  = NULL;
    R  = NULL;
    lrmtx = false;
    lsslip_unc = false;
    ldslip_unc = false;
    // Error handling
    if (l1 <= 0 || l2 <= 0 || l2 != nstr*ndip)
    {
        if (l1 <= 0){LOG_ERRMSG("%s", "Error no observations");}
        if (l2 <= 0){LOG_ERRMSG("%s", "Error no points in fault plane");}
        if (l2 != nstr*ndip)
        {
            LOG_ERRMSG("Error l2 != nstr*ndip %d %d*%d!", l2, nstr, ndip);
        }
        return -1;
    }
    if (utmRecvEasting == NULL || utmRecvNorthing == NULL ||
        fault_xutm == NULL || fault_yutm == NULL ||
        length == NULL || width == NULL || strike == NULL || dip == NULL ||
        sslip == NULL || dslip == NULL)
    {
        if (utmRecvEasting == NULL)
        {
            LOG_ERRMSG("%s", "Error utmRecvEasting is NULL");
        }
        if (utmRecvNorthing == NULL)
        {
            LOG_ERRMSG("%s", "Error utmRecvNorthing is NULL");
        }
        if (fault_xutm == NULL){LOG_ERRMSG("%s", "Error fault_xutm is NULL");}
        if (fault_yutm == NULL){LOG_ERRMSG("%s", "Error fault_yutm is NULL");}
        if (length == NULL){LOG_ERRMSG("%s", "Error length is NULL");}
        if (width == NULL){LOG_ERRMSG("%s", "Error width is NULL");}
        if (strike == NULL){LOG_ERRMSG("%s", "Error strike is NULL");}
        if (dip == NULL){LOG_ERRMSG("%s", "Error dip is NULL");}
        if (sslip == NULL){LOG_ERRMSG("%s", "Error sslip is NULL");}
        if (dslip == NULL){LOG_ERRMSG("%s", "Error dslip is NULL");}
        return -1;
    }
    if (sslip_unc != NULL || dslip_unc != NULL)
    {
        if (sslip_unc != NULL){lsslip_unc = true;}
        if (dslip_unc != NULL){ldslip_unc = true;}
        lrmtx = true;
    } 
    // Compute sizes of matrices in G2 S = UP where G2 = [G; T]
    mrowsG = 3*l1;
    ncolsG = 2*l2;
    mrowsT = 2*ndip*nstr + 2*(2*ndip + nstr - 2);
    ncolsT = 2*l2;
    mrowsG2 = mrowsG + mrowsT;
    ncolsG2 = ncolsG;
    nt = mrowsT*ncolsT;
    ng = mrowsG*ncolsG;
    ng2 = mrowsG2*ncolsG2;
    // Set space
    WUD = memory_calloc64f(mrowsG2);
    UD = memory_calloc64f(mrowsG);
    diagWt = memory_calloc64f(mrowsG);
    if (WUD == NULL || UD == NULL || diagWt == NULL)
    {
        if (WUD == NULL){LOG_ERRMSG("%s", "Error setting space for WUD");}
        if (UD == NULL){LOG_ERRMSG("%s", "Error setting space for UP");}
        if (diagWt == NULL)
        {
            LOG_ERRMSG("%s", "Error setting space for diagWt");
        }
        ierr = 5;
        goto ERROR;
    }
    if (verbose > 2){LOG_DEBUGMSG("%s", "Setting RHS...");}
    // Set the RHS
    ierr = core_ff_setRHS(l1,
                          nObsOffset,
                          eObsOffset,
                          uObsOffset,
                          UD);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error setting right hand side");
        goto ERROR;
    }
    // Compute the diagonal data weights
    ierr = core_ff_setDiagonalWeightMatrix(l1,
                                           nWts,
                                           eWts,
                                           uWts,
                                           diagWt);
    if (ierr != 0)
    {
        LOG_WARNMSG("%s", "Setting data weights to unity");
        array_set64f_work(mrowsG, 1.0, diagWt); 
    }
    // Weight the observations
    ierr = core_ff_weightObservations(mrowsG,
                                      diagWt,
                                      UD,
                                      WUD);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error weighting observations");
        goto ERROR;
    }
    // Begin the grid search on fault planes
    ierr = 0;
    time_tic();
    if (verbose > 2)
    {
        LOG_DEBUGMSG("%s", "Beginning search on fault planes...");
    }
#ifdef PARALLEL_FF
    #pragma omp parallel \
     private(asum, ds_unc, G, G2, i, ierr1, ifp, if_off, ij, io_off, j, \
             lampred, len0, M0, R, res, S, ss_unc, st, T, UP, wid0, \
             xrs, xden, xnum, yrs, zrs) \
     shared(diagWt, dip, dslip, dslip_unc, EN, fault_alt, \
            fault_xutm, fault_yutm, ldslip_unc, length, \
            lrmtx, lsslip_unc, Mw, mrowsG, mrowsG2, ncolsG, ncolsG2, \
            ng, ng2, NN, nt, sslip, sslip_unc, staAlt, strike, \
            vr, WUD, UD, UN, utmRecvEasting, utmRecvNorthing, width) \
     reduction(+:ierr) default(none)
    {
#endif
    R = NULL;
    G  = memory_calloc64f(ng);
    G2 = memory_calloc64f(ng2);
    S  = memory_calloc64f(ncolsG2);
    T  = memory_calloc64f(nt);
    UP = memory_calloc64f(mrowsG);
    xrs = memory_calloc64f(l1*l2);
    yrs = memory_calloc64f(l1*l2);
    zrs = memory_calloc64f(l1*l2);
    if (lrmtx){R = memory_calloc64f(ncolsG2*ncolsG2);}
#ifdef PARALLEL_FF
    #pragma omp for
#endif
    // Loop on fault planes
    for (ifp=0; ifp<nfp; ifp++)
    {
        // Set the offsets
        if_off = ifp*l2; // Offset the fault plane
        io_off = ifp*l1; // Offset the observations/estimates
        // Null out G2, regularizer, and possibly R matrix
        memset(G, 0, (size_t) ng*sizeof(double));
        memset(G2, 0, (size_t) ng2*sizeof(double));
        memset(T, 0, (size_t) nt*sizeof(double));
        if (lrmtx)
        {
            memset(R, 0, (size_t) (ncolsG2*ncolsG2)*sizeof(double));
        }
        // Compute the site/fault patch offsets
        for (i=0; i<l2; i++)
        {
            for (j=0; j<l1; j++)
            {
                ij = l1*i + j;
                xrs[ij] = utmRecvEasting[j]  - fault_xutm[if_off+i];
                yrs[ij] = utmRecvNorthing[j] - fault_yutm[if_off+i];
                zrs[ij] = fault_alt[if_off+i]*1.e3 + staAlt[j];
            }
        }
        // Compute the forward modeling matrix (which is in row major format)
        ierr1 = core_ff_setForwardModel__okadagreenF(l1, l2,
                                                     xrs, yrs, zrs,
                                                     &strike[if_off],
                                                     &dip[if_off],
                                                     &width[if_off],
                                                     &length[if_off],
                                                     G);
        if (ierr1 != 0)
        {
            LOG_ERRMSG("%s", "Error setting forward model");
            ierr = ierr + 1;
            continue;
        }
        // Weight the column major diagonal forward modeling matrix 
        ierr1 = core_ff_weightForwardModel(mrowsG, ncolsG,
                                           diagWt,
                                           G,
                                           G2);
        if (ierr1 != 0)
        {
            LOG_ERRMSG("%s", "Error weighting forward modeling matrix");
            ierr = ierr + 1;
            continue;
        }
        // Set the regularizer (also in row major format)
        ierr1 = core_ff_setRegularizer(l2,
                                       nstr,
                                       ndip,
                                       nt,
                                       &width[if_off],
                                       &length[if_off],
                                       T);
        if (ierr1 != 0)
        {
            LOG_ERRMSG("%s", "Error setting regulizer");
            ierr = ierr + 1;
            continue;
        }
        // Compute scale factor for regularizer
        asum = cblas_dasum(ng, G2, 1);
        len0 = length[if_off];
        wid0 = width[if_off];
        lampred = 1.0/pow( (double) l2*2.0, 2);
        lampred = lampred/(asum/(double) ng);
        lampred = lampred/4.0*len0*wid0/1.e6;
        // Append lampred*T to G2
        cblas_daxpy(nt, lampred, T, 1, &G2[ng], 1);
        // Solve the least squares problem
        ierr1 = linalg_lstsq_qr64f_work(LAPACK_ROW_MAJOR,
                                        mrowsG2, ncolsG2, 1, lrmtx,
                                        G2, WUD,
                                        S, R);
        if (ierr1 != 0)
        {
            LOG_ERRMSG("%s", "Error solving least squares problem");
            ierr = ierr + 1;
            continue;
        }
        // From the right matrix compute the covariance matrix: inv(G^T*G)
        // Notice, because G = QR the inner term is G^T G = R^T Q^T Q R = R^T R
        // Thus, we must simply solve C = inv(R^T R) = inv(R) inv(R^T).  Hence
        // it is sufficient to compute Ri = inv(R) then multiply Ri*Ri^T.
        // Futhermore, we only retain the diagonals so computing a full matrix
        // matrix multiply is unnecessary.  What is necessary is instead taking
        // the inner products which would produce the diagonal elements.
        // Note, to finish the computation you need an estimate of the standard
        // deviation and a confidence level.  For example, if you had a standard
        // deviation of say, 8, and a confidence level of 95 percent you'd compute
        // sslip_unc = sslip_unc*1.96*8.
        if (lrmtx)
        {
            ierr1 = LAPACKE_dtrtri(LAPACK_ROW_MAJOR, 'U', 'N', ncolsG2,
                                   R, ncolsG2);
            if (ierr1 != 0)
            {
                LOG_ERRMSG("%s", "Error inverting triangular matrix!");
                ierr = ierr + 1;
                continue;
            }
            // Unpack the uncertainties
            if (lsslip_unc && ldslip_unc)
            {
                for (i=0; i<l2; i++)
                {
                    ss_unc = cblas_ddot(ncolsG2, &R[(2*i+0)*ncolsG2], 1,
                                                 &R[(2*i+0)*ncolsG2], 1); 
                    ds_unc = cblas_ddot(ncolsG2, &R[(2*i+1)*ncolsG2], 1,
                                                 &R[(2*i+1)*ncolsG2], 1); 
                    sslip_unc[if_off+i] = sqrt(ss_unc);
                    dslip_unc[if_off+i] = sqrt(ds_unc);
                }
            }
            // Unpack only desired uncertainties
            else
            {
                for (i=0; i<l2; i++)
                {
                    if (ldslip_unc)
                    {
                        ss_unc = cblas_ddot(ncolsG2, &R[(2*i+0)*ncolsG2], 1,
                                                     &R[(2*i+0)*ncolsG2], 1);
                        sslip_unc[if_off+i] = sqrt(ss_unc);
                    }
                    if (ldslip_unc)
                    {
                        ds_unc = cblas_ddot(ncolsG2, &R[(2*i+1)*ncolsG2], 1,
                                                     &R[(2*i+1)*ncolsG2], 1);
                        sslip_unc[if_off+i] = sqrt(ds_unc);
                    }
                }
            }
        }
        // Compute the forward problem UP = G*S (ignoring regularizer)
        cblas_dgemv(CblasRowMajor, CblasNoTrans,
                    mrowsG, ncolsG, 1.0, G, ncolsG, S, 1, 0.0, UP, 1);
        // Compute the variance reduction
        xnum = 0.0;
        xden = 0.0;
#ifdef _OPENMP
        #pragma omp simd reduction(+:xnum, xden)
#endif
        for (i=0; i<mrowsG; i++)
        {
            res = diagWt[i]*(UP[i] - UD[i]);
            xnum = xnum + res*res;
            xden = xden + WUD[i]*WUD[i];
        }
        vr[ifp] = (1.0 - xnum/xden)*100.0;
        // Extract the estimates
#ifdef _OPENMP
        #pragma omp simd
#endif
        for (i=0; i<l1; i++)
        {
            EN[io_off+i] = UP[3*i+0];
            NN[io_off+i] = UP[3*i+1];
            UN[io_off+i] = UP[3*i+2];
            //Einp[io_off+i] = UD[3*i+0];
            //Ninp[io_off+i] = UD[3*i+1];
            //Uinp[io_off+i] = UD[3*i+2];
        }
        // Extract the slip
#ifdef _OPENMP
        #pragma omp simd
#endif
        for (i=0; i<l2; i++)
        {
            sslip[if_off+i] = S[2*i+0];
            dslip[if_off+i] = S[2*i+1];
        }
        // Compute the magnitude
        M0 = 0.0;
        for (i=0; i<l2; i++)
        {
            st = sqrt( pow(sslip[if_off+i], 2) + pow(dslip[if_off+i], 2) );
            M0 = M0 + 3.e10*st*length[if_off+i]*width[if_off+i];
        }
        Mw[ifp] = 0.0;
        if (M0 > 0.0){Mw[ifp] = (log10(M0*1.e7) - 16.1)/1.5;}
    } // Loop on fault planes 
    memory_free64f(&G);
    memory_free64f(&G2);
    memory_free64f(&S);
    memory_free64f(&T);
    memory_free64f(&UP);
    memory_free64f(&xrs);
    memory_free64f(&yrs);
    memory_free64f(&zrs);
    memory_free64f(&R);
#ifdef PARALLEL_FF
    }
#endif
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "There was an error in the fault plane grid-search");
        ierr = 4;
    }
    else
    {
        if (verbose > 2)
        {
            LOG_DEBUGMSG("Fault plane grid-search time: %f (s)", time_toc());
        }
    }
ERROR:;
    // Clean up
    memory_free64f(&WUD);
    memory_free64f(&UD);
    memory_free64f(&diagWt);
    return ierr;
}
