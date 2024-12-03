# include(FindLibraryWithDebug)
#if (NOT KAFKA_INCLUDE_DIR AND KAFKA_LIBRARY)
if (KAFKA_INCLUDE_DIR AND KAFKA_LIBRARY)
	set(KAFKA_FIND_QUIETLY TRUE)
endif (KAFKA_INCLUDE_DIR AND KAFKA_LIBRARY)
find_path(KAFKA_INCLUDE_DIR
    NAMES rdkafka.h
    HINTS /usr/include /usr/local/include $ENV{KAFKA_DIR}/include
  )
  find_library(KAFKA_LIBRARY
    NAMES rdkafka
    HINTS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 $ENV{KAFKA_DIR}/lib
  )
  if (KAFKA_INCLUDE_DIR)
	  message("Found KAFKA include")
  endif (KAFKA_INCLUDE_DIR)
  if (KAFKA_LIBRARY)
	  message("Found KAFKA library")
  endif (KAFKA_LIBRARY)
  include(FindPackageHandleStandardArgs)
#endif()
find_package_handle_standard_args(KAFKA DEFAULT_MSG
	KAFKA_INCLUDE_DIR KAFKA_LIBRARY)

mark_as_advanced(KAFKA_INCLUDE_DIR KAFKA_LIBRARY)
