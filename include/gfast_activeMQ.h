#ifndef gfast_activemq_h
#define gfast_activemq_h 1
#include "gfast_struct.h"

#ifdef __cplusplus
extern "C"
{
#endif /*ifdef __cplusplus*/

/* Helps C start and stop the ActiveMQ library */
void activeMQ_start(void);
void activeMQ_stop(void);
/* Private stuff - really not for general use */
void activeMQ_setInit(void);
void activeMQ_setUninit(void);
bool activeMQ_isInit(void);

/* Initialize and finalize activeMQ library */
void activeMQ_initialize(void);
void activeMQ_finalize(void);

/* Read parmaeters from the ini file */
int activeMQ_readIni(const char *propfilename,
                     const char *group,
                     struct GFAST_activeMQ_struct *activeMQ_props);

/* Initialize activeMQ parameters */
// void *activeMQ_consumer_initialize(const char AMQuser[],
//                                    const char AMQpassword[],
//                                    const char AMQurl[],
//                                    const char AMQdestination[],
//                                    const int msReconnect,
//                                    const int maxAttempts,
//                                    const bool useTopic,
//                                    const bool clientAck,
//                                    const bool luseListener,
//                                    const unsigned int maxMessages,
//                                    const int verbose,
//                                    int *ierr);
/* Get a message from activeMQ */
// char *activeMQ_consumer_getMessage(void *consumer,
//                                    const int ms_wait, int *ierr);
/* Get a message from activeMQ listener */
// char *activeMQ_consumer_getMessageFromListener(void *consumer,
//                                                int *ierr);
/* Shut down activeMQ consumer */
// void activeMQ_consumer_finalize(void *consumerIn);
/* Shut down activeMQ producer */
// void activeMQ_producer_finalize(void *producerIn);
/* Send a text message */
// int activeMQ_producer_sendMessage(void *producerIn,
//                                   const char *message);
/* Initialize the ActiveMQ producer */
void *activeMQ_producer_initialize(const char AMQuser[],
                                   const char AMQpassword[],
                                   const char AMQurl[],
                                   const char AMQdestination[],
                                   const bool useTopic,
                                   const bool clientAck,
                                   const int verbose,
                                   int *ierr);
/* Convenience function to set the tcp URI request */
char *activeMQ_setTcpURIRequest(const char *url,
                                const int msReconnect,
                                const int maxAttempts);


#ifndef __cplusplus
// #define GFAST_activeMQ_consumer_initialize(...)       \
//               activeMQ_consumer_initialize(__VA_ARGS__)
// #define GFAST_activeMQ_consumer_getMessage(...)       \
//               activeMQ_consumer_getMessage(__VA_ARGS__)
// #define GFAST_activeMQ_consumer_finalize(...)       \
//               activeMQ_consumer_finalize(__VA_ARGS__)
// #define GFAST_activeMQ_producer_finalize(...)       \
//               activeMQ_producer_finalize(__VA_ARGS__)
// #define GFAST_activeMQ_producer_sendMessage(...)       \
//               activeMQ_producer_sendMessage(__VA_ARGS__)
#define GFAST_activeMQ_setTcpURIRequest(...)       \
              activeMQ_setTcpURIRequest(__VA_ARGS__)
#endif /*ifndef __cplusplus*/

#ifdef __cplusplus
}
#endif

#endif /* _gfast_activemq_h__ */
