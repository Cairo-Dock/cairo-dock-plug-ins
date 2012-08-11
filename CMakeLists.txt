########### requirements ###############

cmake_minimum_required (VERSION 2.6)
find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)
include ("${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/GNUInstallDirs.cmake")

########### project ###############

project ("cairo-dock-plugins")
set (VERSION "3.0.99.beta1")

add_definitions (-std=c99 -Wextra -Wwrite-strings -Wuninitialized -Werror-implicit-function-declaration -Wstrict-prototypes) # removed for stable versions: -Wstrict-prototypes #-Wunreachable-code -Wno-unused-parameter -Wall
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

add_custom_target(dist
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
    COMMAND pkg-config gldi --variable=pluginsdir  # /usr/lib/cairo-dock # or /usr/lib/x86_64-linux-gnu/cairo-dock
    OUTPUT_VARIABLE pluginsdir)
STRING (REGEX REPLACE "\n" "" pluginsdir ${pluginsdir})  # remove the \n
# get plug-ins data dir
execute_process(
    COMMAND pkg-config gldi --variable=pluginsdatadir  # /usr/share/cairo-dock/plug-ins
    OUTPUT_VARIABLE pluginsdatadir)
STRING (REGEX REPLACE "\n" "" pluginsdatadir ${pluginsdatadir})
# get prefix dir
execute_process(
    COMMAND pkg-config gldi --variable=prefix  # /usr/share/cairo-dock/plug-ins
    OUTPUT_VARIABLE prefix)
STRING (REGEX REPLACE "\n" "" prefix ${prefix})
# get GTK version (must be the same as the core, as GTK2 and GTK3 can't coexist at runtime)
execute_process(
    COMMAND pkg-config gldi --variable=gtkversion  # 2 or 3
    OUTPUT_VARIABLE gtkversion)
STRING (REGEX REPLACE "\n" "" gtkversion ${gtkversion})
# check that version matches with the core
execute_process(
    COMMAND pkg-config --modversion gldi  # 2.2.0-3
    OUTPUT_VARIABLE dock_version)
STRING (REGEX REPLACE "\n" "" dock_version ${dock_version})
if (NOT "${dock_version}" STREQUAL "${VERSION}")		# Version
	if ("${PACKAGEMENT}" STREQUAL "")
		MESSAGE (FATAL_ERROR "Error : version mismatch with the core : " ${VERSION} <> ${dock_version})
	else ()
		MESSAGE (WARNING "Warning : version mismatch with the core : " ${VERSION} <> ${dock_version})
	endif ()
endif()

# check that installation dir matches with the core
GET_FILENAME_COMPONENT(libdir "${pluginsdir}/.." ABSOLUTE)  # /usr/lib # or /usr/lib/x86_64-linux-gnu
GET_FILENAME_COMPONENT(datadir "${pluginsdatadir}/../.." ABSOLUTE)  # /usr/share
if (NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "${prefix}"
		OR NOT "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" STREQUAL "${libdir}"
		OR NOT "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}" STREQUAL "${datadir}")
	message (STATUS "It seems that the current CMAKE_INSTALL_{PREFIX,LIBDIR,DATAROOTDIR} flags are not the same that you have used with the core.")
	message (STATUS " It will be replaced by this value: ${prefix}")
	message (WARNING "Plug-ins should be installed in the same directory as the core, that is to say in ${pluginsdir}")
	set (CMAKE_INSTALL_PREFIX "${prefix}")
	#set (libdir "${CMAKE_INSTALL_PREFIX}/${libname}/cairo-dock")
endif()

if (NOT "${force-icon-in-menus}" STREQUAL "no")  # we believe that not showing icons in the menus by default is a terrible idea; unfortunately, it's not easily undoable for an end-user; so until this is fixed by a big player (Gnome, Ubuntu or other), we'll force the display, unless "-Dforce-icon-in-menus=yes" is provided in the cmake command.
	add_definitions (-DCAIRO_DOCK_FORCE_ICON_IN_MENUS=1)
else()
	add_definitions (-DCAIRO_DOCK_FORCE_ICON_IN_MENUS=0)
endif()

# set internationalisation
set (GETTEXT_PLUGINS "cairo-dock-plugins")
set (localedir "${prefix}/${CMAKE_INSTALL_LOCALEDIR}")
set (gaugesdir "${datadir}/cairo-dock/gauges")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")  # additionnal FindPackage files

set (MODULES_MISSING "")
set (MODULES_INTEGRATION_MISSING "")
set (PROGRAMS_MISSING "")

########### dependancies ###############

message ("")
message (STATUS "=====================")
message (STATUS "Check dependencies...")
message (STATUS "=====================")
message ("")

pkg_check_modules ("PACKAGE" REQUIRED "cairo" "librsvg-2.0" "dbus-1" "dbus-glib-1" "libxml-2.0")
#pkg_check_modules ("PACKAGE" REQUIRED "ig")
pkg_check_modules ("GLDI" REQUIRED "gldi")  # we don't want to link with gldi, so we check it separately.
set (PACKAGE_INCLUDE_DIRS "${GLDI_INCLUDE_DIRS}")

add_definitions (-DGTK_DISABLE_DEPRECATED="1")

#############    GLIB    #################
pkg_check_modules (GLIB glib-2.0)
	STRING (REGEX REPLACE "\\..*" "" GLIB_MAJOR "${GLIB_VERSION}") # 2.28.3 => 2
	STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" GLIB_MINOR "${GLIB_VERSION}")  # 2.28.3 => 2.28
	STRING (REGEX REPLACE "\\.[0-9]*" "" GLIB_MINOR "${GLIB_MINOR}") # 2.28 => 28
	STRING (REGEX REPLACE ".*\\." "" GLIB_NANO "${GLIB_VERSION}") # 2.28.3 => 3
	STRING (REGEX REPLACE "-.*" "" GLIB_NANO "${GLIB_NANO}")

############# SHARED-FILES #################
set (shared_filesdatadir "${pluginsdatadir}/shared-files")
add_subdirectory (shared-files)

############# INDICATOR-APPLET #################
message (STATUS "> Indicator-Applet:")
# Note: the names of dbusmenu-glib, dbusmenu-gtk and indicator have changed...
set (DBUSMENU_MODULE dbusmenu-glib-0.4)
pkg_check_modules (DBUSMENU ${DBUSMENU_MODULE})
if (NOT "${DBUSMENU_FOUND}")
	set (DBUSMENU_MODULE dbusmenu-glib)
	pkg_check_modules (DBUSMENU ${DBUSMENU_MODULE})
endif()

if ("${gtkversion}" STREQUAL "2")
	set (DBUSMENU_GTK_MODULE dbusmenu-gtk-0.4)
else()
	set (DBUSMENU_GTK_MODULE dbusmenu-gtk3-0.4)
endif()
pkg_check_modules (DBUSMENU_GTK ${DBUSMENU_GTK_MODULE})
if (NOT "${DBUSMENU_GTK_FOUND}")
	if ("${gtkversion}" STREQUAL "2")
		set (DBUSMENU_GTK_MODULE dbusmenu-gtk)
	else()
		set (DBUSMENU_GTK_MODULE dbusmenu-gtk3)
	endif()
	pkg_check_modules (DBUSMENU_GTK ${DBUSMENU_GTK_MODULE})
endif()

if ("${gtkversion}" STREQUAL "2")
	set (INDICATOR_APPLET_MODULE indicator-0.4)
else()
	set (INDICATOR_APPLET_MODULE indicator3-0.4)
endif()
pkg_check_modules (INDICATOR_APPLET ${INDICATOR_APPLET_MODULE})
if ("${INDICATOR_APPLET_FOUND}" STREQUAL "1")
	set (INDICATOR_APPLICATIONADDED_HAS_HINT 1) # now the ApplicationAdded signal (Status-Notifier) has a new parameter (hint) => https://code.launchpad.net/~ted/indicator-application/name-hints/+merge/67213
	set (INDICATOR_NEW_VERSION 1)  # oneiric and newer
else()
	if ("${gtkversion}" STREQUAL "2")
		set (INDICATOR_APPLET_MODULE indicator)
	else()
		set (INDICATOR_APPLET_MODULE indicator3)
	endif()
	pkg_check_modules (INDICATOR_APPLET ${INDICATOR_APPLET_MODULE})
	set (INDICATOR_APPLICATIONADDED_HAS_HINT 0)
endif()

if ("${gtkversion}" STREQUAL "2")
	set (IDO_MODULE libido-0.1)
else()
	set (IDO_MODULE libido3-0.1)
endif()
pkg_check_modules (IDO ${IDO_MODULE})

if ("${DBUSMENU_FOUND}" STREQUAL "1" AND "${DBUSMENU_GTK_FOUND}" STREQUAL "1")
	STRING (REGEX REPLACE "\\..*" "" DBUSMENU_MAJOR "${DBUSMENU_VERSION}")
	STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" DBUSMENU_MINOR "${DBUSMENU_VERSION}")  # 0.1.7 => 1.7
	STRING (REGEX REPLACE "\\.[0-9]*" "" DBUSMENU_MINOR "${DBUSMENU_MINOR}")
	STRING (REGEX REPLACE ".*\\." "" DBUSMENU_NANO "${DBUSMENU_VERSION}")
	STRING (REGEX REPLACE "-.*" "" DBUSMENU_NANO "${DBUSMENU_NANO}")

	STRING (REGEX REPLACE "\\..*" "" DBUSMENU_GTK_MAJOR "${DBUSMENU_GTK_VERSION}")
	STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" DBUSMENU_GTK_MINOR "${DBUSMENU_GTK_VERSION}")  # 0.1.7 => 1.7
	STRING (REGEX REPLACE "\\.[0-9]*" "" DBUSMENU_GTK_MINOR "${DBUSMENU_GTK_MINOR}")
	STRING (REGEX REPLACE ".*\\." "" DBUSMENU_GTK_NANO "${DBUSMENU_GTK_VERSION}")
	STRING (REGEX REPLACE "-.*" "" DBUSMENU_GTK_NANO "${DBUSMENU_GTK_NANO}")
	
	if (${DBUSMENU_GTK_MAJOR} GREATER 0 OR ${DBUSMENU_GTK_MINOR} GREATER 5
	    OR (${DBUSMENU_GTK_MINOR} EQUAL 5 AND ${DBUSMENU_GTK_NANO} GREATER 89))
		set (DBUSMENU_GTK3_NEW 1)
		set (INDICATOR_APPLICATIONADDED_HAS_TITLE 1) # Status-Notifier: new parameter "Title" in the "added" signal since Precise
	else()
		set (INDICATOR_APPLICATIONADDED_HAS_TITLE 0)
	endif()
endif()

if ("${INDICATOR_APPLET_FOUND}" STREQUAL "1")
	STRING (REGEX REPLACE "\\..*" "" INDICATOR_MAJOR "${INDICATOR_APPLET_VERSION}")
	STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" INDICATOR_MINOR "${INDICATOR_APPLET_VERSION}")  # 0.1.7 => 1.7
	STRING (REGEX REPLACE "\\.[0-9]*" "" INDICATOR_MINOR "${INDICATOR_MINOR}")
	STRING (REGEX REPLACE ".*\\." "" INDICATOR_NANO "${INDICATOR_APPLET_VERSION}")
	STRING (REGEX REPLACE "-.*" "" INDICATOR_NANO "${INDICATOR_NANO}")
	
	if (${INDICATOR_MAJOR} GREATER 0 OR ${INDICATOR_MINOR} GREATER 4
	    OR (${INDICATOR_MINOR} EQUAL 4 AND ${INDICATOR_NANO} GREATER 89))
		set (SOUND_SERVICE_VERSION 1) # Sound-Menu: new DBus path
		set (INDICATOR_MESSAGES_HAS_LOZENGE 1)   # Messaging Menu has 'right-is-lozenge'
	else() # older than Precise.
		set (SOUND_SERVICE_VERSION 0)
		set (INDICATOR_MESSAGES_HAS_LOZENGE 0)
	endif()

	if ("${INDICATOR_NEW_VERSION}" STREQUAL "1")  # oneiric and newer
		if ("${gtkversion}" STREQUAL "2")
			execute_process(
				COMMAND pkg-config --variable=iconsdir indicator-0.4
				OUTPUT_VARIABLE INDICATORICONSDIR)
		else()
			execute_process(
				COMMAND pkg-config --variable=iconsdir indicator3-0.4
				OUTPUT_VARIABLE INDICATORICONSDIR)
		endif()
	else()
		if ("${gtkversion}" STREQUAL "2")
			execute_process(
				COMMAND pkg-config --variable=iconsdir indicator
				OUTPUT_VARIABLE INDICATORICONSDIR)
		else()
			execute_process(
				COMMAND pkg-config --variable=iconsdir indicator3
				OUTPUT_VARIABLE INDICATORICONSDIR)
		endif()
	endif()
	STRING (REGEX REPLACE "\n" "" INDICATORICONSDIR ${INDICATORICONSDIR})  # la commande rajoute un retour chariot ...
	add_subdirectory (Indicator-applet)
	set (with_indicator "yes")
endif()

if (NOT "${DBUSMENU_FOUND}" STREQUAL "1" OR NOT "${DBUSMENU_GTK_FOUND}" STREQUAL "1" OR NOT "${INDICATOR_APPLET_FOUND}" STREQUAL "1")
	message (WARNING "These modules are required to compile Indicators applet (MeMenu, Messaging-Menu, Sound-Control, Status-Notifier and Global-Menu): ${INDICATOR_APPLET_MODULE}(-0.4), ${IDO_MODULE}, ${DBUSMENU_MODULE}(-0.4), ${DBUSMENU_GTK_MODULE}(-0.4)")
	set (MODULES_MISSING "${MODULES_MISSING} ${INDICATOR_APPLET_MODULE}(-0.4) (${IDO_MODULE}) ${DBUSMENU_MODULE}(-0.4) ${DBUSMENU_GTK_MODULE}(-0.4)")
endif()

############# INDICATOR-NAME #################
message (STATUS "> Indicator Names:")
set (INDICATOR_OLD_NAMES 0)
if ("${force_old_indicator_names}" STREQUAL "yes")
	set (INDICATOR_OLD_NAMES 1)
	message (STATUS "  Indicators: use old Ayatana Dbus names (forced)")
#~ elseif ("${DISTRO_ID}" STREQUAL "Ubuntu")
	#~ STRING (REGEX REPLACE "\\..*" "" DISTRO_RELEASE_MAJOR ${DISTRO_RELEASE})
	#~ # message (STATUS "DISTRO_RELEASE_MAJOR: ${DISTRO_RELEASE_MAJOR}")
	#~ if (${DISTRO_RELEASE_MAJOR} LESS 11)  # from 11.04, new names are used.
		#~ set (INDICATOR_OLD_NAMES 1)
		#~ message (STATUS "  Indicators: use old Ayatana Dbus names")
	#~ endif()
elseif ("${INDICATOR_APPLET_FOUND}" STREQUAL "1")
	if (NOT (${INDICATOR_MAJOR} GREATER 0 OR ${INDICATOR_MINOR} GREATER 3 OR (${INDICATOR_MINOR} EQUAL 3 AND ${INDICATOR_NANO} GREATER 20)))
		set (INDICATOR_OLD_NAMES 1)  # maverick or older
		message (STATUS "  Indicators: use old Ayatana Dbus names")
	endif()
endif()

############# DISTRIBUTION #################
message (STATUS "> Distribution:")
# We try to detect if the user is on Ubuntu to know which bus we have to use...
execute_process(
	COMMAND lsb_release -i
	OUTPUT_VARIABLE DISTRO_ID)  # -> "Distributor ID:	Ubuntu"
if (NOT "${DISTRO_ID}" STREQUAL "") # this tool isn't available on all distributions
	STRING (REGEX REPLACE "\n" "" DISTRO_ID ${DISTRO_ID})  # retour chariot
	STRING (REGEX REPLACE ".*: *\t*" "" DISTRO_ID ${DISTRO_ID})

	execute_process(
		COMMAND lsb_release -r
		OUTPUT_VARIABLE DISTRO_RELEASE)  # -> "Release:	10.10"
	STRING (REGEX REPLACE "\n" "" DISTRO_RELEASE ${DISTRO_RELEASE})  # retour chariot
	STRING (REGEX REPLACE ".*: *\t*" "" DISTRO_RELEASE ${DISTRO_RELEASE})
else()
	# on Ubuntu deb builders, lsb_release isn't available but we can have a look to /etc/issue.net
	get_filename_component(ISSUE_NET "/etc/issue.net" ABSOLUTE)
	if (EXISTS ${ISSUE_NET}) # to not have an error with cat
		execute_process(
			COMMAND cat ${ISSUE_NET}
			OUTPUT_VARIABLE DISTRO_RELEASE)  # -> Ubuntu 10.10 || Ubuntu natty (development branch)
		if (NOT "${DISTRO_RELEASE}" STREQUAL "")
			STRING (REGEX REPLACE "\n" "" DISTRO_RELEASE ${DISTRO_RELEASE})  # retour chariot
			STRING (REGEX REPLACE " (.+)" "" DISTRO_ID ${DISTRO_RELEASE})
			if ("${DISTRO_ID}" STREQUAL "Ubuntu")
				STRING (REGEX REPLACE ".[(]development branch[)]" "" DISTRO_RELEASE ${DISTRO_RELEASE})
				STRING (REGEX REPLACE ".LTS" "" DISTRO_RELEASE ${DISTRO_RELEASE}) # Ubuntu 10.04.1 LTS
				STRING (REGEX REPLACE "(.+) " "" DISTRO_RELEASE ${DISTRO_RELEASE})
				if ("${DISTRO_RELEASE}" STREQUAL "natty")
					set (DISTRO_RELEASE "11.04")
				elseif ("${DISTRO_RELEASE}" STREQUAL "oneiric")
					set (DISTRO_RELEASE "11.10")
				elseif ("${DISTRO_RELEASE}" STREQUAL "precise")
					set (DISTRO_RELEASE "12.04")
				endif()
			endif()
		endif()
	endif()
endif()

if (NOT "${DISTRO_ID}" STREQUAL "")
	message (STATUS "  DISTRO_ID: ${DISTRO_ID}, DISTRO_RELEASE: ${DISTRO_RELEASE}")
endif()

############# ALSA_MIXER #################
message (STATUS "> AlsaMixer:")
set (with_alsa "no")
if (NOT "${enable-alsa-mixer}" STREQUAL "no")
	pkg_check_modules (ALSA_MIXER_PACKAGE alsa)
	if ("${ALSA_MIXER_PACKAGE_FOUND}" STREQUAL "")
		message (STATUS "Could not find alsa; Cairo-Dock won't be built with AlsaMixer applet.")
		message (WARNING "This module is required to compile AlsaMixer applet: alsa")
		set (MODULES_MISSING "${MODULES_MISSING} alsa")
	else()
		set (GETTEXT_ALSA_MIXER ${GETTEXT_PLUGINS})
		set (VERSION_ALSA_MIXER "2.1.1")
		set (PACKAGE_ALSA_MIXER "cd-AlsaMixer")
		set (with_alsa "yes")
		set (alsa_mixerdatadir "${pluginsdatadir}/AlsaMixer")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/alsaMixer/data/AlsaMixer.conf.in ${CMAKE_CURRENT_BINARY_DIR}/alsaMixer/data/AlsaMixer.conf)
		if ("${with_indicator}" STREQUAL "yes" AND "${IDO_FOUND}"
			AND "${INDICATOR_OLD_NAMES}" STREQUAL "0")
			set (with_soundmenu "yes")
		endif()
		add_subdirectory ("alsaMixer")
	endif()
endif()

############# ANIMATED ICONS #################
message (STATUS "> Animated Icons:")
set (GETTEXT_ANIMATED_ICONS ${GETTEXT_PLUGINS})
set (VERSION_ANIMATED_ICONS "1.0.11")
set (PACKAGE_ANIMATED_ICONS "cd-Animated-icons")
set (animated_iconsdatadir "${pluginsdatadir}/Animated-icons")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Animated-icons/data/Animated-icons.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Animated-icons/data/Animated-icons.conf)
add_subdirectory (Animated-icons)

############# CAIRO_PENGUIN #################
message (STATUS "> Cairo Penguin:")
set (GETTEXT_CAIRO_PENGUIN ${GETTEXT_PLUGINS})
set (VERSION_CAIRO_PENGUIN "1.1.11")
set (PACKAGE_CAIRO_PENGUIN "cd-Cairo-Penguin")
set (cairo_penguinuserdirname "Cairo-Penguin")
set (cairo_penguindatadir "${pluginsdatadir}/Cairo-Penguin")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Cairo-Penguin/data/Cairo-Penguin.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Cairo-Penguin/data/Cairo-Penguin.conf)
add_subdirectory (Cairo-Penguin)

############# CLIPPER #################
message (STATUS "> Clipper:")
set (GETTEXT_CLIPPER ${GETTEXT_PLUGINS})
set (VERSION_CLIPPER "1.1.7")
set (PACKAGE_CLIPPER "cd-Clipper")
set (Clipperdatadir "${pluginsdatadir}/Clipper")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Clipper/data/Clipper.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Clipper/data/Clipper.conf)
add_subdirectory (Clipper)

############# CLOCK #################
message (STATUS "> Clock:")
set (with_ical "yes")
pkg_check_modules ("LIBICAL_PACKAGE" "libical")
if ("${LIBICAL_PACKAGE_FOUND}" STREQUAL "")
	set (with_ical "no")
	message (STATUS "Could not find libical; Clock plugin won't be built with iCal support.")
	message (WARNING "This module is required to compile Clock applet with iCal support: libical")
	set (MODULES_MISSING "${MODULES_MISSING} libical")
endif()
set (GETTEXT_CLOCK ${GETTEXT_PLUGINS})
set (VERSION_CLOCK "2.2.1")
set (PACKAGE_CLOCK "cd-clock")
set (clockuserdirname "clock")
set (clockdatadir "${pluginsdatadir}/clock")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/clock/data/clock.conf.in ${CMAKE_CURRENT_BINARY_DIR}/clock/data/clock.conf)
add_subdirectory (clock)

############# COMPOSITE_MANAGER #################
message (STATUS "> Composite Manager:")
set (GETTEXT_COMPOSITE_MANAGER ${GETTEXT_PLUGINS})
set (VERSION_COMPOSITE_MANAGER "1.0.2")
set (PACKAGE_COMPOSITE_MANAGER "cd-Composite-Manager")
set (composite_managerdatadir "${pluginsdatadir}/Composite-Manager")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Composite-Manager/data/Composite-Manager.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Composite-Manager/data/Composite-Manager.conf)
add_subdirectory (Composite-Manager)

############# DBUS #################
message (STATUS "> DBus:")
message (STATUS " * Python:")
find_program (PYTHON_EXECUTABLE python2)  # if python v3 is the default python interpreter (e.g. on ArchLinux), python2 program should exist and we have to use it...
if ("${PYTHON_EXECUTABLE}" STREQUAL "" OR NOT EXISTS ${PYTHON_EXECUTABLE})
	find_program (PYTHON_EXECUTABLE python)
endif()
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print sys.version" OUTPUT_VARIABLE PYTHON_VERSION)
if ("${PYTHON_VERSION}" STREQUAL "")
	message (STATUS "Could not find Python, won't install Python interface.")
	message (WARNING "This program is required to compile DBus applet with Python interface: python (version 2)")
	set (PROGRAMS_MISSING "${PROGRAMS_MISSING} python")
	set (with_python "no")
else()
	STRING (REGEX REPLACE "\n" "" PYTHON_VERSION ${PYTHON_VERSION})
	message (STATUS "   Python Version: ${PYTHON_VERSION}")
	message (STATUS "   Python executable program: ${PYTHON_EXECUTABLE}")
	GET_FILENAME_COMPONENT(DEBIAN_VERSION /etc/debian_version ABSOLUTE)
	if (EXISTS ${DEBIAN_VERSION})
		message (STATUS "   will use '--install-layout deb' with 'python setup.py install'")
		set (DEBIAN_INSTALL_LAYOUT "--install-layout deb")
	endif()
	set (PYTHON_FOUND "TRUE")
	set (with_python "yes")
endif()

message (STATUS " * Ruby:")
# find_package(Ruby) found libs of ruby-dev but we only need a directory where we can install ruby libs.
find_program (RUBY_EXECUTABLE ruby)
EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['rubylibdir']"
	OUTPUT_VARIABLE RUBY_LIB_DIR)
message (STATUS "   Ruby library dir: ${RUBY_LIB_DIR}")
if ("${RUBY_LIB_DIR}" STREQUAL "" OR "${RUBY_LIB_DIR}" STREQUAL "nil")
	message (STATUS "Could not find ruby libs, won't install Ruby interface.")
	message (WARNING "This program is required to compile DBus applet with Ruby interface: ruby")
	set (PROGRAMS_MISSING "${PROGRAMS_MISSING} ruby")
	set (with_ruby "no")
else()
	# /usr/lib/ruby/1.8 or /usr/lib/ruby/1.9.1
	string (REGEX REPLACE "lib/ruby/[0-9].[0-9]?.+" "" RUBY_LIB_DIR_INSTALL "${RUBY_LIB_DIR}")
	string (REGEX REPLACE "${RUBY_LIB_DIR_INSTALL}" "" RUBY_LIB_DIR_INSTALL "${RUBY_LIB_DIR}")
	set (RUBY_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${RUBY_LIB_DIR_INSTALL}")
	message (STATUS "   will be installed in: ${RUBY_LIB_DIR}")
	set (RUBY_FOUND "TRUE")
	set (with_ruby "yes")
endif()

message (STATUS " * Mono:")
#find_package (Mono)
find_program (GMCS_EXECUTABLE gmcs)
if ("${GMCS_EXECUTABLE}" STREQUAL "" OR NOT EXISTS ${GMCS_EXECUTABLE})
	message (STATUS "Could not find Mono compiler gmcs, won't build Mono interface.")
	set (with_mono "no")
else()
	pkg_check_modules (MONO_PACKAGE glib-sharp-2.0 ndesk-dbus-1.0 ndesk-dbus-glib-1.0)
	if ("${MONO_PACKAGE_FOUND}" STREQUAL "")
		message (STATUS "Could not find glib-sharp-2.0, ndesk-dbus-1.0 or ndesk-dbus-glib-1.0; won't be built Mono interface.")
		message (WARNING "These modules are required to compile DBus applet with Mono interface: glib-sharp-2.0, ndesk-dbus-1.0 and ndesk-dbus-glib-1.0")
		set (MODULES_MISSING "${MODULES_MISSING} glib-sharp-2.0 ndesk-dbus-1.0 ndesk-dbus-glib-1.0")
		set (with_mono "no")
	else()
		set (MONO_FOUND "TRUE")
		set (with_mono "yes")
	endif()
endif()

message (STATUS " * Vala:")
# Valac is only required to convert vala files to C files.
#  So we can directly use produced files (c, h, vapi) without using valac.
set (with_vala "yes")
set (with_valac "no")
find_program (VALAC_EXE valac)
message (STATUS "   Path to valac: ${VALAC_EXE}")

if ("${WITH_VALA}" STREQUAL "no")
	set (with_vala "no")
# Glib < 2.26
elseif (${GLIB_MAJOR} LESS 3 AND ${GLIB_MINOR} LESS 26)
	set (with_vala "no")
elseif ( (NOT "${VALAC_EXE}" STREQUAL "VALAC_EXE-NOTFOUND")
	AND (EXISTS ${VALAC_EXE}) ) # now it works with 0.10 and 0.11 (= next 0.12) # $VALAC_EXE may be the correct path, even if valac is not installed !
	execute_process(COMMAND ${VALAC_EXE} "--version"
		OUTPUT_VARIABLE "VALA_VERSION")
	string(REPLACE "Vala" "" "VALA_VERSION" ${VALA_VERSION})
	string(STRIP ${VALA_VERSION} "VALA_VERSION")
	message (STATUS "   Vala version: ${VALA_VERSION}")

	STRING (REGEX REPLACE "\\..*" "" VALA_MAJOR "${VALA_VERSION}")
	# message (STATUS "VALA_MAJOR : ${VALA_MAJOR}")

	STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" VALA_MINOR "${VALA_VERSION}")  # 0.1.7 => 1.7
	STRING (REGEX REPLACE "\\.[0-9]*" "" VALA_MINOR "${VALA_MINOR}")
	# message (STATUS "VALA_MINOR : ${VALA_MINOR}")

	STRING (REGEX REPLACE ".*\\." "" VALA_NANO "${VALA_VERSION}")
	STRING (REGEX REPLACE "-.*" "" VALA_NANO "${VALA_NANO}")
	# message (STATUS "VALA_NANO  : ${VALA_NANO}")

	if (${VALA_MAJOR} GREATER 0 OR ${VALA_MINOR} GREATER 9) # vala >= 0.10 
		# AND (${VALA_MAJOR} LESS 1 AND ${VALA_MINOR} LESS 13)) # and <= 0.12
		message (STATUS "   Vala compiler OK (>= 0.10).")# and <= 0.12).")
		set (VALAC_FOUND "TRUE")
		set (with_valac "yes")
	else()
		message (STATUS "   Vala compiler is too old (0.10 required) or too new (> 0.12), won't build Vala interface.")
	endif()
else()
	message (STATUS "Could not find ValaC, won't build Vala interface with the version of your distribution.")
	message (WARNING "This program is required to compile DBus applet with Vala interface: valac (version > 0.10)")
	set (PROGRAMS_MISSING "${PROGRAMS_MISSING} valac")
endif()


set (GETTEXT_DBUS ${GETTEXT_PLUGINS})
set (VERSION_DBUS "1.2.1")
set (PACKAGE_DBUS "cd-Dbus")
set (dbusdatadir "${pluginsdatadir}/Dbus")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Dbus/data/Dbus.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Dbus/data/Dbus.conf)
add_subdirectory (Dbus)

############# DESKLET_RENDERING #################
message (STATUS "> Desklet Rendering:")
set (GETTEXT_DESKLET_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DESKLET_RENDERING "1.5.6")
set (PACKAGE_DESKLET_RENDERING "cd-desklet-rendering")
set (desklet_renderingdatadir "${pluginsdatadir}/desklet-rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/desklet-rendering/data/desklet-rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/desklet-rendering/data/desklet-rendering.conf)
add_subdirectory (desklet-rendering)

############# DIALOG_RENDERING #################
message (STATUS "> Dialog Rendering:")
set (GETTEXT_DIALOG_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DIALOG_RENDERING "0.5.1")
set (PACKAGE_DIALOG_RENDERING "cd-dialog-rendering")
set (dialog_renderingdatadir "${pluginsdatadir}/dialog-rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dialog-rendering/data/dialog-rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dialog-rendering/data/dialog-rendering.conf)
add_subdirectory (dialog-rendering)

############# DISKS #################
set (with_disks "no")
if ("${enable-disks}" STREQUAL "yes")
	message (STATUS "> Disks:")
	set (GETTEXT_DISKS ${GETTEXT_PLUGINS})
	set (VERSION_DISKS "0.0.5")
	set (PACKAGE_DISKS "cd-disks")
	set (with_disks "yes")
	set (disksdatadir "${pluginsdatadir}/Disks")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Disks/data/Disks.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Disks/data/Disks.conf)
	add_subdirectory ("Disks")
endif()

############# DND2SHARE #################
message (STATUS "> DND2Share:")
set (GETTEXT_DND2SHARE ${GETTEXT_PLUGINS})
set (VERSION_DND2SHARE "1.0.9")
set (PACKAGE_DND2SHARE "cd-dnd2share")
set (dnd2sharedatadir "${pluginsdatadir}/dnd2share")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dnd2share/data/dnd2share.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dnd2share/data/dnd2share.conf)
add_subdirectory (dnd2share)

############# DOCK RENDERING #################
message (STATUS "> Dock Rendering:")
set (GETTEXT_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_RENDERING "1.5.9")
set (PACKAGE_RENDERING "cd-rendering")
set (renderingdatadir "${pluginsdatadir}/rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dock-rendering/data/rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dock-rendering/data/rendering.conf)
add_subdirectory (dock-rendering)

############# DONCKY #################
set (with_doncky "no")
if ("${enable-doncky}" STREQUAL "yes")
	message (STATUS "> Doncky:")
	set (GETTEXT_DONCKY ${GETTEXT_PLUGINS})
	set (VERSION_DONCKY "0.0.7")
	set (PACKAGE_DONCKY "cd-doncky")
	set (with_doncky "yes")
	set (donckydatadir "${pluginsdatadir}/Doncky")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doncky/data/Doncky.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Doncky/data/Doncky.conf)
	add_subdirectory (Doncky)
endif()

############# DROP INDICATOR #################
message (STATUS "> Drop Indicator:")
set (GETTEXT_DROP_INDICATOR ${GETTEXT_PLUGINS})
set (VERSION_DROP_INDICATOR "1.1.5")
set (PACKAGE_DROP_INDICATOR "cd-drop_indicator")
set (drop_indicatordatadir "${pluginsdatadir}/drop-indicator")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/drop-indicator/data/drop_indicator.conf.in ${CMAKE_CURRENT_BINARY_DIR}/drop-indicator/data/drop_indicator.conf)
add_subdirectory (drop-indicator)

############# DUSTBIN #################
message (STATUS "> Dustbin:")
set (GETTEXT_DUSTBIN ${GETTEXT_PLUGINS})
set (VERSION_DUSTBIN "2.3.4")
set (PACKAGE_DUSTBIN "cd-dustbin")
set (dustbinuserdirname "dustbin")
set (dustbindatadir "${pluginsdatadir}/dustbin")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dustbin/data/dustbin.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dustbin/data/dustbin.conf)
add_subdirectory (dustbin)

############# FOLDERS #################
message (STATUS "> Folders:")
set (GETTEXT_FOLDERS ${GETTEXT_PLUGINS})
set (VERSION_FOLDERS "0.2.4")
set (PACKAGE_FOLDERS "cd-Folders")
set (foldersdatadir "${pluginsdatadir}/Folders")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Folders/data/Folders.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Folders/data/Folders.conf)
add_subdirectory (Folders)

############# GLOBAL-MENU #################
set (with_global_menu "no")
if ("${enable-global-menu}" STREQUAL "yes" AND "${INDICATOR_OLD_NAMES}" STREQUAL "0" AND "${DBUSMENU_FOUND}" AND "${DBUSMENU_GTK_FOUND}")  # currently only supported with new name of the indicator module and with newer version of dbusmenu
	message (STATUS "> Global-Menu:")
	set (GETTEXT_GLOBAL_MENU ${GETTEXT_PLUGINS})
	set (VERSION_GLOBAL_MENU "0.1.1")
	set (PACKAGE_GLOBAL_MENU "cd-Global-Menu")
	set (with_global_menu "yes")
	set (global_menudatadir "${pluginsdatadir}/Global-Menu")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Global-Menu/data/Global-Menu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Global-Menu/data/Global-Menu.conf)
	add_subdirectory (Global-Menu)
endif()

############# GMENU #################
message (STATUS "> GMenu:")
set (with_gmenu "no")
if (NOT "${enable-gmenu}" STREQUAL "no")
	if ("${gtkversion}" STREQUAL "2")
		set (GMENU_MODULE libgnome-menu)
	else()
		set (GMENU_MODULE libgnome-menu)
		# set (GMENU_MODULE libgnome-menu-3.0) # TODO
	endif()
	pkg_check_modules (GMENU_PACKAGE ${GMENU_MODULE})
	if ("${GMENU_PACKAGE_FOUND}" STREQUAL "")
		message (STATUS "Could not find ${GMENU_MODULE}; Cairo-Dock won't be built with GMenu applet.")
		message (WARNING "This module is required to compile GMenu applet: ${GMENU_MODULE}")
		set (MODULES_MISSING "${MODULES_MISSING} ${GMENU_MODULE}")
	else()
		set (GETTEXT_GMENU ${GETTEXT_PLUGINS})
		set (VERSION_GMENU "1.1.10")
		set (PACKAGE_GMENU "cd-GMenu")
		set (with_gmenu "yes")
		set (gmenudatadir "${pluginsdatadir}/GMenu")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/GMenu/data/GMenu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/GMenu/data/GMenu.conf)
		add_subdirectory ("GMenu")
	endif()
endif()

############# GVFS-INTEGRATION #################
message (STATUS "> GVFS-Integration:")
pkg_check_modules ("LIBGIO" "gio-2.0")
add_subdirectory (gvfs-integration)

############# GNOME-INTEGRATION #################
message (STATUS "> Gnome-Integration:")
set (with_gnome_integration "no")
if (NOT "${enable-gnome-integration}" STREQUAL "no")
	pkg_check_modules (GNOME_INTEGRATION gio-2.0)
	if ("${GNOME_INTEGRATION_FOUND}" STREQUAL "")
		message (STATUS "Could not find gio; Cairo-Dock won't be built with Gnome>=2.22 support.")
		message (STATUS "This module is required to compile Gnome-Integration applet: gio-2.0")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} gio-2.0")
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
	message (STATUS "> Gnome-Integration:")
	message (WARNING "This applet is deprecated")
	pkg_check_modules (OLD_GNOME_INTEGRATION gnome-vfs-2.0 libgnomeui-2.0)
	if ("${OLD_GNOME_INTEGRATION_FOUND}" STREQUAL "")
		message (STATUS "Could not find gnome-vfs and/or gnomeui; Cairo-Dock won't be built with Gnome<2.22 support.")
		message (STATUS "These modules are required to compile Gnome-Integration-Old applet: gnome-vfs-2.0 libgnomeui-2.0")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} gnome-vfs-2.0 libgnomeui-2.0")
	else()
		set (VERSION_GNOME_INTEGRATION_OLD "1.0.4")
		set (PACKAGE_GNOME_INTEGRATION_OLD "cd_gnome-integration-old")
		set (with_gnome_integration_old "yes")
		set (gnome_integration_olddatadir "${pluginsdatadir}/gnome-integration-old")
		add_subdirectory ("gnome-integration-old")
	endif()
endif()

############# ICON EFFECTS #################
message (STATUS "> Icon Effects:")
set (GETTEXT_ICON_EFFECTS ${GETTEXT_PLUGINS})
set (VERSION_ICON_EFFECTS "1.2.4")
set (PACKAGE_ICON_EFFECTS "cd-icon-effect")
set (icon_effectsdatadir "${pluginsdatadir}/icon-effect")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/icon-effect/data/icon-effect.conf.in ${CMAKE_CURRENT_BINARY_DIR}/icon-effect/data/icon-effect.conf)
add_subdirectory (icon-effect)

############# IMPULSE #################
set (with_impulse "no")
message (STATUS "> Impulse:")
pkg_check_modules (LIBPULSE libpulse)
pkg_check_modules (FFTW3 fftw3)  # optional, not advised for distributions packages (it's not a small dependence...)
if ("${LIBPULSE_FOUND}" STREQUAL "")
	message (STATUS "Could not find libpulse; Cairo-Dock won't be built with Impulse applet.")
	message (WARNING "These modules are required to compile Impulse applet: libpulse (and fftw3 - optional)")
	set (MODULES_MISSING "${MODULES_MISSING} libpulse")
else()
	set (GETTEXT_IMPULSE ${GETTEXT_PLUGINS})
	set (VERSION_IMPULSE "0.0.5")
	set (PACKAGE_IMPULSE "cd-Impulse")
	set (with_impulse "yes")
	set (impulsedatadir "${pluginsdatadir}/Impulse")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Impulse/data/Impulse.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Impulse/data/Impulse.conf)
	add_subdirectory (Impulse)
endif()

############# ILLUSION #################
message (STATUS "> Illusion:")
set (GETTEXT_ILLUSION ${GETTEXT_PLUGINS})
set (VERSION_ILLUSION "1.0.7")
set (PACKAGE_ILLUSION "cd-illusion")
set (illusiondatadir "${pluginsdatadir}/illusion")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/illusion/data/illusion.conf.in ${CMAKE_CURRENT_BINARY_DIR}/illusion/data/illusion.conf)
add_subdirectory (illusion)

############# KDE-INTEGRATION #################
message (STATUS "> KDE-Integration:")
set (with_kde_integration "no")
set (with_kde_integration2 "no")
if ("${enable-kde-integration2}" STREQUAL "yes")
	#find_package(KDE4)
	find_package(Qt4)
	if (QT4_FOUND)
		message (STATUS " * Qt Includes: ${QT_INCLUDES}")
		message (STATUS " * Qt Definitions: ${QT_DEFINITIONS}")
		message (STATUS " * QtCore Library: ${QT_QTCORE_LIBRARY} / ${PACKAGE_LIBRARIES}")
	else()
		message (STATUS " * Qt unavailable")
	endif()

	find_path(KDECORE_INCLUDE_DIR "kdecore_export.h")
	find_library(KDECORE_LIBRARY NAMES "kdecore")
	GET_FILENAME_COMPONENT(KDECORE_LIBRARY ${KDECORE_LIBRARY} PATH)

	find_path(KIO_INCLUDE_DIR "kio_export.h" PATH_SUFFIXES "kio")
	find_library(KIO_LIBRARY NAMES "kio")
	GET_FILENAME_COMPONENT(KIO_LIBRARY ${KIO_LIBRARY} PATH)

	find_path(KDE_INCLUDE_DIR "KDirWatch" PATH_SUFFIXES "KDE")

	if (NOT "${KDECORE_LIBRARY}" STREQUAL "")
		message (STATUS " * KDECORE Dir: ${KDECORE_INCLUDE_DIR}")
		message (STATUS " * KDECORE Library: ${KDECORE_LIBRARY}")
	else()
		message (STATUS " * KDECORE unavailable")
	endif()
	if (NOT "${KIO_LIBRARY}" STREQUAL "")
		message (STATUS " * KIO Dir: ${KIO_INCLUDE_DIR}")
		message (STATUS " * KIO Library: ${KIO_LIBRARY}")
	else()
		message (STATUS " * KIO unavailable")
	endif()
	if (NOT "${KDE_LIBRARY}" STREQUAL "") ## always empty?
		message (STATUS " * KDE4 Dir: ${KDE_INCLUDE_DIR}")
		message (STATUS " * KDE4 Library: ${KDE_LIBRARY}")
	else()
		message (STATUS " * KDE4 unavailable")
	endif()

	if (QT4_FOUND
		 AND KDECORE_INCLUDE_DIR
		 AND KDECORE_LIBRARY
		 AND KIO_INCLUDE_DIR
		 AND KIO_LIBRARY
		 AND KDE_INCLUDE_DIR)
		message (STATUS "  KDE: OK")
		set (VERSION_KDE_INTEGRATION "0.0.3")
		set (PACKAGE_KDE_INTEGRATION "cd_kde-integration")
		set (with_kde_integration2 "yes")
		set (kde_integrationdatadir "${pluginsdatadir}/kde-integration2")
		add_subdirectory ("kde-integration2")
	else()
		message (STATUS "Could not find kde libs; Cairo-Dock won't be built with KDE support.")
		message (STATUS "These libraries are required to compile KDE experimental applet: kdecore, kio, kde4")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} kdecore, kio, kde4")
	endif()
elseif (NOT "${enable-kde-integration}" STREQUAL "no")
	pkg_check_modules (KDE_INTEGRATION gio-2.0)
	message (STATUS "  KDE_INTEGRATION_FOUND: ${KDE_INTEGRATION_FOUND}")
	if (NOT KDE_INTEGRATION_FOUND)
		message (STATUS "Could not find gio; Cairo-Dock won't be built with KDE support.")
		message (STATUS "This module is required to compile KDE-Integration applet: gio-2.0")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} gio-2.0")
	else()
		set (VERSION_KDE_INTEGRATION "1.0.3")
		set (PACKAGE_KDE_INTEGRATION "cd_kde-integration")
		set (with_kde_integration "yes")
		set (kde_integrationdatadir "${pluginsdatadir}/kde-integration")
		add_subdirectory ("kde-integration")
	endif()
endif()

############# KEYBOARD_INDICATOR #################
message (STATUS "> Keyboard-Indicator:")
set (with_keyboard_indicator "no")
pkg_check_modules (KEYBOARD_INDICATOR_PACKAGE libxklavier)
if ("${KEYBOARD_INDICATOR_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find libxklavier; Cairo-Dock won't be built with keyboard-indicator applet.")
	message (WARNING "This module is required to compile Keyboard-Indicator applet: libxklavier")
	set (MODULES_MISSING "${MODULES_MISSING} libxklavier")
else()
	set (GETTEXT_KEYBOARD_INDICATOR ${GETTEXT_PLUGINS})
	set (VERSION_KEYBOARD_INDICATOR "1.1.7")
	set (PACKAGE_KEYBOARD_INDICATOR "cd-keyboard-indicator")
	set (with_keyboard_indicator "yes")
	set (keyboard_indicatordatadir "${pluginsdatadir}/keyboard-indicator")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/keyboard-indicator/data/keyboard-indicator.conf.in ${CMAKE_CURRENT_BINARY_DIR}/keyboard-indicator/data/keyboard-indicator.conf)
	add_subdirectory ("keyboard-indicator")
endif()

############# LOGOUT #################
message (STATUS "> LogOut:")
pkg_check_modules (UPOWER upower-glib)  # useful for Powermanager too.
if (${UPOWER_FOUND})
	set (with_upower_support "yes")
else()
	set (with_upower_support "no")
	message (STATUS "Could not find upower-glib; LogOut and PowerManager plugin won't be built with UPower support.")
	message (WARNING "This module is required to compile LogOut and PowerManager applet with UPower support: upower-glib")
	set (MODULES_MISSING "${MODULES_MISSING} upower-glib")
endif()
set (GETTEXT_LOGOUT ${GETTEXT_PLUGINS})
set (VERSION_LOGOUT "2.0.2")
set (PACKAGE_LOGOUT "cd-logout")
set (logoutdatadir "${pluginsdatadir}/logout")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/logout/data/logout.conf.in ${CMAKE_CURRENT_BINARY_DIR}/logout/data/logout.conf)
add_subdirectory (logout)

############# MAIL #################
message (STATUS "> Mail:")
set (with_mail "no")
# find the compilation flags
find_program (LIBETPAN_CONFIG_EXECUTABLE libetpan-config)
execute_process(
	COMMAND ${LIBETPAN_CONFIG_EXECUTABLE} --cflags
	OUTPUT_VARIABLE MAIL_PACKAGE_CFLAGS)
if (NOT "${MAIL_PACKAGE_CFLAGS}" STREQUAL "" AND NOT "${MAIL_PACKAGE_CFLAGS}" STREQUAL "\n") # if there is a problem with the previous, we don't want to have a lot of errors
	STRING (REGEX REPLACE "\n" "" TMP_VARIABLE "${MAIL_PACKAGE_CFLAGS}") # to not skip the last option
	STRING (REGEX MATCHALL "(^| )-I[^ ]+( |$)" TMP_VARIABLE "${TMP_VARIABLE}") # first extract the "-I" options
	STRING (REGEX REPLACE ";" "" TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE " $" "" TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE "^ " "" TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE "  " " " TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE "-I([^ ]+)" "\\1" MAIL_PACKAGE_INCLUDE_DIRS "${TMP_VARIABLE}")  # then remove the "-I" string
endif()
# find the link flags
execute_process(
	COMMAND ${LIBETPAN_CONFIG_EXECUTABLE} --libs
	OUTPUT_VARIABLE MAIL_PACKAGE_LIBS)
if (NOT "${MAIL_PACKAGE_LIBS}" STREQUAL "")
	STRING (REGEX REPLACE "\n" "" MAIL_PACKAGE_LIBS "${MAIL_PACKAGE_LIBS}")
	#   find the link libraries
	STRING (REGEX MATCHALL "(^| )-l[^ ]+( |$)" TMP_VARIABLE "${MAIL_PACKAGE_LIBS}") # extract the "-l" options (only if it's separated by two blank spaces or the end/beginning of the line => -L/usr/lib/x86_64-linux-gnu)
	STRING (REGEX REPLACE ";" "" TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE "^ " "" TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE "  " " " TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE " $" "" MAIL_PACKAGE_LIBRARIES "${TMP_VARIABLE}")
	#   find the link directories
	STRING (REGEX MATCHALL "(^| )-L[^ ]+( |$)" TMP_VARIABLE "${MAIL_PACKAGE_LIBS}") # extract the "-L" options
	STRING (REGEX REPLACE ";" "" TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE " $" "" TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE "^ " "" TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE "  " " " TMP_VARIABLE "${TMP_VARIABLE}")
	STRING (REGEX REPLACE "-L([^ ]+)" "\\1" MAIL_PACKAGE_LIBRARY_DIRS "${TMP_VARIABLE}")  # then remove the "-L" string
endif()
if ("${MAIL_PACKAGE_LIBS}" STREQUAL "")
	message (STATUS "warning : Could not find libetpan; Cairo-Dock won't be built with Mail applet.")
	message (WARNING "This module is required to compile Mail applet: libetpan")
	set (PROGRAMS_MISSING "${PROGRAMS_MISSING} libetpan-config")
else()
	message (STATUS "  libetpan found. Using the following options:")
	message (STATUS "   Include directories: ${MAIL_PACKAGE_INCLUDE_DIRS}")
	message (STATUS "   Link directories: ${MAIL_PACKAGE_LIBRARY_DIRS}")
	message (STATUS "   Link libraries: ${MAIL_PACKAGE_LIBRARIES}")
	set (GETTEXT_MAIL ${GETTEXT_PLUGINS})
	set (VERSION_MAIL "1.0.12")
	set (PACKAGE_MAIL "cd-mail")
	set (maildatadir "${pluginsdatadir}/mail")
	set (with_mail "yes")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/mail/data/mail.conf.in ${CMAKE_CURRENT_BINARY_DIR}/mail/data/mail.conf)
	add_subdirectory ("mail")
endif()

############# MEMENU #################
set (with_me_menu "no")
if ("${INDICATOR_NEW_VERSION}" STREQUAL "1" AND NOT "${enable-memenu}" STREQUAL "yes")
	message (STATUS "> MeMenu: this applet has been automatically merged with Messaging-Menu")  # oneiric or newer
elseif ("${with_indicator}" STREQUAL "yes" AND "${IDO_FOUND}")
	message (STATUS "> MeMenu:")
	set (GETTEXT_MEMENU ${GETTEXT_PLUGINS})
	set (VERSION_MEMENU "1.0.4")
	set (PACKAGE_MEMENU "cd-MeMenu")
	set (memenudatadir "${pluginsdatadir}/MeMenu")
	set (with_me_menu "yes")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/MeMenu/data/MeMenu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/MeMenu/data/MeMenu.conf)
	add_subdirectory (MeMenu)
endif()

############# MESSAGING_MENU #################
set (with_messaging_menu "no")
if ("${with_indicator}" STREQUAL "yes") 
	message (STATUS "> Messaging-Menu:")
	set (GETTEXT_MESSAGING_MENU ${GETTEXT_PLUGINS})
	set (VERSION_MESSAGING_MENU "1.0.5")
	set (PACKAGE_MESSAGING_MENU "cd-Messaging-Menu")
	set (with_messaging_menu "yes")
	set (messaging_menudatadir "${pluginsdatadir}/Messaging-Menu")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Messaging-Menu/data/Messaging-Menu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Messaging-Menu/data/Messaging-Menu.conf)
	add_subdirectory (Messaging-Menu)
endif()

############# MOTION BLUR #################
message (STATUS "> Motion Blur:")
set (GETTEXT_MOTION_BLUR ${GETTEXT_PLUGINS})
set (VERSION_MOTION_BLUR "1.0.4")
set (PACKAGE_MOTION_BLUR "cd-motion_blur")
set (motion_blurdatadir "${pluginsdatadir}/motion-blur")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/motion-blur/data/motion_blur.conf.in ${CMAKE_CURRENT_BINARY_DIR}/motion-blur/data/motion_blur.conf)
add_subdirectory (motion-blur)

############# MUSICPLAYER #################
message (STATUS "> MusicPlayer:")
set (GETTEXT_MUSICPLAYER ${GETTEXT_PLUGINS})
set (VERSION_MUSICPLAYER "2.0.2")
set (PACKAGE_MUSICPLAYER "cd-musicPlayer")
set (musicplayerdatadir "${pluginsdatadir}/musicPlayer")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/musicPlayer/data/musicPlayer.conf.in ${CMAKE_CURRENT_BINARY_DIR}/musicPlayer/data/musicPlayer.conf)
add_subdirectory (musicPlayer)

############# NETSPEED #################
message (STATUS "> NetSpeed:")
set (GETTEXT_NETSPEED ${GETTEXT_PLUGINS})
set (VERSION_NETSPEED "1.2.10")
set (PACKAGE_NETSPEED "cd-netspeed")
set (netspeeddatadir "${pluginsdatadir}/netspeed")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/netspeed/data/netspeed.conf.in ${CMAKE_CURRENT_BINARY_DIR}/netspeed/data/netspeed.conf)
add_subdirectory (netspeed)

############# NETWORK_MONITOR #################
set (with_network_monitor "no")
if ("${enable-network-monitor}" STREQUAL "yes")
	message (STATUS "> Network Monitor:")
	set (GETTEXT_NETWORK_MONITOR ${GETTEXT_PLUGINS})
	set (VERSION_NETWORK_MONITOR "0.2.8")
	set (PACKAGE_NETWORK_MONITOR "cd-network-monitor")
	set (with_network_monitor "yes")
	set (network_monitordatadir "${pluginsdatadir}/Network-Monitor")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Network-Monitor/data/Network-Monitor.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Network-Monitor/data/Network-Monitor.conf)
	add_subdirectory (Network-Monitor)
endif()

############# POWERMANAGER #################
message (STATUS "> PowerManager:")
set (GETTEXT_POWERMANAGER ${GETTEXT_PLUGINS})
set (VERSION_POWERMANAGER "1.3.10")
set (PACKAGE_POWERMANAGER "cd-powermanager")
set (powermanagerdatadir "${pluginsdatadir}/powermanager")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/powermanager/data/powermanager.conf.in ${CMAKE_CURRENT_BINARY_DIR}/powermanager/data/powermanager.conf)
add_subdirectory (powermanager)

############# QUICK BROWSER #################
message (STATUS "> Quick Browser:")
set (GETTEXT_QUICK_BROWSER ${GETTEXT_PLUGINS})
set (VERSION_QUICK_BROWSER "1.0.11")
set (PACKAGE_QUICK_BROWSER "cd-quick-browser")
set (quick_browserdatadir "${pluginsdatadir}/quick_browser")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/quick-browser/data/quick-browser.conf.in ${CMAKE_CURRENT_BINARY_DIR}/quick-browser/data/quick-browser.conf)
add_subdirectory (quick-browser)

############# RECENT-EVENTS #################
message (STATUS "> Recent-Events:")
set (with_recent_events "no")
pkg_check_modules (RECENT_EVENTS zeitgeist-1.0)
if ("${RECENT_EVENTS_FOUND}" STREQUAL "")
	message (STATUS "Could not find libzeitgeist; Cairo-Dock won't be built with Zeitgeist support.")
	message (WARNING "This module is required to compile Recent-Events applet: zeitgeist-1.0")
	set (MODULES_MISSING "${MODULES_MISSING} zeitgeist-1.0")
else()
	set (GETTEXT_RECENT_EVENTS ${GETTEXT_PLUGINS})
	set (VERSION_RECENT_EVENTS "1.0.2")
	set (PACKAGE_RECENT_EVENTS "cd-Recent-Events")
	set (with_recent_events "yes")
	set (recent_eventsdatadir "${pluginsdatadir}/Recent-Events")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Recent-Events/data/Recent-Events.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Recent-Events/data/Recent-Events.conf)
	add_subdirectory ("Recent-Events")
endif()

############# REMOTE_CONTROL #################
message (STATUS "> Remote Control:")
set (GETTEXT_REMOTE_CONTROL ${GETTEXT_PLUGINS})
set (VERSION_REMOTE_CONTROL "1.0.1")
set (PACKAGE_REMOTE_CONTROL "cd-Remote-Control")
set (remote_controldatadir "${pluginsdatadir}/Remote-Control")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Remote-Control/data/Remote-Control.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Remote-Control/data/Remote-Control.conf)
add_subdirectory (Remote-Control)

############# RSSREADER #################
message (STATUS "> RSSreader:")
set (GETTEXT_RSS_READER ${GETTEXT_PLUGINS})
set (VERSION_RSS_READER "1.0.6")
set (PACKAGE_RSS_READER "cd-rssreader")
set (rss_readerdatadir "${pluginsdatadir}/RSSreader")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/RSSreader/data/RSSreader.conf.in ${CMAKE_CURRENT_BINARY_DIR}/RSSreader/data/RSSreader.conf)
add_subdirectory (RSSreader)

############# SCOOBY_DO #################
set (with_scooby_do "no")
if ("${enable-scooby-do}" STREQUAL "yes")
	message (STATUS "> Scooby-Do:")
	set (GETTEXT_SCOOBY_DO ${GETTEXT_PLUGINS})
	set (VERSION_SCOOBY_DO "0.1.2")
	set (PACKAGE_SCOOBY_DO "cd-scooby-do")
	set (with_scooby_do "yes")
	set (scooby_dodatadir "${pluginsdatadir}/Scooby-Do")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Scooby-Do/data/Scooby-Do.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Scooby-Do/data/Scooby-Do.conf)
	add_subdirectory (Scooby-Do)
endif()

############# SHORTCUTS #################
message (STATUS "> Shortcuts:")
set (GETTEXT_SHORTCUTS ${GETTEXT_PLUGINS})
set (VERSION_SHORTCUTS "1.3.4")
set (PACKAGE_SHORTCUTS "cd-shortcuts")
set (shortcutsdatadir "${pluginsdatadir}/shortcuts")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/data/shortcuts.conf.in ${CMAKE_CURRENT_BINARY_DIR}/shortcuts/data/shortcuts.conf)
add_subdirectory (shortcuts)

############# SHOW DESKTOP #################
message (STATUS "> ShowDesktop:")
set (with_xrandr "yes")
pkg_check_modules (SHOW_DESKTOP_XRANDR xrandr)
if ("${SHOW_DESKTOP_XRANDR_FOUND}" STREQUAL "")
	message (STATUS "Could not find xrandr; ShowDesktop won't be built with screen resolution abilities.")
	message (WARNING "This module is required to compile ShowDesktop applet with xRandr support: xrandr")
	set (MODULES_MISSING "${MODULES_MISSING} xrandr")
	set (with_xrandr "no")
endif()
set (GETTEXT_SHOW_DESKTOP ${GETTEXT_PLUGINS})
set (VERSION_SHOW_DESKTOP "1.2.7")
set (PACKAGE_SHOW_DESKTOP "cd-showDesktop")
set (show_desktopdatadir "${pluginsdatadir}/showDesktop")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/showDesktop/data/showDesktop.conf.in ${CMAKE_CURRENT_BINARY_DIR}/showDesktop/data/showDesktop.conf)
add_subdirectory (showDesktop)

############# SHOW MOUSE #################
message (STATUS "> Show Mouse:")
set (GETTEXT_SHOW_MOUSE ${GETTEXT_PLUGINS})
set (VERSION_SHOW_MOUSE "1.0.4")
set (PACKAGE_SHOW_MOUSE "cd-show_mouse")
set (show_mousedatadir "${pluginsdatadir}/show_mouse")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/show-mouse/data/show_mouse.conf.in ${CMAKE_CURRENT_BINARY_DIR}/show-mouse/data/show_mouse.conf)
add_subdirectory (show-mouse)

############# SLIDER #################
message (STATUS "> Slider:")
set (with_exif "yes")
pkg_check_modules (EXIF libexif)
if ("${EXIF_FOUND}" STREQUAL "")
	message (STATUS "Could not find libexif; Slider won't be able to rotate the images accordingly to their exif data.")
	message (WARNING "This module is required to compile Slider applet with Exif support: libexif")
	set (MODULES_MISSING "${MODULES_MISSING} libexif")
	set (with_exif "no")
endif()
set (GETTEXT_SLIDER ${GETTEXT_PLUGINS})
set (VERSION_SLIDER "2.0.13")
set (PACKAGE_SLIDER "cd-slider")
set (sliderdatadir "${pluginsdatadir}/slider")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/slider/data/slider.conf.in ${CMAKE_CURRENT_BINARY_DIR}/slider/data/slider.conf)
add_subdirectory (slider)

############# STACK #################
message (STATUS "> Stack:")
set (GETTEXT_STACK ${GETTEXT_PLUGINS})
set (VERSION_STACK "0.3.4")
set (PACKAGE_STACK "cd-stack")
set (stackdatadir "${pluginsdatadir}/stack")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/stack/data/stack.conf.in ${CMAKE_CURRENT_BINARY_DIR}/stack/data/stack.conf)
add_subdirectory (stack)

############# STATUS NOTIFIER #################
message (STATUS "> Status Notifier:")
if ("${DBUSMENU_GTK_FOUND}" STREQUAL "")
	message (STATUS "Could not find Status-Notifier libs; Cairo-Dock won't be built with Status-Notifier support.")
	message (WARNING "This module is required to compile Status Notifier applet: ${DBUSMENU_GTK_MODULE}(-0.4)")
	# set (MODULES_MISSING "${MODULES_MISSING} dbusmenu-gtk(-0.4)") # it should already be in the list.
	set (with_status_notifier "no")
else()
	if ("${force_indicator_applicationadded_have_hint}" STREQUAL "yes")
		set (INDICATOR_APPLICATIONADDED_HAS_HINT 1)
	elseif ("${force_indicator_applicationadded_have_hint}" STREQUAL "no")
		set (INDICATOR_APPLICATIONADDED_HAS_HINT 0)
	endif()
	message (STATUS "  ApplicationAdded signal has 'Hint' = ${INDICATOR_APPLICATIONADDED_HAS_HINT}")
	set (GETTEXT_STATUS_NOTIFIER ${GETTEXT_PLUGINS})
	set (VERSION_STATUS_NOTIFIER "0.1.7")
	set (PACKAGE_STATUS_NOTIFIER "cd-status-notifier")
	set (with_status_notifier "yes")
	set (status_notifierdatadir "${pluginsdatadir}/Status-Notifier")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Status-Notifier/data/Status-Notifier.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Status-Notifier/data/Status-Notifier.conf)
	add_subdirectory (Status-Notifier)
endif()

############# SWITCHER #################
message (STATUS "> Switcher:")
set (GETTEXT_SWITCHER ${GETTEXT_PLUGINS})
set (VERSION_SWITCHER "2.1.10")
set (PACKAGE_SWITCHER "cd-switcher")
set (switcherdatadir "${pluginsdatadir}/switcher")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/switcher/data/switcher.conf.in ${CMAKE_CURRENT_BINARY_DIR}/switcher/data/switcher.conf)
add_subdirectory (switcher)

############# SYSTEM MONITOR #################
message (STATUS "> System Monitor:")
set (with_sensors "yes")
find_package (Sensors)
if (NOT "${SENSORS_FOUND}" STREQUAL "TRUE")
	message (STATUS "Could not find libsensors; System-Monitor won't be able to monitor fan speed and CPU temperature.")
	message (WARNING "This package is required to compile System Monitor applet with Sensors support: sensors")
	set (PROGRAMS_MISSING "${PROGRAMS_MISSING} sensors")
	set (with_sensors "no")
endif()
set (GETTEXT_SYSTEM_MONITOR ${GETTEXT_PLUGINS})
set (VERSION_SYSTEM_MONITOR "1.0.12")
set (PACKAGE_SYSTEM_MONITOR "cd-system-monitor")
set (system_monitordatadir "${pluginsdatadir}/System-monitor")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/System-Monitor/data/System-Monitor.conf.in ${CMAKE_CURRENT_BINARY_DIR}/System-Monitor/data/System-Monitor.conf)
add_subdirectory (System-Monitor)

############# SYSTRAY #################
message (STATUS "> Systray:")
set (GETTEXT_SYSTRAY ${GETTEXT_PLUGINS})
set (VERSION_SYSTRAY "0.2.5")
set (PACKAGE_SYSTRAY "cd-systray")
set (systraydatadir "${pluginsdatadir}/systray")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/systray/data/systray.conf.in ${CMAKE_CURRENT_BINARY_DIR}/systray/data/systray.conf)
add_subdirectory (systray)

############# TERMINAL #################
message (STATUS "> Terminal:")
if ("${gtkversion}" STREQUAL "2")
	set (TERMINAL_MODULE vte)
else()
	set (TERMINAL_MODULE vte-2.90)
endif()
pkg_check_modules (TERMINAL_PACKAGE ${TERMINAL_MODULE})
if ("${TERMINAL_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find ${TERMINAL_MODULE}; Cairo-Dock won't be built with terminal applet.")
	message (WARNING "This module is required to compile Terminal applet: ${TERMINAL_MODULE}")
	set (MODULES_MISSING "${MODULES_MISSING} ${TERMINAL_MODULE}")
	set (with_terminal "no")
else()
	set (GETTEXT_TERMINAL ${GETTEXT_PLUGINS})
	set (VERSION_TERMINAL "1.0.13")
	set (PACKAGE_TERMINAL "cd-terminal")
	set (with_terminal "yes")
	set (terminaldatadir "${pluginsdatadir}/terminal")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/terminal/data/terminal.conf.in ${CMAKE_CURRENT_BINARY_DIR}/terminal/data/terminal.conf)
	add_subdirectory (terminal)
endif()

############# TOMBOY #################
message (STATUS "> Tomboy:")
set (GETTEXT_TOMBOY ${GETTEXT_PLUGINS})
set (VERSION_TOMBOY "1.3.4")
set (PACKAGE_TOMBOY "cd-tomboy")
set (tomboydatadir "${pluginsdatadir}/tomboy")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/tomboy/data/tomboy.conf.in ${CMAKE_CURRENT_BINARY_DIR}/tomboy/data/tomboy.conf)
add_subdirectory (tomboy)

############# TOONS #################
message (STATUS "> Toons:")
set (GETTEXT_TOONS ${GETTEXT_PLUGINS})
set (VERSION_TOONS "1.0.13")
set (PACKAGE_TOONS "cd-Toons")
set (toonsdatadir "${pluginsdatadir}/Toons")
set (toonsuserdirname "Toons")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Toons/data/Toons.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Toons/data/Toons.conf)
add_subdirectory (Toons)

############# WEATHER #################
message (STATUS "> Weather:")
set (GETTEXT_WEATHER ${GETTEXT_PLUGINS})
set (VERSION_WEATHER "1.2.14")
set (PACKAGE_WEATHER "cd-weather")
set (weatherdatadir "${pluginsdatadir}/weather")
set (weatheruserdirname "weather")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/weather/data/weather.conf.in ${CMAKE_CURRENT_BINARY_DIR}/weather/data/weather.conf)
add_subdirectory (weather)

############# WEBLETS #################
message (STATUS "> Weblets:")
set (with_weblets "no")
if (NOT "${enable-weblets}" STREQUAL "no")
	if ("${gtkversion}" STREQUAL "2")
		set (WEBKIT_MODULE webkit-1.0)
	else()
		set (WEBKIT_MODULE webkitgtk-3.0)
	endif()
	pkg_check_modules (WEBKIT ${WEBKIT_MODULE})
	if ("${WEBKIT_FOUND}" STREQUAL "")
		message (STATUS "Could not find ${WEBKIT_MODULE}; Cairo-Dock won't be built with Weblets applet.")
		message (WARNING "This module is required to compile Weblets applet: ${WEBKIT_MODULE}")
		set (MODULES_MISSING "${MODULES_MISSING} ${WEBKIT_MODULE}")
	else()
		set (GETTEXT_WEBLETS ${GETTEXT_PLUGINS})
		set (VERSION_WEBLETS "0.0.13")
		set (PACKAGE_WEBLETS "cd-weblets")
		set (webletsdatadir "${pluginsdatadir}/weblets")
		set (with_weblets "yes")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/weblets/data/weblets.conf.in ${CMAKE_CURRENT_BINARY_DIR}/weblets/data/weblets.conf)
		add_subdirectory ("weblets")
	endif()
endif()

############# WIFI #################
message (STATUS "> WiFi:")
set (GETTEXT_WIFI ${GETTEXT_PLUGINS})
set (VERSION_WIFI "1.3.1")
set (PACKAGE_WIFI "cd-wifi")
set (wifidatadir "${pluginsdatadir}/wifi")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/wifi/data/wifi.conf.in ${CMAKE_CURRENT_BINARY_DIR}/wifi/data/wifi.conf)
add_subdirectory (wifi)

############# XFCE-INTEGRATION #################
message (STATUS "> XFCE-Integration:")
set (with_xfce_integration "no")
if (NOT "${enable-xfce-integration}" STREQUAL "no")
	pkg_check_modules (XFCE_INTEGRATION_THUNAR QUIET thunar-vfs-1) # thunar-vfs is not used if gvfs (gio) is available
	pkg_check_modules (XFCE_INTEGRATION_GVFS gio-2.0)
	if ("${XFCE_INTEGRATION_THUNAR_FOUND}" STREQUAL "" AND "${XFCE_INTEGRATION_GVFS_FOUND}" STREQUAL "")
		message (STATUS "Could find neither thunar-vfs or libgio-2.0; Cairo-Dock won't be built with XFCE support.")
		message (STATUS "This module is required to compile XFCE-Integration applet: gio-2.0 (or thunar-vfs-1)")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} gio-2.0")
	else()
		set (VERSION_XFCE_INTEGRATION "1.0.3")
		set (PACKAGE_XFCE_INTEGRATION "cd_xfce-integration")
		set (with_xfce_integration "yes")
		set (xfce_integrationdatadir "${pluginsdatadir}/xfce-integration")
		add_subdirectory ("xfce-integration")
	endif()
endif()

############# XGAMMA #################
message (STATUS "> XGamma:")
pkg_check_modules (XGAMMA_PACKAGE x11 xxf86vm)
if ("${XGAMMA_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find xxf86vm; Cairo-Dock won't be built with Xgamma applet.")
	message (WARNING "These modules are required to compile XGamma applet: x11 and xxf86vm")
	set (MODULES_MISSING "${MODULES_MISSING} x11 xxf86vm")
	set (with_xgamma "no")
else()
	set (GETTEXT_XGAMMA ${GETTEXT_PLUGINS})
	set (VERSION_XGAMMA "1.2.3")
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
message (STATUS "Stable:")
message (STATUS " Integration plug-ins:")
message (STATUS " - with Gnome support:             ${with_gnome_integration}")
message (STATUS " - with KDE support:               ${with_kde_integration}")
message (STATUS " - with XFCE support:              ${with_xfce_integration}")
message (STATUS " Plug-ins:")
message (STATUS " - with Alsa-Mixer applet:         ${with_alsa}")
message (STATUS " - with GMenu applet:              ${with_gmenu}")
message (STATUS " - with Impulse applet:            ${with_impulse}")
message (STATUS " - with Keyboard-indicator applet: ${with_keyboard_indicator}")
message (STATUS " - with Mail applet:               ${with_mail}")
if (NOT "${INDICATOR_NEW_VERSION}" STREQUAL "1" OR "${enable-memenu}" STREQUAL "yes")  # oneiric or newer
message (STATUS " - with Me-Menu applet applet:     ${with_me_menu}")
endif()
message (STATUS " - with Messaging-Menu applet:     ${with_messaging_menu}")
message (STATUS " - with Recent-Events applet:      ${with_recent_events}")
message (STATUS " - with Status-Notifier applet:    ${with_status_notifier}")
message (STATUS " - with Terminal applet:           ${with_terminal}")
message (STATUS " - with Weblets applet:            ${with_weblets}")
message (STATUS " - with Xgamma applet:             ${with_xgamma}")
message (STATUS "Add On:")
message (STATUS " - with Sound-Menu support:        ${with_soundmenu}")
message (STATUS " - with Screen Resolution support: ${with_xrandr}")
message (STATUS " - with Sensors support:           ${with_sensors}")
message (STATUS " - with UPower support:            ${with_upower_support}")
message (STATUS " - with iCal support:              ${with_ical}")
message (STATUS "Third Party Interfaces:")
message (STATUS " - with Python interface:          ${with_python}")
message (STATUS " - with Ruby interface:            ${with_ruby}")
message (STATUS " - with Mono interface:            ${with_mono}")
message (STATUS " - with Vala interface:            ${with_vala}")
message (STATUS " - with Vala Translator (valac):   ${with_valac}")
message (STATUS "Unstable:")
message (STATUS " - with Global-Menu applet:        ${with_global_menu}")
message (STATUS " - with Disks applet:              ${with_disks}")
message (STATUS " - with Doncky applet:             ${with_doncky}")
message (STATUS " - with KDE experimental support:  ${with_kde_integration2}")
message (STATUS " - with Network-Monitor applet:    ${with_network_monitor}")
message (STATUS " - with Scooby-Do applet:          ${with_scooby_do}")
message (STATUS "Deprecated:")
message (STATUS " - with old Gnome support:         ${with_gnome_integration_old}")

if (NOT "${MODULES_MISSING}" STREQUAL "")
	message ("")
	message (STATUS "WARNING: It seems that one (or more) module(s) is (are) missing:${MODULES_MISSING}")
	message (STATUS "It can be interesting to install them (especially if you're a maintainer of Cairo-Dock's packages).")
endif ()

if (NOT "${PROGRAMS_MISSING}" STREQUAL "")
	message ("")
	message (STATUS "WARNING: It seems that one (or more) program(s) or package(s) is (are) missing:${PROGRAMS_MISSING}")
	message (STATUS "It can be interesting to install them (especially if you're a maintainer of Cairo-Dock's packages).")
endif ()

if (NOT "${MODULES_INTEGRATION_MISSING}" STREQUAL "")
	message ("")
	message (STATUS "WARNING: It seems that one (or more) module(s) for the integration with the system is (are) missing:${MODULES_INTEGRATION_MISSING}")
	message (STATUS "It's maybe not required for the user but it's recommended to install them if you're a maintainer of Cairo-Dock's packages.")
endif ()
message ("")
