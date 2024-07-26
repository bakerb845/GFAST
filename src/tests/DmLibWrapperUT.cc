/**
 * @file DmLibWrapperUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the dmlib/ directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"
#include "ut_xml_utils.h"

#include "gfast.h"
#include "gfast_eewUtils.h"
#include "dmlibWrapper.h"

#include "gfast_ut_utils.h"

#include "iscl/memory/memory.h"
#include "iscl/array/array.h"
#include "iscl/iscl/iscl_enum.h"


TEST(DmLibWrapper, testCreatePGDXml) {
    char *pgdXML;
    const char *filenm = "data/final_pgd.maule.txt\0";
    const char *program_instance = "gfast@eew-uw-dev1\0";
    const char *message_type = "update\0";
    struct GFAST_pgd_props_struct pgd_props;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_pgdResults_struct pgd_ref, pgd;
    double SA_lat, SA_lon, SA_dep, age_of_event;
    int i, ierr, max_assoc_stations, pgdOpt;
    double currentTime = 0;
    enum isclError_enum ierr_iscl;

    pgdXML = NULL;
    memset(&pgd_props, 0, sizeof(pgd_props));
    memset(&pgd_data, 0, sizeof(pgd_data));
    memset(&pgd_ref, 0, sizeof(pgd_ref));
    memset(&pgd, 0, sizeof(pgd));
    age_of_event = 0;
    max_assoc_stations = 1;
    ierr = read_pgd_results(filenm,
                            &pgd_props,
                            &pgd_data,
                            &pgd_ref,
                            &SA_lat, &SA_lon, &SA_dep);

    EXPECT_EQ(0, ierr) << "Error reading input file";
    
    // Set space
    pgd.nsites = pgd_ref.nsites;
    pgd.ndeps = pgd_ref.ndeps;
    pgd.nlats = 1;
    pgd.nlons = 1;
    pgd.mpgd    = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.mpgd_sigma = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.mpgd_vr = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.dep_vr_pgd = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.iqr = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.UP = ISCL_memory_calloc__double(pgd.ndeps*pgd.nsites);
    pgd.UPinp = ISCL_memory_calloc__double(pgd.nsites);
    pgd.srcDepths = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.srdist = ISCL_memory_calloc__double(pgd.ndeps*pgd.nsites);
    pgd.lsiteUsed = ISCL_memory_calloc__bool(pgd.nsites);
    for (i=0; i<pgd.ndeps; i++)
    {
        pgd.srcDepths[i] = pgd_ref.srcDepths[i];
    }
    ierr = eewUtils_drivePGD(pgd_props,
                             SA_lat, SA_lon, SA_dep, age_of_event,
                             pgd_data, &pgd);

    EXPECT_EQ(0, ierr) << "Error computing PGD!";

    struct coreInfo_struct core;
    memset(&core, 0, sizeof(struct coreInfo_struct));
    ierr = eewUtils_fillCoreEventInfo("123\0", 1,
        SA_lat, SA_lon, SA_dep, 6, 0, 0, &core);
    
    pgdOpt = array_argmax64f(pgd.ndeps, pgd.dep_vr_pgd, &ierr_iscl);
    core.depth = pgd.srcDepths[pgdOpt];
    core.mag = pgd.mpgd[pgdOpt];
    core.magUncer = pgd.mpgd_sigma[pgdOpt];
    core.numStations = pgd_data.nsites;

    ierr = startEventEncoder();
    pgdXML = dmlibWrapper_createPGDXML(
        currentTime,
        REAL_TIME_EEW,
        GFAST_VERSION,
        program_instance,
        message_type,
        max_assoc_stations,
        &core,
        &pgd,
        &pgd_data,
        &ierr);

    EXPECT_EQ(core.numStations, tests::count_elements("pgd_obs", "obs", pgdXML));
    EXPECT_NEAR(std::stod(tests::extract_xml_value("mag", pgdXML)), core.mag, 1e-4);
    EXPECT_DOUBLE_EQ(std::stod(tests::extract_xml_value("lat", pgdXML)), SA_lat);
    EXPECT_DOUBLE_EQ(std::stod(tests::extract_xml_value("lon", pgdXML)), SA_lon);
    EXPECT_DOUBLE_EQ(std::stoi(tests::extract_xml_value("num_stations", pgdXML)), core.numStations);
    EXPECT_EQ(tests::extract_xml_attribute("mag", "units", pgdXML), "Mw");
    EXPECT_EQ(tests::extract_xml_attribute("orig_time", "units", pgdXML), "UTC");

    // Clean up
    core_scaling_pgd_finalizeData(&pgd_data);
    core_scaling_pgd_finalizeResults(&pgd);
    core_scaling_pgd_finalizeResults(&pgd_ref);
    free(pgdXML);

}