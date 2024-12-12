#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "gfast.h"
#include "gfast_core.h"
#include "iscl/memory/memory.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"
#include <dirent.h>
#include "dmlibWrapper.h"

#include <time.h>

#define MAX_MESSAGES 100000

/*!
 * @brief poll directory for new messages.
 * @param[in] dirname directory to search for messages
 * @param[out] error code
 * @return contents of message(s?) read from directory
 */
char *check_dir_for_messages(const char *dirname, int *ierr);

/*!
 * @brief Print program and library information
 * @param[in] use_dmlib Is dmlib being used?
 * @param[in] fcnm What is the name of the main function?
 */

void printProgramInfo(const char *fcnm) {
    const int bufflen = 1024;
    char *message=NULL;
    int ind;
    unsigned H5majnum, H5minnum, H5relnum;

    ind = 0;
    message = (char *)malloc(bufflen * sizeof(char));

    ind += snprintf(message + ind, bufflen - ind, "%s Version: %s (Build %s %s by %s)\n",
        fcnm, GFAST_VERSION, __DATE__, __TIME__, BUILDER);
    
#ifdef GFAST_USE_DMLIB
    char *dmlib_version = getDmLibVersion();
    ind += snprintf(message + ind, bufflen - ind, "%s\n", dmlib_version);
    free(dmlib_version);
#endif
    H5get_libversion(&H5majnum, &H5minnum, &H5relnum);
    ind += snprintf(message + ind, bufflen - ind, "HDF5 library version: %u.%u.%u\n", 
        H5majnum, H5minnum, H5relnum);
    ind += snprintf(message + ind, bufflen - ind, "ISCL library version: %s\n", ISCL_VERSION);
    
#ifdef ENABLE_PLOG
    LOG_MSG("%s", message);
#else
    // Need to use printf until GFAST_core_properties_initialize is called
    printf("%s\n", message);
#endif
    free(message);
}

/*!
 * @brief GFAST earthquake early warning driver routine
 * First argument assumed to be configuration file name.  Defaults to gfast.props.
 *
 */
int main(int argc, char **argv)
{
    const char *fcnm = "gfast_eew\0";
    char propfilename[PATH_MAX];
    char *message_dir; 
    struct GFAST_activeEvents_struct events;
    struct GFAST_activeEvents_xml_status xml_status;
    struct GFAST_cmtResults_struct cmt;
    struct GFAST_ffResults_struct ff;
    struct h5traceBuffer_struct h5traceBuffer;
    struct gnsstraceData_struct gnsstraceData;
    struct GFAST_offsetData_struct cmt_data, ff_data;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_data_struct gps_data;
    struct GFAST_pgdResults_struct pgd;
    struct GFAST_props_struct props;
    struct GFAST_shakeAlert_struct SA;
    struct GFAST_xmlMessages_struct xmlMessages;
    struct ewRing_struct ringInfo;
    char *msgs;
    double t0, t1;
    const enum opmode_type opmode = REAL_TIME_EEW;
    /*activeMQ variables*/
    char *amqMessage;
    // const bool useTopic = true;              /**< ShakeAlert uses topics */
    // const bool clientAck = false;            /**< False means set session acknowledge transacations */
    // const bool luseListener = false;         /**< C can't trigger so turn this off (remove?) */
    double tstatus, tstatus0, tstatus1;
    // static void *amqMessageListener = NULL;  /**< pointer to ShakeAlertConsumer object */
    int ierr, im, msWait, nTracebufs2Read;
    bool lacquire, lnewEvent, in_loop;
    const int rdwt = 2; // H5 file is read only (1) or read/write (2)
#ifndef ENABLE_PLOG
    char errorLogFileName[PATH_MAX];
    char infoLogFileName[PATH_MAX];
    char debugLogFileName[PATH_MAX];
    char warnLogFileName[PATH_MAX];
#endif
    bool check_message_dir = false;
    bool USE_AMQ = false;
    bool USE_DMLIB = false;
    int niter = 0;
#ifdef GFAST_USE_AMQ
    USE_AMQ = true;
// GFAST_USE_DMLIB defined inside GFAST_USE_AMQ since dmlib won't work without amq
#ifdef GFAST_USE_DMLIB
    USE_DMLIB = true;
#endif
#endif

#ifdef ENABLE_PLOG
    // logging stuff
    init_plog();
#endif

    printProgramInfo(fcnm);

    // Initialize. Only works if propfile is specified
    if (argc > 1) {
        strncpy(propfilename, argv[1], PATH_MAX - 1);
    } else {
#ifdef ENABLE_PLOG
        LOG_MSG("%s", "Usage: gfast_eew propfilename");
#else
        printf("%s\n", "Usage: gfast_eew propfilename");
#endif
        return EXIT_SUCCESS;
    }

    // Get program instance for ShakeAlert-approved xml header
    char longprog[PATH_MAX], program_instance[PATH_MAX];

    // Get program name
    strncpy(longprog, argv[0], PATH_MAX - 1);
    char *pch = strrchr(longprog, '/');
    size_t lenshort = &longprog[strlen(longprog)] - pch;
    if (pch != NULL) {
        memcpy(program_instance, pch + 1, lenshort - 1);
        program_instance[lenshort] = '\0';
    } else {
        strncpy(program_instance, argv[0], PATH_MAX - 1);
    }

    // Add host name to program_instance
    char name[1000];
    if(gethostname(name, (int)sizeof(name) - 1) == 0) {
        // if eew-bk-dev1.geo.berkeley.edu, shorten to eew-bk-dev1
        char *c = strstr(name, ".");
        if(c != NULL) *c = '\0';
        strcat(program_instance, "@\0");
        strcat(program_instance, name);
    }

    ierr = 0;
    msgs = NULL;
    memset(&props,    0, sizeof(struct GFAST_props_struct));
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));
    memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
    memset(&xml_status, 0, sizeof(struct GFAST_activeEvents_xml_status));
    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
    memset(&pgd_data, 0, sizeof( struct GFAST_peakDisplacementData_struct));
    memset(&cmt_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ff_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ringInfo, 0, sizeof(struct ewRing_struct)); 
    memset(&xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
    memset(&h5traceBuffer, 0, sizeof(struct h5traceBuffer_struct));
    memset(&gnsstraceData, 0, sizeof(struct gnsstraceData_struct));

    // Read the program properties
#ifdef ENABLE_PLOG
    LOG_MSG("%s: Reading configuration from %s", fcnm, propfilename);
#else
    printf("%s: Reading configuration from %s\n", fcnm, propfilename);
#endif
    ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);
    if (ierr != 0) {
        LOG_ERRMSG("%s: Error reading GFAST initialization file: %s\n", fcnm, propfilename);
        goto ERROR;
    }

    if (props.verbose > 2){GFAST_core_properties_print(props);}
    // Initialize the stations locations/names/sampling periods for the module
    if (props.verbose > 0) {
        LOG_MSG("%s: Initializing the data buffers...", fcnm);
    }

    ierr = core_data_initialize(props, &gps_data);
    if (ierr != 0) {
        LOG_ERRMSG("%s: Error initializing data buffers\n", fcnm);
        goto ERROR;
    }

    // Set the trace buffer names and open the HDF5 datafile
    ierr = GFAST_traceBuffer_h5_setTraceBufferFromGFAST(
        props.bufflen, gps_data, &h5traceBuffer);
    if (ierr != 0) {   
        LOG_ERRMSG("%s: Error setting the H5 tracebuffer\n", fcnm);
        goto ERROR;
    }
    // Initialize the tracebuffer h5 archive
    ierr = traceBuffer_h5_initialize(rdwt, true, "./\0", "work.h5\0", 
                    &h5traceBuffer);
    if (ierr != 0) {
        LOG_ERRMSG("%s: Error initializing the HDF5 wave file\n", fcnm);
        goto ERROR;
    }

    if (USE_AMQ) {
        activeMQ_start();  // start library
        /* dmlib startup */
        if (USE_DMLIB) {
            /* 
            Start connection, to be used by DMMessageSender.
            This should be written using just dmlib - CWU
            */
            ierr = startDestinationConnection(
                props.activeMQ_props.user,
                props.activeMQ_props.password,
                props.activeMQ_props.destinationURL,
                props.activeMQ_props.msReconnect,
                props.activeMQ_props.maxAttempts,
                props.verbose);
            if (ierr == 0) {
                LOG_ERRMSG("%s: Attempted to re-initialize activeMQ connection object", fcnm);
            }
            if (ierr < 0) {
                LOG_ERRMSG("%s: Error initializing activeMQ connection object", fcnm);
                goto ERROR;
            }
            /*start message receiver*/
            if (props.verbose > 0) {
                LOG_MSG("%s: Initializing event receiver on %s...", 
                    fcnm, props.activeMQ_props.originTopic);
            }
            ierr = startEventReceiver(
                props.activeMQ_props.originURL,
                props.activeMQ_props.user,
                props.activeMQ_props.password,
                props.activeMQ_props.originTopic,
                props.activeMQ_props.msReconnect,
                props.activeMQ_props.maxAttempts);
            if (ierr == 0) {
                LOG_ERRMSG("%s: Attempted to re-initialize active event receiver object", fcnm);
            }
            if (ierr < 0) {
                LOG_ERRMSG("%s: Error initializing event receiver object", fcnm);
                goto ERROR;
            }

            /* start heartbeat producer and set to manual heartbeats */
            if ((props.activeMQ_props.hbTopic != NULL) &&
                (strlen(props.activeMQ_props.hbTopic) > 0)) 
            {
                char senderstr[100], *pp;
                int ii;
                strcpy(senderstr, "gfast.");
                ii = strlen(senderstr);
                gethostname(senderstr + ii, 90);   /*append hostname*/
                pp = strchr(senderstr + ii, '.');  /*find . in hostname if any*/
                if (pp != NULL) *pp = '\0';        /*truncate long hostname*/
                if (props.verbose > 0) {
                    LOG_MSG("%s: Initializing heartbeat sender %s on %s...", fcnm,
                        senderstr, props.activeMQ_props.hbTopic);
                }
                ierr = startHBProducer(
                    senderstr,
                    props.activeMQ_props.hbTopic,
                    props.activeMQ_props.hbInterval,
                    props.verbose);
                if (ierr == 0) {
                    LOG_ERRMSG("%s: Attempted to re-initialize active HB producer object", fcnm);
                }
                if (ierr < 0) {
                    LOG_ERRMSG("%s: Error initializing HB producer object", fcnm);
                    goto ERROR;
                }
            }
            /*start message encoder*/
            if (props.verbose > 0) {
                LOG_MSG("%s: Initializing event xml encoder...", fcnm);
            }
            ierr = startEventEncoder();
            if (ierr < 0) {
                LOG_ERRMSG("%s: Error initializing event encoder object", fcnm);
                goto ERROR;
            }

            /*start message sender*/
            if (props.verbose > 0) {
                LOG_MSG("%s: Initializing event sender on %s, time to live %fs ...", 
                    fcnm, props.activeMQ_props.destinationTopic, props.activeMQ_props.msg_ttl);
            }
            ierr = startEventSender(
                props.activeMQ_props.destinationTopic,
                props.activeMQ_props.msg_ttl);
            if (ierr == 0) {
                LOG_ERRMSG("%s: Attempted to re-initialize active event sender object", fcnm);
            }
            if (ierr < 0) {
                LOG_ERRMSG("%s: Error initializing event sender object", fcnm);
                goto ERROR;
            }
        } /* end of if USE_DMLIB */
    } /* end if USE_AMQ */

    if (strlen(props.SAeventsDir)) {
        message_dir = props.SAeventsDir;
        check_message_dir = true;
    }

    // Initialize PGD
    ierr = core_scaling_pgd_initialize(props.pgd_props, gps_data, &pgd, &pgd_data);
    if (ierr != 0) {   
        LOG_ERRMSG("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = core_cmt_initialize(props.cmt_props, gps_data, &cmt, &cmt_data);
    if (ierr != 0) {   
        LOG_ERRMSG("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = core_ff_initialize(props.ff_props, gps_data, &ff, &ff_data);
    if (ierr != 0) {   
        LOG_ERRMSG("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }
    // Set up the SNCL's to target
    ierr = traceBuffer_gnsstrace_setGnsstraceDataFromGFAST(&gps_data, &gnsstraceData);
    if (ierr != 0) {
        LOG_ERRMSG("%s: Error setting gnsstraceData\n", fcnm);
        goto ERROR;
    }
    // Connect to the earthworm ring
    LOG_MSG("%s: Connecting to earthworm ring %s", fcnm, ringInfo.ewRingName);
    ierr = dataexchange_earthworm_initialize(props.ew_props.gpsRingName,
                                       10,
                                       &ringInfo);
    if (ierr != 0) {
        LOG_ERRMSG("%s: Error initializing tracebuf reader\n", fcnm);
        goto ERROR;
    }
    // Flush the buffer
    LOG_MSG("%s: Flushing ring %s", fcnm, ringInfo.ewRingName);
    ierr = dataexchange_earthworm_flushRing(&ringInfo);
    if (ierr != 0) {
        LOG_ERRMSG("%s: Error flusing the ring\n", fcnm);
        goto ERROR;
    }
    // Begin the acquisition loop
    LOG_MSG("%s: Beginning the acquisition...", fcnm);
    amqMessage = NULL;
    t0 = time_timeStamp();
    //unused: t_now = (double) (long) (time_timeStamp() + 0.5);
    //unused: tbeg = t0; 
    tstatus = t0;
    tstatus0 = t0;
    lacquire = true;
    in_loop = false;

    /***************************************************
     * Start of main acquisition loop
     ***************************************************/
    while(lacquire) {
        // Initialize the iteration
        amqMessage = NULL;
        // Don't start loop until prop.waitTime has elapsed (default 1 second)
        t1 = time_timeStamp();
        double tloop = t1 - t0;
        
        // Print time for the previous iteration. Only print once until
        // props.waitTime has been reached and iteration actually starts again.
        if (in_loop) {
            LOG_MSG("== [GFAST t1:%f] Main loop [Timing: %.4fs]\n", t1, tloop);
            in_loop = false;
        }

        if (tloop < props.waitTime) {
            usleep((props.waitTime - tloop) * 1000 * 1000);
            // Put in a continue here since t1 is reset at the top of the loop
            // Want to make sure it is representing the actual start
            continue;
        }
        else if ((props.waitTime > 0.0) && (tloop >= 2 * props.waitTime)) {
            LOG_MSG("== [GFAST t :%f] Main loop [Timing: %.4fs] >= 2x%f s waitTimes. not keeping up",
                time_timeStamp(), tloop, props.waitTime);
        }

        t0 = t1;
        tstatus1 = t0;
        in_loop = true;
        LOG_MSG("== [GFAST t0:%f] Beginning main loop", t0);

        if (tstatus1 - tstatus0 > 3600.0) {
            LOG_MSG("%s: GFAST has been running for %d hours, start time %f",
                fcnm, (int) ((tstatus1 - tstatus)/3600.0), tstatus);
            printProgramInfo(fcnm);
            
            tstatus0 = tstatus1;
        } 

        LOG_MSG("== [GFAST t :%f] Get the msgs off the EW ring", time_timeStamp());
        double tbeger = time_timeStamp();
        memory_free8c(&msgs);
        msgs = dataexchange_earthworm_getMessagesFromRing(MAX_MESSAGES,
                                                   false,
                                                   &ringInfo,
                                                   gnsstraceData.hashmap,
                                                   &nTracebufs2Read,
                                                   &ierr);
        LOG_MSG("== [GFAST t :%f] getMessages returned nTracebufs2Read:%d",
            time_timeStamp(), nTracebufs2Read);

        if (ierr < 0 || (msgs == NULL && nTracebufs2Read > 0)) {
            if (ierr ==-1) {
                LOG_ERRMSG("%s: Terminate message received from ring\n", fcnm);
                ierr = 1;
            } else if (ierr ==-2) {
                LOG_ERRMSG("%s: Read error encountered on ring\n", fcnm);
                ierr = 1;
            } else if (ierr ==-3) {
                LOG_ERRMSG("%s: Ring info structure never initialized\n", fcnm);
                ierr = 1;
            } else if (msgs == NULL) {
                LOG_ERRMSG("%s: Message allocation error\n", fcnm);
                ierr = 1;
            }
            goto ERROR;
        }
        LOG_MSG("scrounge [Timing: %.4fs]", time_timeStamp() - tbeger);
        tbeger = time_timeStamp();
        
        // Unpackage the tracebuf2 messages
        LOG_MSG("%s", "== Calling unpackTraceBuf2Messages");
        ierr = traceBuffer_gnsstrace_unpackTraceBuf2Messages(nTracebufs2Read,
            msgs, &gnsstraceData);
        memory_free8c(&msgs);
        LOG_MSG("== Ending unpackTraceBuf2Messages: [Timing: %.4fs]",
            time_timeStamp() - tbeger);
        if (ierr != 0) {
            LOG_ERRMSG("%s: Error unpacking tracebuf2 messages\n", fcnm);
            goto ERROR;
        }
        
        // Update the hdf5 buffers
        ierr = traceBuffer_h5_setData(t1,
                                      gnsstraceData,
                                      h5traceBuffer);
        if (ierr != 0) {
            LOG_ERRMSG("%s: Error setting data in H5 file\n", fcnm);
            goto ERROR;
        }
        
        // Check for an event
        if (USE_AMQ) {
            if (props.verbose > 2) {
                LOG_MSG("%s: Checking Activemq for events", fcnm);
            }
            msWait = props.activeMQ_props.msWaitForMessage;
            // amqMessage = GFAST_activeMQ_consumer_getMessage(amqMessageListener, msWait, &ierr);
            amqMessage = eventReceiverGetMessage(msWait, &ierr);
            if ((props.verbose > 2) && (amqMessage == NULL)) {
                LOG_MSG("%s: Activemq returned NULL", fcnm);
            }
        } 
        if (amqMessage == NULL && check_message_dir) {
            // Alternatively, check for SA message trigger in message_dir
            amqMessage = check_dir_for_messages(message_dir, &ierr);
            if ((ierr != 0) && (props.verbose > 2)) {
                LOG_MSG("check_dir_for_messages returned ierr=%d\n", ierr);
                ierr=0;
            }
        }

        if (ierr != 0) {
            LOG_ERRMSG("%s: Internal error when getting message\n", fcnm);
            goto ERROR;
        }
        // If there's a message then process it
        if (amqMessage != NULL) {
            LOG_MSG("== [GFAST t0:%f] Got new amqMessage:", t0);
            LOG_MSG("%s", amqMessage);
            // Parse the event message 
            ierr = GFAST_eewUtils_parseCoreXML(amqMessage, -12345.0, &SA);
            if (ierr != 0) {
                LOG_ERRMSG("%s: Error parsing the activeMQ trigger message\n",
                    fcnm);
                LOG_ERRMSG("%s\n", amqMessage);
                goto ERROR;
            }
            if (t1 - SA.time > props.processingTime) {
                // If a message comes in more than processingTime s late, the hd5 file
                // would already be closed and would lead to a crash when trying to reopen.
                LOG_MSG("Ignoring message from SA, v%d, current time %f - origin time %f > process time %f",
                    SA.version, t1, SA.time, props.processingTime);
            } else if (strncasecmp(SA.orig_sys, "gfast", 5) == 0) {
                LOG_MSG("ignoring activeMQ message from gfast: evid=%s orig_sys=%s",
                    SA.eventid, SA.orig_sys);
            } else  { // don't trigger on gfast messages
                // If this is a new event we have some file handling to do
                LOG_MSG("MTH: call newEvent events.nev=%d xml_status.nev=%d\n",
                    events.nev, xml_status.nev);
                lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);
                LOG_MSG("%s", "MTH: call newEvent DONE");
                if (lnewEvent){
                    LOG_MSG("NEW event evid:%s lat:%7.3f lon:%8.3f dep:%6.2f mag:%.2f time:%.2f age_now:%.0f",
                        SA.eventid, SA.lat, SA.lon, SA.dep, SA.mag, SA.time, t0 - SA.time);
                } else {
                    LOG_MSG("This is NOT a new event: evid=%s", SA.eventid);
                }
                if (lnewEvent) {
                    // And the logs
                    if (props.verbose > 0) {
                        LOG_INFOMSG("%s: New event %s added", fcnm, SA.eventid);
                        if (props.verbose > 2){GFAST_core_events_printEvents(SA);}
                    }
#ifndef ENABLE_PLOG
                    // Set the log file names
                    eewUtils_setLogFileNames(SA.eventid,props.SAoutputDir,
                                             errorLogFileName, infoLogFileName,
                                             debugLogFileName, warnLogFileName);
                    if (os_path_isfile(errorLogFileName)) {
                        remove(errorLogFileName);
                    }
                    if (os_path_isfile(infoLogFileName)) {
                        remove(infoLogFileName);
                    }
                    if (os_path_isfile(debugLogFileName)) {
                        remove(debugLogFileName);
                    }
                    if (os_path_isfile(warnLogFileName)) {
                        remove(warnLogFileName);
                    }
#endif
                    // Initialize the HDF5 file
                    ierr = GFAST_hdf5_initialize(props.h5ArchiveDir,
                                                 SA.eventid,
                                                 props.propfilename);
                    if (ierr != 0) {
                        LOG_ERRMSG("%s: Error initializing the archive file", fcnm);
                        goto ERROR;
                    }
                }
            }  //end gfast message if/else check
            free(amqMessage);
            amqMessage = NULL;
        } // End check on ActiveMQ message

        // Are there events to process?
        if (events.nev < 1){continue;} 
        if (props.verbose > 2) {
            LOG_DEBUGMSG("%s: Processing events...", fcnm);
        }
        LOG_MSG("== [GFAST t0:%f] Call driveGFAST:", t0);
        ierr = eewUtils_driveGFAST(t1,
                                   program_instance,
                                   props,
                                   &events,
                                   &gps_data,
                                   &h5traceBuffer,
                                   &pgd_data,
                                   &cmt_data,
                                   &ff_data,
                                   &pgd,
                                   &cmt,
                                   &ff,
                                   &xmlMessages,
                                   &xml_status);
        if (ierr != 0) {
            LOG_ERRMSG("%s: Error calling GFAST driver!\n", fcnm);
            goto ERROR; 
        }

        // This whole xmlMessages struct isn't currently necessary since messages are handled and
        // sent in driveGFAST, but keeping for now in case we want to change back - CWU
        // Send the messages where they need to go
        if (xmlMessages.mmessages > 0) {
            for (im=0; im<xmlMessages.nmessages; im++) {
                // if ((USE_DMLIB) && (xmlMessages.pgdXML[im] != NULL)) {
                //     sendEventXML(xmlMessages.pgdXML[im]);
                // }
                // LOG_MSG("== [GFAST t0:%f] evid:%s pgdXML=[%s]\n",
                //     t0, xmlMessages.evids[im], xmlMessages.pgdXML[im]);
                // printf("GFAST: evid:%s cmtQML=[%s]\n", xmlMessages.evids[im], xmlMessages.cmtQML[im]);
                // printf("GFAST: evid:%s  ffXML=[%s]\n", xmlMessages.evids[im], xmlMessages.ffXML[im]);
                if (xmlMessages.evids[im] != NULL) {
                    free(xmlMessages.evids[im]);
                }
                if (xmlMessages.cmtQML[im] != NULL) {
                    free(xmlMessages.cmtQML[im]);
                }
                if (xmlMessages.pgdXML[im] != NULL) {
                    free(xmlMessages.pgdXML[im]);
                }
                if (xmlMessages.ffXML[im] != NULL) {
                    free(xmlMessages.ffXML[im]);
                }
            }
            if (xmlMessages.evids  != NULL){ free(xmlMessages.evids);}
            if (xmlMessages.cmtQML != NULL){ free(xmlMessages.cmtQML);}
            if (xmlMessages.ffXML  != NULL){ free(xmlMessages.ffXML);}
            if (xmlMessages.pgdXML != NULL){ free(xmlMessages.pgdXML);}

            memset(&xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
        }
        niter ++;
    }
    /***************************************************
     * End of main acquisition loop
     ***************************************************/
    ERROR:;
    memory_free8c(&msgs);
    core_events_freeEvents(&events);
    traceBuffer_gnsstrace_freeGnsstraceData(&gnsstraceData);
    dataexchange_earthworm_finalize(&ringInfo);
    if (USE_AMQ) {
        // activeMQ_consumer_finalize(amqMessageListener);
        if (USE_DMLIB) {
            stopEventReceiver();
            stopHBProducer();
            stopEventSender();
            stopEventEncoder();
            //stopDestinationConnection();
        }
        activeMQ_stop();
    }
    core_cmt_finalize(&props.cmt_props,
                      &cmt_data,
                      &cmt);
    core_ff_finalize(&props.ff_props,
                     &ff_data,
                     &ff);
    core_scaling_pgd_finalize(&props.pgd_props,
                              &pgd_data,
                              &pgd);
    GFAST_core_data_finalize(&gps_data);
    GFAST_core_properties_finalize(&props);
    traceBuffer_h5_finalize(&h5traceBuffer);
#ifndef ENABLE_PLOG
    // Close the big logfile
    core_log_closeLog();
#endif
    if (ierr != 0) {
        printf("%s: Terminating with error\n", fcnm);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*
 * Function to check for new events by looking in a specified directory.
 * This is only used if the props.SAeventsDir parameter is defined.
 * If ActiveMQ is used (i.e. for ShakeAlert), this function wouldn't typically
 * be used.
 */
char *check_dir_for_messages(const char *dirname, int *ierr)
{
    struct dirent *de; // Pointer for directory entry
    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(dirname);
    int errsv = errno;
    char *ext;
    char *fullpath;
    char *buffer;
    long length = 0;
    FILE *f;
    char *message = NULL;
    int nevents=0;
    size_t dirlen = strlen(dirname);

    *ierr = 1;

    if (dr == NULL) // opendir returns NULL if couldn't open directory
    {
        LOG_WARNMSG("Could not open directory:%s, errno:%d\n", dirname, errsv);
        return NULL;
    }

    // MTH: This is currently set up to expect only 1 .xml file in dir at a time,
    //      but could easily be modified to handle more
    while ((de = readdir(dr)) != NULL) {
        if (de->d_name[0] != '.')  {
            ext = strrchr(de->d_name, '.');
            //printf("%s ext=[%s]\n", de->d_name, ext);
            if (ext && !strcmp(ext+1, "xml")) {
                //printf("%s ext=[%s] len=%ld\n", de->d_name, ext, strlen(de->d_name));

                /* + 2 because of the '/' and the terminating 0 */
                fullpath = malloc(dirlen + strlen(de->d_name) + 2);
                sprintf(fullpath, "%s/%s", dirname, de->d_name);

                f = fopen(fullpath, "r");
                if (f) {
                    fseek(f, 0, SEEK_END);
                    length = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    buffer = (char *)malloc((length+1)*sizeof(char));
                    if (buffer) {
                        fread(buffer,sizeof(char), length, f);
                    }
                    fclose(f);
                    buffer[length] = '\0';
                    message = buffer;
                } else {
                    LOG_ERRMSG("Error reading from file:%s\n", fullpath);
                    //return NULL;
                    goto ERROR;
                }

                // Now remove the event file we just read:
                *ierr = remove(fullpath);
                if (*ierr != 0) {
                    LOG_ERRMSG("gfast_eew: Unable to delete file:%s\n", fullpath);
                }

                free(fullpath);
                nevents++;
            }
        }
    }
    closedir(dr);
    //printf("Return nevents=%d\n" , nevents);
    *ierr = 0;
    return message;

    ERROR:;
    closedir(dr);
    return NULL;
}

