#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"
#include "iscl/sorting/sorting.h"

#include "iscl/time/time.h"

static void fastUnpackI4(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         int *__restrict__ resp);
static void fastUnpackI2(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         int *__restrict__ resp);
static int fastUnpack(const int npts, const int lswap,
                      const int type,
                      const char *__restrict__ msg,
                      int *__restrict__ resp);

/*!
 * MTH: 2021/05 Rewrite of unpackTraceBuf2Messages.c
 *              Replace 4 loops/sorts with one sort of record struct
 * CWU: 2022/10 Rewrite unpackTraceBuf2Messages.c again
 *              Add loc to compare fxn, and use a hashmap into tb2data
 */

/*!
 * @brief Unpacks the tracebuf2 messages read from the ring and returns
 *        the concatenated data for the desired SNCL's in the tb2Data struct
 *
 * @param[in] nRead          number of traces read off the ring
 * @param[in] msgs           tracebuf2 messages read off ring.  the i'th message
 *                           is given by (i-1)*MAX_TRACEBUF_SIZ for
 *                           i=1,2,...,nRead
 *
 * @param[in,out] tb2Data    on input contains the desired SNCL's whose data 
 *                           will be unpacked from the header (should it be
 *                           present).  
 *                           on output contains the data in the messages for
 *                           the SNCL's defined in this structure (should a 
 *                           match be found).
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */           

struct tb_struct {
  char netw[8];
  char stnm[8];
  char chan[8];
  char loc[8];
};

struct string_index {
  char logo[15];
  char nscl[15];
  char net[8];
  char sta[8];
  char cha[8];
  char loc[8];
  double time;
  int indx;
  int nsamps;
  //int k;
  //float data;
};
void print_struct(struct string_index *d, int n);
void sort2(struct string_index *vals, int n);


int traceBuffer_ewrr_unpackTraceBuf2Messages(
    const int nRead,
    const char *msgs,
    struct tb2Data_struct *tb2Data)
{
    char *msg;
    TRACE2_HEADER  *trh;
    double *times, dt;
    int *nsamps;
    int *imap, *imapPtr, *imsg, *iperm, *kpts, *nmsg, *resp,
        dtype, i, i1, i2, ierr, im, indx, ir, k, kndx, l, j,
        lswap, nchunks, nReadPtr, npts, nskip;
    struct tb2_node *node;
    const int maxpts = MAX_TRACEBUF_SIZ/16; // MAX_TRACEBUF_SIZ/sizeof(int16_t)
    const bool clearSNCL = false;

    //char **msg_logos = (char **)malloc(sizeof(char *) * nRead);
    //char msg_logos[nRead][15];
    char buf[256];
    char *logo, *nscl;

    int debug = 0;

    int kold;
    struct string_index *vals, *tmp;
    vals = (struct string_index *) calloc((size_t) nRead, sizeof(struct string_index));
    tmp = (struct string_index *) calloc((size_t) nRead, sizeof(struct string_index));

    //------------------------------------------------------------------------//
    //
    // Check the tb2data was initialized
    if (!tb2Data->linit)
    {
        LOG_ERRMSG("%s", "tb2Data never initialized");
        return -1;
    }
    // Nothing to do
    if (tb2Data->ntraces == 0) {return 0;}
    if (nRead == 0){return 0;}

    // Set the workspace
    msg   = memory_calloc8c(MAX_TRACEBUF_SIZ);
    imap  = memory_calloc32i(nRead+1);
    imsg  = memory_calloc32i(nRead);
    iperm = memory_calloc32i(nRead);
    kpts  = memory_calloc32i(tb2Data->ntraces);
    nmsg  = memory_calloc32i(tb2Data->ntraces);
    imapPtr = memory_calloc32i(nRead + 1); // worst case size
    times = memory_calloc64f(nRead);
    resp  = memory_calloc32i(maxpts);
    nsamps= memory_calloc32i(nRead);
    logo  = memory_calloc8c(15);
    nscl  = memory_calloc8c(15);

    for (i=0; i<nRead+1; i++) {imap[i] = tb2Data->ntraces + 1;}

    LOG_DEBUGMSG("unpackTB2: Enter  nTraces:%d nRead:%d", tb2Data->ntraces, nRead);

    bool dump_tb2Data = false;
    bool dump_nRead = false;
    bool debug_imap = false;
    bool debug_nchunks = false;

    if (dump_tb2Data) {
        for (k=0; k<tb2Data->ntraces; k++){
            LOG_DEBUGMSG("CCC dump_tb2Data: %s.%s.%s.%s",
                tb2Data->traces[k].stnm, tb2Data->traces[k].chan,
                tb2Data->traces[k].netw, tb2Data->traces[k].loc);
        }
        //exit(0);
    }

    // MTH: load up the msg logos, times and nsamp into records to sort once
    for (i=0; i<nRead; i++)
    {
        indx = i*MAX_TRACEBUF_SIZ;
        trh  = (TRACE2_HEADER *) &msgs[indx];

        sprintf(logo, "%s.%s.%s.%s", trh->sta, trh->chan, trh->net, trh->loc);
        sprintf(nscl, "%s.%s.%s.%s", trh->net, trh->sta, trh->chan, trh->loc);
        strcpy(vals[i].logo, logo);
        strcpy(vals[i].nscl, nscl);
        strcpy(vals[i].sta, trh->sta);
        strcpy(vals[i].cha, trh->chan);
        strcpy(vals[i].net, trh->net);
        strcpy(vals[i].loc, trh->loc);
        vals[i].indx = i;
        vals[i].time = trh->starttime;
        vals[i].nsamps = trh->nsamp;
    }
    if (dump_nRead) {
        LOG_DEBUGMSG("%s", "CCC: Dump unsorted structs:");
        print_struct(vals, nRead);
    }

    for (i=0; i<nRead; i++){
        memcpy(&tmp[i], &vals[i], sizeof(struct string_index));
    }

    // Sort the msg records by scnl + time to align with tb2Data slots:
    sort2(tmp, nRead);
    if (dump_nRead){
        LOG_DEBUGMSG("%s", "CCC: Dump sorted structs:");
        print_struct(tmp, nRead);
    }

    for (i=0; i<nRead; i++){
        //tmp[i].k = -9;
        imap[i]  = -9;
    }

    // Loop on waveforms and get workspace count
    // Loop through nRead msgs in sorted order and assign a k value to each
    nskip = 0;
    for (i = 0; i < nRead; i++) {
        j = tmp[i].indx;
        // imsg keeps msg sort order
        imsg[i] = j;

        if ((node = traceBuffer_ewrr_hashmap_contains(tb2Data->hashmap, tmp[i].nscl)) == NULL) {
            if (debug) {
                LOG_DEBUGMSG("unpackTB2: %s is in EW msgs but not in tb2Data!", tmp[i].nscl);
            }
            nskip++;
        } else {
            k = node->i;
            imap[i] = k;
            kpts[k] += vals[j].nsamps;
            nmsg[k] += 1;
            continue;
        }
    }
    // It's now sorted so that as you step through i: 1, ..., nRead,
    // imsg[i] = next msg in sort order, while imap[i] = kth tb2Data scnl msg target
    int n_chan_w_data = 0;
    for (k = 0; k < tb2Data->ntraces; k++) {
        if (nmsg[k] > 0) {
            n_chan_w_data++;
        }
    }
    LOG_DEBUGMSG("unpackTB2: skipped %d msgs, %d sncl with data", nskip, n_chan_w_data);

    if (debug_imap) {
        LOG_DEBUGMSG("%s", "CCC - show imap");
        for (i = 0; i < nRead; i++){
            k = imap[i];
            LOG_DEBUGMSG("CCC == imap[%d] --> k:%d %s.%s",
                i, k, tb2Data->traces[k].stnm, tb2Data->traces[k].chan);
        }
    }

    // Step through the sorted imap[i]=k and figure out where each new k starts = imap[imapPtr[ir]]
    kold = -999;
    ir = 0;
    for (i = 0; i < nRead; i++) {
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

    LOG_DEBUGMSG("unpackTB2 nRead:%d ntraces:%d nReadPtr:%d",
        nRead, tb2Data->ntraces, nReadPtr);

    // Now set the workspace
    for (k = 0; k < tb2Data->ntraces; k++)
    {
        traceBuffer_ewrr_freetb2Trace(clearSNCL, &tb2Data->traces[k]);
        if (kpts[k] > 0)
        {
            tb2Data->traces[k].data  = memory_calloc32i(kpts[k]);
            tb2Data->traces[k].times = memory_calloc64f(kpts[k]);
            tb2Data->traces[k].chunkPtr = memory_calloc32i(nmsg[k] + 1);
            tb2Data->traces[k].npts = kpts[k];
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
            sprintf(buf, "%s.%s.%s.%s", tb2Data->traces[k].netw, tb2Data->traces[k].stnm,
                tb2Data->traces[k].chan, tb2Data->traces[k].loc);
        }

        tb2Data->traces[k].nchunks = 1;

        // Loop on the messages for this SNCL
        for (im = i1; im < i2; im++)
        {
            i = imsg[im];
            if (i < 0 || i >= nRead)
            {
                LOG_ERRMSG("Invalid message number %d", i);
                continue;
            }
            indx = i * MAX_TRACEBUF_SIZ;
            trh  = (TRACE2_HEADER *) &msgs[indx];
            dtype = 4;
            lswap = 0;
            npts = trh->nsamp;
            dt = 1.0/trh->samprate;
            tb2Data->traces[k].dt = dt;

            ierr = fastUnpack(npts, lswap, dtype, &msgs[indx], resp);
            if (ierr != 0) {
                LOG_ERRMSG("%s", "Error unpacking data");
            }

            // Is a new chunk beginning?
            if (im > i1) {
                if (fabs( (tb2Data->traces[k].times[kndx-1] + dt) - trh->starttime ) > 1.e-6) {
                    // starttime exceeds dt --> start a new chunk
                    if (debug) {
                        LOG_DEBUGMSG("ir:%d i1:%d im:%d k:%d %s kndx:%d npts:%d nchunks:%d start a new chunk",
                                     ir, i1, im, k, buf, kndx, npts, tb2Data->traces[k].nchunks);
                    }
                    tb2Data->traces[k].chunkPtr[tb2Data->traces[k].nchunks] = kndx;
                    tb2Data->traces[k].nchunks += 1;
                    tb2Data->traces[k].chunkPtr[tb2Data->traces[k].nchunks] = kndx + npts;
                }
                else {
                    // starttime is within dt --> simply extend current chunk
                    if (debug) {
                        LOG_DEBUGMSG("ir:%d i1:%d im:%d k:%d %s kndx:%d npts:%d nchunks:%d extend current chunk",
                                     ir, i1, im, k, buf, kndx, npts, tb2Data->traces[k].nchunks);
                    }
                    tb2Data->traces[k].chunkPtr[tb2Data->traces[k].nchunks] = kndx + npts;
                }
            }

            // Update the points
#ifdef _OPENMP
            #pragma omp simd
#endif
            for (l=0; l<npts; l++)
            {
                tb2Data->traces[k].data[kndx+l] = resp[l];
                tb2Data->traces[k].times[kndx+l] = trh->starttime + (double) l*dt;

                if (debug) {
                  LOG_DEBUGMSG("unpackTB2 k:%4d scnl:%s time:%.2f val:%d", 
                               k, buf, tb2Data->traces[k].times[kndx+l], tb2Data->traces[k].data[kndx+l]);
                }
            }
            kndx = kndx + npts;

        } // Loop on messages for this SNCL

        // Special case for one message
        if (i2 - i1 == 1 && kpts[k] > 0) {
            tb2Data->traces[k].nchunks = 1;
            tb2Data->traces[k].chunkPtr[tb2Data->traces[k].nchunks] = kpts[k];
        }
        if (debug_nchunks) {
          LOG_DEBUGMSG("unpackTB2: k:%4d scnl:%s nmsg:%d kpts:%d i1:%d i2:%d nchunks:%d",
                       k, buf, nmsg[k], kpts[k], i1, i2, tb2Data->traces[k].nchunks);
        }

        // Reality check
        if (kndx != kpts[k])
        {
            LOG_ERRMSG("Lost count %d %d", kndx, kpts[k]);
            return -1;
        }
        if (tb2Data->traces[k].nchunks > 0)
        {
            nchunks = tb2Data->traces[k].nchunks;
            if (tb2Data->traces[k].chunkPtr[nchunks] != tb2Data->traces[k].npts)
            {
                LOG_ERRMSG("Inconsistent number of points %d %d",
                           tb2Data->traces[k].chunkPtr[nchunks],
                           tb2Data->traces[k].npts);
                return -1;
            }
        }


        if (debug) {
          LOG_DEBUGMSG("unpackTB2  k:%4d nchunks:%d chunkPtr[0]:%d chunkPtr[nchunks]:%d total_npts:%d",
                       k, tb2Data->traces[k].nchunks, tb2Data->traces[k].chunkPtr[0],
                       tb2Data->traces[k].chunkPtr[nchunks], tb2Data->traces[k].npts);
        }

    } // Loop on pointers

    // Free space
    memory_free8c(&msg);
    memory_free32i(&imap);
    memory_free32i(&imsg);
    memory_free32i(&iperm);
    memory_free32i(&kpts);
    memory_free32i(&nmsg);
    memory_free32i(&resp);
    memory_free64f(&times);
    memory_free32i(&imapPtr);
    memory_free32i(&nsamps);
    memory_free8c(&logo);
    memory_free8c(&nscl);
    free(vals);
    free(tmp);
    
    return 0;
}
//============================================================================//
/*!
 * @brief Unpacks a 4 byte integer character data
 *
 * @param[in] npts    number of points to unpack
 * @param[in] lswap   if 0 then do not byte swap the data.
 *                    if 1 then do byte swap the data.
 * @param[in] msg     tracebuf2 message to unpack [MAX_TRACEBUF_SIZ]
 *
 * @param[out] resp   response data [npts]
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 * @bug This doesn't vectorize.
 *
 */
static void fastUnpackI4(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         int *__restrict__ resp)
{
    const int ioff = (int) (sizeof(TRACE2_HEADER));
    char c4[4];
    int i, i4;
    if (lswap == 0)
    {
        //#pragma omp simd
        for (i=0; i<npts; i++)
        {
            c4[0] = msg[ioff+4*i+0];
            c4[1] = msg[ioff+4*i+1]; 
            c4[2] = msg[ioff+4*i+2]; 
            c4[3] = msg[ioff+4*i+3];
            memcpy(&i4, c4, sizeof(int));
            resp[i] = (int) i4; 
        }
    }
    else
    {
        //#pragma omp simd
        for (i=0; i<npts; i++)
        {
            c4[3] = msg[ioff+4*i+0];
            c4[2] = msg[ioff+4*i+1]; 
            c4[1] = msg[ioff+4*i+2]; 
            c4[0] = msg[ioff+4*i+3];
            memcpy(&i4, c4, sizeof(int));
            resp[i] = (int) i4;
        }
    }
    return;
}
//============================================================================//
/*!
 * @brief Unpacks a 2 byte integer character data
 *
 * @param[in] npts    number of points to unpack
 * @param[in] lswap   if 0 then do not byte swap the data.
 *                    if 1 then do byte swap the data.
 * @param[in] msg     tracebuf2 message to unpack [MAX_TRACEBUF_SIZ]
 *
 * @param[out] resp   response data [npts]
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 * @bug This doesn't vectorize.
 *
 */
static void fastUnpackI2(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         int *__restrict__ resp)
{
    const int ioff = (int) (sizeof(TRACE2_HEADER));
    char c2[2];
    int i;
    short i2;
    if (lswap == 0)
    {
        //#pragma omp simd
        for (i=0; i<npts; i++)
        {
            c2[0] = msg[ioff+2*i+0];
            c2[1] = msg[ioff+2*i+1];
            memcpy(&i2, c2, sizeof(short));
            resp[i] = (int) i2;
        }
    }
    else
    {
        //#pragma omp simd
        for (i=0; i<npts; i++)
        {
            c2[1] = msg[ioff+2*i+0];
            c2[0] = msg[ioff+2*i+1]; 
            memcpy(&i2, c2, sizeof(short));
            resp[i] = (int) i2;
        }
    }   
    return;
}
//============================================================================//
/*!
 * @brief Unpacks a 2 or 4 byte integer character data
 *
 * @param[in] npts    number of points to unpack
 * @param[in] lswap   if 0 then do not byte swap the data.
 *                    if 1 then do byte swap the data.
 * @param[in] dtype   if 4 then the data is 4 bytes.
 *                    if 2 then the data is 2 bytes.
 * @param[in] msg     tracebuf2 message to unpack [MAX_TRACEBUF_SIZ]
 *
 * @param[out] resp   response data [npts]
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
static int fastUnpack(const int npts, const int lswap,
                      const int dtype,
                      const char *__restrict__ msg,
                      int *__restrict__ resp)
{
    if (npts < 1){return 0;} // Nothing to do
    if (dtype == 4)
    {
        fastUnpackI4(npts, lswap, msg, resp);
    }
    else if (dtype == 2)
    {
        fastUnpackI2(npts, lswap, msg, resp);
    }
    else
    {
        LOG_ERRMSG("%s", "Invalid type");
        return -1;
    }
    return 0;
}

// Defining comparator function as per the requirement
static int myCompare2(const void *x, const void *y)
{
    // Should sort by s,n,l,c,time
    const struct string_index xx = *(const struct string_index *) x;
    const struct string_index yy = *(const struct string_index *) y;
    int ista, inet, iloc, icha;

    ista = strcmp(xx.sta, yy.sta);
    if (ista == 0) {
        inet = strcmp(xx.net, yy.net);
        if (inet == 0) {
            iloc = strcmp(xx.loc, yy.loc);
            if (iloc == 0) {
                icha = strcmp(xx.cha, yy.cha);
                if (icha == 0) {
                    if (xx.time > yy.time) {
                        return 1;
                    }
                    else if (xx.time < yy.time) {
                        return -1;
                    }
                    else {
                        return 0;
                    }
                }
                else { // order by {LYZ, LYN, LYE} to match tb2Data
                    return -1 * icha;
                }
            }
            else {
                return iloc;
            }
        }
        else {
            return inet;
        }
    }
    else {
        return ista;
    }
}

// Function to sort the array
void sort2(struct string_index values[], int n)
{
    // calling qsort function to sort the array
    // with the help of Comparator
    qsort((void *) values, (size_t) n, sizeof(struct string_index), myCompare2);
}

void print_struct(struct string_index *d, int n) {
    int i;
    for (i=0; i<n; i++){
        LOG_DEBUGMSG("CCC struct[%d] indx:%5d: logo:%s nsamps:%d time:%.2f",
            i, d[i].indx, d[i].logo, d[i].nsamps, d[i].time);
    }
}
