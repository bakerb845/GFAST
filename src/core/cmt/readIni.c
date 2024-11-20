#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include <iniparser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_core.h"
#include "iscl/os/os.h"

static void setVarName(const char *group, const char *variable,
                       char *var)
{
    memset(var, 0, 256*sizeof(char));
    sprintf(var, "%s:%s", group, variable);
    return;
}
/*!
 * @brief Reads the CMT properties from the initialization file.
 *
 * @param[in] propfilename   Name of properties file.
 * @param[in] group          Group in ini file.  Likely "CMT".
 * @param[in] verbose        This is the verbosity from the general
 *                           parameters.
 * @param[in] utm_zone       This is the default UTM zone from the
 *                           general parameters.
 *
 * @param[out] cmt_props     CMT scaling properties.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_cmt_readIni(const char *propfilename,
                    const char *group,
                    const int verbose, const int utm_zone,
                    struct GFAST_cmt_props_struct *cmt_props)
{
    char var[256];
    int ierr;
    dictionary *ini;
    ierr = 1;
    memset(cmt_props, 0, sizeof(struct  GFAST_cmt_props_struct));
    if (!os_path_isfile(propfilename))
    {
        LOG_ERRMSG("Properties file: %s does not exist", propfilename);
        return ierr;
    }
    ini = iniparser_load(propfilename);
    // Read the properties
    cmt_props->verbose = verbose;
    cmt_props->utm_zone = utm_zone;
    setVarName(group, "do_cmt\0", var);
    cmt_props->do_cmt = iniparser_getboolean(ini, var, true);
    setVarName(group, "deltaLatitude\0", var);
    cmt_props->dLat = iniparser_getdouble(ini, var, 0.1);
    if (cmt_props->dLat < 0.0)
    {   
        LOG_ERRMSG("Error CMT latitude search delta %f must be positive",
                   cmt_props->dLat);
        goto ERROR;
    }
    setVarName(group, "deltaLongitude\0", var);
    cmt_props->dLon = iniparser_getdouble(ini, var, 0.1);
    if (cmt_props->dLon < 0.0)
    {   
        LOG_ERRMSG("Error CMT longitude search delta %f must be positive",
                   cmt_props->dLon);
        goto ERROR;
    }
    setVarName(group, "nlats_in_cmt_gridSearch\0", var);
    cmt_props->ngridSearch_lats = iniparser_getint(ini, var, 1);
    if (cmt_props->ngridSearch_lats < 1)
    {
        LOG_ERRMSG("Error CMT grid search lats %d must be positive",
                   cmt_props->ngridSearch_lats);
        goto ERROR;
    }
    if (cmt_props->ngridSearch_lats%2 == 0)
    {
        LOG_WARNMSG("%s", "Adding 1 point to CMT lat gridsearch");
        cmt_props->ngridSearch_lats = cmt_props->ngridSearch_lats + 1;
    }
    setVarName(group, "nlons_in_cmt_gridSearch\0", var);
    cmt_props->ngridSearch_lons = iniparser_getint(ini, var, 1);
    if (cmt_props->ngridSearch_lons < 1)
    {
        LOG_ERRMSG("Error CMT grid search lons %d must be positive",
                   cmt_props->ngridSearch_lons);
        goto ERROR;
    }
    if (cmt_props->ngridSearch_lons%2 == 0)
    {
        LOG_WARNMSG("%s", "Adding 1 point to CMT lon gridsearch");
        cmt_props->ngridSearch_lons = cmt_props->ngridSearch_lons + 1;
    }
    setVarName(group, "ndepths_in_cmt_gridSearch\0", var);
    cmt_props->ngridSearch_deps = iniparser_getint(ini, var, 100);
    if (cmt_props->ngridSearch_deps < 1)
    {
        LOG_ERRMSG("Error CMT grid search depths %d must be positive",
                   cmt_props->ngridSearch_deps);
        goto ERROR;
    }
    setVarName(group, "cmt_min_sites\0", var);
    cmt_props->min_sites = iniparser_getint(ini, var, 4);
    if (cmt_props->min_sites < 3)
    {
        LOG_ERRMSG("%s", "Error at least two sites needed to estimate CMT!");
        goto ERROR;
    }
    setVarName(group, "cmt_window_vel\0", var);
    cmt_props->window_vel = iniparser_getdouble(ini, var, 2.0);
    if (cmt_props->window_vel <= 0.0)
    {
        LOG_ERRMSG("%s", "Error window velocity must be positive!");
        goto ERROR;
    }
    setVarName(group, "cmt_window_avg\0", var);
    cmt_props->window_avg = iniparser_getdouble(ini, var, 0.0);
    if (cmt_props->window_avg < 0.0)
    {   
        LOG_ERRMSG("%s", "Error window average time must be positive!");
        goto ERROR;
    }
    setVarName(group, "ldeviatoric_cmt\0", var);
    cmt_props->ldeviatoric
        = iniparser_getboolean(ini, var, true);
    if (!cmt_props->ldeviatoric)
    {
        LOG_ERRMSG("%s", "Error general CMT inversions not yet programmed");
        goto ERROR;
    }
    ierr = 0;
ERROR:;
    iniparser_freedict(ini);
    return ierr;
}
