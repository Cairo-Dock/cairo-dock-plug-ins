
SET(impl_SRCS
	indicator-applet3.c			indicator-applet3.h
	indicator-applet3-utils.c	indicator-applet3-utils.h
)

add_library(indicator-applet3 STATIC ${impl_SRCS})

# Needed for amd64 architecture
add_definitions (-fPIC)
add_definitions (-DINDICATOR_DIR="${INDICATORDIR}")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")

set (EXTRAS_LIBRARY_DIRS ${IDO_LIBRARY_DIRS})
set (EXTRAS_INCLUDE_DIRS ${IDO_INCLUDE_DIRS})
set (EXTRAS_LIBRARIES    ${IDO_LIBRARIES})

link_directories(
	${PACKAGE_LIBRARY_DIRS}
	${EXTRAS_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS})

include_directories(
	${PACKAGE_INCLUDE_DIRS}
	${EXTRAS_INCLUDE_DIRS}
	${INDICATOR_APPLET_INCLUDE_DIRS})

target_link_libraries (indicator-applet3
	${PACKAGE_LIBRARIES}
	${EXTRAS_LIBRARIES}
	${INDICATOR_APPLET_LIBRARIES})

########### install files ###############
