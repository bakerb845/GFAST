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
#include "gfast_traceBuffer.h"

#ifdef GFAST_USE_EW
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#endif
#include <transport.h>
#include <earthworm.h>
#include <trace_buf.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
int WaveMsg2MakeLocal( TRACE2_HEADER* wvmsg );
struct ewRing_struct
{
    char ewRingName[512];  /*!< Earthworm ring name to which we will connect */
    SHM_INFO region;       /*!< Earthworm shared memory region corresponding to
                                the earthworm ring */
    MSG_LOGO *getLogo;     /*!< Logos to scrounge from the ring [nlogo] */
    long ringKey;          /*!< Ring key number */
    short nlogo;           /*!< Number of logos */
    bool linit;            /*!< True if the structure is initialized.
                                False if the structure is not initialized. */
    unsigned msWait;       /*!< milliseconds to wait after reading ring */
    unsigned char
       traceBuffer2Type;   /*!< traceBuffer2type earthworm type */ 
    unsigned char
       heartBeatType;      /*!< earthworm heartbeat type */ 
    unsigned char
       errorType;          /*!< earthworm error type */
    unsigned char  
       instLocalID;        /*!< earthworm local installation ID type */
    unsigned char
       instWildcardID;     /*!< installation wildcard ID */
    unsigned char
       modWildcardID;      /*!< module wildcard ID */
};
#else
#ifndef MAX_TRACEBUF_SIZ
#define MAX_TRACEBUF_SIZ 4096
#endif
struct ewRing_struct
{
    bool linit;            /*!< Bogus value so that compilation proceeds */ 
};
#endif

// General functions
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
#ifdef GFAST_USE_EW
/* Classify the result of earthworm tport_copyfrom */
int dataexchange_earthworm_classifyGetRetval(const int retval);
/* Finalize earthworm ring reader */
int dataexchange_earthworm_finalize(struct ewRing_struct *ringInfo);
/* Flush an earthworm ring */
int dataexchange_earthworm_flushRing(struct ewRing_struct *ringInfo);
  /* Read messages from the ring */
char *dataexchange_earthworm_getMessagesFromRing(const int messageBlock,
                                           const bool showWarnings,
                                           struct ewRing_struct *ringInfo,
                                           struct gnsstrace_hashmap_struct *hashmap,
                                           int *nRead, int *ierr);
/* Initialize the earthworm ring reader connection */
int dataexchange_earthworm_initialize(const char *ewRing,
                                const int msWait,
                                struct ewRing_struct *ringInfo);

/* Classify return value from Earthworm get transport call */
int dataexchange_earthworm_classifyGetRetval(const int retval);
/* Flush an earthworm ring */
int dataexchange_earthworm_flushRing(struct ewRing_struct *ringInfo);
/* Finalize earthworm ring reader */
int dataexchange_earthworm_finalize(struct ewRing_struct *ringInfo);
#endif /* GFAST_USE_EW */

// NATS
#ifdef GFAST_USE_NATS
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
#endif /* GFAST_USE_NATS */

// Kafka

#endif /* GFAST_DATAEXCHANGE_H__ */