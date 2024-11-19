/**
 * @file Hdf5UT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the hdf5/ directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"

#include "gfast.h"
#include "gfast_hdf5.h"

#include <stdio.h>


class Hdf5Fixture : public::testing::Test {
    protected:

        const char *dir_tmp = "data/tmp\0";
        const char *evid = "1234\0";
        // Careful with expected_path!! It will be deleted on TearDown
        const char *expected_path = "data/tmp/1234_archive.h5\0";
        const char *propfilename = "data/gfast.props\0";
        int ierr;
        hid_t file;

        void SetUp() {
            ierr = hdf5_initialize(dir_tmp, evid, propfilename);

            EXPECT_EQ(0, ierr);
            htri_t is_file;
            is_file = H5Fis_hdf5(expected_path);
            EXPECT_EQ(1, is_file);
        }

        void TearDown() {
            if (H5Fis_hdf5(expected_path)) {
                file = H5Fopen(expected_path, H5F_ACC_RDWR, H5P_DEFAULT);
                H5Fflush(file, H5F_SCOPE_GLOBAL);
                H5Fclose(file);
                remove(expected_path);
            }
        }

};

TEST_F(Hdf5Fixture, testSetFileName) {
    char fname[PATH_MAX];

    ierr = hdf5_setFileName(dir_tmp, evid, fname);

    EXPECT_EQ(0, ierr);
    EXPECT_STREQ(expected_path, fname);
}

TEST_F(Hdf5Fixture, testSetFileNameNullEvid) {
    char fname[PATH_MAX];
    evid = NULL;

    ierr = hdf5_setFileName(dir_tmp, evid, fname);

    EXPECT_EQ(-1, ierr);
}

TEST_F(Hdf5Fixture, testSetFileNameEmptyEvid) {
    char fname[PATH_MAX];
    evid = "";

    ierr = hdf5_setFileName(dir_tmp, evid, fname);

    EXPECT_EQ(-1, ierr);
}

TEST_F(Hdf5Fixture, testSetFileNameNullAdir) {
    char fname[PATH_MAX];
    dir_tmp = NULL;

    ierr = hdf5_setFileName(dir_tmp, evid, fname);

    EXPECT_EQ(0, ierr);
    EXPECT_STREQ("./1234_archive.h5", fname);
}

TEST_F(Hdf5Fixture, testSetFileNameEmptyAdir) {
    char fname[PATH_MAX];
    dir_tmp = "";

    ierr = hdf5_setFileName(dir_tmp, evid, fname);

    EXPECT_EQ(0, ierr);
    EXPECT_STREQ("./1234_archive.h5", fname);
}

TEST_F(Hdf5Fixture, testCinterItemExists) {
    file = H5Fopen(expected_path, H5F_ACC_RDONLY, H5P_DEFAULT);
    ASSERT_EQ(1, h5_item_exists(file, "/DataStructures"));
    EXPECT_EQ(1, h5_item_exists(file, "/DataStructures/peakDisplacementDataStructure"));
    ASSERT_EQ(1, h5_item_exists(file, "/InitializationFile"));
    EXPECT_EQ(1, h5_item_exists(file, "/InitializationFile/IniFile"));
    EXPECT_EQ(1, h5_item_exists(file, "/GFAST_History"));
    EXPECT_EQ(1, h5_item_exists(file, "/Summary"));
    EXPECT_EQ(0, h5_item_exists(file, "/Fake"));
    H5Fclose(file);
}
