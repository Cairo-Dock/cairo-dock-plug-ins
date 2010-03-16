
########### sources ###############

SET(cd-mail_LIB_SRCS
	cd-mail-applet-init.c
	cd-mail-applet-init.h
	cd-mail-applet-config.c
	cd-mail-applet-config.h
	cd-mail-applet-notifications.c
	cd-mail-applet-notifications.h
	cd-mail-applet-struct.h
	cd-mail-applet-etpan.c
	cd-mail-applet-etpan.h
	cd-mail-applet-accounts.c
	cd-mail-applet-accounts.h
)

add_library(${PACKAGE_MAIL} SHARED ${cd-mail_LIB_SRCS})

########### compil ###############
add_definitions (-D_POSIX_SOURCE=1 -DHAVE_SSL_SUPPORT=1)
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(maildatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="mail.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="mail")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_MAIL)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_MAIL)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="cd_mail_icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${MAIL_PACKAGE_CFLAGS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})
add_definitions (${MAIL_PACKAGE_LIBS})
target_link_libraries (${PACKAGE_MAIL}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_MAIL} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-mail.la
#
#
#libcd_mail_la_SOURCES = 
#	cd-mail-applet-init.c
#	cd-mail-applet-init.h
#	cd-mail-applet-config.c
#	cd-mail-applet-config.h
#	cd-mail-applet-notifications.c
#	cd-mail-applet-notifications.h
#	cd-mail-applet-struct.h
#	cd-mail-applet-etpan.c
#	cd-mail-applet-etpan.h
#	cd-mail-applet-accounts.c
#	cd-mail-applet-accounts.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_mail_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	$(MAIL_PACKAGE_CFLAGS)
#	-D_POSIX_SOURCE=1 -DHAVE_SSL_SUPPORT=1
#	-DMY_APPLET_SHARE_DATA_DIR=""$(maildatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""mail.conf""
#	-DMY_APPLET_USER_DATA_DIR=""mail""
#	-DMY_APPLET_VERSION=""$(VERSION_MAIL)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_MAIL)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""cd_mail_icon.png""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-DGL_GLEXT_PROTOTYPES=""1""
#	-std=c99
#	-Werror-implicit-function-declaration
#	-g -ggdb -W
##	-fgnu89-inline
#
#
#libcd_mail_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm 
#	$(MAIL_PACKAGE_LIBS)
#
#
#libcd_mail_la_LDFLAGS =
#	-avoid-version -module
