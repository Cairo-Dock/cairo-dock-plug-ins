
########### sources ###############

SET (cd-Dbus_LIB_SRCS
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-dbus.c 				applet-dbus.h
	interface-main-methods.c 	interface-main-methods.h
	interface-main-query.c 		interface-main-query.h
	interface-applet-object.c 	interface-applet-object.h
	interface-applet-methods.c 	interface-applet-methods.h
	interface-applet-signals.c 	interface-applet-signals.h
	applet-struct.h
)

add_library (${PACKAGE_DBUS} SHARED ${cd-Dbus_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${dbusdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="Dbus.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Dbus")
add_definitions (-DMY_APPLET_VERSION="${VERSION_DBUS}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_DBUS}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DCD_PLUGINS_DIR="${pluginsdir}")

IF (${CMAKE_SYSTEM_NAME} MATCHES "^FreeBSD")
	set(kvm "kvm")
endif()

if (DBUSMENU_FOUND AND DBUSMENU_GTK_FOUND)
	message (STATUS " * With DBusMenu support")
	add_definitions (-DDBUSMENU_GTK_FOUND=1)

	include_directories (
		${DBUSMENU_INCLUDE_DIRS}
		${DBUSMENU_GTK_INCLUDE_DIRS}
		${PACKAGE_INCLUDE_DIRS})

	link_directories (
		${DBUSMENU_LIBRARY_DIRS}
		${DBUSMENU_GTK_LIBRARY_DIRS}
		${PACKAGE_LIBRARY_DIRS})

	target_link_libraries (${PACKAGE_DBUS}
		${DBUSMENU_LIBRARIES}
		${DBUSMENU_GTK_LIBRARIES}
		${PACKAGE_LIBRARIES}
		${kvm})
else()
	include_directories (
		${PACKAGE_INCLUDE_DIRS})

	link_directories (
		${PACKAGE_LIBRARY_DIRS})

	target_link_libraries (${PACKAGE_DBUS}
		${PACKAGE_LIBRARIES}
		${kvm})
endif()

########### install files ###############

install (TARGETS ${PACKAGE_DBUS} DESTINATION ${pluginsdir})
