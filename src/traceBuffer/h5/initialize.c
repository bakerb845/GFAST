#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <hdf5.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/os/os.h"
/*!
 * @brief Initializes the HDF5 file for archiving the acquisition or
 *        reading in playback mode. 
 *
 * @param[in] job               If job = 1 then the file will be opened as
 *                              read only.
 *                              If job = 2 then the file will be opened as
 *                              read/write.
 * @param[in] linMemory         If true then keep the opened file in memory.
 * @param[in] h5dir             directory where HDF5 file exists
 * @param[in] h5file            name of the HDF5 file in the directory
 *
 * @param[inout] h5TraceBuffer  On input, contains the names of the HDF5
 *                              dataset names
 *                              On output, contains the HDF5 file handle
 *                              from which we'll read and write.
 *
 * @result 0 indicates success
 * 
 */
int traceBuffer_h5_initialize(const int job,
                              const bool linMemory,
                              const char *h5dir,
                              const char *h5file,
                              struct h5traceBuffer_struct *h5traceBuffer)
{
    const char *fcnm = "traceBuffer_h5_initialize\0";
    FILE *fp;
    char h5name[PATH_MAX];
    herr_t status;
    hid_t groupID, properties;
    int i, ierr;
    size_t blockSize;
    // Make sure there is data
    if (h5traceBuffer->traces == NULL || h5traceBuffer->ntraces < 1)
    {
        log_errorF("%s: Input traces do not exist\n", fcnm);
        return -1;
    }
    // Set the filename
    ierr = traceBuffer_h5_setFileName(h5dir, h5file, h5name);
    if (ierr != 0)
    {
        log_errorF("%s: Errror setting the HDF5 filename\n", fcnm);
        return -1;
    }
    // In this instance the file is simply opened for reading
    if (job == 1)
    {
        // Get the file size
        if (!os_path_isfile(h5file))
        {
            log_errorF("%s: Error HDF5 file does %s not exist!\n",
                       fcnm, h5file);
            return -1;
        }
        // Get the size of the file
        fp = fopen(h5file, "rb\0");
        fseek(fp, 0L, SEEK_END);
        blockSize = ftell(fp);
        fclose(fp);
        // Open the file
        properties = H5Pcreate(H5P_FILE_ACCESS);
        if (linMemory)
        {
            status = H5Pset_fapl_core(properties, blockSize, false);
            if (status < 0)
            {
                log_errorF("%s: Error setting properties list\n", fcnm);
                return -1; 
            }
        }
        h5traceBuffer->fileID = H5Fopen(h5name, H5F_ACC_RDONLY, properties);
        status = H5Pclose(properties);
        if (status < 0)
        {
            log_errorF("%s: Error closing the properties list\n", fcnm);
            return -1;
        }
        // Verify the group is there
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            if (H5Lexists(h5traceBuffer->fileID,
                          h5traceBuffer->traces[i].groupName,
                          H5P_DEFAULT) != 1)
            {
                log_errorF("%s: Error couldn't find group: %s\n", fcnm,
                           h5traceBuffer->traces[i].groupName);
                return -1;
            }
        }
    }
    // Otherwise the file size must be estimated and opened for writing
    else
    {
        // Get the file size
        if (os_path_isfile(h5file))
        {
            log_warnF("%s: Deleting file %s\n", fcnm, h5file);
            return -1; 
        }
        blockSize = 0;
        // Space estimate
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            if (h5traceBuffer->traces[i].maxpts <= 0)
            {
                if (h5traceBuffer->traces[i].maxpts < 0)
                {
                    log_warnF("%s: maxpts cannot be negative\n", fcnm); 
                }
                else
                {
                    log_warnF("%s: maxpts is 0 for trace %d\n", fcnm, i+1);
                }
            }
            blockSize = blockSize
                      + 8*2*h5traceBuffer->traces[i].maxpts
                      + 8*3 + 4;
        }
        blockSize = (int) (double) (blockSize*1.1 + 0.5); // add a little extra
        properties = H5Pcreate(H5P_FILE_ACCESS); 
        status = H5Pset_fapl_core(properties, blockSize, false);
        if (status < 0)
        {
            log_errorF("%s: Error setting properties list\n", fcnm);
            return -1; 
        }
        if (linMemory)
        {
            h5traceBuffer->fileID = H5Fcreate(h5name, H5F_ACC_TRUNC,
                                              H5P_DEFAULT, properties);
        }
        else
        {
            h5traceBuffer->fileID = H5Fcreate(h5name, H5F_ACC_TRUNC,
                                              H5P_DEFAULT, H5P_DEFAULT); 
        }
        status = H5Pclose(properties);
        if (status < 0)
        {
            log_errorF("%s: Error closing the properties list\n", fcnm);
            return -1; 
        }
        // Make the groups
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            groupID = H5Gcreate2(h5traceBuffer->fileID,
                                 h5traceBuffer->traces[i].groupName,
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            status = H5Gclose(groupID);
        }
    }
    h5traceBuffer->linit = true;
    return 0;
}
