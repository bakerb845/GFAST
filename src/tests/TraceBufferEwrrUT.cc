/**
 * @file TraceBufferEwrrUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the traceBuffer/ewrr directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"

#include "gfast.h"


TEST(CoreTraceBuffer, testSetTb2DataFromGFAST) {
    const char *metaDataFile;
    char **metaDataNetworks;
    int n_networks, ierr;
    struct GFAST_data_struct gps_data;
    struct tb2Data_struct tb2Data;

    // initialize
    metaDataFile = "data/merged_chanfile_coord.dat\0";
    metaDataNetworks = NULL;
    n_networks = 0;
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));
    memset(&tb2Data, 0, sizeof(struct tb2Data_struct));

    ierr = core_data_readMetaDataFile(metaDataFile,
                                      metaDataNetworks,
                                      n_networks,
                                      &gps_data);
    EXPECT_EQ(0, ierr) << "Error reading sites file";
    EXPECT_EQ(492, gps_data.stream_length);

    ierr = traceBuffer_ewrr_settb2DataFromGFAST(&gps_data, &tb2Data);
    EXPECT_EQ(0, ierr) << "Error setting tb2Data";

    EXPECT_GT((int) tb2Data.hashmap->hashsize, 0);

    core_data_finalize(&gps_data);
}

class CoreTraceBufferFixture : public::testing::Test {
    protected:
        struct tb2_hashmap_struct *hashmap;
        struct tb2_node *node;
        const uint32_t hashsize = 31;

        void SetUp() {
            hashmap = (struct tb2_hashmap_struct *)
                malloc(sizeof(struct tb2_hashmap_struct));

            hashmap->hashsize = hashsize;
            hashmap->map = (struct tb2_node **) calloc(hashsize, sizeof(struct tb2_node *));

            // Add one node
            traceBuffer_ewrr_hashmap_add(hashmap, "Test1", 1);
        }

        void TearDown() {
            traceBuffer_ewrr_free_hashmap(hashmap);
            // free(node);
        }
};

TEST_F(CoreTraceBufferFixture, testHashmapAddContains) {
    node = traceBuffer_ewrr_hashmap_contains(hashmap, "Test1");
    ASSERT_TRUE(node != NULL);
    EXPECT_EQ(1, node->i);
    EXPECT_STREQ("Test1", node->name);

    node = traceBuffer_ewrr_hashmap_contains(hashmap, "Test2");
    ASSERT_TRUE(node == NULL);
}

TEST_F(CoreTraceBufferFixture, testHashmapRemove) {
    int ret;
    ret = traceBuffer_ewrr_hashmap_remove(hashmap, "Test1");
    EXPECT_EQ(1, ret);

    // Also check via the api
    node = traceBuffer_ewrr_hashmap_contains(hashmap, "Test1");
    EXPECT_TRUE(node == NULL);

    // Try to remove it again, nothing happens
    ret = traceBuffer_ewrr_hashmap_remove(hashmap, "Test1");
    EXPECT_EQ(0, ret);

    // Try to remove something that isn't there, nothing happens
    ret = traceBuffer_ewrr_hashmap_remove(hashmap, "Test12");
    EXPECT_EQ(0, ret);
}

TEST_F(CoreTraceBufferFixture, testHashmapAddPrintDebug) {
    // With a hashsize over 5 these shouldn't collide (using one
    // of the simpler hashing algorithms at least)
    traceBuffer_ewrr_hashmap_add(hashmap, "Test2", 2);
    traceBuffer_ewrr_hashmap_add(hashmap, "Test3", 3);
    traceBuffer_ewrr_hashmap_add(hashmap, "Test4", 4);
    traceBuffer_ewrr_hashmap_add(hashmap, "Test5", 5);

    traceBuffer_ewrr_print_hashmap(hashmap);

    EXPECT_EQ(0, traceBuffer_ewrr_print_true_collisions(hashmap));
}
