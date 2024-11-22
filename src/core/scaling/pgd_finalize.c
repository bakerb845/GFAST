#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Releases the memory on the PGD data data structure
 *
 * @param[out] pgd_data    freed PGD data data structure
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_scaling_pgd_finalizeData(
     struct GFAST_peakDisplacementData_struct *pgd_data)
{
    int i;
    // Free the PGD data data
    if (pgd_data->stnm != NULL)
    {   
        for (i=0; i<pgd_data->nsites; i++)
        {
            free(pgd_data->stnm[i]);
        }
        free(pgd_data->stnm);
    }   
    memory_free64f(&pgd_data->pd);
    memory_free64f(&pgd_data->wt);
    memory_free64f(&pgd_data->sta_lat);
    memory_free64f(&pgd_data->sta_lon);
    memory_free64f(&pgd_data->sta_alt);
    memory_free64f(&pgd_data->pd_time);
    memory_free8l(&pgd_data->lmask);
    memory_free8l(&pgd_data->lactive);
    memset(pgd_data, 0, sizeof(struct GFAST_peakDisplacementData_struct));
    return;
}
//============================================================================//
/*!
 * @brief Releases the memory on the PGD result structure
 *
 * @param[out] pgd         freed PGD results data structure
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_scaling_pgd_finalizeResults(
    struct GFAST_pgdResults_struct *pgd)
{
    memory_free64f(&pgd->mpgd);
    memory_free64f(&pgd->mpgd_sigma);
    memory_free64f(&pgd->mpgd_vr);
    memory_free64f(&pgd->dep_vr_pgd);
    memory_free64f(&pgd->iqr);
    memory_free64f(&pgd->UP);
    memory_free64f(&pgd->UPinp);
    memory_free64f(&pgd->srcDepths);
    memory_free64f(&pgd->srcLats);
    memory_free64f(&pgd->srcLons);
    memory_free64f(&pgd->srdist);
    memory_free8l(&pgd->lsiteUsed);
    memset(pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    return;
}
//============================================================================//
/*!
 * @brief Releases the memory on the PGD data structures
 *
 * @param[out] pgd_props   nulled out PGD properties structure
 * @param[out] pgd_data    freed PGD data data structure
 * @param[out] pgd         freed PGD results data structure
 *
 * @author Ben Baker (ISTI)
 *
 */ 
void core_scaling_pgd_finalize(
    struct GFAST_pgd_props_struct *pgd_props,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    struct GFAST_pgdResults_struct *pgd)
{
    core_properties_finalizePGDProperties(pgd_props);
    core_scaling_pgd_finalizeData(pgd_data);
    core_scaling_pgd_finalizeResults(pgd);
    return;
}
