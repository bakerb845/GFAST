#ifndef GFAST_ENUM_H
#define GFAST_ENUM_H 1

enum dtinit_type
{
    INIT_DT_FROM_DEFAULT = 1,  /*!< Sets GPS sampling period to default */
    INIT_DT_FROM_FILE = 2,     /*!< Obtains GPS sampling period from file */
    INIT_DT_FROM_TRACEBUF = 3, /*!< Obtains GPS sampling period from 
                                    Earthworm tracebuf */
    INIT_DT_FROM_SAC = 4       /*!< Obtains GPS sampling period from 
                                    SAC file */
};

enum locinit_type
{
    INIT_LOCS_FROM_FILE = 1,     /*!< Sets the GPS site locations from 
                                      SOPAC SECTOR web service file */
    INIT_LOCS_FROM_TRACEBUF = 2, /*!< Sets the GPS site locations from
                                      Earthworm tracbuf */
    INIT_LOCS_FROM_SAC = 3       /*!< Sets the GPS site locations from
                                      SAC file */
};

enum opmode_type
{
    REAL_TIME_EEW = 1,  /*!< GFAST is running in real time mode for
                             earthquake early warning */
    //REAL_TIME_PTWC = 2, /*!< GFAST is running in real time mode for PTWC */
    //REAL_TIME_ATWC = 3, /*!< GFAST is running in real time mode for ATWC */
    PLAYBACK = 21,      /*!< GFAST is running in historical playback mode */
    OFFLINE = 31        /*!< GFAST is running offline and obtaining data 
                             and configuration purely from files */
};

enum acquisition_type
{
    DATA_FROM_EARTHWORM = 1, /*!< GFAST will acquire data from earthworm */
    DATA_FROM_H5 = 2         /*!< GFAST will read data from disk */
};

enum pgd_return_enum
{
    PGD_SUCCESS = 0,           /*!< PGD computation was successful */
    PGD_STRUCT_ERROR = 1,      /*!< PGD structure is invalid */
    PGD_PD_DATA_ERROR = 2,     /*!< PGD data structure invalid */
    PGD_INSUFFICIENT_DATA = 3, /*!< Insufficient data to invert */
    PGD_COMPUTE_ERROR = 4      /*!< An internal error was encountered */
};

enum cmt_return_enum
{
    CMT_SUCCESS = 0,            /*!< CMT computation was successful */
    CMT_STRUCT_ERROR = 1,       /*!< CMT structure is invalid */
    CMT_OS_DATA_ERROR = 2,      /*!< CMT offset data structure invalid */
    CMT_INSUFFICIENT_DATA = 3,  /*!< Insufficient data to invert */
    CMT_COMPUTE_ERROR = 4       /*!< An internal error was encountered */
};

enum ff_return_enum
{
    FF_SUCCESS = 0,            /*!< FF computation was successful */
    FF_STRUCT_ERROR = 1,       /*!< FF structure is invalid */
    FF_OS_DATA_ERROR = 2,      /*!< FF offset data structure is invalid */
    FF_INSUFFICIENT_DATA = 3,  /*!< Insufficient data to invert */
    FF_COMPUTE_ERROR = 4,      /*!< An internal error was encountered */
    FF_MEMORY_ERROR = 5        /*!< Error during memory allocation */
};

enum alert_units_enum
{
    UNKNOWN_UNITS = 0,    /*!< No units defined */
    DEGREES = 1,          /*!< Distance/location - degrees */
    KILOMETERS = 2,       /*!< Distance - kilometers */
    METERS = 3,           /*!< Distance - meters */
    SECONDS = 4,          /*!< Time - seconds */
    UTC = 5,              /*!< Time - UTC */
    MOMENT_MAGNITUDE = 6, /*!< Moment magnitude Mw */
    DYNE_CENTIMETERS = 7, /*!< Torque - dyne centimeters */
    NEWTON_METERS= 8      /*!< Torque - Newton meters */
};

#endif /* _gfast_enum__ */
