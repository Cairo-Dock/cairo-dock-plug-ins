
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-notifications.c 		applet-notifications.h
)

# 12.10
if (INDICATOR_MESSAGES_WITH_IND3)
	list (APPEND MODULE_SRCS applet-indicator3.c)
else ()
	list (APPEND MODULE_SRCS applet-messaging.c applet-menu.c)
endif()

add_library(${PACKAGE_MESSAGING_MENU} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${messaging_menudatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Messaging-Menu.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Messaging-Menu")
add_definitions (-DMY_APPLET_VERSION="${VERSION_MESSAGING_MENU}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_MESSAGING_MENU}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DINDICATOR_OLD_NAMES=${INDICATOR_OLD_NAMES})
add_definitions (-DINDICATOR_MESSAGES_HAS_LOZENGE=${INDICATOR_MESSAGES_HAS_LOZENGE})
### uncomment the following line to allow multi-instance applet.
#add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
### uncomment the following line to allow extended OpenGL drawing.
#add_definitions (-DGL_GLEXT_PROTOTYPES="1")
if (${DBUSMENU_GTK3_NEW})
	add_definitions (-DDBUSMENU_GTK3_NEW=1)
endif()

if (INDICATOR_MESSAGES_WITH_IND3)
	message (STATUS "   With Indicator-Applet3")
	add_definitions (-DINDICATOR_MESSAGES_WITH_IND3=1)
	set (EXTRAS_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/Indicator-applet3)
	set (EXTRAS_LIBRARY_DIRS ${CMAKE_SOURCE_DIR}/Indicator-applet3)
	set (EXTRAS_LIBRARIES indicator-applet3)
else()
	set (EXTRAS_INCLUDE_DIRS ${DBUSMENU_INCLUDE_DIRS} ${DBUSMENU_GTK_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/Indicator-applet)
	set (EXTRAS_LIBRARY_DIRS ${DBUSMENU_LIBRARY_DIRS} ${DBUSMENU_GTK_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/Indicator-applet)
	set (EXTRAS_LIBRARIES ${DBUSMENU_LIBRARIES} ${DBUSMENU_GTK_LIBRARIES} indicator-applet)
endif()

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${INDICATOR_APPLET_INCLUDE_DIRS}
	${EXTRAS_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS}
	${EXTRAS_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_MESSAGING_MENU}
	${PACKAGE_LIBRARIES}
	${INDICATOR_APPLET_LIBRARIES}
	${EXTRAS_LIBRARIES})
########### install files ###############

install(TARGETS ${PACKAGE_MESSAGING_MENU} DESTINATION ${pluginsdir})
