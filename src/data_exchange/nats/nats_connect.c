
#include <nats/nats.h>

int data_exchange_nats_connect() {

    LOG_MSG("Listening on NATS subject %s\n", NATS_SUBJECT);

    natsConnection      *conn = NULL;
    natsSubscription    *sub  = NULL;
    natsStatus          s;

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
    return 1;
}