/*!
 * @file dmlibWrapper.cpp
 * @brief variables and functions to expose dmlib functionality to GFAST c code
 */

#include <string>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <cms/Connection.h>
#include "DMLib.h"
#include "FiniteFaultMessage.h"
#include "CoreEventInfo.h"
#include "HBProducer.h"
#include "DMMessageReceiver.h"
#include "DMMessageSender.h"
#include "DMMessageEncoder.h"
#include "dmlibWrapper.h"
#include "gfast_activeMQ.h"
#include "gfast_struct.h"
#include "gfast_core.h"
#include "gfast_enum.h"
#include "gfast_xml.h"
#include "iscl/iscl/iscl_enum.h"
#include "iscl/time/time.h"

/*static variables local to this file*/
namespace {
  static cms::Connection *destinationConnection=NULL;
  static HBProducer *hbproducer=NULL;
  static DMMessageReceiver *eventreceiver=NULL;
  static DMMessageSender *eventsender=NULL;
  static DMMessageEncoder *eventencoder=NULL;
  static CoreEventInfo *eventmessage=NULL;
  static std::string hbSender="";
  static std::string hbTopic="";
  static int conVerbose=0;
  static int hbVerbose=0;
}

/*
 * compareDist and dist_index are both used to sort pgd observations by distance from source
 * when creating xml message. Needed for adding the 'assoc' attribute correctly.
 */
static int compareDist(const void *x, const void *y);
struct dist_index {
  double dist;
  int indx;
};

int startDestinationConnection(const char AMQuser[],
			       const char AMQpassword[],
			       const char destinationURL[],
			       const int msReconnect,
			       const int maxAttempts,
			       const int verbose=1) {
  conVerbose = verbose;
  if (destinationConnection != NULL) {
    LOG_DEBUGMSG("%s: connection already exists",__func__);
    return 0;
  }
  char *brokerURI;
  brokerURI = activeMQ_setTcpURIRequest(destinationURL,
					msReconnect, maxAttempts);
  try
    {
      // Create a connection factory
      if (conVerbose > 2)
	{
	  LOG_DEBUGMSG("%s: Setting the broker URI: %s",
		 __func__, brokerURI);
	}
      auto_ptr<cms::ConnectionFactory> connectionFactory(
	   cms::ConnectionFactory::createCMSConnectionFactory(brokerURI));

      if (conVerbose > 2)
	{
	  LOG_DEBUGMSG("%s: Creating connection for username (%s)",
		 __func__, AMQuser);
	}
      // Create a connection
      destinationConnection = connectionFactory->createConnection(AMQuser, AMQpassword);
      connectionFactory.reset();
      free(brokerURI);
      if (destinationConnection==NULL) return -1;
    }
  catch (cms::CMSException &e)
    {
      LOG_ERRMSG("%s: Exception encountered creating dmlib connection\n%s",__func__,e.what());
      e.printStackTrace();
      free(brokerURI);
      return -1;
    }
  return 1;
}

int stopDestinationConnection() {
  if ( destinationConnection == NULL ) {
    LOG_DEBUGMSG("%s: connection already dead",__func__);
    return 0;
  } else {
    try {
      /*if ( not destinationConnection->isClosed() {
	  destinationConnection->stop();
	  destinationConnection->close();
	  }*/
      delete destinationConnection;
      destinationConnection=NULL;
    }
    catch (cms::CMSException &e) {
      LOG_ERRMSG("%s: CMSException encountered closing dmlib destination connection\n%s",__func__,e.what());
      e.printStackTrace();
      return -1;
    }
    catch (exception &e) {
      LOG_ERRMSG("%s: Exception encountered closing dmlib destination connection\n%s",__func__,e.what());
      return -1;
    }
  }
  return 1;
}

bool isAMQconnected() {
  if (destinationConnection==NULL) {
    return false;
  }
  //vck: add real connected test here and make default -1
  LOG_DEBUGMSG("%s: dummy placeholder function call",__func__);
  return true;
}

int startEventReceiver(const char originURL[],
                       const char user[],
                       const char password[],
                       const char originTopic[],
                       const int msReconnect,
                       const int maxAttempts) {
  std::string brokerURI;
  if (conVerbose > 2)
    {
      LOG_DEBUGMSG("%s: Starting dmlib DMMessageReceiver on topic: %s",
	     __func__, originTopic);
    }
  if (eventreceiver != NULL) 
    {
      LOG_DEBUGMSG("%s: Event receiver already initalized",__func__);
      return 0;
    }
  try {
    // Set the URI 
    char *brokerURIchar;
    brokerURIchar = activeMQ_setTcpURIRequest(originURL, msReconnect, maxAttempts);
    brokerURI = std::string(brokerURIchar);
    delete[] brokerURIchar;

    eventreceiver = new DMMessageReceiver(brokerURI, std::string(user), std::string(password),
        std::string(originTopic));
  }
  catch (exception &e)
    {
      LOG_ERRMSG("%s: Encountered Exception creating DMMessageReceiver\n%s",__func__,e.what());
      return -1;
    }
  try {
    eventreceiver->run();
  }
  catch (exception &e)
    {
      LOG_ERRMSG("%s: Encountered Exception running DMMessageReceiver%s",__func__,e.what());
      return -1;
    }
  return 1;
}

char *eventReceiverGetMessage(const int ms_wait, int *ierr) {
  char *message = NULL;
  CoreEventInfo *cei = NULL;
  *ierr = 0;
  if (eventreceiver == NULL) 
    {
      LOG_DEBUGMSG("%s: Event receiver not started!",__func__);
      *ierr = -1;
      return message;
    }

  // Finally, encode algMessage as xml, using the eventsenders encoder
  std::string msg_tmp = "";
  // LOG_DEBUGMSG("%s: Calling eventreceiver->receive(%d, msg_tmp)", __func__, ms_wait);
  cei = eventreceiver->receive(ms_wait, msg_tmp);

  if (msg_tmp.length() > 0) 
    {
      message = (char *)calloc(msg_tmp.length() + 1, sizeof(char));
      strncpy(message, msg_tmp.c_str(), msg_tmp.length());
    }

  // LOG_MSG("%s - Returning", __func__);
  delete cei;
  cei = NULL;
  return message;
}

int stopEventReceiver() {
  if (eventreceiver == NULL) {
    LOG_DEBUGMSG("%s: Event receiver not running", __func__);
    return 0;
  } else {
    delete eventreceiver;
    eventreceiver = NULL;
  }
  return 1;
}

int startEventSender(const char eventtopic[]) {
  if (conVerbose > 2)
    {
      LOG_DEBUGMSG("%s: Starting dmlib DMMessageSender on topic: %s",
	     __func__, eventtopic);
    }
  if (!isAMQconnected()) {
      LOG_ERRMSG("%s: Cannot start event sender without activeMQ connection.",__func__);
      return -1;
    }
  if (eventsender != NULL) {
      LOG_DEBUGMSG("%s: Event sender already initalized",__func__);
      return 0;
    }
  try {
    eventsender = new DMMessageSender(destinationConnection,eventtopic);
  }
  catch (exception &e)
    {
      LOG_ERRMSG("%s: Encountered Exception creating DMMessageSender\n%s",__func__,e.what());
      return -1;
    }
  try {
    eventsender->run();
  }
  catch (exception &e)
    {
      LOG_ERRMSG("%s: Encountered Exception running DMMessageSender%s",__func__,e.what());
      return -1;
    }
  return 1;
}

int stopEventSender() {
  if (eventsender == NULL) {
    LOG_DEBUGMSG("%s: Event sender not running", __func__);
    return 0;
  } else {
    delete eventsender;
    eventsender = NULL;
  }
  if (eventmessage != NULL) {
    delete eventmessage;
    eventmessage = NULL;
  }
  return 1;
}

int startEventEncoder() {
  if (conVerbose > 2)
    {
      LOG_DEBUGMSG("%s: Starting dmlib DMMessageEncoder", __func__);
    }
  if (eventencoder != NULL) {
      LOG_DEBUGMSG("%s: Event encoder already initalized", __func__);
      return 0;
    }
  try {
    eventencoder = new DMMessageEncoder();
  }
  catch (exception &e)
    {
      LOG_ERRMSG("%s: Encountered Exception creating DMMessageEncoder\n%s",__func__,e.what());
      return -1;
    }
  return 1;
}

int stopEventEncoder() {
  if (eventencoder == NULL) {
    LOG_DEBUGMSG("%s: Event encoder not running",__func__);
    return 0;
  } else {
    delete eventencoder;
    eventencoder = NULL;
  }
  return 1;
}

int sendEventMessage() {
  if (conVerbose > 2)
    {
      LOG_DEBUGMSG("%s: Sending event message to activemq topic.",
	     __func__);
    }
  if (eventmessage==NULL) {
    LOG_DEBUGMSG("%s: No message to send",__func__);
    return 0;
  }
  try {
    eventsender->sendMessage(eventmessage);
  }
  catch (exception &e) {
    LOG_ERRMSG("%s: DMMessageSender error: %s",__func__,e.what());
  }
  if (conVerbose>1) {
    LOG_MSG("%s: sent GFAST message to activemqfor evid:%s",__func__,eventmessage->getID().c_str());
  }
  return 1;
}

int sendEventXML(const char xmlstr[]) {
  if (conVerbose > 2)
    {
      LOG_DEBUGMSG("%s: Sending preformatted xml message to event topic.",
	           __func__);
    }
  if (eventsender == NULL)
    {
      LOG_WARNMSG("%s: Event sender not running", __func__);
      return 0;
    }

  eventsender->sendString(xmlstr);
  if (conVerbose>1) {
    LOG_MSG("%s: sent xml message to activemq",__func__);
  }
  return 1;
}

int startHBProducer(const char sender[],
		    const char hbtopic[],
		    int interval=0,
		    int verbose=1) {
  if (destinationConnection==NULL) {
    LOG_ERRMSG("%s: Error: AMQ connection must be started before HBProducer",__func__);
    return -1;
  }
  if (hbproducer!=NULL) {
    LOG_DEBUGMSG("%s: HBProducer already started",__func__);
    return 0;
  }
  hbVerbose=verbose;
  hbSender=string(sender);
  hbTopic=string(hbtopic);
  if (hbVerbose > 2) {
    LOG_DEBUGMSG("%s: Starting heartbeat producer on topic: %s",__func__, hbtopic);
  }
  try {
    hbproducer = new HBProducer(destinationConnection,hbSender,hbTopic,interval);
  }
  catch (exception &e) {
    LOG_ERRMSG("%s: Encountered Exception creating dmlib HB producer%s",__func__,e.what());
    return -1;
  }
  /* this part is not working (steals the program) 
  if (interval > 0) {
    try {
      hbproducer->run();
    }
    catch (exception &e) {
      printf("%s: Encountered Exception in dmlib HB producer\n%s",__func__,e.what());
      return -1;
    }
  }
  */

  return (hbproducer==NULL)?-1:1;
}

int stopHBProducer() {
  if (hbVerbose > 2)
    {
      LOG_DEBUGMSG("%s: killing heartbeat producer",__func__);
    }

  if (hbproducer==NULL) {
    LOG_DEBUGMSG("%s: HB producer not running",__func__);
      return 0;
  } else {
    hbproducer->stop();
    delete hbproducer;
    hbproducer=NULL;
  }
  return 1;
}

int sendHeartbeat(){
  std::string timestr;
  if (hbproducer==NULL) {
    LOG_DEBUGMSG("%s: HB producer not running",__func__);
      return 0;
  }
  hbproducer->sendHeartbeat("","","");
  return 1;
}

char *getDmLibVersion() {
  char *version=NULL;
  std::string version_string;
  version_string = DMLib::getVersion();

  version = (char *)calloc(version_string.length() + 1, sizeof(char));
  strcpy(version, version_string.c_str());

  return version;
}

char *dmlibWrapper_createPGDXML(const double currentTime,
                                const enum opmode_type mode,
                                const char *alg_vers,
                                const char *instance,
                                const char *message_type, 
                                const int max_assoc_stations,
                                const struct coreInfo_struct *core,
                                const struct GFAST_pgdResults_struct *pgd,
                                const struct GFAST_peakDisplacementData_struct *pgd_data,
                                int *ierr) {

  char *xmlmsg;
  xmlmsg = NULL;
  *ierr = 0;

  struct dist_index *vals;
  vals = (struct dist_index *) calloc((size_t) pgd->nsites, sizeof(struct dist_index));
  // int max_assoc_stations = 6;

  // Convert enum units to char
  char magUnits[32], magUncerUnits[32], latUnits[32], latUncerUnits[32], lonUnits[32],
      lonUncerUnits[32], depthUnits[32], depthUncerUnits[32], origTimeUnits[32],
      origTimeUncerUnits[32];
  __xml_units__enum2string(core->magUnits, magUnits);
  __xml_units__enum2string(core->magUncerUnits, magUncerUnits);
  __xml_units__enum2string(core->latUnits, latUnits);
  __xml_units__enum2string(core->latUncerUnits, latUncerUnits);
  __xml_units__enum2string(core->lonUnits, lonUnits);
  __xml_units__enum2string(core->lonUncerUnits, lonUncerUnits);
  __xml_units__enum2string(core->depthUnits, depthUnits);
  __xml_units__enum2string(core->depthUncerUnits, depthUncerUnits);
  __xml_units__enum2string(core->origTimeUnits, origTimeUnits);
  __xml_units__enum2string(core->origTimeUncerUnits, origTimeUncerUnits);

  // LOG_MSG("%s", "createEventXML - set unit chars");

  // convert to CoreEventInfo MessageCategory
  enum MessageCategory imode;
  if (mode == REAL_TIME_EEW) {
    imode = LIVE;
  } else if (mode == PLAYBACK) {
    imode = TEST;
  } else if (mode == OFFLINE) {
    imode = TEST;
  } else {
    LOG_DEBUGMSG("%s", "Defaulting to live mode");
    imode = LIVE;
  }

  // convert to CoreEventInfo nudMessageType
  enum nudMessageType itype;
  if (strcmp(message_type, "new") == 0) {
    itype = NEW;
  } else if (strcmp(message_type, "update") == 0) {
    itype = UPDATE;
  } else {
    LOG_DEBUGMSG("%s", "Message type not recognized! Defaulting to update");
    itype = UPDATE;
  }

  // required to create FiniteFaultMessage
  enum FaultSegment::FaultSegmentShape shape = FaultSegment::UNKNOWN_SEGMENT;

  // Get time stamp for when the message is sent, based on the timestamp sent from the outer
  // driveGFAST loop. Could be based on the actual current time, or the last time we have data for.
  int rc;
  char cnow[128];
  rc = xml_epoch2string(currentTime, cnow);
  if (rc != 0) {
    LOG_DEBUGMSG("%s", "Error getting time string!");
  }

  FiniteFaultMessage algMessage(GFAST, shape, core->id, core->mag, core->magUncer, core->lat,
      core->latUncer, core->lon, core->lonUncer, core->depth, core->depthUncer, core->origTime,
      core->origTimeUncer, core->likelihood, itype, core->version, imode, cnow, alg_vers,
      instance, core->numStations, magUnits, magUncerUnits, latUnits,
      latUncerUnits, lonUnits, lonUncerUnits, depthUnits, depthUncerUnits, origTimeUnits,
      origTimeUncerUnits);

  // Replace time based on datestr. Accounts for potential leapseconds issues when switching
  // between ISCL and ShakeAlert/qlib2 time handling. 
  // ShakeAlert wants 2011-05-06T18:12:37.038Z" format which is provided by xml_epoch2string
  char origTimeChar[128];
  rc = xml_epoch2string(core->origTime, origTimeChar);
  if (rc != 0) {
    LOG_DEBUGMSG("%s", "Error getting time string!");
  }
  std::string origTimeStr(origTimeChar);
  algMessage.setOriginTime(origTimeStr);
  LOG_DEBUGMSG("old origTime: %lf, origTimeChar: %s, origTimeStr: %s, new origTime: %lf",
    core->origTime, origTimeChar, origTimeStr.c_str(), algMessage.getOriginTime());

  // Now add pgd observations to algMessage
  int i, j;
  int scnl_n = 8;
  char obs_sta[scnl_n], obs_net[scnl_n], obs_chan[scnl_n], obs_loc[scnl_n];
  char *token = NULL, *work = NULL;
  bool assoc_flag = false;
  int n_assoc = 0;
  double tmp_lon;

  enum ObservationType obs_type = DISPLACEMENT_OBS;
  
  // Later we assume pgd->nsites = pgd_data->nsites and indices correspond
  if (pgd->nsites != pgd_data->nsites) 
    {
      LOG_WARNMSG("%s: nsites don't match for pgd, pgd_data! %d, %d" ,__func__, pgd->nsites,
                   pgd_data->nsites);
      *ierr = -1;
      return xmlmsg;
    }

  // Extra step to sort observations by distance, so that the first max_assoc_stations get the
  // assoc_flag = true. This is used by the Solution Aggregator to associate events. Other
  // algorithms might sort by observation value, but in this case we want to be sure to associate
  // with the input location, since GFAST doesn't determine its own event location.
  // Load distance and index to sort pgd obs by distance from source
  for ( i = 0; i < pgd->nsites; i++ ) 
    {
      vals[i].indx = i;
      // srdist has ndep*nsites. Just use the first depth, they should all be relatively the same
      vals[i].dist = pgd->srdist[i];
    }
  
  // Sort by distance
  qsort((void *) vals, (size_t) pgd->nsites, sizeof(struct dist_index), compareDist);

  // Go through the observations in ascending order of source-receiver distance
  for ( j = 0; j < pgd->nsites; j++ ) 
    {
      i = vals[j].indx;
      // skip site if it wasn't used
      if (!pgd->lsiteUsed[i]) { continue; }

      // see core/data/readMetaDataFile for similar SNCL parsing
      memset(obs_sta, 0, scnl_n*sizeof(char));
      memset(obs_net, 0, scnl_n*sizeof(char));
      memset(obs_chan, 0, scnl_n*sizeof(char));
      memset(obs_loc, 0, scnl_n*sizeof(char));

      work = (char *)calloc(strlen(pgd_data->stnm[i])+1, sizeof(char));
      strcpy(work, pgd_data->stnm[i]);

      token = strtok(work, ".");
      int i_tok = 0;
      while (token) 
        {
          if (i_tok == 0) { strcpy(obs_net, token); }
          if (i_tok == 1) { strcpy(obs_sta, token); }
          if (i_tok == 2) { strcpy(obs_chan, token); }
          if (i_tok == 3) { strcpy(obs_loc, token); }
          i_tok++;
          token = strtok(NULL, ".");
        }
      free(work);
      work = NULL;
      
      assoc_flag = (n_assoc >= max_assoc_stations) ? false : true;
      // Make sure longitude follows ShakeAlert convention
      tmp_lon = pgd_data->sta_lon[i];
      tmp_lon = (tmp_lon > 180) ? tmp_lon - 360 : tmp_lon;

      // Make sure to convert pgd to cm (from m)
      algMessage.addGMObservation(obs_type,
                                  obs_sta,
                                  obs_net,
                                  obs_chan,
                                  obs_loc,
                                  pgd_data->pd[i] * 100.,
                                  pgd_data->sta_lat[i],
                                  tmp_lon,
                                  pgd_data->pd_time[i],
                                  "cm\0",
                                  "deg\0",
                                  "deg\0",
                                  "UTC\0",
                                  "gfast\0",
                                  assoc_flag);

      n_assoc++;
    }

  LOG_MSG("%s", "createEventXML - finished adding gmobs, encoding message");

  // Finally, encode algMessage as xml
  std::string msg_tmp;

  try {
    // Using separate Encoder instead of the Sender's Encoder allows for easier testing and separation
    // of tasks.
    msg_tmp = eventencoder->encodeMessage(&algMessage);
    // msg_tmp = eventsender->getEncodedMessage(&algMessage);
  }
  catch (exception &e) {
    LOG_ERRMSG("%s: DMMessageSender error while encoding: %s",__func__,e.what());
    *ierr = -1;
    return xmlmsg;
  }

  xmlmsg = (char *)calloc(msg_tmp.length() + 1, sizeof(char));
  strncpy(xmlmsg, msg_tmp.c_str(), msg_tmp.length());

  free(token);
  free(vals);
  LOG_MSG("%s", "createEventXML - Returning");
  return xmlmsg;
}

static int compareDist(const void *x, const void *y) {
  const struct dist_index xx = *(const struct dist_index *) x;
  const struct dist_index yy = *(const struct dist_index *) y;

  if (xx.dist < yy.dist) return -1;
  if (xx.dist > yy.dist) return 1;
  return 0;
}
