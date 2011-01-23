
########### sources ###############

SET(cd-dnd2share_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-dnd2share.c
	applet-dnd2share.h
	applet-backend-uppix.c
	applet-backend-uppix.h
	applet-backend-imagebin.c
	applet-backend-imagebin.h
	applet-backend-imageshack.c
	applet-backend-imageshack.h
	applet-backend-free.c
	applet-backend-free.h
	applet-backend-pastebin.c
	applet-backend-pastebin.h
	applet-backend-custom.c
	applet-backend-custom.h
	applet-backend-dropbox.c
	applet-backend-dropbox.h
	applet-backend-ubuntuone.c
	applet-backend-ubuntuone.h
	applet-backend-videobin.c
	applet-backend-videobin.h
	applet-backend-paste-ubuntu.c
	applet-backend-paste-ubuntu.h
	applet-backend-pastebin-mozilla.c
	applet-backend-pastebin-mozilla.h
	applet-backend-codepad.c
	applet-backend-codepad.h
	applet-struct.h
)

add_library(${PACKAGE_DND2SHARE} SHARED ${cd-dnd2share_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${dnd2sharedatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="dnd2share.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="dnd2share")
add_definitions (-DMY_APPLET_VERSION="${VERSION_DND2SHARE}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_DND2SHARE}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_DND2SHARE}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_DND2SHARE} DESTINATION ${pluginsdir})

