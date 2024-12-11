#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <iniparser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_core.h"
#include "iscl/os/os.h"
#ifdef GFAST_USE_AMQ
#include "gfast_activeMQ.h"
#endif
#include "gfast_dataexchange.h"

int set_array_from_string_int(const char *prop, int *array, int max_size);
int set_array_from_string_str(const char *prop, char **array, int max_size);

/*!
 * @brief Initializes the GFAST properties (parameter) structure
 *
 * @param[in] propfilename   Name of properties file.
 * @param[in] opmode         GFAST operational mode.  Can be OFFLINE
 *                           for playback or REAL_TIME_EEW for real-time
 *                           earthquake early warning.
 *
 * @param[out] props         On successful exit holds the GFAST properties
 *                           structure.
 *
 * @result 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int core_properties_initialize(const char *propfilename,
                               const enum opmode_type opmode,
                               struct GFAST_props_struct *props)
{
  const char *s;
  char cwork[PATH_MAX];
  int ierr, itemp;
  // int lenos;
  dictionary *ini;
  //------------------------------------------------------------------------//
  // Require the properties file exists
  if (!os_path_isfile(propfilename))
    {
#ifdef ENABLE_PLOG
      LOG_ERRMSG("properties file (%s) does not exist\n", propfilename);
#else
      printf("properties file (%s) does not exist\n", propfilename);
#endif
      return -1;
    }
   
  ierr =-1;
  memset(props, 0, sizeof(struct GFAST_props_struct));
  props->opmode = opmode;

  // Load the ini file
  ini = iniparser_load(propfilename);
  if (ini == NULL) {
#ifdef ENABLE_PLOG
    LOG_ERRMSG("Iniparser could not read: %s\n", propfilename);
#else
    printf("Iniparser could not read: %s\n", propfilename);
#endif
    return -1;
  }
  strcpy(props->propfilename, propfilename);
  //-------------------------GFAST General Parameters-----------------------//

#ifndef ENABLE_PLOG
  // set open output log file.
  s = iniparser_getstring(ini, "general:logFileName\0",
			  "gfast.log\0");
  printf("Opening %s for log output\n",s);
  core_log_openLog(s);
  if (!os_path_isfile(s))
    {
      printf("Cannot open log output file %s\n", s);
      return -1;
    }
#endif

  //metadata file
  s = iniparser_getstring(ini, "general:metaDataFile\0",
			  "GFAST_streams.txt\0");
  strcpy(props->metaDataFile, s);
  if (!os_path_isfile(props->metaDataFile))
    {
      LOG_ERRMSG("Cannot find station list (%s)\n", props->metaDataFile);
      return -1;
    }

  // Option to restrict the networks used
  int n_networks;
  s = iniparser_getstring(ini, "general:metaDataNetworks\0", "\0");
  n_networks = set_array_from_string_str(s, props->metaDataNetworks, 16);
  props->n_networks = n_networks;
    
  //site mask file
  s = iniparser_getstring(ini, "general:siteMaskFile\0", NULL);
  if (s != NULL)
    {
      strcpy(props->siteMaskFile, s);
      if (!os_path_isfile(props->siteMaskFile))
        {
	  memset(props->siteMaskFile, 0, sizeof(props->siteMaskFile));
        }
    }

  s = iniparser_getstring(ini, "general:output_interval_mins\0", NULL);
  int n_intervals;
  n_intervals = set_array_from_string_int(s, props->output_interval_mins, MAX_OUTPUT_INTERVALS);
  if (n_intervals > MAX_OUTPUT_INTERVALS) {
    LOG_MSG("WARNING: output_interval_mins exceeds MAX_OUTPUT_INTERVALS=%d, capping at %d!\n",
            MAX_OUTPUT_INTERVALS, MAX_OUTPUT_INTERVALS);
    n_intervals = MAX_OUTPUT_INTERVALS;
  }
  props->n_intervals = n_intervals;

  s = iniparser_getstring(ini, "general:SA_events_dir\0", ".\0");
  if (s != NULL)
    {
      strcpy(props->SAeventsDir, s);
      if (!os_path_isdir(props->SAeventsDir))
        {
	  LOG_ERRMSG("SA events directory %s doesn't exist",
		     props->SAeventsDir);
	  goto ERROR; 
        }
      if (strlen(props->SAeventsDir) == 0)
        {
	  strcpy(props->SAeventsDir, "./\0");
        }
      else
        {
	  if (props->SAeventsDir[strlen(props->SAeventsDir)-1] != '/')
            {
	      strcat(props->SAeventsDir, "/\0");
            }
        }
    }
    else
      {
        //strcpy(props->SAeventsDir, "\0");
        LOG_MSG("No SA events directory specified --> Use:%s", ".");
        /*strcpy(props->SAeventsDir, "./\0");*/
      }

  s = iniparser_getstring(ini, "general:SA_output_dir\0", ".");
  if (s != NULL)
    {
      strcpy(props->SAoutputDir, s);
      if (!os_path_isdir(props->SAoutputDir))
	{
	  LOG_ERRMSG("SA output directory %s doesn't exist",
		     props->SAoutputDir);
	  goto ERROR; 
	}
      if (strlen(props->SAoutputDir) == 0)
	{
	  strcpy(props->SAoutputDir, "./\0");
	}
      else
	{
	  if (props->SAoutputDir[strlen(props->SAoutputDir)-1] != '/')
	    {
	      strcat(props->SAoutputDir, "/\0");
	    }
	}
    }
  else
    {
      //strcpy(props->SAoutputDir, "\0");
      LOG_MSG("No SA output directory specified --> Use:%s", ".");
    }

  props->bufflen = iniparser_getdouble(ini, "general:bufflen\0", 1800.0);
  if (props->bufflen <= 0.0)
    {
      LOG_ERRMSG("Buffer lengths=%f must be positive!", props->bufflen);
      goto ERROR;
    }
  if (props->opmode == OFFLINE)
    {
      s = iniparser_getstring(ini, "general:eewsfile\0", NULL);
      if (s == NULL)
        {
	  LOG_ERRMSG("%s", "Could not find decision module XML file!");
	  goto ERROR;
        }
      strcpy(props->eewsfile, s);
      s = iniparser_getstring(ini, "general:observed_data_dir\0", NULL);
      if (s != NULL)
        {
	  strcpy(props->obsdataDir, s);
	  if (!os_path_isdir(props->obsdataDir))
            {
	      LOG_ERRMSG("Observed data directory %s doesn't exist",
			 props->obsdataDir);
	      goto ERROR; 
            }
	  if (strlen(props->obsdataDir) == 0)
            {
	      strcpy(props->obsdataDir, "./\0");
            }
	  else
            {
	      if (props->obsdataDir[strlen(props->obsdataDir)-1] != '/')
                {
		  strcat(props->obsdataDir, "/\0");
                }
            }
        }
      else
        {
	  strcpy(props->obsdataDir, "./\0");
        }
      s = iniparser_getstring(ini, "general:observed_data_file\0", NULL);
      if (s != NULL)
        {
	  strcpy(props->obsdataFile, s);
	  memset(cwork, 0, sizeof(cwork));
	  strcpy(cwork, props->obsdataDir);
	  strcat(cwork, props->obsdataFile);
	  if (!os_path_isfile(cwork))
            {
	      LOG_ERRMSG("Observed data file %s doesn't exist", cwork);
	      goto ERROR;
            }
        }
      else
        {
	  LOG_ERRMSG("%s", "Must specify observed data file!");
        }
      s = iniparser_getstring(ini, "general:synthetic_data_prefix\0", "LX\0");
    }
  // UTM zone
  props->utm_zone = iniparser_getint(ini, "general:utm_zone\0", -12345);
  if (props->utm_zone < 0 || props->utm_zone > 60)
    {
      if (props->utm_zone !=-12345)
        {
	  LOG_WARNMSG("UTM zone %d is invalid estimating from hypocenter",
		      props->utm_zone);
	  props->utm_zone =-12345;
        } 
    }
  // Verbosity
  props->verbose = iniparser_getint(ini, "general:verbose\0", 2);
  // Sampling period
  props->dt_default = iniparser_getdouble(ini, "general:dt_default\0", 1.0);
  if (props->dt_default <= 0.0)
    {
      LOG_WARNMSG("Default sampling period %f invalid; defaulting to %f!",
		  props->dt_default, 1.0);
      props->dt_default = 1.0;
    }
  itemp = iniparser_getint(ini, "general:dt_init\0", 3);
  props->dt_init = (enum dtinit_type) itemp; //iniparser_getint(ini, "general:dt_init\0", 3);
  if (props->opmode != OFFLINE)
    {
      if (props->dt_init != INIT_DT_FROM_TRACEBUF)
        {
	  LOG_WARNMSG("%s", "Obtaining sampling period from tracebuf");
	  props->dt_init = INIT_DT_FROM_TRACEBUF; //3;
        }
    }
  if (props->opmode == OFFLINE)
    {
      // Make sure the EEW XML file exists
      if (!os_path_isfile(props->eewsfile))
        {
	  LOG_ERRMSG("Cannot find EEW XML file %s!", props->eewsfile);
	  goto ERROR;
        }
      // Figure out how to initialize sampling period
      if (props->dt_init == INIT_DT_FROM_FILE)
        {
	  itemp = iniparser_getint(ini, "general:dt_init\0", 1);
	  props->dt_init = (enum dtinit_type) itemp;
	  if (s == NULL)
            {
	      LOG_ERRMSG("%s", "Must specify metaDataFile!");
	      goto ERROR; 
            }
        }
      else if (props->dt_init == INIT_DT_FROM_SAC)
        {
	  props->dt_init = INIT_DT_FROM_SAC;
        }
      else
        {
	  if (props->dt_init != INIT_DT_FROM_DEFAULT)
            {
	      LOG_WARNMSG("%s", "Setting dt from default");
	      props->dt_init = INIT_DT_FROM_DEFAULT;
            }
        }
    }
  else
    {
      if (props->dt_init != INIT_DT_FROM_TRACEBUF)
        {
	  LOG_WARNMSG("%s", "Will get GPS sampling period from tracebuffer!");
	  props->dt_init = INIT_DT_FROM_TRACEBUF;
        }
    }
  // Wait time
  props->waitTime = 1.0;
  if (props->opmode == REAL_TIME_EEW)
    {
      props->waitTime = iniparser_getdouble(ini, "general:waitTime\0", 1.0);
      if (props->waitTime < 0.0)
        {
	  LOG_ERRMSG("Invalid wait time %f!", props->waitTime);
	  goto ERROR;
        } 
    }        
  // Location initialization
  itemp = iniparser_getint(ini, "general:loc_init\0", 1);
  props->loc_init = (enum locinit_type) itemp;
  if (props->opmode == OFFLINE)
    {
      if (props->loc_init == INIT_LOCS_FROM_TRACEBUF)
        {
	  LOG_ERRMSG("%s", "offline cant initialize locations from tracebuf");
	  goto ERROR;
        }
    }
  // Synthetic runtime
  if (props->opmode == OFFLINE)
    {
      props->synthetic_runtime
	= iniparser_getdouble(ini, "general:synthetic_runtime\0", 0.0); 
    } 
  // Processing time
  props->processingTime
    = iniparser_getdouble(ini, "general:processing_time\0", 300.0);
  if (props->processingTime > props->bufflen)
    {
      LOG_ERRMSG("%s", "Error processing time cannot exceed buffer length");
      goto ERROR;
    }
  // Default earthquake depth
  props->eqDefaultDepth
    = iniparser_getdouble(ini, "general:default_event_depth\0", 8.0);
  if (props->eqDefaultDepth < 0.0)
    {
      LOG_ERRMSG("Error default earthquake depth must be positive %f",
		 props->eqDefaultDepth);
      goto ERROR;
    }
  // H5 archive directory
  s = iniparser_getstring(ini, "general:h5ArchiveDirectory\0", NULL);
  if (s == NULL)
    {
      strcpy(props->h5ArchiveDir, "./\0");
    }
  else
    {
      strcpy(props->h5ArchiveDir, s);
      if (!os_path_isdir(props->h5ArchiveDir))
        {
	  LOG_WARNMSG("Archive directory %s doesn't exist",
		      props->h5ArchiveDir);
	  LOG_WARNMSG("%s", "Will use current working directory");
	  memset(props->h5ArchiveDir, 0, sizeof(props->h5ArchiveDir));
	  strcpy(props->h5ArchiveDir, "./\0");
        }
    }
  // Only write summary HDF5 files?
  props->lh5SummaryOnly = iniparser_getboolean(ini, "general:H5SummaryOnly\0",
					       false);

  // ANSS informaiton
  s = iniparser_getstring(ini, "general:anssNetwork\0", "UW\0");
  strcpy(props->anssNetwork, s);
  s = iniparser_getstring(ini, "general:anssDomain\0", "anss.org\0"); 
  strcpy(props->anssDomain, s);
  //---------------------------Earthworm parameters-------------------------//
  if (props->opmode == REAL_TIME_EEW)
    {
      s = iniparser_getstring(ini, "earthworm:gpsRingName\0", "WAVE_RING\0");
      strcpy(props->ew_props.gpsRingName, s);
      if (strlen(props->ew_props.gpsRingName) < 1)
        {
	  LOG_WARNMSG("%s", "GPS ring name may not be specified");
        }
      s = iniparser_getstring(ini, "earthworm:moduleName\0", "geojson2ew\0");
      strcpy(props->ew_props.moduleName, s);
      if (strlen(props->ew_props.moduleName) < 1)
        {
	  LOG_ERRMSG("%s", "Module name is not specified");
	  goto ERROR;
        }
    }
  // Free the ini file
  iniparser_freedict(ini);
  //------------------------------PGD Parameters----------------------------//
  ierr = core_scaling_pgd_readIni(propfilename, "PGD\0",
				  props->verbose, props->utm_zone,
				  &props->pgd_props);
  if (ierr != 0)
    {
      LOG_ERRMSG("%s", "Error reading PGD parameters");
      goto ERROR;
    }
  //----------------------------CMT Parameters------------------------------//
  ierr = core_cmt_readIni(propfilename, "CMT\0",
			  props->verbose, props->utm_zone,
			  &props->cmt_props); 
  if (ierr != 0)
    {
      LOG_ERRMSG("%s", "Error reading CMT parameters");
      goto ERROR;
    } 
  //------------------------------FF Parameters-----------------------------//
  ierr = core_ff_readIni(propfilename, "FF\0",
			 props->verbose, props->utm_zone,
			 props->cmt_props.min_sites,
			 &props->ff_props);
  if (ierr != 0)
    {
      LOG_ERRMSG("%s", "Error reading FF parameters");
      goto ERROR; 
    }
  //---------------------------ActiveMQ Parameters--------------------------//
#ifdef GFAST_USE_AMQ
  if (props->opmode == REAL_TIME_EEW) 
    {
      ierr = activeMQ_readIni(propfilename, "ActiveMQ\0",
			      &props->activeMQ_props);
      if (ierr != 0)
        {
	  LOG_ERRMSG("%s", "Error reading ActiveMQ group parameters");
	  goto ERROR; 
        }
    } // End check on need for ActiveMQ
#endif
  //---------------------------Data Connection Parameters--------------------------//
  ierr = dataexchange_readIni(propfilename,
           "DataConn\0",
            &props->data_conn_props);
  if (ierr != 0) {
    LOG_ERRMSG("%s", "Error reading Data Connection group parameters");
    goto ERROR; 
  }
  //
  // Success!
  ierr = 0;
  return ierr;
 ERROR:;
  return ierr;
}

int set_array_from_string_int(const char *prop, int *array, int max_size) {
  int i, j = 0;
  int *arr = array;
  // ensure the array is initialized to zeros
  arr[0] = 0;
  if (prop != NULL)
    {
      //// Traverse the string
      for (i = 0; prop[i] != '\0'; i++) {
        if (prop[i] == ',') {
          j++;
          // stop if the max_size is exceeded
          if (j >= max_size) break;
          // ensure the array is initialized to zeros
          arr[j] = 0;
        }
        else {
          // ASCII decimal for '0' is 48
          arr[j] = arr[j] * 10 + (prop[i] - 48);
        }
      }
    }

  // return the number of items added to the array
  return j + 1;
}

int set_array_from_string_str(const char *prop, char **array, int max_size) {

    char *tok, *p, *last;
    char *s = strdup(prop);
    int i = 0, lens;

    tok = s;
    while(((p = strtok_r(tok, ", \t", &last)) != NULL) && (i < max_size)) {
        tok = NULL;

        lens = (int) (strlen(p));
        array[i] = (char *)calloc((size_t) (lens+1), sizeof(char));
        strcpy(array[i], p);
        i++;
    }
    free(s);

    return i;
} 
