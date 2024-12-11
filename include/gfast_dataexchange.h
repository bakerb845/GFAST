#ifndef GFAST_DATAEXCHANGE_H__
#define GFAST_DATAEXCHANGE_H__ 1

#ifdef GFAST_USE_KAFKA
typedef rd_kafka_t* data_conn_ptr; /*!< Pointer to the data connection */
// typedef void* data_sub_ptr;        /*!< Pointer to the data subscription */
#endif
#ifdef GFAST_USE_NATS
#include <nats/nats.h>
typedef natsConnection * data_conn_ptr; /*!< Pointer to the data connection */
#endif
#include "gfast_struct.h"

// typedef void * data_sub_ptr;        /*!< Pointer to the data subscription */

// struct dataconn_props_struct {
//     char groupid[128];      /*!< */
//     char servers[128];      /*!< Bootstrap servers and ports, e.g. host1:9092,host2:9092 */
//     char topic[128];        /*!< Topic, but shd this be topics if we do data and event? List of topics to subscribe to */
// };

void dataexchange_initializeDataConnection(
    struct dataconn_props_struct *props,
    void **connection,
    void **subscription);
char *dataexchange_getMessages(
    void **subscription,
    const int max_payload_size,
    const int message_block,
    int *n_messages,
    int *ierr);
void dataexchange_closeDataConnection(
    void **connection,
    void **subscription);
int dataexchange_readIni(const char *propfilename,
                         const char *group,
                         struct dataconn_props_struct* data_conn_props);

// Earthworm

// NATS
int dataexchange_nats_connect(
    struct dataconn_props_struct *props,
    void **connection,
    void **subscription);
char *dataexchange_nats_getMessages(
    void **subscription,
    const int max_payload_size,
    const int message_block,
    int *n_messages,
    int *ierr);
int dataexchange_nats_close(void **connection, void **subscription);

// Kafka

#endif /* GFAST_DATAEXCHANGE_H__ */