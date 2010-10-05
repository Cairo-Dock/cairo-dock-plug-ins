# - Try to find the sensors directory library
# Once done this will define
#
#  SENSORS_FOUND - system has SENSORS
#  SENSORS_INCLUDE_DIR - the SENSORS include directory
#  SENSORS_LIBRARIES - The libraries needed to use SENSORS
#
# taken from kde-base (kdebase-workspace: /cmake/modules)

FIND_PATH(SENSORS_INCLUDE_DIR sensors/sensors.h)

FIND_LIBRARY(SENSORS_LIBRARIES NAMES sensors)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sensors DEFAULT_MSG SENSORS_INCLUDE_DIR SENSORS_LIBRARIES )

#MARK_AS_ADVANCED(SENSORS_INCLUDE_DIR SENSORS_LIBRARIES)

if (NOT "${SENSORS_FOUND}" STREQUAL "TRUE")  # si no ntrouves, ils sont a NOTFOUND, ce qui est idiot.
	set (SENSORS_INCLUDE_DIR "")
	set (SENSORS_LIBRARY_DIRS "")
	set (SENSORS_LIBRARIES "")
endif()
