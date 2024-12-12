
#include <string.h>
#include "gfast_dataexchange.h"
#include "gfast_core.h"
#include <nats/nats.h>

/*!
 * @brief Initializes NATS consumer.
 * @param[in] props         Data connection properties.
 * 
 * @param[out] connection    Data connection pointer (void *)
 * @param[out] subscription  Data subscription pointer (void *)
 * @result 0 indicates success.
 */
int dataexchange_nats_connect(
    struct dataconn_props_struct *props,
    void **connection,
    void **subscription) 
{
    char *nats_subject = props->topic;
    LOG_MSG("Listening on NATS subject %s", nats_subject);

    natsConnection      *conn = NULL;
    natsSubscription    *sub  = NULL;
    natsStatus          s;

    // See nats.c/examples/getstarted/syncsub.c
    // Creates a connection to the default NATS URL
    s = natsConnection_ConnectTo(&conn, props->servers);
    if (s == NATS_OK)
    {
        // Creates a synchronous subscription on subject nats_subject.
        s = natsConnection_SubscribeSync(&sub, conn, nats_subject);
    } else {
        nats_PrintLastErrorStack(stderr);
    }

    LOG_MSG("%s", "Flushing NATS buffer");
    if (s == NATS_OK)
    {
        s = natsConnection_FlushTimeout(conn, 1000);
    }

    // assign connection to data_conn_ptr
    *connection = conn;
    *subscription = sub;
    return 0;
}

/*!
 * @brief Closes NATS consumer.
 * @param[in] connection    Data connection pointer (void *)
 * @param[in] subscription  Data subscription pointer (void *)
 * @result 0 indicates success.
 */
int dataexchange_nats_close(void **connection, void **subscription) {
    natsConnection *conn = (natsConnection *) (*connection);
    natsSubscription *sub  = (natsSubscription *) (*subscription);

    // Anything that is created needs to be destroyed
    natsSubscription_Destroy(sub);
    natsConnection_Destroy(conn);

    return 0;
}


/*!
 * @brief Read data.
 * @param[in] connection    Data connection pointer (void *)
 * @param[in] subscription  Data subscription pointer (void *)
 * @result 0 indicates success.
 */
char *dataexchange_nats_getMessages(
    void **subscription,
    const int max_payload_size,
    const int message_block,
    int *n_messages,
    int *ierr) 
{
    natsSubscription *sub  = (natsSubscription *) (*subscription);
    natsMsg *msg = NULL;
    natsStatus s = NATS_OK;
    char *msgs;
    int kdx;

    *ierr = 0;
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
                LOG_DEBUGMSG("Uh oh, msg is too long (%d > %d), skipping!",
                    natsMsg_GetDataLength(msg), max_payload_size);
                continue;
            }
            kdx = *n_messages * max_payload_size;
            memcpy(&msgs[kdx], natsMsg_GetData(msg), max_payload_size*sizeof(char));
            (*n_messages) += 1;

            // Need to destroy the message!
            natsMsg_Destroy(msg);
            if (*n_messages >= message_block) {
                LOG_ERRMSG("Uh oh, exceeded message block size (%d), exiting.", message_block);
                break;
            }
        } else {
            // LOG_DEBUGMSG("%s", "No more messages?");
            // nats_PrintLastErrorStack(stderr);
        }
    }

    LOG_DEBUGMSG("nats_getMessages received %d messages", *n_messages);
    return msgs;
}

