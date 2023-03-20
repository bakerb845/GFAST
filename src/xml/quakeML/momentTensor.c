#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_xml.h"
#include "gfast_core.h"
/*!
 * @brief Writes the moment tensor, scalar moment, double couple percentage,
 *        and CLVD percentage.
 *
 * @param[in] publicIDroot    QuakeML public ID root
 *                            (e.g. quakeml:us.anss.org/)
 * @param[in] evid            Event ID.
 * @param[in] method          Method by which the moment tensor was computed
 *                            (for this project use gps).
 * @param[in] M_use           The moment tensor terms (Nm) in USE coordinates
 *                            packed:
 *                            \f$ \{ m_{rr},
 *                                    m_{\theta \theta},
 *                                    m_{\phi \phi},
 *                                    m_{r \theta}, m_{r \phi},
 *                                    m_{\theta \phi} \} \f$.
 * @param[in] M0              Scalar moment (Nm).
 * @param[in] dc_pct          Percent double couple [0,100].
 * @param[in] clvd_pct        Percent CLVD [0,100].
 *
 * @param[in,out] xml_writer  On input this is a pointer to the
 *                            xmlTextWriterPtr. 
 *                            On successful output the moment tensor has been
 *                            appended to the xml_writer as a QuakeML moment
 *                            tensor.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int xml_quakeML_writeMomentTensor(const char *publicIDroot,
                                  const char *evid,
                                  const char *method,
                                  const double M_use[6],
                                  const double M0,
                                  const double dc_pct,
                                  const double clvd_pct,
                                  void *xml_writer)
{
    xmlTextWriterPtr writer;
    char publicID[512];
    double Mrr, Mtt, Mpp, Mrt, Mrp, Mtp;
    int ierr, rc;
    size_t lenos;
    //------------------------------------------------------------------------//
    //
    rc = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // Set the publicID
    memset(publicID, 0, 512*sizeof(char));
    if (publicIDroot != NULL){strcat(publicID, publicIDroot);} 
    lenos = strlen(publicID);
    if (lenos > 0)
    {
        if (publicID[lenos - 1] != '/'){strcat(publicID, "/\0");}
    } 
    strcat(publicID, "momenttensor/\0");
    if (evid != NULL){
        strcat(publicID, evid);
        strcat(publicID, "/\0");
    }
    if (method != NULL){strcat(publicID, method);}
    // <momentTensor>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "momentTensor\0");
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "publicID\0",
                                      BAD_CAST publicID);
    // <scalarMoment>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "scalarMoment\0"); 
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", M0);
    rc += xmlTextWriterEndElement(writer); // </scalarMoment>
    // Write the tensor
    Mrr = M_use[0];
    Mtt = M_use[1];
    Mpp = M_use[2];
    Mrt = M_use[3];
    Mrp = M_use[4];
    Mtp = M_use[5];
    ierr = xml_quakeML_writeTensor(Mrr, Mtt, Mpp,
                                   Mrt, Mrp, Mtp,
                                   (void *) writer);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing tensor");
        return -1;
    }
    // <doubleCouple>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "doubleCouple\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", dc_pct/100.0);
    rc += xmlTextWriterEndElement(writer); // </doubleCouple>
    // <clvd>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "clvd\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", clvd_pct/100.0);
    rc += xmlTextWriterEndElement(writer); // </clvd>
    // </momentTensor>
    rc += xmlTextWriterEndElement(writer); // </momentTensor>
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing momentTensor");
        return -1;
    }
    return 0;
}
