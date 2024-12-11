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
    *connection = conn;
    *subscription = sub;
    return 1;
}