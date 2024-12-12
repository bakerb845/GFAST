
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "iscl/memory/memory.h"
#include <jansson.h>
#include <string.h>

struct geojson_struct {
    long time;      /*!< Unix time in ms */
    int quality;    /*!< Quality channel (last digit is use flag) */
    char type[8];     /*!< Should be ENU */
    char sncl[15];     /*!< SNCL */
    double coor[3]; /*!< Positions (m) */
    double err[3];  /*!< Positional error (m) */
    double rate;    /*!< Sampling rate (s) */
};

// struct string_index {
//   char nscl[15];
//   char net[8];
//   char sta[8];
//   char cha[8];
//   char loc[8];

//   double time;
//   int indx;
//   int nsamps;
// };
// void print_struct(struct string_index *d, int n);
// void sort2(struct string_index *vals, int n);
int splitSNCL(const char *cline, char stnm[64], char netw[64], char chan[64], char loc[64]);

struct geojson_struct *unpackMessage(const char *msg, const int msg_size);
void print_json_msg(struct geojson_struct *json_msg);

int traceBuffer_generictrace_unpackGeojsonMessages(
    const int nRead,
    const char *msgs,
    const int max_payload_size,
    struct h5traceBuffer_struct *h5traceBuffer,
    struct generictraceData_struct *generictraceData)
{
    int i, j, k, indx, ierr, nskip, debug, kold, ir, nReadPtr, i1, i2, kndx, im, npts, nchunks, ic, kchan, value;
    int *imap, *imsg, *kpts, *nmsg, *imapPtr;
    double dt;
    char netw[64], stnm[64], chan[64], loc[64], buf[256];
    char *nscl, *tmpchan;
    struct geojson_struct *json_msg;
    struct geojson_struct *json_msgs;
    struct string_index *vals, *tmp;
    struct generictrace_node *node;
    const bool clearSNCL = false;
    const int nchan = 7; // 1, 3, 6, 7

    // bool dump_generictraceData = false;
    bool dump_nRead = false;
    // bool debug_imap = false;
    bool debug_nchunks = false;
    bool debug_cwu = false;

    // Nothing to do
    if (generictraceData->ntraces == 0) {return 0;}
    if (nRead == 0){return 0;}

    /** Variable descriptions
     * Length nRead variables (use i)
     * imsg      (nRead) sorted order into msgs. imsg[i] is the i'th message in sorted order (imsg[i] = vals[i].indx)
     * json_msgs (nRead) input messages
     * 
     * Length nRead * nchan (+ 1) where nchan is channels per packet = 7 for geojson (use i)
     * vals      (nRead) header values of input packets, unsorted (input order)
     * tmp       (nRead) header values of input packets, sorted
     * imap      (nRead + 1, or nRead * 7 + 1) kth generictraceData scnl msg target (imap[i] = kth generictraceData trace, only for traces that have data, otherwise -9)
     * imapPtr   (nRead + 1, or nRead * 7 + 1) pointer back to imap somehow? each new k starts = imap[imapPtr[ir]]?
     * 
     * Length generictraceData->ntraces variables (use k)
     * kpts      (ntraces) kpts[k] = number of samples for given trace. Since data pts per message = 1, this should equal number of messages
     * nmsg      (ntraces) nmsg[k] = number of messages for given trace. Since data pts per message = 1, this should equal number of data points
     * 
     */

    debug = 0;
    imap  = memory_calloc32i(nRead * nchan + 1);
    imsg  = memory_calloc32i(nRead * nchan);
    kpts  = memory_calloc32i(generictraceData->ntraces);
    nmsg  = memory_calloc32i(generictraceData->ntraces);
    imapPtr = memory_calloc32i(nRead * nchan + 1); // worst case size
    vals = (struct string_index *) calloc((size_t) nRead * nchan, sizeof(struct string_index));
    tmp = (struct string_index *) calloc((size_t) nRead * nchan, sizeof(struct string_index));
    json_msgs = (struct geojson_struct *) malloc((size_t) (nRead * sizeof(struct geojson_struct)));
    nscl  = memory_calloc8c(15);
    tmpchan  = memory_calloc8c(4);

    // First parse all of the geojson messages, also add info to string_index
    for (i = 0; i < nRead; i++) {

        // struct geojson_struct *json_msg;
        indx = i * max_payload_size * sizeof(char);

        json_msg = NULL;
        json_msg = unpackMessage(&msgs[indx], max_payload_size);

        // copy json_msg data to json_msgs. Is there a more efficient way?
        json_msgs[i].time = json_msg->time;
        json_msgs[i].quality = json_msg->quality;
        strcpy(json_msgs[i].type, json_msg->type);
        strcpy(json_msgs[i].sncl, json_msg->sncl);
        json_msgs[i].rate = json_msg->rate;
        for (j = 0; j < 3; j++) {
            json_msgs[i].coor[j] = json_msg->coor[j];
            json_msgs[i].err[j] = json_msg->err[j];
        }
    
        ierr = splitSNCL(json_msg->sncl, stnm, netw, chan, loc);
        if (ierr != 0) {
            LOG_ERRMSG("%s", "Error unpacking data");
        }

        for (ic = 0; ic < nchan; ic++) {
            // add header info for later sorting
            strncpy(tmpchan, chan, 2);
            tmpchan[2] = '\0';
            if (ic == 0) {strcat(tmpchan, "Z\0");}
            if (ic == 1) {strcat(tmpchan, "N\0");}
            if (ic == 2) {strcat(tmpchan, "E\0");}
            if (ic == 3) {strcat(tmpchan, "3\0");}
            if (ic == 4) {strcat(tmpchan, "2\0");}
            if (ic == 5) {strcat(tmpchan, "1\0");}
            if (ic == 6) {strcat(tmpchan, "Q\0");}
            sprintf(nscl, "%s.%s.%s.%s", netw, stnm, tmpchan, loc);
            strcpy(vals[i * nchan + ic].nscl, nscl);
            strcpy(vals[i * nchan + ic].sta, stnm);
            strcpy(vals[i * nchan + ic].cha, tmpchan);
            strcpy(vals[i * nchan + ic].net, netw);
            strcpy(vals[i * nchan + ic].loc, loc);
            vals[i * nchan + ic].indx = i;
            vals[i * nchan + ic].time = json_msg->time;
            // Setting nsamps to 1 is required for geojson?
            vals[i * nchan + ic].nsamps = 1;
        }
        free(json_msg);
    }

    if (debug_cwu) {
        for (i = 0; i < nRead; i++) {
            LOG_DEBUGMSG("json_msgs[%d]: %s, %ld, %f", i, json_msgs[i].sncl, json_msgs[i].time,
                json_msgs[i].coor[0]);
        }
    }

    // prepare to use tmp to sort header information
    for (i = 0; i < nRead * nchan; i++){
        memcpy(&tmp[i], &vals[i], sizeof(struct string_index));
    }

    // Sort the msg records by scnl + time to align with generictraceData slots:
    traceBuffer_generictrace_sort2(tmp, nRead * nchan);

    if (dump_nRead) {
        LOG_DEBUGMSG("%s", "CCC: Dump unsorted structs:");
        traceBuffer_generictrace_printStringindex(vals, nRead * nchan);
        LOG_DEBUGMSG("%s", "CCC: Dump sorted structs:");
        traceBuffer_generictrace_printStringindex(tmp, nRead * nchan);
    }

    for (i = 0; i < nRead * nchan + 1; i++){
        imap[i]  = -9;
    }

    // Loop on waveforms and get workspace count
    // Loop through nRead msgs in sorted order and assign a k value to each
    // channels are in order "ZNE321Q"
    nskip = 0;
    kchan = 0;
    for (i = 0; i < nRead * nchan; i++) {
        j = tmp[i].indx;
        // imsg keeps msg sort order
        imsg[i] = j;

        sprintf(nscl, "%s.%s.%s.%s", tmp[i].net, tmp[i].sta, tmp[i].cha, tmp[i].loc);
        // printf("nscl:%s, tmpchan:%s\n", nscl, tmpchan);

        if ((node = traceBuffer_generictrace_hashmap_contains(generictraceData->hashmap, nscl)) == NULL) {
            if (debug_cwu) {
                LOG_DEBUGMSG("unpack: %s is in geojson msgs but not in generictraceData!", nscl);
            }
            nskip++;
        } else {
            k = node->i;
            imap[i] = k;
            kpts[k] += vals[j].nsamps; // this should always be 1 for geojson
            nmsg[k] += 1;

            if (debug_cwu) {
                LOG_DEBUGMSG("node(%s) has k=%d, imap[%d]=%d, kpts[%d]=%d, nmsg[%d]=%d",
                    nscl, k, i, k, k, kpts[k], k, nmsg[k]);
            }
        }
        kchan++;
    }

    // It's now sorted so that as you step through i: 1, ..., nRead,
    // imsg[i] = next msg in sort order, while imap[i] = kth generictraceData scnl msg target
    int n_chan_w_data = 0;
    for (k = 0; k < generictraceData->ntraces; k++) {
        if (nmsg[k] > 0) {
            n_chan_w_data++;
        }
    }
    LOG_DEBUGMSG("unpackGeojson: skipped %d channels, %d sncl with data", nskip, n_chan_w_data);

    // Step through the sorted imap[i]=k and figure out where each new k starts = imap[imapPtr[ir]]
    kold = -999;
    ir = 0;
    for (i = 0; i < nRead * nchan; i++) {
        k = imap[i];
        if (k != kold) {
            if (k > -1) {
                imapPtr[ir] = i;
                ir += 1;
            }
            kold = k;
        }
    }

    nReadPtr = ir;

    LOG_DEBUGMSG("unpackGeojson nRead:%d ntraces:%d nReadPtr:%d",
        nRead, generictraceData->ntraces, nReadPtr);

    if (debug_cwu) {
        LOG_DEBUGMSG("%s", "Printing generictraceData");
        traceBuffer_generictrace_printGenerictraceData(generictraceData);
    }

    // Now set the workspace
    for (k = 0; k < generictraceData->ntraces; k++)
    {
        traceBuffer_generictrace_freeGenerictrace(clearSNCL, &generictraceData->traces[k]);
        if (kpts[k] > 0)
        {
            generictraceData->traces[k].data  = memory_calloc32i(kpts[k]);
            generictraceData->traces[k].times = memory_calloc64f(kpts[k]);
            generictraceData->traces[k].chunkPtr = memory_calloc32i(nmsg[k] + 1);
            generictraceData->traces[k].npts = kpts[k];
            if (debug_cwu) {
                LOG_DEBUGMSG("k:%d, %s.%s.%s.%s: kpts[k]=%d, nmsg[k]=%d",
                    k, generictraceData->traces[k].netw, generictraceData->traces[k].stnm, generictraceData->traces[k].chan, generictraceData->traces[k].loc,
                    kpts[k], nmsg[k]);
            }
        }
    }

    if (debug_cwu) {
        for (i = 0; i < nRead * nchan + 1; i++) {
            LOG_DEBUGMSG("imsg[%d]:%d, imap[%d]=%d, imapPtr[%d]=%d",
                 i, imsg[i], i, imap[i], i, imapPtr[i]);
        }
        for (k = 0; k < generictraceData->ntraces; k++) {
            LOG_DEBUGMSG("kpts[%d]=%d, nmsg[%d]=%d",
                k, kpts[k], k, nmsg[k]);
        }
    }

    // Final loop to load traces
    for (ir = 0; ir < nReadPtr; ir++)
    {
        i1 = imapPtr[ir];
        k = imap[i1];
        i2 = i1 + nmsg[k];
        kndx = 0;
        if (1) {
            sprintf(buf, "%s.%s.%s.%s", generictraceData->traces[k].netw, generictraceData->traces[k].stnm,
                generictraceData->traces[k].chan, generictraceData->traces[k].loc);
        }

        generictraceData->traces[k].nchunks = 1;

        // Loop on the messages for this SNCL
        for (im = i1; im < i2; im++)
        {
            i = imsg[im];
            if (debug_cwu) {
                LOG_DEBUGMSG("*** %s ir=%d, i1=imapPtr[ir]=%d, k=imap[i1]=%d, i2=i1+nmsg[k]=%d, kndx=%d, im=%d, i=%d",
                    buf, ir, i1, k, i2, kndx, im, i);
            }
            if (i < 0 || i >= nRead)
            {
                LOG_ERRMSG("Invalid message number %d", i);
                continue;
            }
            // indx = i * max_payload_size;
            npts = 1;
            dt = 1.0;

            // indx = i * MAX_TRACEBUF_SIZ;
            // trh  = (TRACE2_HEADER *) &msgs[indx];
            // dtype = 4;
            // lswap = 0;
            // npts = trh->nsamp;
            // dt = 1.0/trh->samprate;

            generictraceData->traces[k].dt = dt;
            

            // ierr = fastUnpack(npts, lswap, dtype, &msgs[indx], resp);
            // if (ierr != 0) {
            //     LOG_ERRMSG("%s", "Error unpacking data");
            // }

            // Is a new chunk beginning?
            if (im > i1) {
                // if (fabs( (generictraceData->traces[k].times[kndx-1] + dt) - trh->starttime ) > 1.e-6) {
                if (fabs( (generictraceData->traces[k].times[kndx-1] + dt) - (double)(json_msgs[i].time) / 1000 ) > 1.e-6) {
                    
                    // starttime exceeds dt --> start a new chunk
                    if (debug) {
                        LOG_DEBUGMSG("  ir:%d i1:%d im:%d k:%d %s kndx:%d npts:%d nchunks:%d start a new chunk",
                                     ir, i1, im, k, buf, kndx, npts, generictraceData->traces[k].nchunks);
                    }
                    generictraceData->traces[k].chunkPtr[generictraceData->traces[k].nchunks] = kndx;
                    generictraceData->traces[k].nchunks += 1;
                    generictraceData->traces[k].chunkPtr[generictraceData->traces[k].nchunks] = kndx + npts;
                }
                else {
                    // starttime is within dt --> simply extend current chunk
                    if (debug) {
                        LOG_DEBUGMSG("  ir:%d i1:%d im:%d k:%d %s kndx:%d npts:%d nchunks:%d extend current chunk",
                                     ir, i1, im, k, buf, kndx, npts, generictraceData->traces[k].nchunks);
                    }
                    generictraceData->traces[k].chunkPtr[generictraceData->traces[k].nchunks] = kndx + npts;
                }
            }

            // Update the points, reverse apply the gain to get int value 
            // Gain will be reapplied in traceBuffer_h5_copyTraceBufferToGFAST
            if      (generictraceData->traces[k].chan[2] == 'Z'){value = (int)(json_msgs[i].coor[2] * h5traceBuffer->traces[k].gain);}
            else if (generictraceData->traces[k].chan[2] == 'N'){value = (int)(json_msgs[i].coor[1] * h5traceBuffer->traces[k].gain);}
            else if (generictraceData->traces[k].chan[2] == 'E'){value = (int)(json_msgs[i].coor[0] * h5traceBuffer->traces[k].gain);}
            else if (generictraceData->traces[k].chan[2] == '3'){value = (int)(json_msgs[i].err[2] * h5traceBuffer->traces[k].gain);}
            else if (generictraceData->traces[k].chan[2] == '2'){value = (int)(json_msgs[i].err[1] * h5traceBuffer->traces[k].gain);}
            else if (generictraceData->traces[k].chan[2] == '1'){value = (int)(json_msgs[i].err[0] * h5traceBuffer->traces[k].gain);}
            else if (generictraceData->traces[k].chan[2] == 'Q'){value = json_msgs[i].quality;}
            else    {LOG_DEBUGMSG("ALERT! channel doesn't match ZNE321Q! %c", generictraceData->traces[k].chan[2]);}

            // generictraceData->traces[k].data[kndx] = json_msgs[i]->coor;
            generictraceData->traces[k].data[kndx] = value;
            // generictraceData->traces[k].times[kndx + l] = trh->starttime + (double) l*dt;
            generictraceData->traces[k].times[kndx] = (double)(json_msgs[i].time) / 1000;

            if (debug) {
                LOG_DEBUGMSG("  unpackGeojson  k:%4d scnl:%s time:%.2f data:%d, value:%d", 
                            k, buf, generictraceData->traces[k].times[kndx], generictraceData->traces[k].data[kndx], value);
            }
            kndx = kndx + npts;

        } // Loop on messages for this SNCL

        // Special case for one message
        if (i2 - i1 == 1 && kpts[k] > 0) {
            generictraceData->traces[k].nchunks = 1;
            generictraceData->traces[k].chunkPtr[generictraceData->traces[k].nchunks] = kpts[k];
        }
        if (debug_nchunks) {
          LOG_DEBUGMSG("  unpackGeojson  k:%4d scnl:%s nmsg:%d kpts:%d i1:%d i2:%d nchunks:%d",
                       k, buf, nmsg[k], kpts[k], i1, i2, generictraceData->traces[k].nchunks);
        }

        // Reality check
        if (kndx != kpts[k])
        {
            LOG_ERRMSG("Lost count %d %d", kndx, kpts[k]);
            return -1;
        }
        if (generictraceData->traces[k].nchunks > 0)
        {
            nchunks = generictraceData->traces[k].nchunks;
            if (generictraceData->traces[k].chunkPtr[nchunks] != generictraceData->traces[k].npts)
            {
                LOG_ERRMSG("Inconsistent number of points %d %d",
                           generictraceData->traces[k].chunkPtr[nchunks],
                           generictraceData->traces[k].npts);
                return -1;
            }
        }


        if (debug) {
          LOG_DEBUGMSG("  unpackGeojson  k:%4d nchunks:%d chunkPtr[0]:%d chunkPtr[nchunks]:%d total_npts:%d",
                       k, generictraceData->traces[k].nchunks, generictraceData->traces[k].chunkPtr[0],
                       generictraceData->traces[k].chunkPtr[nchunks], generictraceData->traces[k].npts);
        }

    } // Loop on pointers

    if (debug_cwu) {
        traceBuffer_generictrace_printGenerictraceData(generictraceData);
    }

    // Free space
    memory_free32i(&imap);
    memory_free32i(&imsg);
    memory_free32i(&kpts);
    memory_free32i(&nmsg);
    memory_free32i(&imapPtr);
    memory_free8c(&nscl);
    memory_free8c(&tmpchan);
    free(json_msgs);
    free(vals);
    free(tmp);
    
    return 0;
}

// Move this to a different directory?
struct geojson_struct *unpackMessage(const char *msg, const int msg_size) {
    struct geojson_struct *json_msg;
    char *text;

    json_msg = malloc(sizeof(struct geojson_struct));
    text = malloc(msg_size * sizeof(char));

    memcpy(text, msg, msg_size);

    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if(!root) {
        LOG_ERRMSG("error: on line %d: %s", error.line, error.text);
        return json_msg;
    }

    // printf("json is type %d\n", (int)(root->type));

    if(!json_is_object(root)) {

        LOG_ERRMSG("error: root is not an object, it is %d", (int)(root->type));
        json_decref(root);
        return json_msg;
    }

    // Could also just have one 'json_value' that is reused?
    // json_t *j_time, *j_Q, *j_type, *j_SNCL, *j_coor, *j_err, *j_rate;
    json_t *json_value;

    // NOTE (CWU 12/5/24): when parsing numbers, use json_number_value. It will check if real or
    // int and return in the form of a double. It can be casted afterwards to what you expect/want.

    // time
    json_value = json_object_get(root, "time");
    if (!json_is_number(json_value)) {
        LOG_DEBUGMSG("time is not a number, it is %s!", json_string_value(json_object_get(json_value, "type")));
    }
    json_msg->time = (long)json_number_value(json_value);

    // Q
    json_value = json_object_get(root, "Q");
    if (!json_is_number(json_value)) {
        LOG_DEBUGMSG("Q is not a number, it is %s!", json_string_value(json_object_get(json_value, "type")));
    }
    json_msg->quality = (int)json_number_value(json_value);

    // type
    json_value = json_object_get(root, "type");
    if (!json_is_string(json_value)) {
        LOG_DEBUGMSG("type is not a string, it is %s!", json_string_value(json_object_get(json_value, "type")));
    }
    memcpy(json_msg->type, json_string_value(json_value), sizeof json_msg->type);

    // SNCL
    json_value = json_object_get(root, "SNCL");
    if (!json_is_string(json_value)) {
        LOG_DEBUGMSG("SNCL is not a string, it is %s!", json_string_value(json_object_get(json_value, "type")));
    }
    memcpy(json_msg->sncl, json_string_value(json_value), sizeof json_msg->sncl);

    // coor
    json_value = json_object_get(root, "coor");
    if (!json_is_array(json_value)) {
        LOG_DEBUGMSG("coor is not an array, it is %s!", json_string_value(json_object_get(json_value, "type")));
    }
    int i;
    for (i = 0; i < (int)json_array_size(json_value); i++) {
        json_t *data;
        data = json_array_get(json_value, i);

        if(!json_is_number(data)) {
            LOG_DEBUGMSG("data is not a number, it is %s!", json_string_value(json_object_get(data, "type")));
        }
        json_msg->coor[i] = (double)json_number_value(data);
    }

    // err
    json_value = json_object_get(root, "err");
    if (!json_is_array(json_value)) {
        LOG_DEBUGMSG("err is not an array, it is %s!", json_string_value(json_object_get(json_value, "type")));
    }
    for (i = 0; i < (int)json_array_size(json_value); i++) {
        json_t *data;
        data = json_array_get(json_value, i);

        if(!json_is_number(data)) {
            LOG_DEBUGMSG("data is not a number, it is %s!", json_string_value(json_object_get(data, "type")));
        }

        json_msg->err[i] = (double)json_number_value(data);
    }

    // rate
    json_value = json_object_get(root, "rate");
    if (!json_is_number(json_value)) {
        LOG_DEBUGMSG("rate is not a number it is %s!", json_string_value(json_object_get(json_value, "type")));
    }
    json_msg->rate = (double)json_number_value(json_value);

    // printf("%s\n", msg);
    // print_json_msg(json_msg);

    return json_msg;
}

void print_json_msg(struct geojson_struct *json_msg) {
    printf("(%ld) ", json_msg->time);
    printf("(%d) ", json_msg->quality);
    printf("(%s) ", json_msg->type);
    printf("(%s) ", json_msg->sncl);
    printf("[%.3f,%.3f,%.3f] ", json_msg->coor[0], json_msg->coor[1], json_msg->coor[2]);
    printf("[%.3f,%.3f,%.3f] ", json_msg->err[0], json_msg->err[1], json_msg->err[2]);
    printf("(%f)\n", json_msg->rate);
}

// Defining comparator function as per the requirement
// static int myCompare2(const void *x, const void *y)
// {
//     // Should sort by s,n,l,c,time
//     const struct string_index xx = *(const struct string_index *) x;
//     const struct string_index yy = *(const struct string_index *) y;
//     int ista, inet, iloc, icha;

//     ista = strcmp(xx.sta, yy.sta);
//     if (ista == 0) {
//         inet = strcmp(xx.net, yy.net);
//         if (inet == 0) {
//             iloc = strcmp(xx.loc, yy.loc);
//             if (iloc == 0) {
//                 icha = strcmp(xx.cha, yy.cha);
//                 if (icha == 0) {
//                     if (xx.time > yy.time) {
//                         return 1;
//                     }
//                     else if (xx.time < yy.time) {
//                         return -1;
//                     }
//                     else {
//                         return 0;
//                     }
//                 }
//                 else { // order by {LYZ, LYN, LYE} to match generictraceData
//                     return -1 * icha;
//                 }
//             }
//             else {
//                 return iloc;
//             }
//         }
//         else {
//             return inet;
//         }
//     }
//     else {
//         return ista;
//     }
// }

// // Function to sort the array
// void sort2(struct string_index values[], int n)
// {
//     // calling qsort function to sort the array
//     // with the help of Comparator
//     qsort((void *) values, (size_t) n, sizeof(struct string_index), myCompare2);
// }

// void print_struct(struct string_index *d, int n) {
//     int i;
//     for (i = 0; i < n; i++){
//         LOG_DEBUGMSG("CCC struct[%3d] indx:%5d: nscl:%s nsamps:%d time:%.2f",
//             i, d[i].indx, d[i].nscl, d[i].nsamps, d[i].time);
//     }
// }

int splitSNCL(const char *cline, char stnm[64], char netw[64], char chan[64], char loc[64]) {
    char *token, *work;
    int i, ierr;
    const char *split = ".";
    // Copy the input
    ierr = 0;
    i = 0;

    work = (char *)calloc(strlen(cline) + 1, sizeof(char));
    strcpy(work, cline);
    // Set the outputs
    memset(netw, 0, sizeof(char)*64);
    memset(stnm, 0, sizeof(char)*64);
    memset(chan, 0, sizeof(char)*64);
    memset(loc,  0, sizeof(char)*64);

    token = strtok(work, split);
    while (token)
    {   
        if (i == 0){strcpy(stnm, token);}
        if (i == 1){strcpy(netw, token);}
        if (i == 2){strcpy(chan,  token);}
        if (i == 3){strcpy(loc, token);}
        i = i + 1;
        token = strtok(NULL, split);
    }   

    if (i != 4) 
    {   
        LOG_ERRMSG("Failed to split line %d %s", i, cline);
        ierr = 1;
    }   
    free(work);
    return ierr;
}