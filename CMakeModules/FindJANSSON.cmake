# include(FindLibraryWithDebug)
#if (NOT JANSSON_INCLUDE_DIR AND JANSSON_LIBRARY)
if (JANSSON_INCLUDE_DIR AND JANSSON_LIBRARY)
  set(JANSSON_FIND_QUIETLY TRUE)
endif (JANSSON_INCLUDE_DIR AND JANSSON_LIBRARY)
find_path(JANSSON_INCLUDE_DIR
    NAMES jansson.h
    HINTS /usr/include /usr/local/include $ENV{JANSSON_DIR}/include
  )
  find_library(JANSSON_LIBRARY
    NAMES jansson
    HINTS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 $ENV{JANSSON_DIR}/lib
  )
  if (JANSSON_INCLUDE_DIR)
    message("Found JANSSON include")
  endif (JANSSON_INCLUDE_DIR)
  if (JANSSON_LIBRARY)
    message("Found JANSSON library")
  endif (JANSSON_LIBRARY)
  include(FindPackageHandleStandardArgs)
#endif()
find_package_handle_standard_args(JANSSON DEFAULT_MSG
  JANSSON_INCLUDE_DIR JANSSON_LIBRARY)

mark_as_advanced(JANSSON_INCLUDE_DIR JANSSON_LIBRARY)
