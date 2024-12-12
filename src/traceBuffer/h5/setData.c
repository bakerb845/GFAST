#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "gfast_hdf5.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"

/*
static int update_dataSet(const hid_t groupID,
                          const char *dataSetName, 
                          int i1, int i2, const int npts,
                          const double *__restrict__ data);
*/

/*!
 * @brief Sets the data in the HDF5 file from the data on the generictraceData 
 *        buffer.  If any data is late then it will be filled with NaN's
 *        in the HDF5 file.
 *
 * @param[in] currentTime    current time (UTC seconds since epoch) to which to
 *                           update the HDF5 data buffers. 
 * @param[in] generictraceData        holds the tracebuffer2 data to be written to 
 *                           disk.  the SNCLs on generictraceData should correspond
 *                           to the SNCLs on h5TraceBuffer and the data should
 *                           be in temporal order. 
 * @param[in] h5traceBuffer  holds the HDF5 group names for each trace and
 *                           HDF5 file details
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 */
int traceBuffer_h5_setData(const double currentTime,
                           struct generictraceData_struct generictraceData,
                           struct h5traceBuffer_struct h5traceBuffer)
{
    double *dwork, *gains, *work;
    int *map, c1, c2, chunk, i, i1, i2, idt, ierr, ierrAll,
        indx, is, ishift, jndx, k, k1, k2, maxpts, nchunks, ncopy, ntraces;
    double dt, ts1, ts2;
    bool *lhaveData;
    hsize_t dims[2];
    hid_t attribute, groupID, dataSet, dataSpace;
    herr_t status;
    int debug = 0;
    //------------------------------------------------------------------------//
    //
    // Require both items are set
    if (debug) {
        LOG_DEBUGMSG("%s", "Starting traceBuffer_h5_setData");
    }
    ierr = 0;
    ierrAll = 0;
    if (!generictraceData.linit || !h5traceBuffer.linit)
    {
        if (!generictraceData.linit){LOG_ERRMSG("%s", "Error generictraceData not set");}
        if (!h5traceBuffer.linit){LOG_ERRMSG("%s", "h5traceBuffer not set");}
        return -1;
    }
    // Nothing to do
    if (h5traceBuffer.ntraces < 1)
    {
        return 0;
    }
    // Not finished yet
    if (h5traceBuffer.ndtGroups > 1)
    {
        LOG_ERRMSG("%s", "Multiple dt groups not yet done");
        return -1;
    }
    // Make a map from the HDF5 trace buffer to the tracebuffer2 data
    if (debug) {
        LOG_DEBUGMSG("%s", "Make a map from the HDF5 trace buffer to the tracebuffer2 data");
    }
    map = array_set32i(h5traceBuffer.ntraces, -1, &ierr);
    lhaveData = memory_calloc8l(h5traceBuffer.ntraces);
    for (i=0; i<h5traceBuffer.ntraces; i++)
    {
        // Take an educated guess
        if (i < generictraceData.ntraces)
        {
            if ((strcasecmp(h5traceBuffer.traces[i].netw,
                                  generictraceData.traces[i].netw) == 0) &&
                (strcasecmp(h5traceBuffer.traces[i].stnm,
                                  generictraceData.traces[i].stnm) == 0) && 
                (strcasecmp(h5traceBuffer.traces[i].chan,
                                  generictraceData.traces[i].chan) == 0) &&
                (strcasecmp(h5traceBuffer.traces[i].loc,
                                  generictraceData.traces[i].loc)  == 0))
            {
                map[i] = i;
                if (generictraceData.traces[i].npts > 0)
                {
                    lhaveData[i] = true; 
                }
                goto NEXT_TRACE; 
            }
        }
        if (debug) {
            LOG_DEBUGMSG("h5traceBuffer and generictraceData didn't match, going hunting (inefficient)! i:%d, %s.%s.%s.%s",
                i,
                h5traceBuffer.traces[i].netw,
                h5traceBuffer.traces[i].stnm,
                h5traceBuffer.traces[i].chan,
                h5traceBuffer.traces[i].loc)
        }
        // Hunt through the tracebuffers
        for (k=0; k<generictraceData.ntraces; k++)
        {
            if ((strcasecmp(h5traceBuffer.traces[i].netw,
                                  generictraceData.traces[k].netw) == 0) &&
                (strcasecmp(h5traceBuffer.traces[i].stnm,
                                  generictraceData.traces[k].stnm) == 0) &&  
                (strcasecmp(h5traceBuffer.traces[i].chan,
                                  generictraceData.traces[k].chan) == 0) &&
                (strcasecmp(h5traceBuffer.traces[i].loc,
                                  generictraceData.traces[k].loc)  == 0)) 
            {
                map[i] = k;
                if (generictraceData.traces[i].npts > 0)
                {
                    lhaveData[i] = true;
                }
                goto NEXT_TRACE;
            }
        }
NEXT_TRACE:;
           /*
           printf("setData: %s.%s.%s.%s map[%d]=%d\n",
               h5traceBuffer.traces[i].netw,
               h5traceBuffer.traces[i].stnm,
               h5traceBuffer.traces[i].chan,
               h5traceBuffer.traces[i].loc,
               i, map[i]);
           */
    }

    if (debug) {
        LOG_DEBUGMSG("%s", "setData: Done making map for HDF5 to tracebuffer2");
        LOG_DEBUGMSG("setData: Looping over %d sampling groups", h5traceBuffer.ndtGroups);
    }

    for (idt=0; idt<h5traceBuffer.ndtGroups; idt++)
    {
        gains = NULL;
        work = NULL;
        k1 = h5traceBuffer.dtPtr[idt];
        k2 = h5traceBuffer.dtPtr[idt+1];
        ntraces = k2 - k1;
        //printf("MTH: idt=%d ntraces=%d\n", idt, ntraces);
        if (ntraces == 0){continue;}
        // Open + read the data and attributes for this dataset 
        if (debug) {
            LOG_DEBUGMSG("Opening fileID: %ld, dtGroupName[%d]: '%s'",
                (long) h5traceBuffer.fileID,
                idt,
                h5traceBuffer.dtGroupName[idt]);
        }
        groupID = H5Gopen2(h5traceBuffer.fileID,
                           h5traceBuffer.dtGroupName[idt], H5P_DEFAULT);
        gains = memory_calloc64f(ntraces);
        work = traceBuffer_h5_readData(groupID, ntraces,
                                       &maxpts, &dt, &ts1, &ts2, gains, &ierr);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error reading data");
            return -1; 
        }
        // Check what i just read
        if (dt <= 0.0)
        {
            LOG_ERRMSG("Invalid sampling period %f", dt);
            return -1; 
        }
        if (maxpts < 1)
        {
            LOG_ERRMSG("Invalid number of points %d", maxpts);
            return -1; 
        }
        if (currentTime < ts2)
        {
            LOG_ERRMSG("Update time is less than most recent time %f %f",
                       currentTime, ts2);
            return -1;
        }
        // Copy the old traces onto the new traces
        //dwork = array_set64f(maxpts*ntraces, (double) NAN, &ierr);
        // MTH: quick hack to prevent mem leak on line 205
        dwork = array_set64f((maxpts+1)*ntraces, (double) NAN, &ierr);
        //dwork = array_set64f((maxpts+1)*ntraces, (double) NAN, &ierr);
        ishift = (int) ((currentTime - ts2)/dt + 0.5);
        ncopy = maxpts - ishift;
        LOG_DEBUGMSG("setData: currentTime:%f - ts2:%f = ishift=%d", currentTime, ts2, ishift);
        for (k=0; k<ntraces; k++)
        {
            indx = k*maxpts + ishift;
            jndx = k*maxpts;
            ierr = array_copy64f_work(ncopy, &work[indx], &dwork[jndx]);
            if (ierr != 0)
            {
                LOG_ERRMSG("Error copying trace %d %d", k+1, ntraces);
                return -1;
            }
        }
        // Update the times
        ts2 = currentTime;
        ts1 = currentTime - (double) (maxpts - 1)*dt;
        // Insert the new data
        for (k=k1; k<k2; k++)
        {
            if (!lhaveData[k]){continue;}
            i = map[k]; // points to generictrace
            nchunks = generictraceData.traces[i].nchunks;
            c1 = generictraceData.traces[i].chunkPtr[0]; 
            c2 = generictraceData.traces[i].chunkPtr[nchunks];
            if (c2 - c1 <= 0 || c2 - c1 != generictraceData.traces[i].npts)
            {
                LOG_ERRMSG("npts to update is invalid %d %d %d",
                           c1, c2, generictraceData.traces[i].npts);
                LOG_DEBUGMSG("setData: i=%d generictraceData %s.%s npts:%d nchunks:%d chunkPtr[0]=%d chunkPtr[nchunks]=%d (c2-c1) != npts!",
                        i, generictraceData.traces[i].stnm, generictraceData.traces[i].chan, generictraceData.traces[i].npts, generictraceData.traces[i].nchunks, c1, c2);
                return -1;
            }
            for (chunk=0; chunk<nchunks; chunk++)
            {
                i1 = generictraceData.traces[i].chunkPtr[chunk];
                i2 = generictraceData.traces[i].chunkPtr[chunk+1];
                for (is=i1; is<i2; is++)
                {
                    // data expired
                    if (generictraceData.traces[i].times[is] < ts1){
                      continue;
                    }
                    // insert it
                    indx = k*maxpts
                         + (int) ((generictraceData.traces[i].times[is] - ts1)/dt + 0.5);
                    //printf("k=%d indx=%d set dwork[indx]\n", k, indx);

                    dwork[indx] = (double) generictraceData.traces[i].data[is];
                    // MTH: The current data point is always getting inserted to the same indx as
                    // the buffer moves along, so does ts1, ts2, generictraceData[i].times[is], maintaining their offset

                    if (debug) {
                        LOG_DEBUGMSG("setData: ts2:%.2f is:%d %s.%s.%s.%s chk:%d indx0:%d ts1:%.2f msg t:%.3f =idx:%4d set dwork[%4d]:%8.2f dwork[%4d]:%8.2f",
                            ts2,
                            is,
                            generictraceData.traces[i].netw, generictraceData.traces[i].stnm,
                            generictraceData.traces[i].chan, generictraceData.traces[i].loc,
                            chunk,
                            (k*maxpts),
                            ts1,
                            generictraceData.traces[i].times[is],
                            indx,
                            indx,
                            dwork[indx],
                            indx-1,
                            dwork[indx-1]);
                      }

                } // for is
            } // Loop on data chunks 
        } // Loop on waveforms in this group
        // Write the new dataset
        dims[0] = (hsize_t) ntraces;
        dims[1] = (hsize_t) maxpts;
        dataSpace = H5Screate_simple(2, dims, NULL);
        dataSet = H5Dopen(groupID, "Data\0", H5P_DEFAULT); 
        status = H5Dwrite(dataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                          H5P_DEFAULT, dwork);
        if (status < 0)
        {
            LOG_ERRMSG("%s", "Error writing data chunk");
            return -1;
        }
        attribute = H5Aopen(dataSet, "StartTime\0", H5P_DEFAULT);
        status = H5Awrite(attribute, H5T_NATIVE_DOUBLE, &ts1);
        if (status < 0)
        {
            LOG_ERRMSG("%s", "Error updating start time");
            return -1;
        }
        status = H5Aclose(attribute);
        status = H5Sclose(dataSpace);
        status = H5Dclose(dataSet);
        status = H5Gclose(groupID);
        if (status < 0)
        {
            LOG_ERRMSG("Error closing group %s",
                       h5traceBuffer.dtGroupName[idt]);
            return -1; 
        }
        // free memory
        memory_free64f(&gains);
        memory_free64f(&work); 
        memory_free64f(&dwork);
    }

    if (debug) {
        LOG_DEBUGMSG("%s", "Ending h5_setData");
    }
    // Free memory
    memory_free32i(&map);
    memory_free8l(&lhaveData);
    return ierrAll;
}
