#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_xml.h"
#include "gfast_core.h"

/*!
 * @brief Writes an origin to the xml_writer
 *
 * @param[in] publicIDroot    QuakeML public ID root
 *                            (e.g. quakeml:us.anss.org/).
 * @param[in] evid            Event ID.
 * @param[in] method          Method by which the origin was computed.
 * @param[in] origin          Contains the event origin information.
 *
 * @param[in,out] xml_writer  On input this is a pointer to the
 *                            xmlTextWriterPtr. 
 *                            On successful output the origin has been
 *                            appended to the xml_writer as a QuakeML origin.
 * @result 0 indicates success.
 *
 * @author Ben Baker, ISTI
 *
 */
int xml_quakeML_writeOrigin(const char *publicIDroot,
                            const char *evid,
                            const char *method,
                            struct qmlOrigin_struct origin,
                            void *xml_writer)
{
    xmlTextWriterPtr writer;
    char publicID[512];
    int ierr, rc;
    size_t lenos;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (!origin.lhaveEllipse &&
        !origin.lhaveOriUnc &&
        !origin.lhaveLatitude &&
        !origin.lhaveLongitude &&
        !origin.lhaveDepth &&
        !origin.lhaveOriginTime)
    {
        return 0;
    }
    // Initialize
    rc = 0;
    ierr = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // Set the publicID
    memset(publicID, 0, 512*sizeof(char));
    if (publicIDroot != NULL){strcat(publicID, publicIDroot);}
    lenos = strlen(publicID);
    if (lenos > 0)
    {   
        if (publicID[lenos - 1] != '/'){strcat(publicID, "/\0");}
    }
    strcat(publicID, "origin/\0");
    if (evid != NULL)
    {   
        strcat(publicID, evid);
        strcat(publicID, "/\0");
    }
    if (method != NULL){strcat(publicID, method);}
    // <origin>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "origin\0");
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "publicID\0",
                                      BAD_CAST publicID);
    // <ellipse>
    if (origin.lhaveEllipse)
    {
        LOG_ERRMSG("%s", "Error ellipse not yet done");
    }
    // <originUncertainty>
    if (origin.lhaveOriUnc)
    {
        LOG_ERRMSG("%s", "Error origin uncertainty not yet done");
    }
    // <time>
    if (origin.lhaveOriginTime)
    {
        ierr = xml_quakeML_writeTime(origin.originTime.time,
                                     origin.originTime.time_units,
                                     origin.originTime.lhaveTime,
                                     origin.originTime.timeUncer,
                                     origin.originTime.timeUncer_units,
                                     origin.originTime.lhaveTimeUncer,
                                     origin.originTime.confidenceLevel,
                                     origin.originTime.lhaveConfidence,
                                     (void *) writer);
        if (ierr != 0)
        {   
            LOG_ERRMSG("%s", "Error writing time!");
            return -1; 
        }
    }
    // <latitude>
    if (origin.lhaveLatitude)
    {
        ierr = xml_quakeML_writeLatitude(origin.latitude.latitude,
                                         origin.latitude.latitude_units,
                                         origin.latitude.lhaveLat,
                                         origin.latitude.latUncer,
                                         origin.latitude.latUncer_units,
                                         origin.latitude.lhaveLatUncer,
                                         origin.latitude.confidenceLevel,
                                         origin.latitude.lhaveConfidence,
                                         (void *) writer);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error writing longitude!");
            return -1; 
        }
    }
    // <longitude>
    if (origin.lhaveLongitude)
    {
        ierr = xml_quakeML_writeLongitude(origin.longitude.longitude,
                                          origin.longitude.longitude_units,
                                          origin.longitude.lhaveLon,
                                          origin.longitude.lonUncer,
                                          origin.longitude.lonUncer_units,
                                          origin.longitude.lhaveLonUncer,
                                          origin.longitude.confidenceLevel,
                                          origin.longitude.lhaveConfidence,
                                          (void *) writer);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error writing longitude!");
            return -1;
        }
    }
    // <depth>
    if (origin.lhaveDepth)
    {   
        ierr = xml_quakeML_writeDepth(origin.depth.depth,
                                      origin.depth.depth_units,
                                      origin.depth.lhaveDepth,
                                      origin.depth.depthUncer,
                                      origin.depth.depthUncer_units,
                                      origin.depth.lhaveDepthUncer,
                                      origin.depth.confidenceLevel,
                                      origin.depth.lhaveConfidence,
                                      (void *) writer);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error writing longitude!");
            return -1; 
        }
    }
    // </origin>
    rc += xmlTextWriterEndElement(writer); // </origin>
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s", "Error writing focalMechanism!");
        return -1; 
    }
    return 0;
}
