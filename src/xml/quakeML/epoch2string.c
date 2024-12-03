#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_xml.h"
#include "gfast_core.h"
#include "iscl/iscl/iscl_enum.h"
#include "iscl/time/time.h"
/*!
 * @brief Converts the epochal time (s) to YYYY-MM-DDThh:mm:ss.sssZ
 *        format.
 *
 * @param[in] epoch    Epochal time (UTC seconds) to convert to string format.
 *
 * @param[out] cepoch  Corresponding string formatted time.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker (ISTI)
 *
 */
int xml_epoch2string(double epoch, char cepoch[128])
{
    double sec;
    int ierr, mday, month, nzhour, nzjday, nzmin, nzmusec, nzsec, nzyear;
    memset(cepoch, 0, 128*sizeof(char));
    ierr = time_epoch2calendar(epoch,
                                &nzyear, &nzjday, &month, &mday,
                                &nzhour, &nzmin, &nzsec, &nzmusec);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error converting epochal time");
        return -1;
    }
    sec = (double) nzsec + (double) nzmusec*1.e-6;
    sprintf(cepoch,
            "%04d-%02d-%02dT%02d:%02d:%06.3fZ",
            nzyear, month, mday, nzhour, nzmin, sec);
    return 0;
}
