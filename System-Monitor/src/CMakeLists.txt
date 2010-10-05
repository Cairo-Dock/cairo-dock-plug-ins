
########### sources ###############

SET (cd-System-Monitor_LIB_SRCS
	applet-init.c 		applet-init.h
	applet-config.c 		applet-config.h
	applet-cpusage.c 		applet-cpusage.h
	applet-rame.c 		applet-rame.h
	applet-nvidia.c 		applet-nvidia.h
	applet-top.c 		applet-top.h
	applet-monitor.c 		applet-monitor.h
	applet-notifications.c 	applet-notifications.h
	applet-struct.h
)
message (STATUS "SENSORS_FOUND : ${SENSORS_FOUND}")

if ("${SENSORS_FOUND}" STREQUAL "TRUE")
	SET (cd-System-Monitor_SENSORS_SRCS
		applet-sensors.c 		applet-sensors.h)
endif()
add_library (${PACKAGE_SYSTEM_MONITOR} SHARED
		${cd-System-Monitor_LIB_SRCS}
		${cd-System-Monitor_SENSORS_SRCS})
########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${system_monitordatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="System-Monitor.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="System-Monitor")
add_definitions (-DMY_APPLET_VERSION="${VERSION_SYSTEM_MONITOR}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_SYSTEM_MONITOR}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")

if ("${SENSORS_FOUND}" STREQUAL "TRUE")
	add_definitions (-DHAVE_SENSORS="1")
endif()

include_directories (${PACKAGE_INCLUDE_DIRS}
	${SENSORS_INCLUDE_DIR})

link_directories (${PACKAGE_LIBRARY_DIRS}
	${SENSORS_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_SYSTEM_MONITOR}
	${PACKAGE_LIBRARIES}
	${SENSORS_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_SYSTEM_MONITOR} DESTINATION ${pluginsdir})
