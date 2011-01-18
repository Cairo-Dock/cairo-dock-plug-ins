
SET(impl_SRCS
	indicator-applet.c			indicator-applet.h
)

add_library(indicator-applet STATIC ${impl_SRCS})

# Needed for amd64 architecture
add_definitions (-fPIC)

link_directories(
	${PACKAGE_LIBRARY_DIRS}
	${DBUSMENU_LIBRARY_DIRS}
	${DBUSMENU_GTK_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS})

include_directories(
	${PACKAGE_INCLUDE_DIRS}
	${DBUSMENU_INCLUDE_DIRS}
	${DBUSMENU_GTK_LIBRARY_DIRS}
	${INDICATOR_APPLET_INCLUDE_DIRS})

########### install files ###############
