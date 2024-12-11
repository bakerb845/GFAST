#include "gfast_dataexchange.h"
#include "gfast_core.h"
#include <nats/nats.h>

/*!
 * @brief Initializes NATS consumer.
 * @param[in] props         Data connection properties.
 * @param[in] subscription  Data subscription pointer (void *)
 * @param[out] rk        Kafka consumer handle
 * @result 0 indicates success.
 */
int data_exchange_nats_connect(
    struct dataconn_props_struct *props,
    data_conn_ptr connection,
    data_sub_ptr subscription) 
{
    char *nats_subject = props->topic;
    LOG_MSG("Listening on NATS subject %s\n", nats_subject);

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

    // assign connection to data_conn_ptr
    subscription = sub;
    return 1;
}