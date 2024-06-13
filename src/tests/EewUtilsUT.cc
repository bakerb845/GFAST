/**
 * @file EewUtilsUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the eewUtils directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"
#include "ut_xml_utils.h"
#include "ut_file_utils.h"

#include "gfast.h"

#include "gfast_ut_utils.h"
#include "iscl/memory/memory.h"


/**
 * Test drivePGD. Based on gfast/unit_tests/readCoreInfo.c by Ben Baker
 */
TEST(Eewutils, testParseXML) {

    FILE *xmlfl;
    struct GFAST_shakeAlert_struct SA;
    char *message = NULL;
    long fsize;
    int ierr;
    // Read the message into memory
    xmlfl = fopen("data/sa_message.xml", "rb");
    fseek(xmlfl, 0, SEEK_END);
    fsize = ftell(xmlfl);
    rewind(xmlfl);
    message = (char *)calloc((size_t) (fsize + 1), sizeof(char)); // +1 to null terminate

    ASSERT_NE(0, (int) fread(message, (size_t) fsize, 1, xmlfl));
    fclose(xmlfl);

    ierr = eewUtils_parseCoreXML(message, SA_NAN, &SA);
    free(message);

    ASSERT_EQ(0, ierr);
    // Check values
    EXPECT_STREQ("1663316440", SA.eventid);
    // EXPECT_EQ(1663316440, SA.eventid);
    EXPECT_EQ(2.0021, SA.mag);
    EXPECT_EQ(36.4340, SA.lat);
    // GFAST converts -120 to 240, this should be fixed eventually
    double lon_value = -120.8092;
    EXPECT_TRUE((lon_value == SA.lon) || (360 + lon_value == SA.lon));
    EXPECT_EQ(8.000, SA.dep);

}

/**
 * Test drivePGD. Based on gfast/unit_tests/pgd.c by Ben Baker
 */
TEST(Eewutils, testDrivePGD) {

    const char *filenm = "data/final_pgd.maule.txt\0";
    struct GFAST_pgd_props_struct pgd_props;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_pgdResults_struct pgd_ref, pgd;
    double SA_lat, SA_lon, SA_dep, age_of_event;
    const double tol = 1.e-4;
    int i, ierr;
    memset(&pgd_props, 0, sizeof(pgd_props));
    memset(&pgd_data, 0, sizeof(pgd_data));
    memset(&pgd_ref, 0, sizeof(pgd_ref));
    memset(&pgd, 0, sizeof(pgd));
    age_of_event = 0;
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
    for (i=0; i<pgd.ndeps; i++)
    {
        // EXPECT_NEAR(pgd_ref.mpgd[i], pgd.mpgd[i], tol) << "Error mpgd is wrong";
        // EXPECT_NEAR(pgd_ref.mpgd_vr[i], pgd.mpgd_vr[i], tol) << "Error mpgd_vr is wrong";
        // EXPECT_NEAR(pgd_ref.iqr[i], pgd.iqr[i], tol) << "Error iqr is wrong";
        EXPECT_TRUE(lequal(pgd_ref.mpgd[i], pgd.mpgd[i], tol)) << "Error mpgd is wrong";
        EXPECT_TRUE(lequal(pgd_ref.mpgd_vr[i], pgd.mpgd_vr[i], tol)) << "Error mpgd_vr is wrong";
        EXPECT_TRUE(lequal(pgd_ref.iqr[i], pgd.iqr[i], tol)) << "Error iqr is wrong";
    }
    // Clean up
    core_scaling_pgd_finalizeData(&pgd_data);
    core_scaling_pgd_finalizeResults(&pgd);
    core_scaling_pgd_finalizeResults(&pgd_ref);
}

/**
 * Test driveCMT. Based on gfast/unit_tests/cmt.c by Ben Baker
 */
TEST(Eewutils, testDriveCMT) {
    const char *filenm = "data/final_cmt.maule.txt\0";
    struct GFAST_cmt_props_struct cmt_props;
    struct GFAST_offsetData_struct cmt_data;
    struct GFAST_cmtResults_struct cmt_ref, cmt; 
    double SA_lat, SA_lon, SA_dep;
    int i, ierr, j;
    memset(&cmt_props, 0, sizeof(cmt_props));
    memset(&cmt_data, 0, sizeof(cmt_data));
    memset(&cmt_ref, 0, sizeof(cmt_ref));
    memset(&cmt, 0, sizeof(cmt));
    ierr = read_cmt_results(filenm,
                        &cmt_props,
                        &cmt_data,
                        &cmt_ref,
                        &SA_lat, &SA_lon, &SA_dep);

    EXPECT_EQ(0, ierr) << "Error reading input file";
    // Set space
    cmt.nsites = cmt_ref.nsites;
    cmt.ndeps = cmt_ref.ndeps;
    cmt.nlats = cmt_ref.nlats;
    cmt.nlons = cmt_ref.nlons;
    cmt.l2 = memory_calloc64f(cmt.ndeps);
    cmt.pct_dc = memory_calloc64f(cmt.ndeps);
    cmt.objfn = memory_calloc64f(cmt.ndeps);
    cmt.mts = memory_calloc64f(cmt.ndeps*6);
    cmt.str1 = memory_calloc64f(cmt.ndeps);
    cmt.str2 = memory_calloc64f(cmt.ndeps);
    cmt.dip1 = memory_calloc64f(cmt.ndeps);
    cmt.dip2 = memory_calloc64f(cmt.ndeps);
    cmt.rak1 = memory_calloc64f(cmt.ndeps);
    cmt.rak2 = memory_calloc64f(cmt.ndeps);
    cmt.Mw = memory_calloc64f(cmt.ndeps);
    cmt.srcDepths = memory_calloc64f(cmt.ndeps);
    cmt.EN = memory_calloc64f(cmt.ndeps*cmt_data.nsites);
    cmt.NN = memory_calloc64f(cmt.ndeps*cmt_data.nsites);
    cmt.UN = memory_calloc64f(cmt.ndeps*cmt_data.nsites);
    cmt.Einp = memory_calloc64f(cmt_data.nsites);
    cmt.Ninp = memory_calloc64f(cmt_data.nsites);
    cmt.Uinp = memory_calloc64f(cmt_data.nsites);
    cmt.lsiteUsed = memory_calloc8l(cmt_data.nsites);
    for (i=0; i<cmt.ndeps; i++)
    {
        cmt.srcDepths[i] = cmt_ref.srcDepths[i];
    }
    ierr = eewUtils_driveCMT(cmt_props,
                             SA_lat, SA_lon, SA_dep,
                             cmt_data,
                             &cmt);

    EXPECT_EQ(CMT_SUCCESS, ierr) << "Error computing CMT";
    for (i=0; i<cmt.ndeps; i++)
    {
        EXPECT_TRUE(lequal(cmt.objfn[i], cmt_ref.objfn[i], 1.e-4));
        EXPECT_TRUE(lequal(cmt.Mw[i], cmt_ref.Mw[i], 1.e-3));
        EXPECT_TRUE(lequal(cmt.str1[i], cmt_ref.str1[i], 1.e-4) || 
                    lequal(cmt.str1[i], cmt_ref.str2[i], 1.e-4));
        EXPECT_TRUE(lequal(cmt.str2[i], cmt_ref.str1[i], 1.e-4) || 
                    lequal(cmt.str2[i], cmt_ref.str2[i], 1.e-4));
        EXPECT_TRUE(lequal(cmt.dip1[i], cmt_ref.dip1[i], 1.e-4) || 
                    lequal(cmt.dip1[i], cmt_ref.dip2[i], 1.e-4));
        EXPECT_TRUE(lequal(cmt.dip2[i], cmt_ref.dip1[i], 1.e-4) || 
                    lequal(cmt.dip2[i], cmt_ref.dip2[i], 1.e-4));
        EXPECT_TRUE(lequal(cmt.rak1[i], cmt_ref.rak1[i], 1.e-4) || 
                    lequal(cmt.rak1[i], cmt_ref.rak2[i], 1.e-4));
        EXPECT_TRUE(lequal(cmt.rak2[i], cmt_ref.rak1[i], 1.e-4) || 
                    lequal(cmt.rak2[i], cmt_ref.rak2[i], 1.e-4));

        for (j=0; j<6; j++)
        {
            EXPECT_TRUE(lequal(cmt.mts[6*i+j], cmt_ref.mts[6*i+j], 1.e-4)) << 
                "Error at depth: " << cmt.srcDepths[i] << ", j: " << j;
        } 
    }
    // Clean up
    GFAST_core_cmt_finalizeOffsetData(&cmt_data);
    GFAST_core_cmt_finalizeResults(&cmt);
    GFAST_core_cmt_finalizeResults(&cmt_ref);
}

/**
 * Test eewUtils_fillCoreEventInfo.
 */
TEST(Eewutils, testFillCoreInfo) {
    struct coreInfo_struct core;
    int ierr, version, num_stations;
    double lat, lon, dep, mag, time;
    char eventid[] = "123\0";

    version = 10;
    num_stations = 5;
    lat = 40;
    lon = -120;
    dep = 8;
    mag = 7;
    time = 20;

    memset(&core, 0, sizeof(struct coreInfo_struct));
    ierr = eewUtils_fillCoreEventInfo(eventid, version,
        lat, lon, dep, mag, time, num_stations, &core);

    EXPECT_EQ(0, ierr);
    EXPECT_EQ(version, core.version);
    EXPECT_EQ(num_stations, core.numStations);
    EXPECT_DOUBLE_EQ(lat, core.lat);
    EXPECT_DOUBLE_EQ(lon, core.lon);
    EXPECT_DOUBLE_EQ(dep, core.depth);
    EXPECT_DOUBLE_EQ(mag, core.mag);
    EXPECT_DOUBLE_EQ(time, core.origTime);

}

/*
 * Fixture for testing Finite Fault stuff.
 * Based on gfast/unit_tests/ff.c by Ben Baker
 */
class EewutilsFiniteFaultFixture : public::testing::Test {

    protected:
        struct GFAST_ff_props_struct ff_props;
        struct GFAST_offsetData_struct ff_data;
        struct GFAST_ffResults_struct ff_ref, ff;
        double SA_lat, SA_lon;
        int i, ierr, j, l2;

        void SetUp() {

            const char *fname = "data/final_ff.maule.txt\0";
            memset(&ff_props, 0, sizeof(ff_props));
            memset(&ff_data, 0, sizeof(ff_data));
            memset(&ff_ref, 0, sizeof(ff_ref));
            memset(&ff, 0, sizeof(ff));
            ff_props.verbose = 0;
            ierr = read_ff_results(fname,
                                &ff_props,
                                &ff_data,
                                &ff_ref);
            EXPECT_EQ(0, ierr) << "Error reading results";

            // allocate space for fault
            ff.nfp = ff_ref.nfp;
            ff.fp = (struct GFAST_faultPlane_struct *)
                    calloc((size_t) ff.nfp, sizeof(struct GFAST_faultPlane_struct)); 
            ff.SA_lat = ff_ref.SA_lat;
            ff.SA_lon = ff_ref.SA_lon;
            ff.SA_dep = ff_ref.SA_dep;
            ff.SA_mag = ff_ref.SA_mag;
            SA_lat = ff.SA_lat;
            SA_lon = ff.SA_lon;
            ff.vr = ISCL_memory_calloc__double(ff.nfp);
            ff.Mw = ISCL_memory_calloc__double(ff.nfp); 
            ff.str = ISCL_memory_calloc__double(ff.nfp);
            ff.dip = ISCL_memory_calloc__double(ff.nfp);
            for (i=0; i<ff.nfp; i++)
            {
                ff.fp[i].maxobs = ff_data.nsites;
                ff.fp[i].nstr = ff_props.nstr;
                ff.fp[i].ndip = ff_props.ndip;
                ff.str[i] = ff_ref.str[i];
                ff.dip[i] = ff_ref.dip[i];
                l2 = ff_props.nstr*ff_props.ndip;
                ff.fp[i].lon_vtx = ISCL_memory_calloc__double(4*l2);
                ff.fp[i].lat_vtx = ISCL_memory_calloc__double(4*l2);
                ff.fp[i].dep_vtx = ISCL_memory_calloc__double(4*l2);
                ff.fp[i].fault_xutm = ISCL_memory_calloc__double(l2);
                ff.fp[i].fault_yutm = ISCL_memory_calloc__double(l2);
                ff.fp[i].fault_alt  = ISCL_memory_calloc__double(l2);
                ff.fp[i].strike     = ISCL_memory_calloc__double(l2);
                ff.fp[i].dip        = ISCL_memory_calloc__double(l2);
                ff.fp[i].length     = ISCL_memory_calloc__double(l2);
                ff.fp[i].width      = ISCL_memory_calloc__double(l2);
                ff.fp[i].sslip = ISCL_memory_calloc__double(l2);
                ff.fp[i].dslip = ISCL_memory_calloc__double(l2);
                ff.fp[i].sslip_unc = ISCL_memory_calloc__double(l2);
                ff.fp[i].dslip_unc = ISCL_memory_calloc__double(l2);
                ff.fp[i].Uinp = ISCL_memory_calloc__double(ff.fp[i].maxobs);
                ff.fp[i].UN   = ISCL_memory_calloc__double(ff.fp[i].maxobs);
                ff.fp[i].Ninp = ISCL_memory_calloc__double(ff.fp[i].maxobs);
                ff.fp[i].NN   = ISCL_memory_calloc__double(ff.fp[i].maxobs);
                ff.fp[i].Einp = ISCL_memory_calloc__double(ff.fp[i].maxobs);
                ff.fp[i].EN   = ISCL_memory_calloc__double(ff.fp[i].maxobs);
                ff.fp[i].fault_ptr = ISCL_memory_calloc__int(l2+1);
            }
        }

        void TearDown() {
            GFAST_core_ff_finalizeOffsetData(&ff_data);
            GFAST_core_ff_finalizeResults(&ff_ref);
            GFAST_core_ff_finalizeResults(&ff);
        }
};

TEST_F(EewutilsFiniteFaultFixture, testDriveFF) {
    ierr = eewUtils_driveFF(ff_props,
                            SA_lat, SA_lon,
                            ff_data, &ff);
    EXPECT_EQ(0, ierr) << "Error inverting ff";
    
    l2 = ff_props.nstr*ff_props.ndip;
    for (j=0; j<ff.nfp; j++)
    {
        EXPECT_TRUE(lequal(ff.Mw[j], ff_ref.Mw[j], 1.e-3));
        EXPECT_TRUE(lequal(ff.vr[j], ff_ref.vr[j], 1.e-3));
    }
    for (i=0; i<l2; i++)
    {
        for (j=0; j<ff.nfp; j++)
        {
            EXPECT_TRUE(lequal(ff.fp[j].dslip[i], ff_ref.fp[j].dslip[i], 1.e-3));
            EXPECT_TRUE(lequal(ff.fp[j].sslip[i], ff_ref.fp[j].sslip[i], 1.e-3));
        }
    }
}

TEST_F(EewutilsFiniteFaultFixture, testMakeXmlFF) {
    int ipf, nstrdip;
    ierr = eewUtils_driveFF(ff_props,
                            SA_lat, SA_lon,
                            ff_data, &ff);
    EXPECT_EQ(0, ierr) << "Error inverting ff";

    ipf = ff.preferred_fault_plane;
    nstrdip = ff.fp[ipf].nstr * ff.fp[ipf].ndip;

    char *ffXML = NULL;
    const char *program_instance = "gfast@eew-uw-dev1\0";
    const char *message_type = "update\0";
    char sversion[6];
    snprintf(sversion, 6, "%d", 1);
    const char *version_string = "gfast-1.2.4-2023-10-12.test\0";

    struct coreInfo_struct core;
    memset(&core, 0, sizeof(struct coreInfo_struct));
    eewUtils_fillCoreEventInfo("123\0", 1,
        ff.SA_lat, ff.SA_lon, ff.SA_dep, ff.SA_mag, 0, 0, &core);

    ffXML = eewUtils_makeXML__ff(
        REAL_TIME_EEW,
        "GFAST\0",
        version_string,
        program_instance,
        message_type,
        sversion,
        &core,
        nstrdip,
        ff.fp[ipf].fault_ptr,
        ff.fp[ipf].lat_vtx,
        ff.fp[ipf].lon_vtx,
        ff.fp[ipf].dep_vtx,
        ff.fp[ipf].strike,
        ff.fp[ipf].dip,
        ff.fp[ipf].sslip,
        ff.fp[ipf].dslip,
        ff.fp[ipf].sslip_unc,
        ff.fp[ipf].dslip_unc,
        &ierr); 

    EXPECT_EQ(0, ierr);
    const char *ff_fname = "data/eewUtils_xml_example.ff\0";
    tests::expect_file_no_dates_iso(ff_fname, ffXML);

    free(ffXML);
}

/*
 * Fixture for testing peakDisplacement() function
 * Has two "normal" stations that can be put out of spec in various ways
 * in order to not return peak displacement measurements
 */
class EewutilsFixture : public::testing::Test {
    protected:
        struct GFAST_props_struct props;
        struct GFAST_shakeAlert_struct SA;
        struct GFAST_pgdResults_struct pgd;
        struct GFAST_peakDisplacementData_struct pgd_data;
        struct coreInfo_struct core;
        struct coreInfo_struct last_sent_core;
        
        double lat, lon, dep, mag, time;
        int ierr, version;

        char eventid[16];
        char orig_sys[16];

        // Allow up to this number of stations/observations
        // Use nsites to track how many actually are in use
        const int MAX_STA = 10;

        void SetUp() {
            // Set up so that message will not be throttled/filtered initially
            char propfilename[1024];
            int nloc;
            const enum opmode_type opmode = REAL_TIME_EEW;

            strcpy(eventid, "123\0");
            strcpy(orig_sys, "gfast@test\0");

            // initialize structs
            memset(&props, 0, sizeof(struct GFAST_props_struct));
            memset(&SA, 0, sizeof(struct GFAST_shakeAlert_struct));
            memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
            memset(&pgd_data, 0, sizeof(struct GFAST_peakDisplacementData_struct));
            memset(&core, 0, sizeof(struct coreInfo_struct));
            memset(&last_sent_core, 0, sizeof(struct coreInfo_struct));

            // initialize necessary properties
            strncpy(propfilename,
                    "data/gfast.props",
                    1024-1);
            ierr = 0;
            ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);

            // initialize origin info
            version = 1;
            lat = 46;
            lon = -122;
            dep = 8;
            mag = 6.5;
            time = 0;

            // initialize SA struct
            strcpy(SA.eventid, eventid);
            strcpy(SA.orig_sys, orig_sys);
            SA.version = version;
            SA.lat = lat;
            SA.lon = lon;
            SA.dep = dep;
            SA.mag = mag;
            SA.time = time;

            // initialize pgd results
            pgd.ndeps = 1;
            pgd.nlats = 1;
            pgd.nlons = 1;
            nloc = pgd.ndeps * pgd.nlats * pgd.nlons;
            pgd.mpgd       = memory_calloc64f(nloc);
            pgd.mpgd_sigma = memory_calloc64f(nloc);
            pgd.mpgd_vr    = memory_calloc64f(nloc);
            pgd.dep_vr_pgd = memory_calloc64f(nloc);
            pgd.srcDepths  = memory_calloc64f(pgd.ndeps);
            pgd.iqr        = memory_calloc64f(nloc);
            pgd.UP         = memory_calloc64f(MAX_STA*nloc);
            pgd.srdist     = memory_calloc64f(MAX_STA*nloc);
            pgd.UPinp      = memory_calloc64f(MAX_STA);
            pgd.lsiteUsed  = memory_calloc8l(MAX_STA);
            pgd.nsites     = 0;

            // initialize pgd data
            pgd_data.stnm = (char **)calloc((size_t) MAX_STA,
                                            sizeof(char *));
            pgd_data.pd      = memory_calloc64f(MAX_STA);
            pgd_data.wt      = memory_calloc64f(MAX_STA);
            pgd_data.sta_lat = memory_calloc64f(MAX_STA);
            pgd_data.sta_lon = memory_calloc64f(MAX_STA);
            pgd_data.sta_alt = memory_calloc64f(MAX_STA);
            pgd_data.pd_time = memory_calloc64f(MAX_STA);
            pgd_data.lmask   = memory_calloc8l(MAX_STA);
            pgd_data.lactive = memory_calloc8l(MAX_STA);
            pgd_data.nsites  = 0;

            // initialize current core info
            eewUtils_fillCoreEventInfo(eventid, version,
                lat, lon, dep, mag, time, 0, &core);
            core.magUncer = .4;
            // initialize last core info
            // keep empty for now

            // Add three obs over the 0 second threshold (17.5)
            addObs(.18);
            addObs(.18);
            addObs(.18);
        }

        void TearDown() {
            // Clean up
            core_scaling_pgd_finalizeData(&pgd_data);
            core_scaling_pgd_finalizeResults(&pgd);
        }

        void addObs(const double pd) {
            pgd_data.pd[pgd_data.nsites] = pd;
            pgd.lsiteUsed[pgd.nsites] = true;
            pgd_data.nsites++;
            pgd.nsites++;
        }
};

TEST_F(EewutilsFixture, testSendXMLFilterPass) {
    EXPECT_FALSE(
        eewUtils_sendXMLFilter(&props, &SA, &pgd, &pgd_data, &core, &last_sent_core, 0)
    );
}

TEST_F(EewutilsFixture, testSendXMLFilterPgdThresholdsNotMet) {
    // Make one observation less than the threshold
    pgd_data.pd[0] = .10;
    EXPECT_TRUE(
        eewUtils_sendXMLFilter(&props, &SA, &pgd, &pgd_data, &core, &last_sent_core, 0)
    );
}

TEST_F(EewutilsFixture, testSendXMLFilterLaterPgdThresholdsNotMet) {
    // Change the observation time so threshold is higher
    EXPECT_TRUE(
        eewUtils_sendXMLFilter(&props, &SA, &pgd, &pgd_data, &core, &last_sent_core, 30)
    );
}

TEST_F(EewutilsFixture, testSendXMLFilterSaMagNotMet) {
    SA.mag = 5;
    EXPECT_TRUE(
        eewUtils_sendXMLFilter(&props, &SA, &pgd, &pgd_data, &core, &last_sent_core, 0)
    );
}

TEST_F(EewutilsFixture, testSendXMLFilterMagUncerNotMet) {
    core.magUncer = 1;
    EXPECT_TRUE(
        eewUtils_sendXMLFilter(&props, &SA, &pgd, &pgd_data, &core, &last_sent_core, 0)
    );
}

TEST_F(EewutilsFixture, testSendXMLFilterChangeThresholdsNotMet) {
    eewUtils_fillCoreEventInfo(eventid, version,
        lat, lon, dep, mag, time, 0, &last_sent_core);
    last_sent_core.magUncer = .4;
    
    EXPECT_TRUE(
        eewUtils_sendXMLFilter(&props, &SA, &pgd, &pgd_data, &core, &last_sent_core, 0)
    );
}

TEST_F(EewutilsFixture, testSendXMLFilterChangeThresholdsMet) {
    eewUtils_fillCoreEventInfo(eventid, version,
        lat, lon, dep, mag, time, 0, &last_sent_core);
    // mag uncertainty threshold is 0.05
    last_sent_core.magUncer = .2;
    
    EXPECT_FALSE(
        eewUtils_sendXMLFilter(&props, &SA, &pgd, &pgd_data, &core, &last_sent_core, 0)
    );
}

TEST_F(EewutilsFixture, testChangeThresholdsExceededDefault) {
    eewUtils_fillCoreEventInfo(eventid, version,
        lat, lon, dep, mag, time, 0, &last_sent_core);
    last_sent_core.magUncer = .4;
    props.pgd_props.change_threshold_mag = -1;
    props.pgd_props.change_threshold_mag_uncer = -1;
    props.pgd_props.change_threshold_lat = -1;
    props.pgd_props.change_threshold_lon = -1;
    props.pgd_props.change_threshold_orig_time = -1;
    props.pgd_props.change_threshold_num_stations = -1;
    
    EXPECT_TRUE(
        eewUtils_changeThresholdsExceeded(&props, &core, &last_sent_core)
    );
}

TEST_F(EewutilsFixture, testChangeThresholdsExceededOnlyOneSetExceeded) {
    eewUtils_fillCoreEventInfo(eventid, version,
        lat, lon, dep, mag, time, 0, &last_sent_core);
    last_sent_core.mag = 4;
    props.pgd_props.change_threshold_mag_uncer = -1;
    props.pgd_props.change_threshold_lat = -1;
    props.pgd_props.change_threshold_lon = -1;
    props.pgd_props.change_threshold_orig_time = -1;
    props.pgd_props.change_threshold_num_stations = -1;
    
    EXPECT_TRUE(
        eewUtils_changeThresholdsExceeded(&props, &core, &last_sent_core)
    );
}

TEST_F(EewutilsFixture, testChangeThresholdsExceededOnlyOneSetNotExceeded) {
    eewUtils_fillCoreEventInfo(eventid, version,
        lat, lon, dep, mag, time, 0, &last_sent_core);
    last_sent_core.mag = 6.5;
    props.pgd_props.change_threshold_mag_uncer = -1;
    props.pgd_props.change_threshold_lat = -1;
    props.pgd_props.change_threshold_lon = -1;
    props.pgd_props.change_threshold_orig_time = -1;
    props.pgd_props.change_threshold_num_stations = -1;
    
    EXPECT_FALSE(
        eewUtils_changeThresholdsExceeded(&props, &core, &last_sent_core)
    );
}

TEST_F(EewutilsFixture, testChangeThresholdsExceededConditionsExceeded) {
    eewUtils_fillCoreEventInfo(eventid, version,
        lat, lon, dep, mag, time, 0, &last_sent_core);
    last_sent_core.magUncer = 0.4;
    
    EXPECT_FALSE(
        eewUtils_changeThresholdsExceeded(&props, &core, &last_sent_core)
    );
}

// The eewUtils_makeXML__pgd() function is not used for ShakeAlert
TEST_F(EewutilsFixture, testMakeXmlPGD) {
    char *pgdXML = NULL;
    const char *program_instance = "gfast@eew-uw-dev1\0";
    const char *message_type = "update\0";
    char sversion[6];
    snprintf(sversion, 6, "%d", 1);
    const char *version_string = "gfast-1.2.4-2023-10-12.test\0";

    pgdXML = eewUtils_makeXML__pgd(
        REAL_TIME_EEW,
        "GFAST\0",
        version_string,
        program_instance,
        message_type,
        sversion,
        &core,
        &ierr);

    EXPECT_EQ(0, ierr);
    EXPECT_NEAR(std::stod(tests::extract_xml_value("mag", pgdXML)), core.mag, 1e-4);
    EXPECT_NEAR(std::stod(tests::extract_xml_value("mag_uncer", pgdXML)), core.magUncer, 1e-4);
    EXPECT_DOUBLE_EQ(std::stod(tests::extract_xml_value("lat", pgdXML)), lat);
    EXPECT_DOUBLE_EQ(std::stod(tests::extract_xml_value("lon", pgdXML)), lon);
    EXPECT_DOUBLE_EQ(std::stod(tests::extract_xml_value("depth", pgdXML)), dep);
    // num_stations not included yet
    // EXPECT_DOUBLE_EQ(std::stoi(tests::extract_xml_value("num_stations", pgdXML)), core.numStations);

    const char *pgd_fname = "data/eewUtils_xml_example.pgd\0";
    tests::expect_file_no_dates_iso(pgd_fname, pgdXML);

    free(pgdXML);
}

TEST_F(EewutilsFixture, testMakeXmlCMT) {
    char *cmtQML = NULL;
    const double srcDepth = 8;
    const double mt[6] = {1, 2, 3, 4, 5, 6};
    const char *cmt_fname = "data/eewUtils_quakeML_example.cmt\0";

    cmtQML = eewUtils_makeXML__quakeML(
        "UW\0",
        "anss.org\0",
        SA.eventid,
        SA.lat,
        SA.lon,
        srcDepth,
        SA.time,
        mt,
        &ierr);

    EXPECT_EQ(0, ierr);
    tests::expect_file(cmt_fname, cmtQML);

    free(cmtQML);
}
