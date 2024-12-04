#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_eewUtils.h"
#include <stdbool.h>
#include "gfast_core.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"

static int __verify_cmt_structs(struct GFAST_offsetData_struct cmt_data,
                                struct GFAST_cmtResults_struct *cmt);

/*!
 * @brief Drives the CMT estimation.
 *
 * @param[in] cmt_props  CMT inversion parameters
 * @param[in] SA_lat     event latitude (degrees) [-90,90]
 * @param[in] SA_lon     event longitude (degrees) [0,360]
 * @param[in] SA_dep     event depth (km)
 * @param[in] cmt_data   cmt offset data to invert
 *
 * @param[in,out] cmt    on input contains the depths for the grid search
 *                       on output contains the corresponding variance
 *                       reduction, moment tensors, nodal planes at each
 *                       depth in the CMT grid search, and optimal depth
 *                       index.
 *
 * @result 0 indicates success
 *         1 indicates an error on the cmt structure
 *         2 indicates the offset data structure is invalid
 *         3 indicates there was insufficient data to invert
 *         4 indicates an error encountered during the computation
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int eewUtils_driveCMT(struct GFAST_cmt_props_struct cmt_props,
                      const double SA_lat,
                      const double SA_lon,
                      const double SA_dep,
                      struct GFAST_offsetData_struct cmt_data,
                      struct GFAST_cmtResults_struct *cmt)
{
    double *utmRecvEasting, *utmRecvNorthing, *staAlt,
           *eOffset, *eEst, *eWts, *nOffset, *nEst, *nWts,
           *uOffset, *uEst, *uWts,
           DC_pct, eres, nres, sum_res2, ures,
           *utmSrcEastings, *utmSrcNorthings, wte, wtn, wtu, x1, y1, x2, y2;
    int i, idep, ierr, ierr1, ilat, ilon, ilatLon, indx, k, l1, nlld, nlatlon, zone_loc;
    bool *luse, lnorthp;
    //------------------------------------------------------------------------//
    //
    // Verify the input data structure makes sense
    ierr = CMT_SUCCESS;
    luse = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    staAlt = NULL;
    uOffset = NULL;
    nOffset = NULL;
    eOffset = NULL;
    uWts = NULL;
    nWts = NULL;
    eWts = NULL;
    nEst = NULL;
    eEst = NULL;
    uEst = NULL;
    // Verify input data structures
    ierr =__verify_cmt_structs(cmt_data, cmt);
    if (ierr != CMT_SUCCESS)
    {
        LOG_ERRMSG("%s", "Error failed to verify data structures");
    }
    // Warn in case hypocenter is outside of grid-search
    if (cmt_props.verbose > 1 &&
        (SA_dep < cmt->srcDepths[0] || SA_dep > cmt->srcDepths[cmt->ndeps-1]))
    {
        LOG_WARNMSG("%s", "Warning hypocenter isn't in grid search!");
    }
    // Initialize result
    nlld = cmt->nlats * cmt->nlons * cmt->ndeps;
    cmt->opt_indx = -1;
    cmt->opt_dep = __FLT_MIN__;
    cmt->opt_lat = __FLT_MIN__;
    cmt->opt_lon = __FLT_MIN__;
    // Synthetics as a function of depth
    array_zeros64f_work(cmt->nsites*nlld, cmt->EN);
    array_zeros64f_work(cmt->nsites*nlld, cmt->NN);
    array_zeros64f_work(cmt->nsites*nlld, cmt->UN);
    // Observations
    array_zeros64f_work(cmt->nsites, cmt->Einp);
    array_zeros64f_work(cmt->nsites, cmt->Ninp);
    array_zeros64f_work(cmt->nsites, cmt->Uinp);
    // MT optimizaiton information as a function of depth 
    nlld = cmt->nlats*cmt->nlons*cmt->ndeps;
    array_zeros64f_work(nlld, cmt->l2);
    array_zeros64f_work(nlld, cmt->pct_dc);
    array_zeros64f_work(nlld, cmt->objfn);
    array_zeros64f_work(6 * nlld, cmt->mts);
    array_zeros64f_work(nlld, cmt->str1);
    array_zeros64f_work(nlld, cmt->str2);
    array_zeros64f_work(nlld, cmt->dip1);
    array_zeros64f_work(nlld, cmt->dip2);
    array_zeros64f_work(nlld, cmt->rak1);
    array_zeros64f_work(nlld, cmt->rak2);
    array_zeros64f_work(nlld, cmt->Mw);
    // Require there is a sufficient amount of data to invert
    luse = memory_calloc8l(cmt_data.nsites);
    l1 = 0;
    for (k = 0; k < cmt_data.nsites; k++)
    {
        wtu = cmt_data.wtu[k];
        wtn = cmt_data.wtn[k];
        wte = cmt_data.wte[k];
        if (!cmt_data.lactive[k] || fmax(wtu, fmax(wtn, wte)) <= 0.0)
        {
             continue;
        }
        luse[k] = true;
        l1 = l1 + 1;
    }
    if (l1 < cmt_props.min_sites)
    {
        if (cmt_props.verbose > 1)
        {
            LOG_WARNMSG("Insufficient data to invert %d < %d\n",
                        l1, cmt_props.min_sites);
        }
        ierr = CMT_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Set space
    nlatlon = cmt->nlats * cmt->nlons;
    utmRecvNorthing = memory_calloc64f(l1);
    utmRecvEasting  = memory_calloc64f(l1);
    staAlt  = memory_calloc64f(l1);
    uOffset = memory_calloc64f(l1);
    nOffset = memory_calloc64f(l1);
    eOffset = memory_calloc64f(l1);
    uWts    = memory_calloc64f(l1);
    nWts    = memory_calloc64f(l1);
    eWts    = memory_calloc64f(l1);
    nEst    = memory_calloc64f(l1 * nlld);
    eEst    = memory_calloc64f(l1 * nlld);
    uEst    = memory_calloc64f(l1 * nlld);
    utmSrcNorthings = memory_calloc64f(nlatlon);
    utmSrcEastings  = memory_calloc64f(nlatlon);
    // Get the source location
    zone_loc = cmt_props.utm_zone; // Use input UTM zone
    if (zone_loc ==-12345){zone_loc =-1;} // Figure it out
    for (ilon = 0; ilon < cmt->nlons; ilon++) {
        for (ilat = 0; ilat < cmt->nlats; ilat++) {
            ilatLon = ilon * cmt->nlats + ilat;
            core_coordtools_ll2utm(SA_lat + cmt->srcLats[ilat],
                                   SA_lon + cmt->srcLons[ilon],
                                   &y1, &x1,
                                   &lnorthp, &zone_loc);
            utmSrcNorthings[ilatLon] = y1; 
            utmSrcEastings[ilatLon] = x1;
        }
    }
    // Get cartesian positions and observations onto local arrays
    l1 = 0;
    for (k = 0; k < cmt_data.nsites; k++)
    {
        if (!luse[k]){continue;}
        // Get the recevier UTM
        core_coordtools_ll2utm(cmt_data.sta_lat[k],
                               cmt_data.sta_lon[k],
                               &y2, &x2,
                               &lnorthp, &zone_loc);
        // Copy the pertinent data
        uOffset[l1] = cmt_data.ubuff[k];
        nOffset[l1] = cmt_data.nbuff[k];
        eOffset[l1] = cmt_data.ebuff[k];
        uWts[l1] = cmt_data.wtu[k];
        nWts[l1] = cmt_data.wtn[k];
        eWts[l1] = cmt_data.wte[k];
        utmRecvNorthing[l1] = y2;
        utmRecvEasting[l1] = x2;
        staAlt[l1] = cmt_data.sta_alt[k];
        l1 = l1 + 1;
    }
    // Invert!
    if (cmt_props.verbose > 2)
    { 
        LOG_DEBUGMSG("Inverting for CMT with %d sites", l1);
    }
    ierr = core_cmt_gridSearch(l1,
                               cmt->ndeps, cmt->nlats, cmt->nlons,
                               cmt_props.verbose,
                               cmt_props.ldeviatoric,
                               utmSrcEastings,
                               utmSrcNorthings,
                               cmt->srcDepths,
                               utmRecvEasting,
                               utmRecvNorthing,
                               staAlt,
                               nOffset,
                               eOffset,
                               uOffset,
                               nWts,
                               eWts,
                               uWts,
                               nEst,
                               eEst,
                               uEst,
                               cmt->mts);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s", "Error in CMT gridsearch!");
        ierr = CMT_COMPUTE_ERROR;
        goto ERROR;
    }
    // Get the estimates and observations
    i = 0;
    for (k = 0; k < cmt->nsites; k++)
    {
        cmt->lsiteUsed[k] = luse[k];
        if (!luse[k]) {continue;}
        cmt->Ninp[k] = nOffset[i];
        cmt->Einp[k] = eOffset[i];
        cmt->Uinp[k] = uOffset[i];
        cmt->lsiteUsed[k] = true;
        i = i + 1;
    }
    // Extract results and weight objective fn by percent double couple
    ierr = 0;
#ifdef PARALLEL_CMT
    #pragma omp parallel for collapse(3) \
     private(DC_pct, eres, i, idep, ierr1, ilat, ilon, indx, k, sum_res2, nres, ures) \
     shared(cmt, eOffset, eEst, l1, luse, nOffset, nEst, uOffset, uEst) \
     reduction(+:ierr), default(none) 
#endif
    for (ilon = 0; ilon < cmt->nlons; ilon++)
    {
        for (ilat = 0; ilat < cmt->nlats; ilat++)
        {
            for (idep = 0; idep < cmt->ndeps; idep++)
            {
                // Get location in arrays
                indx = ilon * cmt->ndeps * cmt->nlats
                     + ilat * cmt->ndeps 
                     + idep;
                // Compute the L2 norm
                sum_res2 = 0.0;
#ifdef _OPENMP
                #pragma omp simd reduction(+:sum_res2)
#endif
                for (i = 0; i < l1; i++)
                {
                    nres = nOffset[i] - nEst[indx * l1 + i];
                    eres = eOffset[i] - eEst[indx * l1 + i];
                    ures = uOffset[i] - uEst[indx * l1 + i];
                    sum_res2 = sum_res2 + nres*nres + eres*eres + ures*ures;
                }
                sum_res2 = sqrt(sum_res2);
                // Decompose the moment tensor
                ierr1 = core_cmt_decomposeMomentTensor(1, &cmt->mts[6 * indx],
                                                       &DC_pct,
                                                       &cmt->Mw[indx],
                                                       &cmt->str1[indx],
                                                       &cmt->str2[indx],
                                                       &cmt->dip1[indx],
                                                       &cmt->dip2[indx],
                                                       &cmt->rak1[indx],
                                                       &cmt->rak2[indx]);
                if (ierr1 != 0)
                {
                    LOG_ERRMSG("%s", "Error decomposing mt");
                    ierr = ierr + 1;
                    continue;
                }
                // Prefer results with larger double couple percentages
                cmt->l2[indx] = 0.5 * sqrt(sum_res2);
                cmt->pct_dc[indx] = DC_pct;
                cmt->objfn[indx] = sum_res2/DC_pct;
                // Save the data
                i = 0;
                for (k=0; k<cmt->nsites; k++)
                {
                    cmt->NN[indx * cmt->nsites + k] = 0.0;
                    cmt->EN[indx * cmt->nsites + k] = 0.0;
                    cmt->UN[indx * cmt->nsites + k] = 0.0;
                    if (luse[k])
                    {
                        cmt->NN[indx * cmt->nsites + k] = nEst[indx * l1 + i];
                        cmt->EN[indx * cmt->nsites + k] = eEst[indx * l1 + i];
                        cmt->UN[indx * cmt->nsites + k] = uEst[indx * l1 + i];
                        i = i + 1;
                        if (cmt_props.verbose > 2) {
                            LOG_DEBUGMSG("    sta obs (%d) for %.4f, %.4f, %.1f: %s ENUobs:[%.4f,%.4f,%.4f] ENUpred:[%.4f,%.4f,%.4f]",
                                indx, cmt->srcLats[ilat] + SA_lat, cmt->srcLons[ilon]+ SA_lon, cmt->srcDepths[idep],
                                cmt_data.stnm[i],
                                cmt->Einp[k], cmt->Ninp[k], cmt->Uinp[k],
                                cmt->EN[indx * cmt->nsites + k], cmt->NN[indx * cmt->nsites + k], cmt->UN[indx * cmt->nsites + k]);
                        }
                    }
                }
                // print results
                if (cmt_props.verbose > 2) {
                    LOG_DEBUGMSG("CMT results (%d) for %.4f, %.4f, %.1f: str=[%.2f, %.2f], dip=[%.2f, %.2f], rake=[%.2f, %.2f], pct_dc=%.2f, objfn=%.2f",
                        indx, cmt->srcLats[ilat] + SA_lat, cmt->srcLons[ilon]+ SA_lon, cmt->srcDepths[idep],
                        cmt->str1[indx], cmt->str2[indx], cmt->dip1[indx], cmt->dip2[indx], cmt->rak1[indx], cmt->rak2[indx],
                        cmt->pct_dc[indx], cmt->objfn[indx]);
                }
           } // Loop on depths
       } // loop on latiudes
    } // loop on longitudes
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error decomposing moment tensor");
        ierr = CMT_COMPUTE_ERROR;
    }
    // Get the optimimum index
    { /*vk needed for more stringent c++ compiler*/
      enum isclError_enum isclerr = (enum isclError_enum)ierr;
      cmt->opt_indx = array_argmin64f(nlld, cmt->objfn, &isclerr); 
    }
    // Unpack opt_indx for lat, lon, depth
    int rem;
    idep = cmt->opt_indx % cmt->ndeps;
    rem = (cmt->opt_indx - idep) / cmt->ndeps;
    ilat = rem % cmt->nlats;
    ilon = ((rem - ilat) / cmt->nlats) % cmt->nlons;

    cmt->opt_dep = cmt->srcDepths[idep];
    cmt->opt_lat = cmt->srcLats[ilat] + SA_lat;
    cmt->opt_lon = cmt->srcLons[ilon] + SA_lon;
    // if (cmt->ndeps < nlld)
    // {
    //     LOG_WARNMSG("%s", "NEED to unpack opt_indx and make a cmt->opt_dep");
    // }
ERROR:;
    memory_free8l(&luse);
    memory_free64f(&utmRecvNorthing);
    memory_free64f(&utmRecvEasting);
    memory_free64f(&staAlt);
    memory_free64f(&uOffset);
    memory_free64f(&nOffset);
    memory_free64f(&eOffset);
    memory_free64f(&nEst);
    memory_free64f(&eEst);
    memory_free64f(&uEst);
    memory_free64f(&nWts);
    memory_free64f(&eWts);
    memory_free64f(&uWts);
    return ierr;
}
//============================================================================//
/*!
 * @brief Utility function for verifying input data structures
 *
 * @param[in] cmt_data   cmt_data structure to verify
 * @param[in] cmt        cmt results structure to verify
 *
 * @result CMT_SUCCESS indicates input structures are ready for use
 *
 * @author Ben Baker (ISTI)
 *
 */
static int __verify_cmt_structs(struct GFAST_offsetData_struct cmt_data,
                                struct GFAST_cmtResults_struct *cmt)
{
    int ierr;
    ierr = CMT_SUCCESS;
    // Require there is offset data
    if (cmt_data.nsites < 1)
    {
        ierr = CMT_OS_DATA_ERROR;
        LOG_ERRMSG("%s", "No offset data");
        goto ERROR;
    }
    // Verify the output data structures
    if (cmt->ndeps < 1)
    {
        LOG_ERRMSG("%s", "No depths in CMT gridsearch!");
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    if (cmt->l2 == NULL || cmt->pct_dc == NULL ||
        cmt->objfn == NULL || cmt->mts == NULL ||
        cmt->str1 == NULL || cmt->str2 == NULL ||
        cmt->dip1 == NULL || cmt->dip2 == NULL ||
        cmt->rak1 == NULL || cmt->rak2 == NULL ||
        cmt->Mw == NULL || cmt->srcDepths == NULL ||
        cmt->EN == NULL || cmt->NN == NULL || cmt->UN == NULL ||
        cmt->Einp == NULL || cmt->Ninp == NULL || cmt->Uinp == NULL ||
        cmt->lsiteUsed == NULL)
    {
        if (cmt->l2 == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->l2 is NULL");
        }
        if (cmt->pct_dc == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->pct_dc is NULL");
        }
        if (cmt->objfn == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->objfn is NULL");
        }
        if (cmt->mts == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->mts is NULL");
        }
        if (cmt->str1 == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->str1 is NULL");
        }
        if (cmt->str2 == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->str2 is NULL");
        }
        if (cmt->dip1 == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->dip1 is NULL");
        }
        if (cmt->dip2 == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->dip2 is NULL");
        }
        if (cmt->rak1 == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->rak1 is NULL");
        }
        if (cmt->rak2 == NULL)
        {
            LOG_ERRMSG("%s", "Error cmt->rak2 is NULL");
        }
        if (cmt->Mw == NULL)
        {
            LOG_ERRMSG("%s", "Error Mw is NULL");
        }
        if (cmt->srcDepths == NULL)
        {
            LOG_ERRMSG("%s", "Error srcDepths is NULL");
        }
        if (cmt->EN == NULL)
        {
            LOG_ERRMSG("%s", "Error EN is NULL");
        }
        if (cmt->NN == NULL)
        {
            LOG_ERRMSG("%s", "Error NN is NULL");
        }
        if (cmt->UN == NULL)
        {
            LOG_ERRMSG("%s", "Error UN is NULL");
        }
        if (cmt->Einp == NULL)
        {
            LOG_ERRMSG("%s", "Error Einp is NULL");
        }
        if (cmt->Ninp == NULL)
        {
            LOG_ERRMSG("%s", "Error Ninp is NULL");
        }
        if (cmt->Uinp == NULL)
        {
            LOG_ERRMSG("%s", "Error Uinp is NULL");
        }
        if (cmt->lsiteUsed == NULL)
        {
            LOG_ERRMSG("%s", "Error lsiteUsed is NULL");
        }
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    // Avoid a segfault
    if (cmt->nsites != cmt_data.nsites)
    {
        LOG_ERRMSG("nsites on cmt and cmt_data differs %d %d\n",
                   cmt->nsites, cmt_data.nsites);
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
ERROR:;
    return ierr;
}

