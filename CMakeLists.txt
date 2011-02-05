########### requirements ###############

cmake_minimum_required (VERSION 2.6)
find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)

########### project ###############

project ("cairo-dock-plugins")
set (VERSION "2.3.0~0beta1")

add_definitions (-std=c99 -Wstrict-prototypes -Wextra -Wwrite-strings -Wuninitialized -Werror-implicit-function-declaration) #-Wunreachable-code -Wno-unused-parameter -Wall
if (NOT ${CMAKE_BUILD_TYPE})
	add_definitions (-O3)
endif()
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

############ sources tarball #############

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
set (CPACK_SOURCE_IGNORE_FILES 
	"/build/;/.bzr/;bzrignore$;/misc/;~$;${CPACK_SOURCE_IGNORE_FILES}")
include (CPack)

add_custom_target( dist
	COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
add_custom_target(dist-bzr
	COMMAND bzr export ${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

############ uninstall #############

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)
add_custom_target (uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

########### global variables ###############

if( WIN32 )
	message(FATAL_ERROR "Cairo-Dock requires an air-conditioned room. Please close Windows!")
endif( WIN32 )

set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE ${PACKAGE})

# get plug-ins install dir
execute_process(
    COMMAND pkg-config cairo-dock --variable=pluginsdir		# /usr/lib/cairo-dock
    OUTPUT_VARIABLE pluginsdir)
STRING (REGEX REPLACE "\n" "" pluginsdir ${pluginsdir})  # la commande rajoute un retour chariot ...
# get plug-ins data dir
execute_process(
    COMMAND pkg-config cairo-dock --variable=pluginsdatadir	# /usr/share/cairo-dock/plug-ins
    OUTPUT_VARIABLE pluginsdatadir)
STRING (REGEX REPLACE "\n" "" pluginsdatadir ${pluginsdatadir})
# check that version matches with the core
execute_process(
    COMMAND pkg-config --modversion cairo-dock			# 2.2.0-3
    OUTPUT_VARIABLE dock_version)
STRING (REGEX REPLACE "\n" "" dock_version ${dock_version})
if (NOT "${dock_version}" STREQUAL "${VERSION}")		# Version
	MESSAGE (FATAL_ERROR "warning : version mismatch with the core : " ${VERSION} <> ${dock_version})
endif()

#if( CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT "${FORCE_LIB64}" STREQUAL "")  # 64bits and force install in lib64
#	set (libname "lib64")
#else()
#	set (libname "lib${LIB_SUFFIX}")
#endif()
#set (libdir "${CMAKE_INSTALL_PREFIX}/${libname}/cairo-dock")	# /usr/lib

# check that installation dir matches with the core
GET_FILENAME_COMPONENT(libdir "${pluginsdir}/.." ABSOLUTE)  # /usr/lib
GET_FILENAME_COMPONENT(prefix "${pluginsdir}/../.." ABSOLUTE)  # /usr
if (NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "${prefix}")
	message (STATUS "Warning : plug-ins should be installed in the same directory as the core, that is to say in ${pluginsdir}")
	set (CMAKE_INSTALL_PREFIX "${prefix}")
	#set (libdir "${CMAKE_INSTALL_PREFIX}/${libname}/cairo-dock")
endif()

# set internationalisation
set (GETTEXT_PLUGINS "cairo-dock-plugins")
set (localedir "${CMAKE_INSTALL_PREFIX}/share/locale")
set (gaugesdir "${CMAKE_INSTALL_PREFIX}/share/cairo-dock/gauges")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")  # additionnal FindPackage files

########### dependancies ###############

message ("")
message (STATUS "=====================")
message (STATUS "Check dependancies...")
message (STATUS "=====================")
message ("")

pkg_check_modules ("PACKAGE" REQUIRED "gtk+-2.0" "gthread-2.0" "cairo" "librsvg-2.0" "dbus-1" "dbus-glib-1" "libxml-2.0" "gtkglext-1.0" "cairo-dock")

############# ALSA_MIXER #################
set (with_alsa "no")
if (NOT "${enable-alsa-mixer}" STREQUAL "no")
	pkg_check_modules (ALSA_MIXER_PACKAGE alsa)
	if ("${ALSA_MIXER_PACKAGE_FOUND}" STREQUAL "")
		message (STATUS "Could not find alsa; Cairo-Dock won't be built with AlsaMixer applet.")
	else()
		set (GETTEXT_ALSA_MIXER ${GETTEXT_PLUGINS})
		set (VERSION_ALSA_MIXER "1.0.12")
		set (PACKAGE_ALSA_MIXER "cd-AlsaMixer")
		set (with_alsa "yes")
		set (alsa_mixerdatadir "${pluginsdatadir}/AlsaMixer")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/alsaMixer/data/AlsaMixer.conf.in ${CMAKE_CURRENT_BINARY_DIR}/alsaMixer/data/AlsaMixer.conf)
		add_subdirectory ("alsaMixer")
	endif()
endif()

############# ANIMATED ICONS #################
set (GETTEXT_ANIMATED_ICONS ${GETTEXT_PLUGINS})
set (VERSION_ANIMATED_ICONS "1.0.9")
set (PACKAGE_ANIMATED_ICONS "cd-Animated-icons")
set (animated_iconsdatadir "${pluginsdatadir}/Animated-icons")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Animated-icons/data/Animated-icons.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Animated-icons/data/Animated-icons.conf)
add_subdirectory (Animated-icons)

############# CAIRO_PENGUIN #################
set (GETTEXT_CAIRO_PENGUIN ${GETTEXT_PLUGINS})
set (VERSION_CAIRO_PENGUIN "1.1.8")
set (PACKAGE_CAIRO_PENGUIN "cd-Cairo-Penguin")
set (cairo_penguinuserdirname "Cairo-Penguin")
set (cairo_penguindatadir "${pluginsdatadir}/Cairo-Penguin")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Cairo-Penguin/data/Cairo-Penguin.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Cairo-Penguin/data/Cairo-Penguin.conf)
add_subdirectory (Cairo-Penguin)

############# CLIPPER #################
set (GETTEXT_CLIPPER ${GETTEXT_PLUGINS})
set (VERSION_CLIPPER "1.1.4")
set (PACKAGE_CLIPPER "cd-Clipper")
set (Clipperdatadir "${pluginsdatadir}/Clipper")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Clipper/data/Clipper.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Clipper/data/Clipper.conf)
add_subdirectory (Clipper)

############# CLOCK #################
set (with_ical "yes")
pkg_check_modules ("LIBICAL_PACKAGE" "libical")
if ("${LIBICAL_PACKAGE_FOUND}" STREQUAL "")
	set (with_ical "no")
	message (STATUS "Could not find libical; Clock plugin won't be built with iCal support.")
endif()
set (GETTEXT_CLOCK ${GETTEXT_PLUGINS})
set (VERSION_CLOCK "2.1.2")
set (PACKAGE_CLOCK "cd-clock")
set (clockuserdirname "clock")
set (clockdatadir "${pluginsdatadir}/clock")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/clock/data/clock.conf.in ${CMAKE_CURRENT_BINARY_DIR}/clock/data/clock.conf)
add_subdirectory (clock)

############# COMPIZ_ICON #################
set (GETTEXT_COMPIZ_ICON ${GETTEXT_PLUGINS})
set (VERSION_COMPIZ_ICON "1.1.6")
set (PACKAGE_COMPIZ_ICON "cd-compiz-icon")
set (compiz_icondatadir "${pluginsdatadir}/compiz-icon")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/compiz-icon/data/compiz-icon.conf.in ${CMAKE_CURRENT_BINARY_DIR}/compiz-icon/data/compiz-icon.conf)
add_subdirectory (compiz-icon)

############# DBUS #################
find_program (PYTHON_EXECUTABLE python)
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print sys.version" OUTPUT_VARIABLE PYTHON_VERSION)
if ("${PYTHON_VERSION}" STREQUAL "") 
	message (STATUS "Could not find Python, won't install Python interface.") 
	set (with_python "no") 
else() 
	STRING (REGEX REPLACE "\n" "" PYTHON_VERSION ${PYTHON_VERSION})
	message (STATUS "Python Version: ${PYTHON_VERSION}") 
	GET_FILENAME_COMPONENT(DEBIAN_VERSION /etc/debian_version ABSOLUTE) 
	if (EXISTS ${DEBIAN_VERSION}) 
		message (STATUS "  will use '--install-layout deb' with 'python setup.py install'") 
		set (DEBIAN_INSTALL_LAYOUT "--install-layout deb")
	endif()
	set (PYTHON_FOUND "TRUE")
	set (with_python "yes") 
endif()

# find_package(Ruby) found libs of ruby-dev but we only need a directory where we can install ruby libs.
find_program (RUBY_EXECUTABLE ruby)
EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['rubylibdir']"
	OUTPUT_VARIABLE RUBY_LIB_DIR)
message (STATUS "RUBY_LIBRARY: ${RUBY_LIB_DIR}")
if ("${RUBY_LIB_DIR}" STREQUAL "" OR "${RUBY_LIB_DIR}" STREQUAL "nil")
	message (STATUS "Could not find ruby libs, won't install Ruby interface.")
	set (with_ruby "no")
else()
	string (REGEX REPLACE "lib/ruby/[0-9].[0-9]" "" RUBY_LIB_DIR_INSTALL "${RUBY_LIB_DIR}")
	string (REGEX REPLACE "${RUBY_LIB_DIR_INSTALL}" "" RUBY_LIB_DIR_INSTALL "${RUBY_LIB_DIR}")
	set (RUBY_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${RUBY_LIB_DIR_INSTALL}")
	message (STATUS "  will be installed in: ${RUBY_LIB_DIR}")
	set (RUBY_FOUND "TRUE")
	set (with_ruby "yes")
endif()

#find_package (Mono)
find_program (GMCS_EXECUTABLE gmcs)
if ("${GMCS_EXECUTABLE}" STREQUAL "" OR NOT EXISTS ${GMCS_EXECUTABLE})
	message (STATUS "Could not find Mono compiler gmcs, won't build Mono interface.")
	set (with_mono "no")
else()
	set (MONO_FOUND "TRUE")
	pkg_check_modules (MONO_PACKAGE glib-sharp-2.0 ndesk-dbus-1.0 ndesk-dbus-glib-1.0)
	if ("${MONO_PACKAGE_FOUND}" STREQUAL "")
		message (STATUS "Could not find glib-sharp-2.0, ndesk-dbus-1.0 or ndesk-dbus-glib-1.0; won't be built Mono interface.")
		set (with_mono "no")
	else()
		set (with_mono "yes")
	endif()
endif()

find_program (VALA_EXECUTABLE valac)
if ("${VALA_EXECUTABLE}" STREQUAL "" OR  NOT EXISTS ${VALA_EXECUTABLE})
	message (STATUS "Could not find Vala compiler, won't build Vala interface.")
	set (with_vala "no")
else()
	execute_process(COMMAND ${VALA_EXECUTABLE} "--version"
		OUTPUT_VARIABLE "VALA_VERSION")
    string(REPLACE "Vala" "" "VALA_VERSION" ${VALA_VERSION})
    string(STRIP ${VALA_VERSION} "VALA_VERSION")
    message (STATUS "VALA_VERSION: ${VALA_VERSION}")
    
    STRING (REGEX REPLACE "\\..*" "" VALA_MAJOR "${VALA_VERSION}")
	message (STATUS "VALA_MAJOR : ${VALA_MAJOR}")
	
	STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" VALA_MINOR "${VALA_VERSION}")  # 0.1.7 => 1.7
	STRING (REGEX REPLACE "\\.[0-9]*" "" VALA_MINOR "${VALA_MINOR}")
	message (STATUS "VALA_MINOR : ${VALA_MINOR}")
	
	STRING (REGEX REPLACE ".*\\." "" VALA_NANO "${VALA_VERSION}")
	STRING (REGEX REPLACE "-.*" "" VALA_NANO "${VALA_NANO}")
	message (STATUS "VALA_NANO  : ${VALA_NANO}")
	
	if (${VALA_MAJOR} GREATER 0 OR ${VALA_MINOR} GREATER 9)  # vala > 0.10
		message (STATUS "Vala compiler ok.")
		set (VALA_FOUND "TRUE")
		set (with_vala "yes")
	else()
		message (STATUS "Vala compiler too old (0.10 required), won't build Vala interface.")
		set (with_vala "no")
	endif()
endif()


set (GETTEXT_DBUS ${GETTEXT_PLUGINS})
set (VERSION_DBUS "1.2.1")
set (PACKAGE_DBUS "cd-Dbus")
set (dbusdatadir "${pluginsdatadir}/Dbus")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Dbus/data/Dbus.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Dbus/data/Dbus.conf)
add_subdirectory (Dbus)

############# DESKLET_RENDERING #################
set (GETTEXT_DESKLET_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DESKLET_RENDERING "1.5.6")
set (PACKAGE_DESKLET_RENDERING "cd-desklet-rendering")
set (desklet_renderingdatadir "${pluginsdatadir}/desklet-rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/desklet-rendering/data/desklet-rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/desklet-rendering/data/desklet-rendering.conf)
add_subdirectory (desklet-rendering)

############# DIALOG_RENDERING #################
set (GETTEXT_DIALOG_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DIALOG_RENDERING "0.5.1")
set (PACKAGE_DIALOG_RENDERING "cd-dialog-rendering")
set (dialog_renderingdatadir "${pluginsdatadir}/dialog-rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dialog-rendering/data/dialog-rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dialog-rendering/data/dialog-rendering.conf)
add_subdirectory (dialog-rendering)

############# DISKS #################
set (with_disks "no")
if ("${enable-disks}" STREQUAL "yes")
	set (GETTEXT_DISKS ${GETTEXT_PLUGINS})
	set (VERSION_DISKS "0.0.2")
	set (PACKAGE_DISKS "cd-disks")
	set (with_disks "yes")
	set (disksdatadir "${pluginsdatadir}/Disks")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Disks/data/Disks.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Disks/data/Disks.conf)
	add_subdirectory ("Disks")
endif()

############# DND2SHARE #################
set (GETTEXT_DND2SHARE ${GETTEXT_PLUGINS})
set (VERSION_DND2SHARE "1.0.5")
set (PACKAGE_DND2SHARE "cd-dnd2share")
set (dnd2sharedatadir "${pluginsdatadir}/dnd2share")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dnd2share/data/dnd2share.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dnd2share/data/dnd2share.conf)
add_subdirectory (dnd2share)

############# DOCK RENDERING #################
set (GETTEXT_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_RENDERING "1.5.9")
set (PACKAGE_RENDERING "cd-rendering")
set (renderingdatadir "${pluginsdatadir}/rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dock-rendering/data/rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dock-rendering/data/rendering.conf)
add_subdirectory (dock-rendering)

############# DONCKY #################
set (with_doncky "no")
if ("${enable-doncky}" STREQUAL "yes")
	set (GETTEXT_DONCKY ${GETTEXT_PLUGINS})
	set (VERSION_DONCKY "0.0.4")
	set (PACKAGE_DONCKY "cd-doncky")
	set (with_doncky "yes")
	set (donckydatadir "${pluginsdatadir}/Doncky")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doncky/data/Doncky.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Doncky/data/Doncky.conf)
	add_subdirectory (Doncky)
endif()

############# DROP INDICATOR #################
set (GETTEXT_DROP_INDICATOR ${GETTEXT_PLUGINS})
set (VERSION_DROP_INDICATOR "1.1.4")
set (PACKAGE_DROP_INDICATOR "cd-drop_indicator")
set (drop_indicatordatadir "${pluginsdatadir}/drop-indicator")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/drop-indicator/data/drop_indicator.conf.in ${CMAKE_CURRENT_BINARY_DIR}/drop-indicator/data/drop_indicator.conf)
add_subdirectory (drop-indicator)

############# DUSTBIN #################
set (GETTEXT_DUSTBIN ${GETTEXT_PLUGINS})
set (VERSION_DUSTBIN "2.3.1")
set (PACKAGE_DUSTBIN "cd-dustbin")
set (dustbinuserdirname "dustbin")
set (dustbindatadir "${pluginsdatadir}/dustbin")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dustbin/data/dustbin.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dustbin/data/dustbin.conf)
add_subdirectory (dustbin)

############# FOLDERS #################
set (GETTEXT_FOLDERS ${GETTEXT_PLUGINS})
set (VERSION_FOLDERS "0.2.1")
set (PACKAGE_FOLDERS "cd-Folders")
set (foldersdatadir "${pluginsdatadir}/Folders")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Folders/data/Folders.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Folders/data/Folders.conf)
add_subdirectory (Folders)

############# GMENU #################
set (with_gmenu "no")
if (NOT "${enable-gmenu}" STREQUAL "no")
	pkg_check_modules (GMENU_PACKAGE libgnome-menu)
	if ("${GMENU_PACKAGE_FOUND}" STREQUAL "")
		message (STATUS "Could not find libgnome-menu; Cairo-Dock won't be built with GMenu applet.")
	else()
		set (GETTEXT_GMENU ${GETTEXT_PLUGINS})
		set (VERSION_GMENU "1.1.6")
		set (PACKAGE_GMENU "cd-GMenu")
		set (with_gmenu "yes")
		set (gmenudatadir "${pluginsdatadir}/GMenu")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/GMenu/data/GMenu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/GMenu/data/GMenu.conf)
		add_subdirectory ("GMenu")
	endif()
endif()

############# GVFS-INTEGRATION #################
pkg_check_modules ("LIBGIO" "gio-2.0")
add_subdirectory (gvfs-integration)

############# GNOME-INTEGRATION #################
set (with_gnome_integration "no")
if (NOT "${enable-gnome-integration}" STREQUAL "no")
	pkg_check_modules (GNOME_INTEGRATION gio-2.0)
	if ("${GNOME_INTEGRATION_FOUND}" STREQUAL "")
		message (STATUS "Could not find gio; Cairo-Dock won't be built with Gnome>=2.22 support.")
	else()
		set (VERSION_GNOME_INTEGRATION "1.0.3")
		set (PACKAGE_GNOME_INTEGRATION "cd_gnome-integration")
		set (with_gnome_integration "yes")
		set (gnome_integrationdatadir "${pluginsdatadir}/gnome-integration")
		add_subdirectory ("gnome-integration")
	endif()
endif()

############# GNOME-INTEGRATION-OLD #################
set (with_gnome_integration_old "no")
if ("${enable-old-gnome-integration}" STREQUAL "yes")
	pkg_check_modules (OLD_GNOME_INTEGRATION gnome-vfs-2.0 libgnomeui-2.0)
	if ("${OLD_GNOME_INTEGRATION_FOUND}" STREQUAL "")
		message (STATUS "Could not find gnome-vfs and/or gnomeui; Cairo-Dock won't be built with Gnome<2.22 support.")
	else()
		set (VERSION_GNOME_INTEGRATION_OLD "1.0.4")
		set (PACKAGE_GNOME_INTEGRATION_OLD "cd_gnome-integration-old")
		set (with_gnome_integration_old "yes")
		set (gnome_integration_olddatadir "${pluginsdatadir}/gnome-integration-old")
		add_subdirectory ("gnome-integration-old")
	endif()
endif()

############# ICON EFFECT #################
set (GETTEXT_ICON_EFFECTS ${GETTEXT_PLUGINS})
set (VERSION_ICON_EFFECTS "1.2.4")
set (PACKAGE_ICON_EFFECTS "cd-icon-effect")
set (icon_effectsdatadir "${pluginsdatadir}/icon-effect")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/icon-effect/data/icon-effect.conf.in ${CMAKE_CURRENT_BINARY_DIR}/icon-effect/data/icon-effect.conf)
add_subdirectory (icon-effect)

############# INDICATOR-APPLET #################
# Note: the names of dbusmenu-glib and dbusmenu-gtk have changed...
pkg_check_modules (DBUSMENU_04 QUIET dbusmenu-glib-0.4)
pkg_check_modules (DBUSMENU_GTK_04 QUIET dbusmenu-gtk-0.4)
pkg_check_modules (INDICATOR_APPLET indicator libido-0.1)
if ("${DBUSMENU_04_FOUND}" STREQUAL "1")
	pkg_check_modules (DBUSMENU dbusmenu-glib-0.4)
	# set (DBUSMENU_EXT "-0.4")
else()
	pkg_check_modules (DBUSMENU dbusmenu-glib)
endif()
if ("${DBUSMENU_GTK_04_FOUND}" STREQUAL "1")
	pkg_check_modules (DBUSMENU_GTK dbusmenu-gtk-0.4)
	# set (DBUSMENU_GTK_EXT "-0.4")
else()
	pkg_check_modules (DBUSMENU_GTK dbusmenu-gtk)
endif()
if ("${INDICATOR_APPLET_FOUND}" STREQUAL "1" AND "${DBUSMENU_FOUND}" STREQUAL "1" AND "${DBUSMENU_GTK_FOUND}" STREQUAL "1")
	add_subdirectory (Indicator-applet)
	set (with_indicator "yes")
endif()

############# ILLUSION #################
set (GETTEXT_ILLUSION ${GETTEXT_PLUGINS})
set (VERSION_ILLUSION "1.0.7")
set (PACKAGE_ILLUSION "cd-illusion")
set (illusiondatadir "${pluginsdatadir}/illusion")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/illusion/data/illusion.conf.in ${CMAKE_CURRENT_BINARY_DIR}/illusion/data/illusion.conf)
add_subdirectory (illusion)

############# KDE-INTEGRATION #################
set (with_kde_integration "no")
if (NOT "${enable-kde-integration}" STREQUAL "no")
	pkg_check_modules (KDE_INTEGRATION gio-2.0)
	if ("${KDE_INTEGRATION_FOUND}" STREQUAL "")
		message (STATUS "Could not find kde libs; Cairo-Dock won't be built with KDE support.")
	else()
		set (VERSION_KDE_INTEGRATION "0.0.2")
		set (PACKAGE_KDE_INTEGRATION "cd_kde-integration")
		set (with_kde_integration "yes")
		set (kde_integrationdatadir "${pluginsdatadir}/kde-integration")
		add_subdirectory ("kde-integration")
	endif()
endif()

############# KEYBOARD_INDICATOR #################
set (with_keyboard_indicator "no")
pkg_check_modules (KEYBOARD_INDICATOR_PACKAGE libxklavier)
if ("${KEYBOARD_INDICATOR_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find libxklavier; Cairo-Dock won't be built with keyboard-indicator applet.")
else()
	set (GETTEXT_KEYBOARD_INDICATOR ${GETTEXT_PLUGINS})
	set (VERSION_KEYBOARD_INDICATOR "1.1.3")
	set (PACKAGE_KEYBOARD_INDICATOR "cd-keyboard-indicator")
	set (with_keyboard_indicator "yes")
	set (keyboard_indicatordatadir "${pluginsdatadir}/keyboard-indicator")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/keyboard-indicator/data/keyboard-indicator.conf.in ${CMAKE_CURRENT_BINARY_DIR}/keyboard-indicator/data/keyboard-indicator.conf)
	add_subdirectory ("keyboard-indicator")
endif()

############# LOGOUT #################
set (GETTEXT_LOGOUT ${GETTEXT_PLUGINS})
set (VERSION_LOGOUT "1.2.10")
set (PACKAGE_LOGOUT "cd-logout")
set (logoutdatadir "${pluginsdatadir}/logout")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/logout/data/logout.conf.in ${CMAKE_CURRENT_BINARY_DIR}/logout/data/logout.conf)
add_subdirectory (logout)

############# MAIL #################
set (with_mail "no")
# find the compilation flags
execute_process(
	COMMAND libetpan-config --cflags
	OUTPUT_VARIABLE MAIL_PACKAGE_CFLAGS)
STRING (REGEX MATCHALL "-I[^ ]+ " TMP_VARIABLE "${MAIL_PACKAGE_CFLAGS}") # first extract the "-I" options
STRING (REGEX REPLACE ";" "" TMP_VARIABLE "${TMP_VARIABLE}")
STRING (REGEX REPLACE " $" "" TMP_VARIABLE "${TMP_VARIABLE}")
STRING (REGEX REPLACE "-I([^ ]+)" "\\1" MAIL_PACKAGE_INCLUDE_DIRS "${TMP_VARIABLE}")  # then remove the "-I" string
# find the link flags
execute_process(
	COMMAND libetpan-config --libs
	OUTPUT_VARIABLE MAIL_PACKAGE_LIBS)
#   find the link libraries
STRING (REGEX MATCHALL "-l[^ ]+ " TMP_VARIABLE "${MAIL_PACKAGE_LIBS}") # extract the "-l" options
STRING (REGEX REPLACE ";" "" TMP_VARIABLE "${TMP_VARIABLE}")
STRING (REGEX REPLACE " $" "" MAIL_PACKAGE_LIBRARIES "${TMP_VARIABLE}")
#   find the link directories
STRING (REGEX MATCHALL "-L[^ ]+ " TMP_VARIABLE "${MAIL_PACKAGE_LIBS}") # extract the "-L" options
STRING (REGEX REPLACE ";" "" TMP_VARIABLE "${TMP_VARIABLE}")
STRING (REGEX REPLACE " $" "" TMP_VARIABLE "${TMP_VARIABLE}")
STRING (REGEX REPLACE "-L([^ ]+)" "\\1" MAIL_PACKAGE_LIBRARY_DIRS "${TMP_VARIABLE}")  # then remove the "-L" string

if ("${MAIL_PACKAGE_LIBS}" STREQUAL "")
	message (STATUS "warning : Could not find libetpan; Cairo-Dock won't be built with Mail applet.")
else()
	message (STATUS "libetpan found. Using the following options:")
	message (STATUS "  Include directories: ${MAIL_PACKAGE_INCLUDE_DIRS}")
	message (STATUS "  Link directories: ${MAIL_PACKAGE_LIBRARY_DIRS}")
	message (STATUS "  Link libraries: ${MAIL_PACKAGE_LIBRARIES}")
	set (GETTEXT_MAIL ${GETTEXT_PLUGINS})
	set (VERSION_MAIL "1.0.8")
	set (PACKAGE_MAIL "cd-mail")
	set (maildatadir "${pluginsdatadir}/mail")
	set (with_mail "yes")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/mail/data/mail.conf.in ${CMAKE_CURRENT_BINARY_DIR}/mail/data/mail.conf)
	add_subdirectory ("mail")
endif()

############# MEMENU #################
set (with_me_menu "no")
if ("${with_indicator}" STREQUAL "yes")
	message (STATUS "DBUSMENU_VERSION : ${DBUSMENU_VERSION}")
	
	STRING (REGEX REPLACE "\\..*" "" DBUSMENU_MAJOR "${DBUSMENU_VERSION}")
	message (STATUS "DBUSMENU_MAJOR : ${DBUSMENU_MAJOR}")
	
	STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" DBUSMENU_MINOR "${DBUSMENU_VERSION}")  # 0.1.7 => 1.7
	STRING (REGEX REPLACE "\\.[0-9]*" "" DBUSMENU_MINOR "${DBUSMENU_MINOR}")
	message (STATUS "DBUSMENU_MINOR : ${DBUSMENU_MINOR}")
	
	STRING (REGEX REPLACE ".*\\." "" DBUSMENU_NANO "${DBUSMENU_VERSION}")
	STRING (REGEX REPLACE "-.*" "" DBUSMENU_NANO "${DBUSMENU_NANO}")
	message (STATUS "DBUSMENU_NANO  : ${DBUSMENU_NANO}")
	
	set (GETTEXT_MEMENU ${GETTEXT_PLUGINS})
	set (VERSION_MEMENU "1.0.1")
	set (PACKAGE_MEMENU "cd-MeMenu")
	set (memenudatadir "${pluginsdatadir}/MeMenu")
	set (with_me_menu "yes")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/MeMenu/data/MeMenu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/MeMenu/data/MeMenu.conf)
	add_subdirectory (MeMenu)
endif()

############# MESSAGING_MENU #################
set (with_messaging_menu "no")
if ("${with_indicator}" STREQUAL "yes")
	set (GETTEXT_MESSAGING_MENU ${GETTEXT_PLUGINS})
	set (VERSION_MESSAGING_MENU "1.0.1")
	set (PACKAGE_MESSAGING_MENU "cd-Messaging-Menu")
	set (with_messaging_menu "yes")
	set (messaging_menudatadir "${pluginsdatadir}/Messaging-Menu")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Messaging-Menu/data/Messaging-Menu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Messaging-Menu/data/Messaging-Menu.conf)
	add_subdirectory (Messaging-Menu)
endif()

############# MOTION BLUR #################
set (GETTEXT_MOTION_BLUR ${GETTEXT_PLUGINS})
set (VERSION_MOTION_BLUR "1.0.4")
set (PACKAGE_MOTION_BLUR "cd-motion_blur")
set (motion_blurdatadir "${pluginsdatadir}/motion-blur")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/motion-blur/data/motion_blur.conf.in ${CMAKE_CURRENT_BINARY_DIR}/motion-blur/data/motion_blur.conf)
add_subdirectory (motion-blur)

############# MUSICPLAYER #################
set (GETTEXT_MUSICPLAYER ${GETTEXT_PLUGINS})
set (VERSION_MUSICPLAYER "1.0.7")
set (PACKAGE_MUSICPLAYER "cd-musicPlayer")
set (musicplayerdatadir "${pluginsdatadir}/musicPlayer")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/musicPlayer/data/musicPlayer.conf.in ${CMAKE_CURRENT_BINARY_DIR}/musicPlayer/data/musicPlayer.conf)
add_subdirectory (musicPlayer)

############# NETSPEED #################
set (GETTEXT_NETSPEED ${GETTEXT_PLUGINS})
set (VERSION_NETSPEED "1.2.5")
set (PACKAGE_NETSPEED "cd-netspeed")
set (netspeeddatadir "${pluginsdatadir}/netspeed")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/netspeed/data/netspeed.conf.in ${CMAKE_CURRENT_BINARY_DIR}/netspeed/data/netspeed.conf)
add_subdirectory (netspeed)

############# NETWORK_MONITOR #################
set (with_network_monitor "no")
if ("${enable-network-monitor}" STREQUAL "yes")
	set (GETTEXT_NETWORK_MONITOR ${GETTEXT_PLUGINS})
	set (VERSION_NETWORK_MONITOR "0.2.3")
	set (PACKAGE_NETWORK_MONITOR "cd-network-monitor")
	set (with_network_monitor "yes")
	set (network_monitordatadir "${pluginsdatadir}/Network-Monitor")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Network-Monitor/data/Network-Monitor.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Network-Monitor/data/Network-Monitor.conf)
	add_subdirectory (Network-Monitor)
endif()

############# POWERMANAGER #################
set (GETTEXT_POWERMANAGER ${GETTEXT_PLUGINS})
set (VERSION_POWERMANAGER "1.3.4")
set (PACKAGE_POWERMANAGER "cd-powermanager")
set (powermanagerdatadir "${pluginsdatadir}/powermanager")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/powermanager/data/powermanager.conf.in ${CMAKE_CURRENT_BINARY_DIR}/powermanager/data/powermanager.conf)
add_subdirectory (powermanager)

############# QUICK BROWSER #################
set (GETTEXT_QUICK_BROWSER ${GETTEXT_PLUGINS})
set (VERSION_QUICK_BROWSER "1.0.8")
set (PACKAGE_QUICK_BROWSER "cd-quick-browser")
set (quick_browserdatadir "${pluginsdatadir}/quick_browser")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/quick-browser/data/quick-browser.conf.in ${CMAKE_CURRENT_BINARY_DIR}/quick-browser/data/quick-browser.conf)
add_subdirectory (quick-browser)

############# RECENT-EVENTS #################
set (with_recent_events "no")
if ("${enable-recent-events}" STREQUAL "yes")
	pkg_check_modules (RECENT_EVENTS zeitgeist-1.0)
	if ("${RECENT_EVENTS_FOUND}" STREQUAL "")
		message (STATUS "Could not find libzeitgeist; Cairo-Dock won't be built with Zeitgeist support.")
	else()
		set (GETTEXT_RECENT_EVENTS ${GETTEXT_PLUGINS})
		set (VERSION_RECENT_EVENTS "0.0.2")
		set (PACKAGE_RECENT_EVENTS "cd-Recent-Events")
		set (with_recent_events "yes")
		set (recent_eventsdatadir "${pluginsdatadir}/Recent-Events")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Recent-Events/data/Recent-Events.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Recent-Events/data/Recent-Events.conf)
		add_subdirectory ("Recent-Events")
	endif()
endif()

############# REMOTE_CONTROL #################
set (GETTEXT_REMOTE_CONTROL ${GETTEXT_PLUGINS})
set (VERSION_REMOTE_CONTROL "0.0.2")
set (PACKAGE_REMOTE_CONTROL "cd-Remote-Control")
set (remote_controldatadir "${pluginsdatadir}/Remote-Control")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Remote-Control/data/Remote-Control.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Remote-Control/data/Remote-Control.conf)
add_subdirectory (Remote-Control)

############# RSSREADER #################
set (GETTEXT_RSS_READER ${GETTEXT_PLUGINS})
set (VERSION_RSS_READER "1.0.3")
set (PACKAGE_RSS_READER "cd-rssreader")
set (rss_readerdatadir "${pluginsdatadir}/RSSreader")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/RSSreader/data/RSSreader.conf.in ${CMAKE_CURRENT_BINARY_DIR}/RSSreader/data/RSSreader.conf)
add_subdirectory (RSSreader)

############# SCOOBY_DO #################
set (with_scooby_do "no")
if ("${enable-scooby-do}" STREQUAL "yes")
	set (GETTEXT_SCOOBY_DO ${GETTEXT_PLUGINS})
	set (VERSION_SCOOBY_DO "0.1.1")
	set (PACKAGE_SCOOBY_DO "cd-scooby-do")
	set (with_scooby_do "yes")
	set (scooby_dodatadir "${pluginsdatadir}/Scooby-Do")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Scooby-Do/data/Scooby-Do.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Scooby-Do/data/Scooby-Do.conf)
	add_subdirectory (Scooby-Do)
endif()

############# SHORTCUTS #################
set (GETTEXT_SHORTCUTS ${GETTEXT_PLUGINS})
set (VERSION_SHORTCUTS "1.3.1")
set (PACKAGE_SHORTCUTS "cd-shortcuts")
set (shortcutsdatadir "${pluginsdatadir}/shortcuts")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/data/shortcuts.conf.in ${CMAKE_CURRENT_BINARY_DIR}/shortcuts/data/shortcuts.conf)
add_subdirectory (shortcuts)

############# SHOW DESKTOP #################
set (with_xrandr "yes")
pkg_check_modules (SHOW_DESKTOP_XRANDR xrandr)
if ("${SHOW_DESKTOP_XRANDR_FOUND}" STREQUAL "")
	message (STATUS "Could not find xrandr; ShowDesktop won't be built with screen resolution abilities.")
	set (with_xrandr "no")
endif()
set (GETTEXT_SHOW_DESKTOP ${GETTEXT_PLUGINS})
set (VERSION_SHOW_DESKTOP "1.2.3")
set (PACKAGE_SHOW_DESKTOP "cd-showDesktop")
set (show_desktopdatadir "${pluginsdatadir}/showDesktop")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/showDesktop/data/showDesktop.conf.in ${CMAKE_CURRENT_BINARY_DIR}/showDesktop/data/showDesktop.conf)
add_subdirectory (showDesktop)

############# SHOW MOUSE #################
set (GETTEXT_SHOW_MOUSE ${GETTEXT_PLUGINS})
set (VERSION_SHOW_MOUSE "1.0.4")
set (PACKAGE_SHOW_MOUSE "cd-show_mouse")
set (show_mousedatadir "${pluginsdatadir}/show_mouse")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/show-mouse/data/show_mouse.conf.in ${CMAKE_CURRENT_BINARY_DIR}/show-mouse/data/show_mouse.conf)
add_subdirectory (show-mouse)

############# SLIDER #################
set (with_exif "yes")
pkg_check_modules (EXIF libexif)
if ("${EXIF_FOUND}" STREQUAL "")
	message (STATUS "Could not find libexif; Slider won't be able to rotate the images accordingly to their exif data.")
	set (with_exif "no")
endif()
set (GETTEXT_SLIDER ${GETTEXT_PLUGINS})
set (VERSION_SLIDER "2.0.8")
set (PACKAGE_SLIDER "cd-slider")
set (sliderdatadir "${pluginsdatadir}/slider")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/slider/data/slider.conf.in ${CMAKE_CURRENT_BINARY_DIR}/slider/data/slider.conf)
add_subdirectory (slider)

############# STACK #################
set (GETTEXT_STACK ${GETTEXT_PLUGINS})
set (VERSION_STACK "0.3.1")
set (PACKAGE_STACK "cd-stack")
set (stackdatadir "${pluginsdatadir}/stack")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/stack/data/stack.conf.in ${CMAKE_CURRENT_BINARY_DIR}/stack/data/stack.conf)
add_subdirectory (stack)

############# STATUS NOTIFIER #################
set (with_status_notifier "no")
if ("${enable-status-notifier}" STREQUAL "yes")
	if ("${DBUSMENU_GTK_FOUND}" STREQUAL "")
		message (STATUS "Could not find Status-Notifier libs; Cairo-Dock won't be built with Status-Notifier support.")
	else()
		set (GETTEXT_STATUS_NOTIFIER ${GETTEXT_PLUGINS})
		set (VERSION_STATUS_NOTIFIER "0.1.1")
		set (PACKAGE_STATUS_NOTIFIER "cd-status-notifier")
		set (with_status_notifier "yes")
		set (status_notifierdatadir "${pluginsdatadir}/Status-Notifier")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Status-Notifier/data/Status-Notifier.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Status-Notifier/data/Status-Notifier.conf)
		add_subdirectory (Status-Notifier)
	endif()
endif()

############# SWITCHER #################
set (GETTEXT_SWITCHER ${GETTEXT_PLUGINS})
set (VERSION_SWITCHER "2.1.5")
set (PACKAGE_SWITCHER "cd-switcher")
set (switcherdatadir "${pluginsdatadir}/switcher")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/switcher/data/switcher.conf.in ${CMAKE_CURRENT_BINARY_DIR}/switcher/data/switcher.conf)
add_subdirectory (switcher)

############# SYSTEM MONITOR #################
set (with_sensors "yes")
find_package (Sensors)
if (NOT "${SENSORS_FOUND}" STREQUAL "TRUE")
	message (STATUS "Could not find libsensors; System-Monitor won't be able to monitor fan speed and CPU temperature.")
	set (with_sensors "no")
endif()
set (GETTEXT_SYSTEM_MONITOR ${GETTEXT_PLUGINS})
set (VERSION_SYSTEM_MONITOR "1.0.6")
set (PACKAGE_SYSTEM_MONITOR "cd-system-monitor")
set (system_monitordatadir "${pluginsdatadir}/System-monitor")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/System-Monitor/data/System-Monitor.conf.in ${CMAKE_CURRENT_BINARY_DIR}/System-Monitor/data/System-Monitor.conf)
add_subdirectory (System-Monitor)

############# SYSTRAY #################
set (GETTEXT_SYSTRAY ${GETTEXT_PLUGINS})
set (VERSION_SYSTRAY "0.2.2")
set (PACKAGE_SYSTRAY "cd-systray")
set (systraydatadir "${pluginsdatadir}/systray")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/systray/data/systray.conf.in ${CMAKE_CURRENT_BINARY_DIR}/systray/data/systray.conf)
add_subdirectory (systray)

############# TERMINAL #################
pkg_check_modules (TERMINAL_PACKAGE vte)
if ("${TERMINAL_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find libvte; Cairo-Dock won't be built with terminal applet.")
	set (with_terminal "no")
else()
	set (GETTEXT_TERMINAL ${GETTEXT_PLUGINS})
	set (VERSION_TERMINAL "1.0.10")
	set (PACKAGE_TERMINAL "cd-terminal")
	set (with_terminal "yes")
	set (terminaldatadir "${pluginsdatadir}/terminal")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/terminal/data/terminal.conf.in ${CMAKE_CURRENT_BINARY_DIR}/terminal/data/terminal.conf)
	add_subdirectory (terminal)
endif()

############# TOMBOY #################
set (GETTEXT_TOMBOY ${GETTEXT_PLUGINS})
set (VERSION_TOMBOY "1.3.1")
set (PACKAGE_TOMBOY "cd-tomboy")
set (tomboydatadir "${pluginsdatadir}/tomboy")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/tomboy/data/tomboy.conf.in ${CMAKE_CURRENT_BINARY_DIR}/tomboy/data/tomboy.conf)
add_subdirectory (tomboy)

############# TOONS #################
set (GETTEXT_TOONS ${GETTEXT_PLUGINS})
set (VERSION_TOONS "1.0.8")
set (PACKAGE_TOONS "cd-Toons")
set (toonsdatadir "${pluginsdatadir}/Toons")
set (toonsuserdirname "Toons")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Toons/data/Toons.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Toons/data/Toons.conf)
add_subdirectory (Toons)

############# WEATHER #################
set (GETTEXT_WEATHER ${GETTEXT_PLUGINS})
set (VERSION_WEATHER "1.2.10")
set (PACKAGE_WEATHER "cd-weather")
set (weatherdatadir "${pluginsdatadir}/weather")
set (weatheruserdirname "weather")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/weather/data/weather.conf.in ${CMAKE_CURRENT_BINARY_DIR}/weather/data/weather.conf)
add_subdirectory (weather)

############# WEBLETS #################
set (with_weblets "no")
if (NOT "${enable-weblets}" STREQUAL "no")
	pkg_check_modules (WEBKIT webkit-1.0)
	if ("${WEBKIT_FOUND}" STREQUAL "")
		message (STATUS "Could not find webkit-1.0; Cairo-Dock won't be built with Weblets applet.")
	else()
		set (GETTEXT_WEBLETS ${GETTEXT_PLUGINS})
		set (VERSION_WEBLETS "0.0.10")
		set (PACKAGE_WEBLETS "cd-weblets")
		set (webletsdatadir "${pluginsdatadir}/weblets")
		set (with_weblets "yes")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/weblets/data/weblets.conf.in ${CMAKE_CURRENT_BINARY_DIR}/weblets/data/weblets.conf)
		add_subdirectory ("weblets")
	endif()
endif()

############# WIFI #################
set (GETTEXT_WIFI ${GETTEXT_PLUGINS})
set (VERSION_WIFI "1.2.6")
set (PACKAGE_WIFI "cd-wifi")
set (wifidatadir "${pluginsdatadir}/wifi")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/wifi/data/wifi.conf.in ${CMAKE_CURRENT_BINARY_DIR}/wifi/data/wifi.conf)
add_subdirectory (wifi)

############# XFCE-INTEGRATION #################
set (with_xfce_integration "no")
if (NOT "${enable-xfce-integration}" STREQUAL "no")
	pkg_check_modules (XFCE_INTEGRATION_THUNAR thunar-vfs-1)
	pkg_check_modules (XFCE_INTEGRATION_GVFS gio-2.0)
	if ("${XFCE_INTEGRATION_THUNAR_FOUND}" STREQUAL "" AND "${XFCE_INTEGRATION_GVFS_FOUND}" STREQUAL "")
		message (STATUS "Could find neither thunar-vfs or libgio-2.0; Cairo-Dock won't be built with XFCE support.")
	else()
		set (VERSION_XFCE_INTEGRATION "1.0.3")
		set (PACKAGE_XFCE_INTEGRATION "cd_xfce-integration")
		set (with_xfce_integration "yes")
		set (xfce_integrationdatadir "${pluginsdatadir}/xfce-integration")
		add_subdirectory ("xfce-integration")
	endif()
endif()

############# XGAMMA #################
pkg_check_modules (XGAMMA_PACKAGE x11 xxf86vm)
if ("${XGAMMA_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find xxf86vm; Cairo-Dock won't be built with Xgamma applet.")
	set (with_xgamma "no")
else()
	set (GETTEXT_XGAMMA ${GETTEXT_PLUGINS})
	set (VERSION_XGAMMA "1.1.6")
	set (PACKAGE_XGAMMA "cd-Xgamma")
	set (with_xgamma "yes")
	set (xgammadatadir "${pluginsdatadir}/Xgamma")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Xgamma/data/Xgamma.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Xgamma/data/Xgamma.conf)
	add_subdirectory ("Xgamma")
endif()

message ("")
message (STATUS "===============")
message (STATUS "Language build:")
message (STATUS "===============")
message ("")
add_subdirectory (po)

message ("")
message (STATUS "===============")
message (STATUS "Plug-ins build:")
message (STATUS "===============")
message ("")
message (STATUS " - with Alsa-Mixer applet:         ${with_alsa}")
message (STATUS " - with iCal support:              ${with_ical}")
message (STATUS " - with Disks applet:              ${with_disks}")
message (STATUS " - with Doncky applet:             ${with_doncky}")
message (STATUS " - with GMenu applet:              ${with_gmenu}")
message (STATUS " - with Gnome support:             ${with_gnome_integration}")
message (STATUS " - with old Gnome support:         ${with_gnome_integration_old}")
message (STATUS " - with KDE support:               ${with_kde_integration}")
message (STATUS " - with XFCE support:              ${with_xfce_integration}")
message (STATUS " - with Keyboard-indicator applet: ${with_keyboard_indicator}")
message (STATUS " - with Mail applet:               ${with_mail}")
message (STATUS " - with Me-Menu applet:            ${with_me_menu}")
message (STATUS " - with Messaging-Menu:            ${with_messaging_menu}")
message (STATUS " - with Network-Monitor applet:    ${with_network_monitor}")
message (STATUS " - with Scooby-Do applet:          ${with_scooby_do}")
message (STATUS " - with Screen Resolution support: ${with_xrandr}")
message (STATUS " - with Status-Notifier applet:    ${with_status_notifier}")
message (STATUS " - with Sensors support:           ${with_sensors}")
message (STATUS " - with Terminal applet:           ${with_terminal}")
message (STATUS " - with Weblets applet:            ${with_weblets}")
message (STATUS " - with Xgamma applet:             ${with_xgamma}")
message (STATUS " - with Recent-Events applet:      ${with_recent_events}")
message (STATUS " - with Python interface:          ${with_python}")
message (STATUS " - with Ruby interface:            ${with_ruby}")
message (STATUS " - with Mono interface:            ${with_mono}")
message (STATUS " - with Vala interface:            ${with_vala}")
