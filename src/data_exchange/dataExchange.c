#include "gfast_core.h"
#include "gfast_dataexchange.h"
#include <iniparser.h>
#include "iscl/os/os.h"

static void setVarName(const char *group, const char *variable,
                       char *var)
{
    memset(var, 0, 256*sizeof(char));
    sprintf(var, "%s:%s", group, variable);
    return;
}

void dataexchange_initializeDataConnection(
    struct dataconn_props_struct *props,
    void **connection,
    void **subscription) 
{
#ifdef GFAST_USE_EW
    LOG_MSG("%s", "Initializing Earthworm");
#endif

#ifdef GFAST_ENABLE_GEOJSON
    #ifdef GFAST_USE_NATS
    LOG_MSG("%s", "Initializing NATS");
    dataexchange_nats_connect(props, connection, subscription);
    #elif GFAST_USE_KAFKA
    LOG_MSG("%s", "Initializing Kafka");
    #else 
    LOG_ERRMSG("%s", "No data connections specified!");
    #endif
#endif

}

void dataexchange_closeDataConnection(
    void **connection,
    void **subscription) 
{
#ifdef GFAST_USE_EW
    LOG_MSG("%s", "Closing Earthworm");
#endif

#ifdef GFAST_ENABLE_GEOJSON
    #ifdef GFAST_USE_NATS
    LOG_MSG("%s", "Closing NATS connection");
    dataexchange_nats_close(connection, subscription);
    #elif GFAST_USE_KAFKA
    LOG_MSG("%s", "Closing Kafka");
    #else 
    LOG_ERRMSG("%s", "No data connections specified!");
    #endif
#endif

}

char *dataexchange_getMessages(
    void **subscription,
    const int max_payload_size,
    const int message_block,
    int *n_messages,
    int *ierr) 
{
    char *msgs = NULL;
#ifdef GFAST_USE_EW
    LOG_MSG("%s", "Getting data from Earthworm");
#endif

#ifdef GFAST_ENABLE_GEOJSON
    #ifdef GFAST_USE_NATS
    LOG_MSG("%s", "Getting data from NATS");
    msgs = dataexchange_nats_getMessages(
        subscription,
        max_payload_size,
        message_block,
        n_messages,
        ierr);
    #elif GFAST_USE_KAFKA
    LOG_MSG("%s", "Getting data from Kafka");
    #else 
    LOG_ERRMSG("%s", "No data connections specified!");
    #endif
#endif

    return msgs;
}


/*! 
 * @brief Reads the DataConn properties from the initialization file.
 *
 * @param[in] propfilename     Name of properties file.
 * @param[in] group            Group in ini file.  Likely "DataConn".
 *
 * @param[out] dataconn_props  Data Connection properties.
 *
 * @result 0 indicates success.
 *
 */
int dataexchange_readIni(const char *propfilename,
                     const char *group,
                     struct dataconn_props_struct* data_conn_props) {
  const char *s;
  char var[256];
  int ierr;
  dictionary *ini;
  ierr = 1;
  memset(data_conn_props, 0, sizeof(struct dataconn_props_struct));
  if (!os_path_isfile(propfilename)) {
    LOG_ERRMSG("%s: Properties file: %s does not exist\n",
		__func__, propfilename);
    return ierr;
  }
  ini = iniparser_load(propfilename);
  // Read the properties
  setVarName(group, "servers\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL) {
    LOG_ERRMSG("%s: Could not find servers string!\n", __func__);
    goto ERROR;
  } else {
    strcpy(data_conn_props->servers, s);
  }

  setVarName(group, "topic\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL) {
    LOG_ERRMSG("%s: Could not find topic string!\n", __func__);
    goto ERROR;
  } else {
    strcpy(data_conn_props->topic, s);
  }

  setVarName(group, "groupid\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL) {
      LOG_ERRMSG("%s: Could not find groupid string!\n", __func__);
      goto ERROR;
  } else {
      strcpy(data_conn_props->groupid, s);
  }

  ierr = 0;
ERROR:;
  iniparser_freedict(ini);
  return ierr;
}
