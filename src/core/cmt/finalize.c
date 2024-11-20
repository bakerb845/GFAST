#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Free the CMT results structure.
 *
 * @param[out] cmt   CMT results structure with memory to be freed.
 *
 * @author Ben Baker, ISTI
 *
 */
void core_cmt_finalizeResults(struct GFAST_cmtResults_struct *cmt)
{
    if (cmt == NULL){return;}
    memory_free64f(&cmt->l2);
    memory_free64f(&cmt->pct_dc);
    memory_free64f(&cmt->objfn);
    memory_free64f(&cmt->mts);
    memory_free64f(&cmt->str1);
    memory_free64f(&cmt->str2);
    memory_free64f(&cmt->dip1);
    memory_free64f(&cmt->dip2);
    memory_free64f(&cmt->rak1);
    memory_free64f(&cmt->rak2);
    memory_free64f(&cmt->Mw);
    memory_free64f(&cmt->srcLats);
    memory_free64f(&cmt->srcLons);
    memory_free64f(&cmt->srcDepths);
    memory_free64f(&cmt->EN);
    memory_free64f(&cmt->NN);
    memory_free64f(&cmt->UN);
    memory_free64f(&cmt->Einp);
    memory_free64f(&cmt->Ninp);
    memory_free64f(&cmt->Uinp);
    memory_free8l(&cmt->lsiteUsed);
    memset(cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with the offset data.
 *
 * @param[out] offset_data     Offset data structure with memory to be freed.
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_cmt_finalizeOffsetData(struct GFAST_offsetData_struct *offset_data)
{
    GFAST_core_ff_finalizeOffsetData(offset_data);
    return;
}
//============================================================================//
/*!
 * @brief Convenience function for freeing the CMT data structures and data.
 *
 * @param[out] cmt_props      Nulled out CMT properties.
 * @param[out] offset_data    Offset data structure whose memory has been
 *                            freed and variables set to 0.
 * @param[out] cmt            CMT results structure whose memory hsa been
 *                            freed and variables set to 0.
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_cmt_finalize(struct GFAST_cmt_props_struct *cmt_props,
                       struct GFAST_offsetData_struct *offset_data,
                       struct GFAST_cmtResults_struct *cmt)
{
    core_properties_finalizeCMTProperties(cmt_props);
    core_cmt_finalizeOffsetData(offset_data);
    core_cmt_finalizeResults(cmt);
    return;
}

