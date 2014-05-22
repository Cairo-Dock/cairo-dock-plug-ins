
########### sources ###############

SET(cd-powermanager_LIB_SRCS
	powermanager-struct.h
	powermanager-init.c		powermanager-init.h
	powermanager-draw.c		powermanager-draw.h
	powermanager-config.c		powermanager-config.h
	powermanager-common.c		powermanager-common.h
	powermanager-upower.c		powermanager-upower.h
	powermanager-proc-acpi.c	powermanager-proc-acpi.h
	powermanager-sys-class.c	powermanager-sys-class.h
	powermanager-menu-functions.c	powermanager-menu-functions.h
)

add_library(${PACKAGE_POWERMANAGER} SHARED ${cd-powermanager_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${powermanagerdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="powermanager.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="powermanager")
add_definitions (-DMY_APPLET_VERSION="${VERSION_POWERMANAGER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_POWERMANAGER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
if (${UPOWER_FOUND})
	add_definitions (-DCD_UPOWER_AVAILABLE=1)
	if (NOT UPOWER_SUPPORTS_SUSPEND_HIBERNATE)
		add_definitions (-DCD_UPOWER_0_99=1)
	endif()
endif()

include_directories (
	${UPOWER_INCLUDE_DIRS}
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${UPOWER_LIBRARY_DIRS}
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_POWERMANAGER}
	${UPOWER_LIBRARIES}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_POWERMANAGER} DESTINATION ${pluginsdir})
