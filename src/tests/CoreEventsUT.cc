/**
 * @file CoreEventsUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the core/events directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"

#include "gfast.h"

// test freeEvents, newEvent, printEvent,
// removeExpiredEvent, removeExpiredEvents, syncXMLStatusWithEvents
void get_SA(GFAST_shakeAlert_struct *SA) {
    // Initialize new event
    strcpy(SA->eventid, "12345\0");
    strcpy(SA->orig_sys, "gfast\0");
    SA->version = 0;
    SA->time = 1663827681;
    SA->lat = 36.1947;
    SA->lon = 240.5070;
    SA->dep = 8.0;
    SA->mag = 3.7;
}

class CoreEventsFixture : public::testing::Test {
    protected:
        struct GFAST_activeEvents_struct events;
        struct GFAST_activeEvents_xml_status xml_status;
        struct GFAST_shakeAlert_struct SA;
        bool lnewEvent;

        void SetUp() {
            memset(&SA, 0, sizeof(struct GFAST_shakeAlert_struct));
            memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
            memset(&xml_status, 0, sizeof(struct GFAST_activeEvents_xml_status));

            get_SA(&SA);

            EXPECT_EQ(0, events.nev);
            EXPECT_EQ(0, xml_status.nev);
        }

        void TearDown() {}
};

TEST_F(CoreEventsFixture, testNewEventNoPreviousEvents) {
    lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);
    EXPECT_TRUE(lnewEvent);

    // Check events
    ASSERT_EQ(1, events.nev);
    EXPECT_STREQ(SA.eventid, events.SA[0].eventid);
    EXPECT_STREQ(SA.orig_sys, events.SA[0].orig_sys);
    EXPECT_EQ(SA.version, events.SA[0].version);
    EXPECT_DOUBLE_EQ(SA.time, events.SA[0].time);
    EXPECT_DOUBLE_EQ(SA.lat, events.SA[0].lat);
    EXPECT_DOUBLE_EQ(SA.lon, events.SA[0].lon);
    EXPECT_DOUBLE_EQ(SA.dep, events.SA[0].dep);
    EXPECT_DOUBLE_EQ(SA.mag, events.SA[0].mag);

    // Check xml_status
    ASSERT_EQ(1, xml_status.nev);
    EXPECT_STREQ(SA.eventid, xml_status.SA_status[0].eventid);
}

TEST_F(CoreEventsFixture, testNewEventWithPreviousEvents) {
    lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);

    ASSERT_EQ(1, events.nev);
    ASSERT_EQ(1, xml_status.nev);
    
    lnewEvent = false;

    // Make new event with different evid, lat
    struct GFAST_shakeAlert_struct SA2;
    memset(&SA2, 0, sizeof(struct GFAST_shakeAlert_struct));

    get_SA(&SA2);
    strcpy(SA2.eventid, "999\0");
    SA2.lat = 48.0;

    // Now add new event
    lnewEvent = GFAST_core_events_newEvent(SA2, &events, &xml_status);
    EXPECT_TRUE(lnewEvent);

    // Check events
    ASSERT_EQ(2, events.nev);
    // Check first is the same
    EXPECT_STREQ(SA.eventid, events.SA[0].eventid);
    EXPECT_STREQ(SA.orig_sys, events.SA[0].orig_sys);
    EXPECT_EQ(SA.version, events.SA[0].version);
    EXPECT_DOUBLE_EQ(SA.time, events.SA[0].time);
    EXPECT_DOUBLE_EQ(SA.lat, events.SA[0].lat);
    EXPECT_DOUBLE_EQ(SA.lon, events.SA[0].lon);
    EXPECT_DOUBLE_EQ(SA.dep, events.SA[0].dep);
    EXPECT_DOUBLE_EQ(SA.mag, events.SA[0].mag);
    // Check the new one is represented
    EXPECT_STREQ(SA2.eventid, events.SA[1].eventid);
    EXPECT_STREQ(SA2.orig_sys, events.SA[1].orig_sys);
    EXPECT_EQ(SA2.version, events.SA[1].version);
    EXPECT_DOUBLE_EQ(SA2.time, events.SA[1].time);
    EXPECT_DOUBLE_EQ(SA2.lat, events.SA[1].lat);
    EXPECT_DOUBLE_EQ(SA2.lon, events.SA[1].lon);
    EXPECT_DOUBLE_EQ(SA2.dep, events.SA[1].dep);
    EXPECT_DOUBLE_EQ(SA2.mag, events.SA[1].mag);

    // Check xml_status
    ASSERT_EQ(2, xml_status.nev);
    EXPECT_STREQ(SA.eventid, xml_status.SA_status[0].eventid);
    EXPECT_STREQ(SA2.eventid, xml_status.SA_status[1].eventid);
}

TEST_F(CoreEventsFixture, testNewEventUpdatePreviousEvents) {
    lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);

    ASSERT_EQ(1, events.nev);
    ASSERT_EQ(1, xml_status.nev);
    
    lnewEvent = false;

    // Make new event that updates the previous
    struct GFAST_shakeAlert_struct SA2;
    memset(&SA2, 0, sizeof(struct GFAST_shakeAlert_struct));

    get_SA(&SA2);
    SA2.version = 1;
    SA2.mag = 3.9;

    // Now add new event
    lnewEvent = GFAST_core_events_newEvent(SA2, &events, &xml_status);
    EXPECT_FALSE(lnewEvent);

    // Check events
    ASSERT_EQ(1, events.nev);
    // Check the new one is represented
    EXPECT_STREQ(SA2.eventid, events.SA[0].eventid);
    EXPECT_STREQ(SA2.orig_sys, events.SA[0].orig_sys);
    EXPECT_EQ(SA2.version, events.SA[0].version);
    EXPECT_DOUBLE_EQ(SA2.time, events.SA[0].time);
    EXPECT_DOUBLE_EQ(SA2.lat, events.SA[0].lat);
    EXPECT_DOUBLE_EQ(SA2.lon, events.SA[0].lon);
    EXPECT_DOUBLE_EQ(SA2.dep, events.SA[0].dep);
    EXPECT_DOUBLE_EQ(SA2.mag, events.SA[0].mag);

    // Check xml_status
    ASSERT_EQ(1, xml_status.nev);
    EXPECT_STREQ(SA2.eventid, xml_status.SA_status[0].eventid);

}

TEST_F(CoreEventsFixture, testPrintEvent) {
    EXPECT_NO_THROW(GFAST_core_events_printEvents(SA););
}

TEST_F(CoreEventsFixture, testRemoveExpiredEventsNone) {
    int ret;
    ret = core_events_removeExpiredEvents(100, SA.time, 1, &events);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0, events.nev);
    EXPECT_EQ(0, xml_status.nev);
}

TEST_F(CoreEventsFixture, testRemoveExpiredEventsOneSync) {
    lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);
    EXPECT_EQ(1, events.nev);
    EXPECT_EQ(1, xml_status.nev);
    int ret;
    ret = core_events_removeExpiredEvents(100, SA.time + 200, 1, &events);
    EXPECT_EQ(1, ret);
    EXPECT_EQ(0, events.nev);
    EXPECT_EQ(1, xml_status.nev);

    ret = core_events_syncXMLStatusWithEvents(&events, &xml_status);
    EXPECT_EQ(0, xml_status.nev);
}

TEST_F(CoreEventsFixture, testRemoveExpiredEventsMultipleSync) {
    // Make the second event
    struct GFAST_shakeAlert_struct SA2;
    memset(&SA2, 0, sizeof(struct GFAST_shakeAlert_struct));
    get_SA(&SA2);
    strcpy(SA2.eventid, "9876\0");
    SA2.version = 0;
    SA2.mag = 3.9;
    SA2.time += 300;

    // Add both events, verify
    lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);
    EXPECT_TRUE(lnewEvent);

    lnewEvent = GFAST_core_events_newEvent(SA2, &events, &xml_status);
    EXPECT_TRUE(lnewEvent);

    EXPECT_EQ(2, events.nev);
    EXPECT_EQ(2, xml_status.nev);

    // Now remove one
    int ret;
    ret = core_events_removeExpiredEvents(100, SA.time + 200, 1, &events);
    EXPECT_EQ(1, ret);
    EXPECT_EQ(1, events.nev);
    EXPECT_EQ(2, xml_status.nev);

    ret = core_events_syncXMLStatusWithEvents(&events, &xml_status);
    EXPECT_EQ(1, xml_status.nev);
}

TEST_F(CoreEventsFixture, testRemoveExpiredEventExists) {
    lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);
    EXPECT_EQ(1, events.nev);
    EXPECT_EQ(1, xml_status.nev);

    struct GFAST_shakeAlert_struct SArem;
    memset(&SArem, 0, sizeof(struct GFAST_shakeAlert_struct));
    memcpy(&SArem, &SA, sizeof(struct GFAST_shakeAlert_struct));

    int ret;
    ret = core_events_removeExpiredEvent(100, SA.time + 200, 1, SArem, &events);
    EXPECT_EQ(1, ret);
    EXPECT_EQ(0, events.nev);
    EXPECT_EQ(1, xml_status.nev);
}

TEST_F(CoreEventsFixture, testRemoveExpiredEventNoExists) {
    lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);
    EXPECT_EQ(1, events.nev);
    EXPECT_EQ(1, xml_status.nev);

    struct GFAST_shakeAlert_struct SA2;
    memset(&SA2, 0, sizeof(struct GFAST_shakeAlert_struct));
    get_SA(&SA2);
    strcpy(SA2.eventid, "9876\0");
    SA2.version = 0;
    SA2.mag = 3.9;

    int ret;
    ret = core_events_removeExpiredEvent(100, SA.time + 200, 1, SA2, &events);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(1, events.nev);
    EXPECT_EQ(1, xml_status.nev);
}
