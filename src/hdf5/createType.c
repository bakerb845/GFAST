#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <math.h>
#include "gfast.h"

/*!
 * @brief Creates the peak displacement data type
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t GFAST_HDF5__createType__peakDisplacementData(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__peakDisplacementData\0";
    hid_t dataType, vlenCData, vlenDData, vlenIData, string64Type;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "peakDisplacementDataStructure\0",
                  H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    // String data type 
    string64Type = H5Tcopy(H5T_C_S1);
    H5Tset_size(string64Type, 64);
    vlenCData = H5Tvlen_create(string64Type); 
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_peakDisplacementData_struct));
    ierr += H5Tinsert(dataType, "stationName\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, stnm),
                      vlenCData);
    ierr += H5Tinsert(dataType, "peakDisplacement\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, pd),
                      vlenDData);
    ierr += H5Tinsert(dataType, "weight\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, wt),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteLatitude\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_lat),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteLongitude\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_lon),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteElevation\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_alt),
                      vlenDData);
    ierr += H5Tinsert(dataType, "isMasked\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, lmask),
                      vlenIData);
    ierr += H5Tinsert(dataType, "isActive\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, lactive),
                      vlenIData);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, nsites),
                      H5T_NATIVE_INT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "peakDisplacementDataStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create pgd data structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(vlenCData);
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(vlenIData);
    ierr += H5Tclose(string64Type);
    ierr += H5Tclose(dataType);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the PGD results structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t GFAST_HDF5__createType__pgdResults(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__pgdResults\0";
    hid_t dataType, vlenDData, vlenIData;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "pgdResultsStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_pgdResults_struct));
    ierr += H5Tinsert(dataType, "Magnitude\0",
                      HOFFSET(struct h5_pgdResults_struct, mpgd),
                      vlenDData);
    ierr += H5Tinsert(dataType, "ObjectiveFunction\0",
                      HOFFSET(struct h5_pgdResults_struct, mpgd_vr),
                      vlenDData);
    ierr += H5Tinsert(dataType, "PGDEstimates\0",
                      HOFFSET(struct h5_pgdResults_struct, UP),
                      vlenDData);
    ierr += H5Tinsert(dataType, "PGDObservations\0",
                      HOFFSET(struct h5_pgdResults_struct, UPinp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "sourceDepth\0",
                      HOFFSET(struct h5_pgdResults_struct, srcDepths),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteUsed\0",
                      HOFFSET(struct h5_pgdResults_struct, lsiteUsed),
                      vlenIData);
    ierr += H5Tinsert(dataType, "numberOfGridsearchDepths\0",
                      HOFFSET(struct h5_pgdResults_struct, ndeps),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_pgdResults_struct, nsites),
                      H5T_NATIVE_INT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "pgdResultsStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create pgd results structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(vlenIData);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the peak displacement data type
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t GFAST_HDF5__createType__offsetData(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__offsetData\0";
    hid_t dataType, vlenCData, vlenDData, vlenIData, string64Type;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "offsetDataStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    // String data type 
    string64Type = H5Tcopy(H5T_C_S1);
    H5Tset_size(string64Type, 64);
    vlenCData = H5Tvlen_create(string64Type);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_offsetData_struct));
    ierr += H5Tinsert(dataType, "stationName\0",
                      HOFFSET(struct h5_offsetData_struct, stnm),
                      vlenCData);
    ierr += H5Tinsert(dataType, "verticalOffset\0",
                      HOFFSET(struct h5_offsetData_struct, ubuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northOffset\0",
                      HOFFSET(struct h5_offsetData_struct, nbuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastOffset\0",
                      HOFFSET(struct h5_offsetData_struct, ebuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "verticalWeight\0",
                      HOFFSET(struct h5_offsetData_struct, wtu),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northWeight\0",
                      HOFFSET(struct h5_offsetData_struct, wtn),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastWeight\0",
                      HOFFSET(struct h5_offsetData_struct, wte),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteLatitude\0",
                      HOFFSET(struct h5_offsetData_struct, sta_lat),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteLongitude\0",
                      HOFFSET(struct h5_offsetData_struct, sta_lon),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteElevation\0",
                      HOFFSET(struct h5_offsetData_struct, sta_alt),
                      vlenDData);
    ierr += H5Tinsert(dataType, "isMasked\0",
                      HOFFSET(struct h5_offsetData_struct, lmask),
                      vlenIData);
    ierr += H5Tinsert(dataType, "isActive\0",
                      HOFFSET(struct h5_offsetData_struct, lactive),
                      vlenIData);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_offsetData_struct, nsites),
                      H5T_NATIVE_INT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "offsetDataStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create offset data structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(vlenIData);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the CMT results structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t GFAST_HDF5__createType__cmtResults(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__cmtResults\0";
    hid_t dataType, vlenDData, vlenIData;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "cmtResultsStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_cmtResults_struct));
    ierr += H5Tinsert(dataType, "ObjectiveFunction\0",
                      HOFFSET(struct h5_cmtResults_struct, objfn),
                      vlenDData);
    ierr += H5Tinsert(dataType, "momentTensors\0",
                      HOFFSET(struct h5_cmtResults_struct, mts),
                      vlenDData);
    ierr += H5Tinsert(dataType, "strikesFaultPlane1\0",
                      HOFFSET(struct h5_cmtResults_struct, str1),
                      vlenDData);
    ierr += H5Tinsert(dataType, "dipFaultPlane1\0",
                      HOFFSET(struct h5_cmtResults_struct, dip1),
                      vlenDData);
    ierr += H5Tinsert(dataType, "rakeFaultPlane1\0",
                      HOFFSET(struct h5_cmtResults_struct, rak1),
                      vlenDData);
    ierr += H5Tinsert(dataType, "strikesFaultPlane2\0",
                      HOFFSET(struct h5_cmtResults_struct, str2),
                      vlenDData);
    ierr += H5Tinsert(dataType, "dipFaultPlane2\0",
                      HOFFSET(struct h5_cmtResults_struct, dip2),
                      vlenDData);
    ierr += H5Tinsert(dataType, "rakeFaultPlane2\0",
                      HOFFSET(struct h5_cmtResults_struct, rak2),
                      vlenDData);
    ierr += H5Tinsert(dataType, "momentMagnitudes\0",
                      HOFFSET(struct h5_cmtResults_struct, Mw),
                      vlenDData);
    ierr += H5Tinsert(dataType, "sourceDepths\0",
                      HOFFSET(struct h5_cmtResults_struct, srcDepths),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastEstimates\0",
                      HOFFSET(struct h5_cmtResults_struct, EN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northEstimates\0",
                      HOFFSET(struct h5_cmtResults_struct, NN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "upEstimates\0",
                      HOFFSET(struct h5_cmtResults_struct, UN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastObservedOffset\0",
                      HOFFSET(struct h5_cmtResults_struct, Einp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northObservedOffset\0",
                      HOFFSET(struct h5_cmtResults_struct, Ninp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "upObservedOffset\0",
                      HOFFSET(struct h5_cmtResults_struct, Uinp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteUsed\0",
                      HOFFSET(struct h5_cmtResults_struct, lsiteUsed),
                      vlenIData);

    ierr += H5Tinsert(dataType, "optimumIndex\0",
                      HOFFSET(struct h5_cmtResults_struct, opt_indx),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfGridsearchDepths\0",
                      HOFFSET(struct h5_cmtResults_struct, ndeps),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_cmtResults_struct, nsites),
                      H5T_NATIVE_INT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "cmtResultsStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create cmt results structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(vlenIData);
    ierr += H5Tclose(vlenDData);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the finite fault fault plane structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t GFAST_HDF5__createType__faultPlane(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__faultPlane\0";
    hid_t dataType, vlenDData, vlenIData;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "faultPlaneStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_faultPlane_struct));
    ierr += H5Tinsert(dataType, "longitudeVertices\0",
                      HOFFSET(struct h5_faultPlane_struct, lon_vtx),
                      vlenDData);
    ierr += H5Tinsert(dataType, "latitudeVertices\0",
                      HOFFSET(struct h5_faultPlane_struct, lat_vtx),
                      vlenDData);
    ierr += H5Tinsert(dataType, "depthVertices\0",
                      HOFFSET(struct h5_faultPlane_struct, dep_vtx),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPatchEastingUTM\0",
                      HOFFSET(struct h5_faultPlane_struct, fault_xutm),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPatchNorthingUTM\0",
                      HOFFSET(struct h5_faultPlane_struct, fault_yutm),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPatchDepth\0",
                      HOFFSET(struct h5_faultPlane_struct, fault_alt),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultStrike\0",
                      HOFFSET(struct h5_faultPlane_struct, strike),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultDip\0",
                      HOFFSET(struct h5_faultPlane_struct, dip),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultLength\0",
                      HOFFSET(struct h5_faultPlane_struct, length),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultWidth\0",
                      HOFFSET(struct h5_faultPlane_struct, width),
                      vlenDData);
    ierr += H5Tinsert(dataType, "slipAlongStrike\0",
                      HOFFSET(struct h5_faultPlane_struct, sslip),
                      vlenDData);
    ierr += H5Tinsert(dataType, "slipAlongDip\0",
                      HOFFSET(struct h5_faultPlane_struct, dslip),
                      vlenDData);
    ierr += H5Tinsert(dataType, "slipAlongStrikeUncertainty\0",
                      HOFFSET(struct h5_faultPlane_struct, sslip_unc),
                      vlenDData);
    ierr += H5Tinsert(dataType, "slipAlongDipUncertainty\0",
                      HOFFSET(struct h5_faultPlane_struct, dslip_unc),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastEstimateOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, EN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northEstimateOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, NN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "upEstimateOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, UN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastObservedOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, Einp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northObservedOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, Ninp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "upObservedOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, Uinp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPointerStructure\0",
                      HOFFSET(struct h5_faultPlane_struct, fault_ptr),
                      vlenIData);
    ierr += H5Tinsert(dataType, "maxObservations\0",
                      HOFFSET(struct h5_faultPlane_struct, maxobs),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfSitesUsed\0",
                      HOFFSET(struct h5_faultPlane_struct, nsites_used),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfFaltPatchesAlongStrike\0",
                      HOFFSET(struct h5_faultPlane_struct, nstr),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfFaultPatchesAlongDip\0",
                      HOFFSET(struct h5_faultPlane_struct, ndip),
                      H5T_NATIVE_INT);
    // Commit it
    ierr = H5Tcommit(group_id, "faultPlaneStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create fault plane structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(vlenIData);
    ierr += H5Tclose(vlenDData);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the finite fault results structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t GFAST_HDF5__createType__ffResults(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__ffResults\0";
    hid_t dataType, faultType, vlenDData, vlenFault;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "finiteFaultResultsStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    if (H5Lexists(group_id, "faultPlaneStructure\0", H5P_DEFAULT) == 0)
    {
        log_warnF("%s: Making fault plane structure\n", fcnm);
        ierr = GFAST_HDF5__createType__faultPlane(group_id);
        if (ierr != 0)
        {
            log_errorF("%s: ERror making fault plane structure\n", fcnm);
            return ierr;
        }
    }
    faultType = H5Topen(group_id, "faultPlaneStructure\0", H5P_DEFAULT);
    vlenFault = H5Tvlen_create(faultType);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_ffResults_struct));
    ierr += H5Tinsert(dataType, "faultPlanes\0",
                      HOFFSET(struct h5_ffResults_struct, fp),
                      vlenFault);
    ierr += H5Tinsert(dataType, "varianceReduction\0",
                      HOFFSET(struct h5_ffResults_struct, vr),
                      vlenDData);
    ierr += H5Tinsert(dataType, "momentMagnitude\0",
                      HOFFSET(struct h5_ffResults_struct, Mw),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPlaneStrikes\0",
                      HOFFSET(struct h5_ffResults_struct, str),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPlaneDips\0",
                      HOFFSET(struct h5_ffResults_struct, dip),
                      vlenDData);
    ierr += H5Tinsert(dataType, "sourceLatitude\0",
                      HOFFSET(struct h5_ffResults_struct, SA_lat),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "sourceLongitude\0",
                      HOFFSET(struct h5_ffResults_struct, SA_lon),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "sourceMagnitude\0",
                      HOFFSET(struct h5_ffResults_struct, SA_mag),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "preferredFaultPlane\0",
                      HOFFSET(struct h5_ffResults_struct,
                              preferred_fault_plane),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfFaultPlanes\0",
                      HOFFSET(struct h5_ffResults_struct, nfp),
                      H5T_NATIVE_INT);
    // Commit it
    ierr = H5Tcommit(group_id, "finiteFaultResultsStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create ff results structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(faultType);
    ierr += H5Tclose(vlenFault);
    ierr += H5Tclose(vlenDData);
    return ierr;
}
