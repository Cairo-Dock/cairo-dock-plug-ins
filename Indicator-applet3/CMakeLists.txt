
SET(impl_SRCS
	indicator-applet3.c			indicator-applet3.h
	indicator-applet3-utils.c	indicator-applet3-utils.h
)

add_library(indicator-applet3 STATIC ${impl_SRCS})

# Needed for amd64 architecture
add_definitions (-fPIC)
add_definitions (-DINDICATOR_DIR="${INDICATORDIR}")

link_directories(
	${PACKAGE_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS})

include_directories(
	${PACKAGE_INCLUDE_DIRS}
	${INDICATOR_APPLET_INCLUDE_DIRS})

########### install files ###############
