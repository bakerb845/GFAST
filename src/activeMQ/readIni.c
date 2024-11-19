#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include <iniparser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_core.h"
#include "gfast_activeMQ.h"
#include "iscl/os/os.h"

static void setVarName(const char *group, const char *variable,
                       char *var)
{
  memset(var, 0, 256*sizeof(char));
  sprintf(var, "%s:%s", group, variable);
  return;
}
/*!
 * @brief Reads the ActiveMQ properties from the initialization file.
 *
 * @param[in] propfilename     Name of properties file.
 * @param[in] group            Group in ini file.  Likely "ActiveMQ".
 *
 * @param[out] activeMQ_props  ActiveMQ properties.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker (ISTI)
 *
 */
int activeMQ_readIni(const char *propfilename,
                     const char *group,
                     struct GFAST_activeMQ_struct *activeMQ_props)
{
  const char *s;
  char var[256];
  int ierr;
  dictionary *ini;
  ierr = 1;
  memset(activeMQ_props, 0, sizeof(struct  GFAST_activeMQ_struct));
  if (!os_path_isfile(propfilename))
    {
      LOG_ERRMSG("%s: Properties file: %s does not exist\n",
		 __func__, propfilename);
      return ierr;
    }
  ini = iniparser_load(propfilename);
  // Read the properties
  setVarName(group, "originURL\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL)
    {
      LOG_ERRMSG("%s: Could not find ActiveMQ originURL!\n", __func__);
      goto ERROR;
    }
  else
    {
      strcpy(activeMQ_props->originURL, s);
    }

  setVarName(group, "destinationURL\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL)
    {
      LOG_ERRMSG("%s: Could not find ActiveMQ destinationURL!\n", __func__);
      goto ERROR;
    }
  else
    {
      strcpy(activeMQ_props->destinationURL, s);
    }

  setVarName(group, "user\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL)
    {
      LOG_ERRMSG("%s: Could not find ActiveMQ user!\n", __func__);
      goto ERROR;
    }
  else
    {
      strcpy(activeMQ_props->user, s);
    }

  setVarName(group, "password\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL)
    {
      LOG_ERRMSG("%s: Could not find password!\n", __func__);
      goto ERROR;
    }
  else
    {
      strcpy(activeMQ_props->password, s);
    }

  setVarName(group, "originTopic\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL)
    {
      LOG_ERRMSG("%s: Could not find activeMQ originTopic\n", __func__);
      goto ERROR;
    }
  else
    {
      strcpy(activeMQ_props->originTopic, s);
    }

  setVarName(group, "destinationTopic\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL)
    {
      LOG_ERRMSG("%s: Could not find ActiveMQ destinationTopic!\n",__func__);
      goto ERROR;
    }
  else
    {
      strcpy(activeMQ_props->destinationTopic, s);
    }

  setVarName(group, "hbTopic\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL)
    {
      LOG_ERRMSG("%s: Could not find ActiveMQ hbTopic!\n",
		 __func__);
      goto ERROR;
    }
  else
    {
      strcpy(activeMQ_props->hbTopic, s);
    }
  setVarName(group, "hbInterval\0", var);
  activeMQ_props->hbInterval = iniparser_getint(ini, var, 5);
  if (activeMQ_props->hbInterval < 0)
    {
      LOG_ERRMSG("%s: hbInterval = %d not usable \n", __func__,activeMQ_props->hbInterval);
      goto ERROR;
    }
  setVarName(group, "msReconnect\0", var);
  activeMQ_props->msReconnect = iniparser_getint(ini, var, 500);
  if (activeMQ_props->msReconnect < 0)
    {
      LOG_WARNMSG("%s: Overriding msReconnect to 500\n", __func__);
      activeMQ_props->msReconnect = 500;
    }
  setVarName(group, "maxAttempts\0", var);
  activeMQ_props->maxAttempts = iniparser_getint(ini, var, 5);
  if (activeMQ_props->maxAttempts < 0)
    {
      LOG_WARNMSG("%s: Overriding maxAttempts to 5\n", __func__);
      activeMQ_props->maxAttempts = 5;
    }
  setVarName(group, "msWaitForMessage\0", var);
  activeMQ_props->msWaitForMessage  = iniparser_getint(ini, var, 1);
  if (activeMQ_props->msWaitForMessage < 0)
    {
      LOG_WARNMSG("%s: ActiveMQ could hang indefinitely, overriding to 1\n",
		  __func__);
      activeMQ_props->msWaitForMessage = 1;
    }
  setVarName(group, "maxMessages\0", var);
  activeMQ_props->maxMessages  = iniparser_getint(ini, var, 5);
  if (activeMQ_props->maxMessages <= 0)
    {
      LOG_WARNMSG("%s: ActiveMQ message buffer must be >0, overriding to 5\n",
		  __func__);
      activeMQ_props->maxMessages = 5;
    }
  setVarName(group, "MsgTimeToLive\0", var);
  activeMQ_props->msg_ttl  = iniparser_getdouble(ini, var, 0);
  if (activeMQ_props->msg_ttl < 0)
    {
      LOG_WARNMSG("%s: ActiveMQ message time to live must be >= 0, overriding to 0\n",
		  __func__);
      activeMQ_props->msg_ttl = 0;
    }

  ierr = 0;
 ERROR:;
  iniparser_freedict(ini);
  return ierr;
}
