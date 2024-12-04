#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "gfast_eewUtils.h"
#include "gfast_core.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"

static int __verify_ff_structs(struct GFAST_offsetData_struct ff_data,
                               struct GFAST_ffResults_struct *ff);
/*!
 * @brief Drives the finite fault fault plane grid search inversion 
 *
 * @param[in] ff_props    finite fault inversion parameters
 * @param[in] SA_lat      event latitude (degrees)
 * @param[in] SA_lon      event longitude (degrees) 
 * @param[in] ff_data     offset data for the finite fault inversion
 *
 * @param[in,out] ff      on input contains space for the finite fault
 *                        inversion as well as the fault strike and rake
 *                        on all the nodal planes, and centroid depth
 *                        in the inversion.
 *                        on output contains the results of the finite
 *                        fault inversion which includes slip along
 *                        strike and slip down dip on each plane,
 *                        uncertainties, and estimate data and observed data
 *
 * @result 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
int eewUtils_driveFF(struct GFAST_ff_props_struct ff_props,
                     const double SA_lat,
                     const double SA_lon,
                     struct GFAST_offsetData_struct ff_data,
                     struct GFAST_ffResults_struct *ff)
{
    double *dip, *dslip, *dslip_unc, *eOffset, *EN, *eWts,
           *fault_xutm, *fault_yutm, *fault_alt, *length,
           *Mw, *nOffset, *NN, *nWts, *sslip, *sslip_unc, *staAlt,
           *strike, *uOffset, *utmRecvEasting, *utmRecvNorthing,
           *UN, *uWts, *vr, *width,
           wte, wtn, wtu, x1, x2, y1, y2,
           flen_pct, fwid_pct;
    int i, ierr, ierr1, if_off, ifp, io_off, k, l1, l2,
        ndip, nfp, nstr, zone_loc;
    bool *luse, lnorthp;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = FF_SUCCESS;
    staAlt = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    uWts = NULL;
    nWts = NULL;
    eWts = NULL;
    eOffset = NULL;
    nOffset = NULL;
    uOffset = NULL;
    fault_xutm = NULL;
    fault_yutm = NULL;
    fault_alt = NULL;
    length = NULL;
    width = NULL;
    strike = NULL;
    dip = NULL;
    sslip = NULL;
    dslip = NULL;
    Mw = NULL;
    vr = NULL;
    NN = NULL;
    EN = NULL;
    UN = NULL;
    sslip_unc = NULL;
    dslip_unc = NULL;
    // Verify the input data structures
    ierr = __verify_ff_structs(ff_data, ff);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error failed to verify data structures");
        goto ERROR;
    }
    // Initialize the result
    nfp = ff->nfp;
    nstr = ff->fp[0].nstr;
    ndip = ff->fp[0].ndip;
    l2 = nstr*ndip;
    ff->preferred_fault_plane = 0;
    for (ifp=0; ifp<nfp; ifp++)
    {
        if_off = ifp*l2;
        // Faults plane info
        array_zeros64f_work(l2, ff->fp[ifp].fault_xutm);
        array_zeros64f_work(l2, ff->fp[ifp].fault_yutm);
        array_zeros64f_work(l2, ff->fp[ifp].fault_alt);
        array_zeros64f_work(l2, ff->fp[ifp].strike);
        array_zeros64f_work(l2, ff->fp[ifp].dip);
        array_zeros64f_work(l2, ff->fp[ifp].length);
        array_zeros64f_work(l2, ff->fp[ifp].width);
        array_zeros64f_work(l2, ff->fp[ifp].sslip);
        array_zeros64f_work(l2, ff->fp[ifp].dslip);
        array_zeros64f_work(l2, ff->fp[ifp].sslip_unc);
        array_zeros64f_work(l2, ff->fp[ifp].dslip_unc);
        // Plotting information
        array_zeros64f_work(4*l2, ff->fp[ifp].lon_vtx);
        array_zeros64f_work(4*l2, ff->fp[ifp].lat_vtx);
        array_zeros64f_work(4*l2, ff->fp[ifp].dep_vtx);
        // Observations
        array_zeros64f_work(ff->fp[ifp].maxobs, ff->fp[ifp].EN);
        array_zeros64f_work(ff->fp[ifp].maxobs, ff->fp[ifp].NN);
        array_zeros64f_work(ff->fp[ifp].maxobs, ff->fp[ifp].UN);
        array_zeros64f_work(ff->fp[ifp].maxobs, ff->fp[ifp].Einp);
        array_zeros64f_work(ff->fp[ifp].maxobs, ff->fp[ifp].Ninp);
        array_zeros64f_work(ff->fp[ifp].maxobs, ff->fp[ifp].Uinp);
        ff->fp[ifp].nsites_used = 0;
        ff->Mw[ifp] = 0.0;
        ff->vr[ifp] = 0.0;
    } // Loop on fault planes 
    // Require there is a sufficient amount of data to invert
    luse = memory_calloc8l(ff_data.nsites);
    l1 = 0;
    for (k=0; k<ff_data.nsites; k++)
    {
        wtu = ff_data.wtu[k];
        wtn = ff_data.wtn[k];
        wte = ff_data.wte[k];
        if (!ff_data.lactive[k] || fmax(wtu, fmax(wtn, wte)) <= 0.0)
        {
             continue;
        }
        luse[k] = true;
        l1 = l1 + 1;
    }
    if (l1 < ff_props.min_sites)
    {
        if (ff_props.verbose > 1)
        {
            LOG_WARNMSG("Insufficient data to invert %d < %d\n",
                        l1, ff_props.min_sites);
        }
        ierr = FF_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Set space
    uOffset = memory_calloc64f(l1);
    nOffset = memory_calloc64f(l1);
    eOffset = memory_calloc64f(l1);
    utmRecvEasting  = memory_calloc64f(l1);
    utmRecvNorthing = memory_calloc64f(l1);
    staAlt = memory_calloc64f(l1);
    uWts = memory_calloc64f(l1);
    nWts = memory_calloc64f(l1);
    eWts = memory_calloc64f(l1);
    fault_xutm = memory_calloc64f(l2*nfp);
    fault_yutm = memory_calloc64f(l2*nfp);
    fault_alt  = memory_calloc64f(l2*nfp);
    length     = memory_calloc64f(l2*nfp);
    width      = memory_calloc64f(l2*nfp);
    strike     = memory_calloc64f(l2*nfp);
    dip        = memory_calloc64f(l2*nfp);
    sslip      = memory_calloc64f(l2*nfp);
    dslip      = memory_calloc64f(l2*nfp);
    Mw         = memory_calloc64f(nfp);
    vr         = memory_calloc64f(nfp);
    NN         = memory_calloc64f(l1*nfp);
    EN         = memory_calloc64f(l1*nfp);
    UN         = memory_calloc64f(l1*nfp);
    sslip_unc  = memory_calloc64f(l2*nfp);
    dslip_unc  = memory_calloc64f(l2*nfp);
    // Get the source location
    zone_loc = ff_props.utm_zone; // Use input UTM zone
    if (zone_loc ==-12345){zone_loc =-1;} // Figure it out
    core_coordtools_ll2utm(SA_lat, SA_lon,
                           &y1, &x1,
                           &lnorthp, &zone_loc);
    // Get cartesian positions and observations onto local arrays
    l1 = 0;
    for (k=0; k<ff_data.nsites; k++)
    {
        if (!luse[k]){continue;}
        // Get the recevier UTM
        core_coordtools_ll2utm(ff_data.sta_lat[k],
                               ff_data.sta_lon[k],
                               &y2, &x2,
                               &lnorthp, &zone_loc);
        // Copy the pertinent data
        uOffset[l1] = ff_data.ubuff[k];
        nOffset[l1] = ff_data.nbuff[k];
        eOffset[l1] = ff_data.ebuff[k];
        uWts[l1] = ff_data.wtu[k];
        nWts[l1] = ff_data.wtn[k];
        eWts[l1] = ff_data.wte[k];
        utmRecvNorthing[l1] = y2;
        utmRecvEasting[l1] = x2;
        staAlt[l1] = ff_data.sta_alt[k];
        l1 = l1 + 1;
    }
    //------------------------------Fault Plane Mesher------------------------//
    // Mesh the fault planes remembering the event hypocenter and strike/dip
    // information were defined in the calling routine
    ierr = 0;
    if (ff_props.verbose > 2)
    {
        LOG_DEBUGMSG("%s", "Meshing fault plane...");
    }
    if (ff->SA_mag > ff_props.large_m)
    {
        flen_pct = ff_props.flen_pct_large;
        fwid_pct = ff_props.fwid_pct_large;
        if (ff_props.verbose > 2)
        {
            LOG_DEBUGMSG("Magnitude is above the large M threshold (%.2f>%.2f), using flen_pct:%2f, fwid_pct:%2f",
                ff->SA_mag, ff_props.large_m, flen_pct, fwid_pct);
        }
    } else
    {
        flen_pct = ff_props.flen_pct;
        fwid_pct = ff_props.fwid_pct;
        if (ff_props.verbose > 2)
        {
            LOG_DEBUGMSG("Using flen_pct:%2f, fwid_pct:%2f", flen_pct, fwid_pct);
        }
    }
#ifdef PARALLEL_FF
    #pragma omp parallel for \
     private(ierr1, ifp) \
     shared(ff, ff_props, zone_loc) \
     reduction(+:ierr) default(none)
#endif
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        ierr1 = core_ff_meshFaultPlane(ff->SA_lat, ff->SA_lon, ff->SA_dep,
                                       flen_pct,
                                       fwid_pct,
                                       ff->SA_mag, ff->str[ifp], ff->dip[ifp],
                                       ff->fp[ifp].nstr, ff->fp[ifp].ndip,
                                       zone_loc, ff_props.verbose,
                                       ff->fp[ifp].fault_ptr,
                                       ff->fp[ifp].lat_vtx,
                                       ff->fp[ifp].lon_vtx,
                                       ff->fp[ifp].dep_vtx,
                                       ff->fp[ifp].fault_xutm,
                                       ff->fp[ifp].fault_yutm,
                                       ff->fp[ifp].fault_alt,
                                       ff->fp[ifp].strike,
                                       ff->fp[ifp].dip,
                                       ff->fp[ifp].length,
                                       ff->fp[ifp].width);
        if (ierr1 != 0)
        {
            LOG_ERRMSG("%s", "Error meshing fault plane");
            ierr = ierr + 1;
            continue;
        }
    } // Loop on fault planes
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error meshing fault planes!");
        goto ERROR;
    }
    //------------------------------ Inversion -------------------------------//
    // Map the fault planes to the meshes arrays
    for (ifp=0; ifp<nfp; ifp++)
    {   
        if_off = ifp*l2;
#ifdef _OPENMP
        #pragma omp simd
#endif
        for (i=0; i<l2; i++)
        {
            fault_xutm[if_off+i] = ff->fp[ifp].fault_xutm[i];
            fault_yutm[if_off+i] = ff->fp[ifp].fault_yutm[i];
            fault_alt[if_off+i]  = ff->fp[ifp].fault_alt[i];
            length[if_off+i]     = ff->fp[ifp].length[i];
            width[if_off+i]      = ff->fp[ifp].width[i]; 
            strike[if_off+i]     = ff->fp[ifp].strike[i];
            dip[if_off+i]        = ff->fp[ifp].dip[i];
        }
    }
    // Let user know an inversion is about to happen 
    if (ff_props.verbose > 2)
    {   
        LOG_DEBUGMSG("Inverting for slip on %d planes with %d sites",
                     nfp, l1);
    }
    // Perform the finite fault inversion
    ierr = core_ff_faultPlaneGridSearch(l1, l2,
                                        nstr, ndip, nfp,
                                        ff_props.verbose,
                                        nOffset, eOffset, uOffset,
                                        nWts, eWts, uWts,
                                        utmRecvEasting, utmRecvNorthing,
                                        staAlt,
                                        fault_xutm, fault_yutm, fault_alt,
                                        length, width,
                                        strike, dip,
                                        sslip, dslip,
                                        Mw, vr,
                                        NN, EN, UN,
                                        sslip_unc, dslip_unc);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error performing finite fault grid search");
        goto ERROR;
    }
    //----------------------------Extract the Results-------------------------//
    // Unpack results onto struture and choose a preferred plane
    ff->preferred_fault_plane = 0;
    for (ifp=0; ifp<nfp; ifp++)
    {
        if_off = ifp*l2;
        io_off = ifp*l1;
#ifdef _OPENMP
        #pragma omp simd
#endif
        for (i=0; i<l2; i++)
        {
            ff->fp[ifp].sslip[i] = sslip[if_off+i];
            ff->fp[ifp].dslip[i] = dslip[if_off+i];
            ff->fp[ifp].sslip_unc[i] = sslip_unc[if_off+i];
            ff->fp[ifp].dslip_unc[i] = dslip_unc[if_off+i];
        }
        // Observations
        i = 0;
        for (k=0; k<ff->fp[ifp].maxobs; k++)
        {
            if (luse[k])
            {
                ff->fp[ifp].EN[k] = EN[io_off+i];
                ff->fp[ifp].NN[k] = NN[io_off+i];
                ff->fp[ifp].UN[k] = UN[io_off+i];
                ff->fp[ifp].Einp[k] = eOffset[i];
                ff->fp[ifp].Ninp[k] = nOffset[i];
                ff->fp[ifp].Uinp[k] = uOffset[i];
                i = i + 1;
            }
        }
        ff->fp[ifp].nsites_used = i;
        ff->Mw[ifp] = Mw[ifp]; // moment magnitude 
        ff->vr[ifp] = vr[ifp]; // variance reduction
        // Preferred fault plane has greatest variance reduction
        if (ff->vr[ifp] > ff->vr[ff->preferred_fault_plane])
        {
            ff->preferred_fault_plane = ifp;
        }
    } // Loop on fault planes
ERROR:;
    memory_free8l(&luse);
    memory_free64f(&uOffset);
    memory_free64f(&nOffset);
    memory_free64f(&eOffset);
    memory_free64f(&utmRecvEasting);
    memory_free64f(&utmRecvNorthing);
    memory_free64f(&staAlt);
    memory_free64f(&nWts);
    memory_free64f(&eWts);
    memory_free64f(&uWts);
    memory_free64f(&fault_xutm);
    memory_free64f(&fault_yutm);
    memory_free64f(&fault_alt);
    memory_free64f(&length);
    memory_free64f(&width);
    memory_free64f(&strike);
    memory_free64f(&dip);
    memory_free64f(&sslip);
    memory_free64f(&dslip);
    memory_free64f(&Mw);
    memory_free64f(&vr);
    memory_free64f(&NN);
    memory_free64f(&EN);
    memory_free64f(&UN);
    memory_free64f(&sslip_unc);
    memory_free64f(&dslip_unc);
    return ierr;
}
//============================================================================//
/*!
 * @brief Utility function for verifying input data structures
 *
 * @param[in] ff_data    ff_data structure to verify
 * @param[in] ff         ff results structure to verify
 *
 * @result FF_SUCCESS indicates input structures are ready for use
 *
 * @author Ben Baker (ISTI)
 *
 */
static int __verify_ff_structs(struct GFAST_offsetData_struct ff_data,
                               struct GFAST_ffResults_struct *ff)
{
    int ierr, ifp;
    ierr = FF_SUCCESS;
    if (ff_data.nsites < 1)
    {
        ierr = FF_OS_DATA_ERROR;
        LOG_ERRMSG("%s", "No peak displacement data");
        goto ERROR;
    }
    // Verify the output data structures
    if (ff->nfp < 1)
    {
        LOG_ERRMSG("%s", "No fault planes in fault plane gridsearch!");
        ierr = FF_STRUCT_ERROR;
        goto ERROR;
    }
    if (ff->fp == NULL || ff->vr == NULL || ff->Mw == NULL ||
        ff->str == NULL || ff->dip == NULL)
    {
        if (ff->fp == NULL)
        {
            LOG_ERRMSG("%s", "Error fault plane is NULL");
        }
        if (ff->vr == NULL)
        {
            LOG_ERRMSG("%s", "Error ff->vr is NULL");
        }
        if (ff->Mw == NULL)
        {
            LOG_ERRMSG("%s", "Error ff->Mw is NULL");
        }
        if (ff->str == NULL)
        {
            LOG_ERRMSG("%s", "Error ff->str is NULL");
        }
        if (ff->dip == NULL)
        {
            LOG_ERRMSG("%s", "Error ff->dip is NULL");
        }
        ierr = FF_STRUCT_ERROR;
        goto ERROR;
    }
    // Loop on fault planes and check each fp structure
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        if (ff->fp[ifp].nstr != ff->fp[0].nstr)
        {
            LOG_ERRMSG("%s", "Error inconsistent number of strike patches");
            ierr = FF_STRUCT_ERROR;
        }
        if (ff->fp[ifp].ndip != ff->fp[0].ndip)
        {
            LOG_ERRMSG("%s", "Error inconsistent number of dip patches");
            ierr = FF_STRUCT_ERROR;       
        }
        if (ff->fp[ifp].lon_vtx == NULL ||
            ff->fp[ifp].lat_vtx == NULL ||
            ff->fp[ifp].dep_vtx == NULL ||
            ff->fp[ifp].fault_xutm == NULL ||
            ff->fp[ifp].fault_yutm == NULL ||
            ff->fp[ifp].fault_alt == NULL ||
            ff->fp[ifp].strike == NULL ||
            ff->fp[ifp].dip == NULL ||
            ff->fp[ifp].length == NULL ||
            ff->fp[ifp].width == NULL ||
            ff->fp[ifp].sslip == NULL ||
            ff->fp[ifp].dslip == NULL ||
            ff->fp[ifp].sslip_unc == NULL ||
            ff->fp[ifp].dslip_unc == NULL)
        {
            if (ff->fp[ifp].lon_vtx == NULL)
            {
                LOG_ERRMSG("Error lon_vtx null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].lat_vtx == NULL)
            {
                LOG_ERRMSG("Error lat_vtx null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].dep_vtx == NULL)
            {
                LOG_ERRMSG("Error dep_vtx null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].fault_xutm == NULL)
            {
                LOG_ERRMSG(" Error fault_xutm null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].fault_yutm == NULL)
            {
                LOG_ERRMSG("Error fault_xutm null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].fault_alt == NULL)
            {
                LOG_ERRMSG("Error fault_alt null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].strike == NULL)
            {
                LOG_ERRMSG("Error strike null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].dip == NULL)
            {
                LOG_ERRMSG("Error dip null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].length == NULL)
            {
                LOG_ERRMSG("Error length null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].width == NULL)
            {
                LOG_ERRMSG("Error width null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].sslip == NULL)
            {
                LOG_ERRMSG("Error sslip null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].dslip == NULL)
            {
                LOG_ERRMSG("Error dslip null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].sslip_unc == NULL)
            {
                LOG_ERRMSG("Error sslip_unc null on ifp %d", ifp+1);
            }
            if (ff->fp[ifp].dslip_unc == NULL)
            {
                LOG_ERRMSG("Error dslip_unc null on ifp %d", ifp+1);
            }
            LOG_ERRMSG("Error fp %d is invalid\n", ifp+1);
            ierr = FF_STRUCT_ERROR;
            goto ERROR;
        }
    } // Loop on fault planes
    // Avoid a segfault
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        if (ff->fp[ifp].maxobs != ff_data.nsites)
        {
            LOG_ERRMSG("nsites on ff and ff_data differs %d %d",
                       ff->fp[ifp].maxobs, ff_data.nsites);
            ierr = FF_STRUCT_ERROR;
            goto ERROR;
        }
    }
ERROR:;
    return ierr;
}
