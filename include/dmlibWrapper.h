#ifndef _dmlibWrapper_h
#define _dmlibWrapper_h

#include "gfast_struct.h"

/*!
 * @file dmlibWrapper.h
 * @brief variables and functions to expose dmlib functionality to GFAST c code
 */

#ifdef __cplusplus
extern "C" {
#endif /*ifdef __cplusplus*/
  
  /*!
    @brief Start connection to activemq instance
    @param[in] AMQuser         Authenticating username.
    @param[in] AMQpassword     Authenticating password.
    @param[in] destinationURL  URL of host computer (e.g. tcp://localhost:61616).
    @param[in] msReconnect     Number of milliseconds to wait for a reconnect
                               attempt.  If 0 or if maxAttempts is 0 then this
                               command will be ignored.
    @param[in] maxAttempts     Number of attempts to connect before giving up.
                               If 0 this command will be ignored.
    @param[in] verbose         logging verbosity. default=1
    @return 1 if success, 0 if already connected and -1 for error
    Points new connection to static amqconnection pointer or sets pointer to NULL
    if unsuccessful.
  */
  int startDestinationConnection(const char AMQuser[],
				 const char AMQpassword[],
				 const char destinationURL[],
				 const int msReconnect,
				 const int maxAttempts,
				 const int verbose);
  /*!
    @brief stops active activemq connection
    Stops and destroys connection and sets amqconnection pointer to NULL
    @return 1 if success, 0 if not connected and -1 for error
  */
  int stopDestinationConnection();
  /*!
    @brief tests if connection is started
    @return true if connected, false if not
  */
  bool isDestinationConnected();

  int startEventReceiver(const char originURL[],
                         const char user[],
                         const char password[],
                         const char originTopic[],
                         const int msReconnect,
                         const int maxAttempts);
  char *eventReceiverGetMessage(const int ms_wait, int *ierr);
  int stopEventReceiver();

  /*!
    @brief instantiates and starts new DMMessageSender for events.
    @return 1 if success, 0 if already connected and -1 for error
  */
  int startEventSender(const char topic[], const double msg_ttl);
  /*!
    @brief stops active DMMessageSender for events.
    Stops and destroys DMMessageSender object.
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int stopEventSender();

  /*!
    @brief instantiates and starts new DMMessageEncoder for event xmls.
    @return 1 if success, 0 if already connected and -1 for error
  */
  int startEventEncoder();
  /*!
    @brief stops active DMMessageEncoder for events.
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int stopEventEncoder();
  
  /*!
    @brief sends static event message
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int sendEventMessage();
  
  /*!
    @brief sends event message as preformatted xml
    @param xmlstr preformatted xml message to send to ShakeAlert
    @return 1 if success, 0 if not initialized and -1 for error (todo. currently dummy)
  */
  int sendEventXML(const char xmlstr[]);

  /*!
    @brief start heartbeat producer
    Points new HBProducer to static hbproducer pointer or sets pointer to NULL
    if unsuccessful.
    @param[in] sender      identifier string for sending instance
    @param[in] hbtopic     target topic for heartbeats
    @param[in] interval    heartbeat interval in seconds (0=manual heartbeats)
    @param[in] verbose     logging verbosity. default=1
    @return 1 if success, 0 if already initialized and -1 for error
  */
  int startHBProducer(const char sender[], const char hbtopic[], int interval, int verbose);
  /*!
    @brief stops active HBProducer
    Stops and destroys HBProducer object and sets amqconnection pointer to NULL
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int stopHBProducer();
  /*!
    @brief sends heartbeat message
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int sendHeartbeat();
  /*!
    @brief Returns the dmlib version string
    @return Version string
  */
  char *getDmLibVersion();

  /*!
    @brief create xml message from coreEvent_info and GFAST_peakDisplacementData_struct
    @param[in] mode          Live or test mode
    @param[in] alg_vers      Version of GFAST
    @param[in] instance      Instance running the code
    @param[in] message_type  Describes a new or updated message
    @param[in] max_assoc_stations Max # of stations with the 'assoc=True' attribute for 
                                  SA station association
    @param[in] core          CoreEventInfo to go into message
    @param[in] pgd           Contains information on if a site was used for pgd
    @param[in] pgd_obs       PGD observations to go into gm_info
    @param[out] ierr         0 for success, -1 for error
    @return returns a ShakeAlert xml message
  */
  char *dmlibWrapper_createPGDXML(const double currentTime,
                                  const enum opmode_type mode,
                                  const char *alg_vers,
                                  const char *instance,
                                  const char *message_type, 
                                  const int max_assoc_stations,
                                  const struct coreInfo_struct *core,
                                  const struct GFAST_pgdResults_struct *pgd,
                                  const struct GFAST_peakDisplacementData_struct *pgd_obs,
                                  int *ierr);
  
#ifdef __cplusplus
} /*end of extern "C"*/
#endif

#endif /*_dmlibWrapper_h*/
