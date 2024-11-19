cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(GFAST)
ENABLE_LANGUAGE(C)
ENABLE_LANGUAGE(CXX)
ENABLE_TESTING()
SET(GFAST_VERSION_MAJOR 1)
SET(GFAST_VERSION_MINOR 2)
SET(GFAST_VERSION_PATCH 6)
SET(GFAST_VERSION ${GFAST_VERSION_MAJOR}.${GFAST_VERSION_MINOR}.${GFAST_VERSION_PATCH})
IF ("${GFAST_INSTANCE}" STREQUAL "")
  SET(GFAST_INSTANCE "DEFAULT")
ENDIF()
MESSAGE("Configuring GFAST version: " ${GFAST_VERSION} " for " ${GFAST_INSTANCE}) 

# Add modules for finding requisites
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
SET(CMAKE_TESTING_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests)
SET(CMAKE_BINARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# For testing
if (GFAST_USE_INTEL)
  MESSAGE("Using Intel Libraries")
  #ADD_DEFINITIONS(-DGFAST_USE_INTEL=1)
  find_package(MKL REQUIRED)
  find_package(IPP REQUIRED)
else()
  find_package(CBLAS REQUIRED)
  find_package(LAPACKE REQUIRED)
  find_package(FFTW3 REQUIRED)
endif (GFAST_USE_INTEL)

if (GFAST_USE_AMQ)
  MESSAGE("Will build GFAST ActiveMQ library")
  find_package(AMQ REQUIRED)
endif (GFAST_USE_AMQ)

if (GFAST_USE_EW)
  MESSAGE("Will use Earthworm")
  find_package(EW REQUIRED)
endif (GFAST_USE_EW)
if (UW_AMAZON)
  MESSAGE("Will use Jansson and libpng for UW/Amazon output")
  find_package(JANSSON REQUIRED)
endif (UW_AMAZON)
find_package(INIPARSER REQUIRED)
find_package(ISCL REQUIRED)
#find_package(GEOLIB REQUIRED)
find_package(H5 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(XML2 REQUIRED)

# Bring in headers
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${LIBXML2_INCLUDE_DIR}
  ${COMPEARTH_INCLUDE_DIR}
  ${H5_C_INCLUDE_DIR}
  ${INIPARSER_INCLUDE_DIR}
  ${ISCL_INCLUDE_DIR}
)
# ${CBLAS_INCLUDE_DIR}

IF (GFAST_USE_INTEL)
INCLUDE_DIRECTORIES(
  ${MKL_INCLUDE_DIR}
)
ELSE ()
INCLUDE_DIRECTORIES(
  ${CBLAS_INCLUDE_DIR}
  ${LAPACKE_INCLUDE_DIR}
)
ENDIF()

ADD_SUBDIRECTORY(include)
#ADD_SUBDIRECTORY(src/core)
SET(SRCS_CORE
    src/core/cmt/decomposeMomentTensor.c src/core/cmt/depthGridSearch.c
    src/core/cmt/finalize.c src/core/cmt/gridSearch.c src/core/cmt/initialize.c
    src/core/cmt/readIni.c src/core/cmt/setDiagonalWeightMatrix.c
    src/core/cmt/setForwardModel.c src/core/cmt/setRHS.c
    src/core/cmt/weightForwardModel.c src/core/cmt/weightObservations.c
    src/core/coordtools/ll2utm.c src/core/coordtools/utm2ll.c
    src/core/data/finalize.c src/core/data/initialize.c
    src/core/data/readMetaDataFile.c src/core/data/readSiteMaskFile.c
    src/core/events/freeEvents.c src/core/events/getMinOriginTime.c
    src/core/events/newEvent.c src/core/events/printEvent.c
    src/core/events/removeCancelledEvent.c src/core/events/removeExpiredEvent.c
    src/core/events/removeExpiredEvents.c src/core/events/updateEvent.c
    src/core/events/syncXMLStatusWithEvents.c
    src/core/ff/faultPlaneGridSearch.c src/core/ff/finalize.c src/core/ff/initialize.c
    src/core/ff/meshFaultPlane.c src/core/ff/readIni.c
    src/core/ff/setDiagonalWeightMatrix.c src/core/ff/setForwardModel.c
    src/core/ff/setRegularizer.c src/core/ff/setRHS.c src/core/ff/weightForwardModel.c
    src/core/ff/weightObservations.c
    src/core/log/log.c
    src/core/properties/finalize.c src/core/properties/initialize.c
    src/core/properties/print.c src/core/scaling/pgd_depthGridSearch.c
    src/core/scaling/pgd_finalize.c src/core/scaling/pgd_initialize.c
    src/core/scaling/pgd_gridSearch.c src/core/scaling/pgd_readIni.c
    src/core/scaling/pgd_setDiagonalWeightMatrix.c src/core/scaling/pgd_setForwardModel.c
    src/core/scaling/pgd_setRHS.c src/core/scaling/pgd_weightForwardModel.c
    src/core/scaling/pgd_sanityChecks.c 
    src/core/scaling/pgd_weightObservations.c 
    src/core/scaling/readSigmaLookupFile.c 
    src/core/scaling/readPgdThresholdLookupFile.c 
    src/core/scaling/readRawSigmaThresholdLookupFile.c
    src/core/waveformProcessor/offset.c src/core/waveformProcessor/peakDisplacement.c
    src/core/waveformProcessor/peakDisplacementHelper.c 
    src/core/waveformProcessor/parseQChannel.c
)
#ADD_SUBDIRECTORY(src/eewUtils)
SET(SRCS_EEW src/eewUtils/driveCMT.c src/eewUtils/driveFF.c src/eewUtils/driveGFAST.c
             src/eewUtils/drivePGD.c src/eewUtils/makeXML.c src/eewUtils/parseCoreXML.c
             src/eewUtils/setLogFileNames.c src/eewUtils/fillCoreEventInfo.c
             src/eewUtils/sendXMLFilter.c)
#ADD_SUBDIRECTORY(src/traceBuffer)
SET(SRCS_H5TB src/traceBuffer/h5/copyTraceBufferToGFAST.c src/traceBuffer/h5/finalize.c
              src/traceBuffer/h5/getData.c src/traceBuffer/h5/getDoubleArray.c
              src/traceBuffer/h5/getScalars.c src/traceBuffer/h5/initialize.c
              src/traceBuffer/h5/readData.c src/traceBuffer/h5/setData.c
              src/traceBuffer/h5/setFileName.c src/traceBuffer/h5/setScalars.c
              src/traceBuffer/h5/setTraceBufferFromGFAST.c)
IF (EW_FOUND)
   SET(SRCS_H5TB ${SRCS_H5TB}
       src/traceBuffer/ewrr/classifyRetval.c src/traceBuffer/ewrr/finalize.c
       src/traceBuffer/ewrr/flushRing.c src/traceBuffer/ewrr/freetb2Data.c
       src/traceBuffer/ewrr/freetb2Trace.c src/traceBuffer/ewrr/getMessagesFromRing.c
       src/traceBuffer/ewrr/initialize.c src/traceBuffer/ewrr/settb2Data.c
       src/traceBuffer/ewrr/unpackTraceBuf2Messages.c
       src/traceBuffer/ewrr/settb2DataFromGFAST.c src/traceBuffer/ewrr/tb2_hash.c
       #src/traceBuffer/ewrr/swap.c # This may die in subsequent versions of EW
       )
ENDIF()
#ADD_SUBDIRECTORY(src/xml)
SET(SRCS_XML src/xml/shakeAlert/coreInfo.c src/xml/shakeAlert/segment.c
             src/xml/shakeAlert/slip.c src/xml/shakeAlert/vertex.c
             src/xml/shakeAlert/vertices.c src/xml/shakeAlert/geometry.c
             src/xml/quakeML/depth.c src/xml/quakeML/epoch2string.c
             src/xml/quakeML/focalMechanism.c src/xml/quakeML/latitude.c
             src/xml/quakeML/longitude.c src/xml/quakeML/magnitude.c
             src/xml/quakeML/momentTensor.c src/xml/quakeML/nodalPlanes.c
             src/xml/quakeML/origin.c src/xml/quakeML/principalAxes.c
             src/xml/quakeML/tensor.c src/xml/quakeML/time.c src/xml/quakeML/units.c)
#ADD_SUBDIRECTORY(src/hdf5)
SET(SRCS_HDF5 src/hdf5/copy.c src/hdf5/createType.c src/hdf5/h5_cinter.c
              src/hdf5/getMaxGroupNumber.c src/hdf5/initialize.c
              src/hdf5/memory.c src/hdf5/setFileName.c src/hdf5/update.c)
#ADD_SUBDIRECTORY(unit_tests)
SET(SRCS_UT unit_tests/cmt.c unit_tests/coord.c unit_tests/ff.c
            unit_tests/pgd.c unit_tests/readCoreInfo.c unit_tests/tests.c)

# Have GFAST use ActiveMQ
IF (GFAST_USE_AMQ)
   INCLUDE_DIRECTORIES(${LIBAMQ_INCLUDE_DIR} ${APR_INCLUDE_DIR})
   SET(SRCS_AMQ src/activeMQ/activeMQ.c src/activeMQ/readIni.c
                src/activeMQ/activeMQ.cpp src/activeMQ/consumer.cpp
                src/activeMQ/producer.cpp)
   #ADD_SUBDIRECTORY(src/activeMQ)
ENDIF (GFAST_USE_AMQ)
# Have GFAST use Earthworm
IF (GFAST_USE_EW)
   #ADD_DEFINITIONS(-DUSE_EW=1)
   INCLUDE_DIRECTORIES(${EW_INCLUDE_DIR})
ELSE()
   SET(GFAST_USE_EW OFF)
ENDIF (GFAST_USE_EW)
# Have GFAST propagate results to pnsn.org
IF (UW_AMAZON)
   #ADD_SUBDIRECTORY(src/uw)
   INCLUDE_DIRECTORIES(${JANSSON_INCLUDE_DIR})
   SET(SRCS_GFAST2WEB src/uw/beachball.c)
ENDIF (UW_AMAZON)

# Add earthworm build flags - the earthworm `way' is to modify the system's OS flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EW_BUILD_FLAGS}" CACHE STRING "" FORCE)

ADD_LIBRARY(gfast_shared SHARED ${SRCS_CORE} ${SRCS_XML} ${SRCS_AMQ} ${SRCS_EEW} ${SRCS_HDF5} ${SRCS_H5TB})
ADD_LIBRARY(gfast_static STATIC ${SRCS_CORE} ${SRCS_XML} ${SRCS_AMQ} ${SRCS_EEW} ${SRCS_HDF5} ${SRCS_H5TB})

#ADD_EXECUTABLE(gfast_playback src/gfast_playback.c ${SRCS_EEW})
IF (GFAST_USE_EW)
ADD_EXECUTABLE(gfast_eew src/gfast_eew.c ${SRCS_EEW})
ENDIF()

# These are UW development files and kind of clunky
IF (UW_AMAZON)
   #ADD_EXECUTABLE(gfast2web src/uw/gfast2web.c src/uw/beachball.c)
   ADD_EXECUTABLE(gfast2web src/uw/gfast2web.c ${SRCS_GFAST2WEB})
   #ADD_EXECUTABLE(gfast2web src/uw/gfast2web.c src/uw/beachball.c src/uw/gfast2json.c)
   SET_TARGET_PROPERTIES(gfast2web PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
   #ADD_SUBDIRECTORY(debug)
   #SET(DEBUG_SRC src/debug/fakeQueue.c)
   #ADD_EXECUTABLE(gfast_debug src/debug/debug.c ${DEBUG_SRC})
   #SET_TARGET_PROPERTIES(gfast_debug PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
   #ADD_EXECUTABLE(fakeQuake src/debug/fakeQuake.c ${DEBUG_SRC})
   #SET_TARGET_PROPERTIES(fakeQuake PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
ENDIF (UW_AMAZON)

# Let CMake know where the binaries will live 
#SET_TARGET_PROPERTIES(gfast_playback PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
IF (GFAST_USE_EW)
SET_TARGET_PROPERTIES(gfast_eew PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
ENDIF()

# Unit tests
ADD_EXECUTABLE(xcoreTests ${SRCS_UT})
SET_TARGET_PROPERTIES(xcoreTests
                      PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TESTING_OUTPUT_DIRECTORY})

# Set the libraries
SET(LIB_BASE
    ${ISCL_LIBRARY} ${COMPEARTH_LIBRARY} ${INIPARSER_LIBRARY}
    ${H5_LIBRARY} ${LIBXML2_LIBRARY})
# Add MKL
IF (GFAST_USE_INTEL)
   SET(LIB_BASE ${LIB_BASE}
       -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group ${IPP_LIBRARY})
ELSE()
   SET(LIB_BASE ${LIB_BASE}
       ${LAPACKE_LIBRARY} ${LAPACK_LIBRARY} ${CBLAS_LIBRARY} ${BLAS_LIBRARY}
       ${FFTW3_LIBRARY})
ENDIF()
# Add Earthworm -> problem with fPIC and sort.o -> EW's build system is awful.
#IF (GFAST_USE_EW)
#   SET(LIB_BASE ${LIB_BASE} ${EW_LIBRARY})
#ENDIF(GFAST_USE_EW)
# Add ActiveMQ
IF (GFAST_USE_AMQ)
   SET(LIB_BASE ${LIB_BASE} ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY})
ENDIF()
# Add some extra libraries - there must be a better way to do this
SET(LIB_BASE ${LIB_BASE} -lm -ldl -lpthread)
# Set the libraries to target link executables with
IF (GFAST_USE_EW)
   SET(LIB_ALL ${EW_LIBRARY} ${LIB_BASE})
ELSE()
   SET(LIB_ALL ${LIB_BASE})
ENDIF()
# Target link the shared libraries
TARGET_LINK_LIBRARIES(gfast_shared ${LIB_BASE} ${EW_LIBRARY})
TARGET_LINK_LIBRARIES(gfast_static ${LIB_BASE})
# Target link the executables
IF (GFAST_USE_EW)
TARGET_LINK_LIBRARIES(gfast_eew      gfast_shared ${LIB_ALL} -lhdf5)
ENDIF()
#TARGET_LINK_LIBRARIES(gfast_playback gfast_shared ${LIB_ALL})
TARGET_LINK_LIBRARIES(xcoreTests     gfast_shared ${LIB_ALL})
IF (UW_AMAZON)
   TARGET_LINK_LIBRARIES(gfast2web   gfast_shared ${JANSSON_LIBRARY} ${LIB_ALL} -lcurl -lpng -lhdf5)
   #TARGET_LINK_LIBRARIES(gfast_debug gfast_shared ${JANSSON_LIBRARY} ${LIB_ALL} -lcurl -lpng)
   #TARGET_LINK_LIBRARIES(fakeQuake   gfast_shared ${JANSSON_LIBRARY} ${LIB_ALL} -lcurl -lpng)
ENDIF()

#IF (GFAST_USE_INTEL)
add_test(NAME coreTest 
         WORKING_DIRECTORY ${CMAKE_TESTING_OUTPUT_DIRECTORY}
         COMMAND xcoreTests)
#ENDIF ()

#------------------------------------------------------------------------------#
#                                   CPACK Packaging                            #
#------------------------------------------------------------------------------#
SET(CPACK_PACKAGE_NAME "GFAST")
SET(CPACK_PACKAGE_VENDOR "PNSN")
SET(CPACK_PACKAGE_CONTACT "crowellb@uw.edu")
SET(CPACK_PACKAGE_LICENSE "GNU Public License 3")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Geodetic first approximation of earthquake size and timing")
SET(CPACK_PACKAGE_VERSION_MAJOR ${GFAST_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${GFAST_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${GFAST_VERSION_PATCH})
#IF (WIN32 AND NOT UNIX)

#ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_GENERATOR "TGZ")
  SET(CPACK_SOURCE_GENERATOR TGZ)
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "gfast-${GFAST_VERSION}" )
  #SET(CPACK_SOURCE_IGNORE_FILES ~$ .svn ${CPACK_SOURCE_IGNORE_FILES} )
#ENDIF(WIN32 AND NOT UNIX)
INCLUDE(CPack)

#------------------------------------------------------------------------------#
#                              Configuration Details                           #
#------------------------------------------------------------------------------#
SET(GFAST_LIBRARY iscl)
SET(INCLUDE_INSTALL_DIR include/) #${INCLUDE_DIRECTORIES}) #$include/ ... CACHE)
SET(LIB_INSTALL_DIR lib/)
SET(SYSCONFIG_INSTALL_DIR etc/gfast/)
#SET(LIB_INSTALL_DIR ${LIBRARY_OUTPUT_PATH})
INCLUDE(CMakePackageConfigHelpers)
CONFIGURE_PACKAGE_CONFIG_FILE(${CMAKE_MODULE_PATH}/FindGFAST.cmake.in
                              ${CMAKE_BINARY_DIR}/FindGFAST.cmake
                              INSTALL_DESTINATION ${LIB_INSTALL_DIR}/gfast/cmake
                              PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
WRITE_BASIC_PACKAGE_VERSION_FILE(${LIBRARY_OUTPUT_PATH}/GFASTConfigVersion.cmake #gfast-config.version.cmake
                                 #${CMAKE_BINARY_DIR}/gfast-config-version.cmake
                                 VERSION ${GFAST_VERSION} 
                                 COMPATIBILITY SameMajorVersion) #ExactVersion)
SET(TARGET_NAMES gfast_shared gfast_static)
INSTALL(TARGETS ${TARGET_NAMES}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_EXECUTE WORLD_WRITE WORLD_READ
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
INSTALL(FILES
        ${CMAKE_BINARY_DIR}/FindGFAST.cmake
        ${LIBRARY_OUTPUT_PATH}/GFASTConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/gfast/cmake)
