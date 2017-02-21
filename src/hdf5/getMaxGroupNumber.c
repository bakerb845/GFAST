#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_hdf5.h"
#include "iscl/log/log.h"

#define MAX_GROUP 10000000
int hdf5_getMaxGroupNumber(const hid_t h5fl)
{
    const char *fcnm = "hdf5_getMaxGroupNumber\0";
    char groupName[512];
    int kg, kgroup;
    kgroup =-1;
    for (kg=0; kg<MAX_GROUP; kg++)
    {
        memset(groupName, 0, 512*sizeof(char));
        sprintf(groupName, "/GFAST_History/Iteration_%d", kg+1);
        if (H5Lexists(h5fl, groupName, H5P_DEFAULT) == 0)
        {
            kgroup = kg;
            break;
        }
    }
    if (kgroup ==-1)
    {
        log_errorF("%s: No output groups\n", fcnm);
    }
    return kgroup;
}
