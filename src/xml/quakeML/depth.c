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

int xml_quakeML_writeDepth(const double depth,
                           const enum alert_units_enum depth_units,
                           const bool lhaveDepth,
                           const double depthUncer,
                           const enum alert_units_enum depthUncer_units,
                           const bool lhaveDepthUncer,
                           const double confidence,
                           const bool lhaveConfidence,
                           void *xml_writer)
{
    xmlTextWriterPtr writer;
    char units[128];
    int rc; 
    //------------------------------------------------------------------------//
    //
    // nothing to do
    if (!lhaveDepth && !lhaveDepthUncer && !lhaveConfidence){return 0;}
    // get writer
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // Begin <depth>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "depth\0");
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error starting element");
        return -1;
    }
    if (lhaveDepth)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                             "%f", depth);
        __xml_units__enum2string(depth_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveDepthUncer)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "uncertainty\0",
                                             "%f", depthUncer);
        __xml_units__enum2string(depthUncer_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveConfidence)
    {
        rc += xmlTextWriterWriteFormatElement(writer,
                                              BAD_CAST "confidenceLevel\0",
                                             "%f", confidence);
    }
    // </depth>
    rc += xmlTextWriterEndElement(writer); // </depth>
    if (rc < 0)
    {   
        LOG_ERRMSG("%s", "Error writing depth");
        return -1; 
    }
    return 0;
}
