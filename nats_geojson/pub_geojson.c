#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "geojson_utils.h"

#include <nats/nats.h>

/*
 * Modified from nats.c/examples/getstarted/pubmsg.c
 *
 * Compile with Makefile.nats
 * make -f Makefile.nats clean; make -f Makefile.nats all; ./pub_geojson
 */

int main(int argc, char **argv) {
    int i, messages_published;
    struct json_msg_struct *json_msgs;
    const char *filename = "/home/ulbergc/WORK/ShakeAlert/gfast/src/non_tracked/test_geojson_msgs01.txt\0";

    natsConnection      *conn = NULL;
    natsMsg             *msg  = NULL;
    natsStatus          s;

    messages_published = 0;

    ///// Read in file info
    json_msgs = read_jsonmsgs(filename);
    printf("Total messages from file: %d\n", json_msgs->nmsg);

    // Print out to screen
    // for (i = 0; i < json_msgs->nmsg; i++) {
    //     printf("[%4d]: %s\n", i, json_msgs->msgs[i]);
    // }

    ///// Publish with NATS
    // Create connection
    s = natsConnection_ConnectTo(&conn, NATS_DEFAULT_URL);
    if (s != NATS_OK) {
        nats_PrintLastErrorStack(stderr);
    }

    // Publish each message
    for (i = 0; i < json_msgs->nmsg; i++) {
        // Create message
        if (s == NATS_OK) {
            s = natsMsg_Create(
                &msg,
                NATS_SUBJECT,
                NULL,
                json_msgs->msgs[i],
                MAX_PAYLOAD_SIZE);
        } else {
            nats_PrintLastErrorStack(stderr);
            break;
        }
        // Publish message
        if (s == NATS_OK)
        {
            // Publishes the message on subject NATS_SUBJECT.
            s = natsConnection_PublishMsg(conn, msg);
            messages_published++;
        } else {
            nats_PrintLastErrorStack(stderr);
            break;
        }
        natsMsg_Destroy(msg);
        msg  = NULL;

        // Sleep for 10 ms every time (~100 messages/sec)
        usleep(10 * 1000);
    }

    natsConnection_Destroy(conn);
    free(json_msgs);

    printf("Total messages published: %d\n", messages_published);

    return 0;
}