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
#define GFAST_activeMQ_setTcpURIRequest(...)       \
              activeMQ_setTcpURIRequest(__VA_ARGS__)
#endif /*ifndef __cplusplus*/

#ifdef __cplusplus
}
#endif

#endif /* _gfast_activemq_h__ */
