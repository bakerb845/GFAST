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
 * @brief Reads the finite fault properties from the initialization file.
 *
 * @param[in] propfilename   Name of properties file.
 * @param[in] group          Group in ini file.  Likely "FF".
 * @param[in] cmtMinSites    Minimum number of sites in CMT inversion.
 * @param[in] verbose        This is the verbosity from the general
 *                           parameters.
 * @param[in] utm_zone       This is the default UTM zone from the
 *                           general parameters.
 *
 * @param[out] ff_props      Finite fault properties.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_ff_readIni(const char *propfilename,
                    const char *group,
                    const int verbose, const int utm_zone,
                    const int cmtMinSites,
                    struct GFAST_ff_props_struct *ff_props)
{
    char var[256];
    int ierr;
    dictionary *ini;
    ierr = 1;
    memset(ff_props, 0, sizeof(struct  GFAST_ff_props_struct));
    if (!os_path_isfile(propfilename))
    {
        LOG_ERRMSG("Properties file: %s does not exist", propfilename);
        return ierr;
    }
    ini = iniparser_load(propfilename);
    // Read the properties
    ff_props->verbose = verbose;
    ff_props->utm_zone = utm_zone;
    setVarName(group, "do_ff\0", var);
    ff_props->do_ff = iniparser_getboolean(ini, var, true);
    setVarName(group, "ff_number_of_faultplanes\0", var);
    ff_props->nfp = iniparser_getint(ini, var, 2);
    if (ff_props->nfp != 2)
    {
        LOG_ERRMSG("%s", "Error only 2 fault planes considered in ff");
        goto ERROR;
    }
    setVarName(group, "ff_nstr\0", var);
    ff_props->nstr = iniparser_getint(ini, var, 10);
    if (ff_props->nstr < 1)
    {
        LOG_ERRMSG("%s", "Error no fault patches along strike!");
        goto ERROR;
    }
    setVarName(group, "ff_ndip\0", var);
    ff_props->ndip = iniparser_getint(ini, var, 5);
    if (ff_props->ndip < 1)
    {
        LOG_ERRMSG("%s", "Error no fault patches down dip!");
        goto ERROR;
    }
    setVarName(group, "ff_min_sites\0", var);
    ff_props->min_sites = iniparser_getint(ini, var, 4);
    if (ff_props->min_sites < cmtMinSites)
    {
        LOG_ERRMSG("%s", "Error FF needs at least as many sites as CMT");
        goto ERROR;
    }
    setVarName(group, "ff_window_vel\0", var);
    ff_props->window_vel = iniparser_getdouble(ini, var, 3.0);
    if (ff_props->window_vel <= 0.0)
    {
        LOG_ERRMSG("%s", "Error window velocity must be positive!");
        goto ERROR;
    }
    setVarName(group, "ff_window_avg\0", var);
    ff_props->window_avg = iniparser_getdouble(ini, var, 10.0);
    if (ff_props->window_avg <= 0.0)
    {
        LOG_ERRMSG("%s", "Error window average time must be positive!");
        goto ERROR;
    }
    setVarName(group, "ff_flen_pct\0", var);
    ff_props->flen_pct = iniparser_getdouble(ini, var, 10.0);
    if (ff_props->flen_pct < 0.0)
    {
        LOG_ERRMSG("%s", "Error cannot shrink fault length");
        goto ERROR;
    }
    setVarName(group, "ff_fwid_pct\0", var);
    ff_props->fwid_pct = iniparser_getdouble(ini, var, 10.0);
    if (ff_props->fwid_pct < 0.0)
    {
        LOG_ERRMSG("%s", "Error cannot shrink fault width");
        goto ERROR;
    }
    setVarName(group, "ff_pct_large_m\0", var);
    ff_props->large_m = iniparser_getdouble(ini, var, 10.0);
    setVarName(group, "ff_flen_pct_large\0", var);
    ff_props->flen_pct_large = iniparser_getdouble(ini, var, 40.0);
    if (ff_props->flen_pct_large < 0.0)
    {
        LOG_ERRMSG("%s", "Error cannot shrink fault length");
        goto ERROR;
    }
    setVarName(group, "ff_fwid_pct_large\0", var);
    ff_props->fwid_pct_large = iniparser_getdouble(ini, var, 40.0);
    if (ff_props->fwid_pct_large < 0.0)
    {
        LOG_ERRMSG("%s", "Error cannot shrink fault width");
        goto ERROR;
    }
    ierr = 0;
    ERROR:;
    iniparser_freedict(ini);
    return ierr;
}
