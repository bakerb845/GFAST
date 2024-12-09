#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include <nats/nats.h>

#include "geojson_utils.h"


#ifdef ENABLE_JSON_PARSER
// Use json-parser (https://github.com/json-parser/json-parser)
#include "json.h"
#endif

#ifdef ENABLE_JANSSON
// Use jansson (also used by Earthworm)
#include <jansson.h>
#endif

int max_payload_size = 1024;
int message_block = 100000;

/*
 * Sub/getMsg portion modified from nats.c/examples/getstarted/syncsub.c
 *
 * Compile with Makefile.nats
 * make -f Makefile.nats clean; make -f Makefile.nats all; ./sub_geojson 
 */

/*
Example message:
{
    "time": 1675483870000,
    "Q":    190900,
    "type": "ENU",
    "SNCL": "P373.PW.LY_.00",
    "coor": [0.045 ,0.018, 0.083],
    "err":  [0.017, 0.016, 0.034],
    "rate":  1
}
*/

struct geojson_struct {
    long time;      /*!< Unix time in ms */
    int quality;    /*!< Quality channel (last digit is use flag) */
    char type[8];     /*!< Should be ENU */
    char sncl[15];     /*!< SNCL */
    double coor[3]; /*!< Positions (m) */
    double err[3];  /*!< Positional error (m) */
    double rate;    /*!< Sampling rate (s) */
};

void print_json_msg(struct geojson_struct *json_msg) {
    printf("(%ld) ", json_msg->time);
    printf("(%d) ", json_msg->quality);
    printf("(%s) ", json_msg->type);
    printf("(%s) ", json_msg->sncl);
    printf("[%.3f,%.3f,%.3f] ", json_msg->coor[0], json_msg->coor[1], json_msg->coor[2]);
    printf("[%.3f,%.3f,%.3f] ", json_msg->err[0], json_msg->err[1], json_msg->err[2]);
    printf("(%f)\n", json_msg->rate);
}

#ifdef ENABLE_JSON_PARSER
struct geojson_struct *unpackMessage(const char *msg, const int msg_size) {
    struct geojson_struct *json_msg;

    json_value *obj;
    static char errstr[json_error_max];
    json_settings settings = { .max_memory = 100000 };


    json_msg = malloc(sizeof(struct geojson_struct));

    return json_msg;
}
#endif

#ifdef ENABLE_JANSSON
struct geojson_struct *unpackMessage(const char *msg, const int msg_size) {
    struct geojson_struct *json_msg;
    char *text;

    json_msg = malloc(sizeof(struct geojson_struct));
    text = malloc(msg_size * sizeof(char));

    memcpy(text, msg, msg_size);

    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if(!root) {
        printf("error: on line %d: %s\n", error.line, error.text);
        return json_msg;
    }

    // printf("json is type %d\n", (int)(root->type));

    if(!json_is_object(root)) {

        printf("error: root is not an object, it is %d\n", (int)(root->type));
        json_decref(root);
        return json_msg;
    }

    // Could also just have one 'json_value' that is reused?
    // json_t *j_time, *j_Q, *j_type, *j_SNCL, *j_coor, *j_err, *j_rate;
    json_t *json_value;

    // NOTE (CWU 12/5/24): when parsing numbers, use json_number_value. It will check if real or
    // int and return in the form of a double. It can be casted afterwards to what you expect/want.

    // time
    json_value = json_object_get(root, "time");
    if (!json_is_number(json_value)) {
        printf("time is not a number, it is %s!\n", json_string_value(json_object_get(json_value, "type")));
    }
    json_msg->time = (long)json_number_value(json_value);

    // Q
    json_value = json_object_get(root, "Q");
    if (!json_is_number(json_value)) {
        printf("Q is not a number, it is %s!\n", json_string_value(json_object_get(json_value, "type")));
    }
    json_msg->quality = (int)json_number_value(json_value);

    // type
    json_value = json_object_get(root, "type");
    if (!json_is_string(json_value)) {
        printf("type is not a string, it is %s!\n", json_string_value(json_object_get(json_value, "type")));
    }
    memcpy(json_msg->type, json_string_value(json_value), sizeof json_msg->type);

    // SNCL
    json_value = json_object_get(root, "SNCL");
    if (!json_is_string(json_value)) {
        printf("SNCL is not a string, it is %s!\n", json_string_value(json_object_get(json_value, "type")));
    }
    memcpy(json_msg->sncl, json_string_value(json_value), sizeof json_msg->type);

    // coor
    json_value = json_object_get(root, "coor");
    if (!json_is_array(json_value)) {
        printf("coor is not an array, it is %s!\n", json_string_value(json_object_get(json_value, "type")));
    }
    int i;
    for (i = 0; i < json_array_size(json_value); i++) {
        json_t *data;
        data = json_array_get(json_value, i);

        if(!json_is_number(data)) {
            printf("data is not a number, it is %s!\n", json_string_value(json_object_get(data, "type")));
        }
        json_msg->coor[i] = (double)json_number_value(data);
    }

    // err
    json_value = json_object_get(root, "err");
    if (!json_is_array(json_value)) {
        printf("err is not an array, it is %s!\n", json_string_value(json_object_get(json_value, "type")));
    }
    for (i = 0; i < json_array_size(json_value); i++) {
        json_t *data;
        data = json_array_get(json_value, i);

        if(!json_is_number(data)) {
            printf("data is not a number, it is %s!\n", json_string_value(json_object_get(data, "type")));
        }

        json_msg->err[i] = (double)json_number_value(data);
    }

    // rate
    json_value = json_object_get(root, "rate");
    if (!json_is_number(json_value)) {
        printf("rate is not a number it is %s!\n", json_string_value(json_object_get(json_value, "type")));
    }
    json_msg->rate = (double)json_number_value(json_value);

    printf("%s\n", msg);
    print_json_msg(json_msg);

    return json_msg;
}
#endif

void unpackMessages(const int nRead, const char *msgs) {
    
    int i, indx;
    struct geojson_struct *json_msg;
    struct geojson_struct **json_msgs;
    json_msgs = (struct geojson_struct **)malloc(nRead * sizeof(struct geojson_struct *));

    for (i = 0; i < nRead; i++) {
        indx = i * max_payload_size * sizeof(char);
        // json_msg = NULL;
        json_msg = unpackMessage(&msgs[indx], max_payload_size);

        // memcpy(&json_msgs[indx], json_msg, sizeof(struct geojson_struct *));

    }

}

char *getMessages(natsSubscription *sub, int *n_messages) {
    natsMsg *msg = NULL;
    natsStatus s = NATS_OK;
    char *msgs;
    int kdx;

    msgs = NULL;
    *n_messages = 0;

    msgs = (char *)malloc(max_payload_size * message_block * sizeof(char));


    while (s == NATS_OK) {

        // With synchronous subscriptions, one need to poll
        // using this function. A timeout is used to instruct
        // how long we are willing to wait. The wait is in milliseconds.
        s = natsSubscription_NextMsg(&msg, sub, 0);

        if (s == NATS_OK)
        {
            // If we are here, we should have received a message.
            // printf("Received msg: %s - %.*s\n",
            //     natsMsg_GetSubject(msg),
            //     natsMsg_GetDataLength(msg),
            //     natsMsg_GetData(msg));

            // natsMsg_GetData(msg) can be put into a gfast data structure

            if (natsMsg_GetDataLength(msg) > max_payload_size) {
                printf("Uh oh, msg is too long (%d > %d), skipping!\n",
                    natsMsg_GetDataLength(msg), max_payload_size);
                continue;
            }
            kdx = *n_messages * max_payload_size;
            memcpy(&msgs[kdx], natsMsg_GetData(msg), max_payload_size*sizeof(char));
            (*n_messages) += 1;

            // Need to destroy the message!
            natsMsg_Destroy(msg);
            if (*n_messages > message_block) {
                printf("Uh oh, exceeded message block size (%d), exiting.\n", message_block);
                break;
            }
        } else {
            printf("No more messages?\n");
            // nats_PrintLastErrorStack(stderr);
        }
    }

    printf("getMessages received %d messages\n", *n_messages);
    return msgs;
}

int main(int argc, char **argv) {
    printf("Entering NATS test\n");
    
    int i, messages_read, nRead;
    unsigned int millisecond_delay;
    double waitTime, tloop;
    struct timespec t0, t1;
    char *msgs;

    natsConnection      *conn = NULL;
    natsSubscription    *sub  = NULL;
    natsStatus          s;

    msgs = NULL;
    messages_read = 0;
    waitTime = 1.;
    clock_gettime(CLOCK_REALTIME, &t0);

    printf("Listening on subject %s\n", NATS_SUBJECT);

    // See nats.c/examples/getstarted/syncsub.c
    // Creates a connection to the default NATS URL
    s = natsConnection_ConnectTo(&conn, NATS_DEFAULT_URL);
    if (s == NATS_OK)
    {
        // Creates a synchronous subscription on subject NATS_SUBJECT.
        s = natsConnection_SubscribeSync(&sub, conn, NATS_SUBJECT);
    } else {
        nats_PrintLastErrorStack(stderr);
    }

    //// Main outer loop, like in gfast. Every second, try to read all of the
    //// available data
    while (i++ < 30) {
        printf("\nEntering iteration %d\n", i);

        //// A bunch of stuff to do a second-long loop
        clock_gettime(CLOCK_REALTIME, &t1);
        tloop = (t1.tv_sec + 1e-9 * t1.tv_nsec) - (t0.tv_sec + 1e-9 * t0.tv_nsec);
        
        // If waitTime is not a multiple of 1 second, just sleep until that time has elapsed.
        // Otherwise, sleep until the closest full second to waitTime (after t0)
        if (fabs(waitTime - floor(waitTime)) > 1e-6) {
            millisecond_delay = (waitTime - tloop) * 1000;
        } else {
            double tdiff = t1.tv_sec - t0.tv_sec + t1.tv_nsec * 1e-9;
            millisecond_delay = ( waitTime - tdiff ) * 1000;
        }

        // Sanity checks
        if (tloop > waitTime) {
            printf("*** Main loop exceeded waitTime (%.3f > %.3f), proceeding immediately ***\n",
                tloop, waitTime);
            millisecond_delay = 1;
        } else if (millisecond_delay > waitTime * 1000) {
            printf("*** millisecond_delay = %u, forcing to %.3f ***\n",
                millisecond_delay, waitTime * 1000);
            millisecond_delay = waitTime * 1000;
        }

        usleep((millisecond_delay + 1) * 1000);

        // Reset t1 after the sleep
        clock_gettime(CLOCK_REALTIME, &t1);
        t0 = t1;

        printf("Starting iteration %d at %f\n", i, (double)((intmax_t)t1.tv_sec + t1.tv_nsec * 1e-9));

        // Read the data like in like traceBuffer_ewrr_getMessagesFromRing
        free(msgs);
        msgs = NULL;
        msgs = getMessages(sub, &nRead);
        messages_read += nRead;

        // Unpack the messages like traceBuffer_ewrr_unpackTraceBuf2Messages
        unpackMessages(nRead, msgs);
        free(msgs);
        msgs = NULL;

        clock_gettime(CLOCK_REALTIME, &t1);
        printf("Ending read at %f\n", (double)((intmax_t)t1.tv_sec + t1.tv_nsec * 1e-9));
    }

    printf("\nTotal messages read: %d\n", messages_read);

    // Anything that is created needs to be destroyed
    natsSubscription_Destroy(sub);
    natsConnection_Destroy(conn);

    printf("Exiting NATS test\n");
    return 0;
}
