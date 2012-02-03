
SET(impl_SRCS
	indicator-applet.c			indicator-applet.h
)

add_library(indicator-applet STATIC ${impl_SRCS})

# Needed for amd64 architecture
add_definitions (-fPIC)
add_definitions (-DINDICATOR_ICONS_DIR="${INDICATORICONSDIR}")
if (DBUSMENU_GTK3_NEW)
	add_definitions (-DDBUSMENU_GTK3_NEW=1)
endif()

link_directories(
	${PACKAGE_LIBRARY_DIRS}
	${DBUSMENU_LIBRARY_DIRS}
	${DBUSMENU_GTK_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS})

include_directories(
	${PACKAGE_INCLUDE_DIRS}
	${DBUSMENU_INCLUDE_DIRS}
	${DBUSMENU_GTK_INCLUDE_DIRS}
	${INDICATOR_APPLET_INCLUDE_DIRS})

########### install files ###############
