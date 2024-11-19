#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Allocates space for the CMT grid search 
 *
 * @param[in] props      holds the CMT parameters
 * @param[in] gps_data   holds the site stream length
 *
 * @param[out] cmt       has space allocated for the CMT depth gridsearch
 * @param[out] cmt_data  has space sufficient to carry the offset data
 *                       and weights.  also, contains an initial data
 *                       mask.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @bug Can't accept arbitrary source depths from input file
 *
 */
int core_cmt_initialize(struct GFAST_cmt_props_struct props,
                        struct GFAST_data_struct gps_data,
                        struct GFAST_cmtResults_struct *cmt,
                        struct GFAST_offsetData_struct *cmt_data)
{
    int i, nlld;
    //------------------------------------------------------------------------//
    cmt->ndeps = props.ngridSearch_deps;
    cmt->nlats = props.ngridSearch_lats;
    cmt->nlons = props.ngridSearch_lons;
    if (cmt->ndeps < 1 || cmt->nlats < 1 || cmt->nlons < 1)
    {
        if (cmt->ndeps < 1)
        {
            LOG_ERRMSG("No depths in CMT grid search %d\n",
                       props.ngridSearch_deps);
        }
        if (cmt->nlats < 1)
        {
            LOG_ERRMSG("No lats in CMT grid search %d\n",
                       props.ngridSearch_lats);
        }
        if (cmt->nlons < 1)
        {
            LOG_ERRMSG("No lons in CMT grid search %d\n",
                       props.ngridSearch_lons);
        }
        return -1;
    }
    nlld = cmt->nlats * cmt->nlons * cmt->ndeps;
    cmt->nsites = gps_data.stream_length;
    if (gps_data.stream_length < 1)
    {
        LOG_ERRMSG("Error insufficient data to estimate CMT %d\n",
                   gps_data.stream_length);
    }
    // data
    cmt_data->stnm = (char **)calloc((size_t) gps_data.stream_length,
                                     sizeof(char *));
    cmt_data->ubuff   = memory_calloc64f(gps_data.stream_length);
    cmt_data->nbuff   = memory_calloc64f(gps_data.stream_length); 
    cmt_data->ebuff   = memory_calloc64f(gps_data.stream_length);
    cmt_data->wtu     = memory_calloc64f(gps_data.stream_length);
    cmt_data->wtn     = memory_calloc64f(gps_data.stream_length); 
    cmt_data->wte     = memory_calloc64f(gps_data.stream_length);
    cmt_data->sta_lat = memory_calloc64f(gps_data.stream_length);
    cmt_data->sta_lon = memory_calloc64f(gps_data.stream_length);
    cmt_data->sta_alt = memory_calloc64f(gps_data.stream_length);
    cmt_data->lmask   = memory_calloc8l(gps_data.stream_length);
    cmt_data->lactive = memory_calloc8l(gps_data.stream_length);
    cmt_data->nsites = gps_data.stream_length;
    for (i=0; i<cmt_data->nsites; i++)
    {
        cmt_data->sta_lat[i] = gps_data.data[i].sta_lat;
        cmt_data->sta_lon[i] = gps_data.data[i].sta_lon;
        cmt_data->sta_alt[i] = gps_data.data[i].sta_alt;
        cmt_data->stnm[i] = (char *)calloc(64, sizeof(char));
        strcpy(cmt_data->stnm[i], gps_data.data[i].netw);
        strcat(cmt_data->stnm[i], ".\0");
        strcat(cmt_data->stnm[i], gps_data.data[i].stnm);
        strcat(cmt_data->stnm[i], ".\0");
        strncpy(cmt_data->stnm[i], gps_data.data[i].chan[0], 2);
        strcat(cmt_data->stnm[i], "?.\0");
        if (strlen(gps_data.data[i].loc) > 0)
        {
            strcat(cmt_data->stnm[i], gps_data.data[i].loc);
        } 
        if (gps_data.data[i].lskip_cmt){cmt_data->lmask[i] = true;}
    }
    // cmt structure
    cmt->l2        = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->pct_dc    = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->objfn     = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->mts       = memory_calloc64f(6*nlld); //6*ndeps*nlats*nlons);
    cmt->str1      = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->str2      = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->dip1      = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->dip2      = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->rak1      = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->rak2      = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->Mw        = memory_calloc64f(nlld); //ndeps*nlats*nlons);
    cmt->srcLats   = memory_calloc64f(cmt->nlats); //nlats);
    cmt->srcLons   = memory_calloc64f(cmt->nlons); //nlons);
    cmt->srcDepths = memory_calloc64f(cmt->ndeps); //ndeps);
    cmt->EN        = memory_calloc64f(cmt->nsites*nlld); //nsites*ndeps*nlats*nlons);
    cmt->NN        = memory_calloc64f(cmt->nsites*nlld); //nsites*ndeps*nlats*nlons);
    cmt->UN        = memory_calloc64f(cmt->nsites*nlld); //nsites*ndeps*nlats*nlons);
    cmt->Einp      = memory_calloc64f(cmt->nsites);
    cmt->Ninp      = memory_calloc64f(cmt->nsites);
    cmt->Uinp      = memory_calloc64f(cmt->nsites);
    cmt->lsiteUsed = memory_calloc8l(cmt->nsites);
    /* TODO fix me */
    for (i = 0; i < cmt->ndeps; i++)
    {
        cmt->srcDepths[i] = (double) (i + 1);
    }
    // srcLats is a relative array centered at 0, to be added to the input latitude
    // The first latitude will be -dLat*(nlats - 1)/2
    for (i = 0; i < cmt->nlats; i++)
    {
        cmt->srcLats[i] = props.dLat * (i - (cmt->nlats - 1) / 2);
    }
    // srcLons is a relative array centered at 0, to be added to the input longitude
    // The first longitude will be -dLon*(nlons - 1)/2
    for (i = 0; i < cmt->nlons; i++)
    {
        cmt->srcLons[i] = props.dLon * (i - (cmt->nlons - 1) / 2);
    }
    return 0;
}
