########### requirements ###############

cmake_minimum_required (VERSION 3.16.0)
find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)
include ("${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/GNUInstallDirs.cmake")

########### project ###############

project ("cairo-dock-plugins")
set (VERSION "3.5.99.beta5")
# can be different from VERSION, e.g. if a new version of plugins does not depend on things added to core
set (CORE_REQUIRED_VERSION "3.5.99.beta5")

add_definitions (-std=gnu99 -Wall -Werror-implicit-function-declaration) # -Wextra -Wwrite-strings -Wuninitialized -Werror-implicit-function-declaration -Wstrict-prototypes -Wreturn-type -Wparentheses -Warray-bounds)
if (NOT DEFINED CMAKE_BUILD_TYPE)
	add_definitions (-O3)
endif()
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

############ sources tarball #############

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
set (CPACK_SOURCE_IGNORE_FILES
	"/build/;/.bzr/;/.bzrignore$;/.git/;/.gitignore$;/misc/;~$;${CPACK_SOURCE_IGNORE_FILES}")
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

if ("${PACKAGEMENT}" STREQUAL "") ## do not check if we're using the same version when packaging the application (e.g. if we just want to generate a tarball of a previous version)
	# we need GLDI which contains most global var: check it now to avoid many errors with regex
	pkg_check_modules (GLDI REQUIRED gldi>=${CORE_REQUIRED_VERSION})
endif ()

set (dock_version ${VERSION}) # should be the same except if PACKAGEMENT is set

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

# check that installation dir matches with the core
GET_FILENAME_COMPONENT(libdir "${pluginsdir}/.." ABSOLUTE)  # /usr/lib # or /usr/lib/x86_64-linux-gnu
GET_FILENAME_COMPONENT(pkgdatadir "${pluginsdatadir}/.." ABSOLUTE)  # /usr/share/cairo-dock
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

# set internationalisation
set (GETTEXT_PLUGINS "cairo-dock-plugins")
set (localedir "${prefix}/${CMAKE_INSTALL_LOCALEDIR}")
set (gaugesdir "${datadir}/cairo-dock/gauges")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")  # additionnal FindPackage files

set (MODULES_MISSING "")
set (MODULES_INTEGRATION_MISSING "")
set (PROGRAMS_MISSING "")


###########     Misc     ###############
macro (enable_if_not_defined MODULE1)
	if (NOT DEFINED ${MODULE1}) # true if not defined
		set (${MODULE1} TRUE)
	endif ()
endmacro (enable_if_not_defined)


##########     Config    ###############

enable_if_not_defined (force-icon-in-menus)
if (force-icon-in-menus)  # we believe that not showing icons in the menus by default is a terrible idea; unfortunately, it's not easily undoable for an end-user; so until this is fixed by a big player (Gnome, Ubuntu or other), we'll force the display, unless "-Dforce-icon-in-menus=yes" is provided in the cmake command.
	add_definitions (-DCAIRO_DOCK_FORCE_ICON_IN_MENUS=1)
else()
	add_definitions (-DCAIRO_DOCK_FORCE_ICON_IN_MENUS=0)
endif()


########### dependancies ###############

message ("")
message (STATUS "=====================")
message (STATUS "Check dependencies...")
message (STATUS "=====================")
message ("")

pkg_check_modules ("PACKAGE" REQUIRED "gldi")  # we check first the common libs that most plug-ins would need: basically, all the libs used by the core.
STRING (REGEX REPLACE "gldi;" "" PACKAGE_LIBRARIES "${PACKAGE_LIBRARIES}")  # but we don't want to link to gldi, since we are dl-opened by it.

# add_definitions (-DGTK_DISABLE_DEPRECATED="1")

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



################################################################################
#          STATIC LIBS AND DEPENDENCES USED BY MORE THAN ONE PLUG-IN           #
################################################################################



############# STATIC LIBRARIES ################
message (STATUS "> Static Libraries:")

############# GVFS-INTEGRATION ################
message (STATUS "> GVFS-Integration:")
pkg_check_modules ("LIBGIO" "gio-2.0") ## GIO is used by a few applets: do not offer the possibility to not disable it
add_subdirectory (gvfs-integration)

############# INDICATOR-APPLET #################
message (STATUS "> Indicator-Applet:")
enable_if_not_defined (enable-dbusmenu-support)
if (enable-dbusmenu-support)
	# Note: the names of dbusmenu-glib, dbusmenu-gtk and indicator have changed...
	## DBusMenu Glib
	set (DBUSMENU_MODULE dbusmenu-glib-0.4)
	pkg_check_modules (DBUSMENU ${DBUSMENU_MODULE})
	if (NOT DBUSMENU_FOUND)
		set (DBUSMENU_MODULE dbusmenu-glib)
		pkg_check_modules (DBUSMENU ${DBUSMENU_MODULE})
		set (INDICATOR_APPLICATIONADDED_HAS_HINT 0) # doesn't have 'hint' with old version
	else ()
		set (INDICATOR_APPLICATIONADDED_HAS_HINT 1) # now the ApplicationAdded signal (Status-Notifier) has a new parameter (hint) => https://code.launchpad.net/~ted/indicator-application/name-hints/+merge/67213
	endif()

	## DBusMenu Gtk
	set (DBUSMENU_GTK_MODULE dbusmenu-gtk3-0.4)
	pkg_check_modules (DBUSMENU_GTK ${DBUSMENU_GTK_MODULE})
	if (NOT DBUSMENU_GTK_FOUND)
		set (DBUSMENU_GTK_MODULE dbusmenu-gtk3)
		pkg_check_modules (DBUSMENU_GTK ${DBUSMENU_GTK_MODULE})
	endif()
endif()

## Indicator
enable_if_not_defined (enable-indicator-support)
if (enable-indicator-support)
	set (INDICATOR_APPLET_MODULE ayatana-indicator3-0.4)
	pkg_check_modules (INDICATOR_APPLET ${INDICATOR_APPLET_MODULE})
endif()

## libido
enable_if_not_defined (enable-libido-support)
if (enable-libido-support)
	set (IDO_MODULE libayatana-ido3-0.4)
	pkg_check_modules (IDO ${IDO_MODULE})
endif()

## Extract versions of DBusMenu
if (DBUSMENU_FOUND AND DBUSMENU_GTK_FOUND)
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
		set (INDICATOR_APPLICATIONADDED_HAS_TITLE 1) # Status-Notifier: new parameter "Title" in the "added" signal since Precise
	else()
		set (INDICATOR_APPLICATIONADDED_HAS_TITLE 0)
	endif()
endif()

## Extract version of indicator
if (INDICATOR_APPLET_FOUND)
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

	execute_process(
		COMMAND pkg-config --variable=iconsdir ayatana-indicator3-0.4
		OUTPUT_VARIABLE INDICATORICONSDIR)
	execute_process(
		COMMAND pkg-config --variable=indicatordir ayatana-indicator3-0.4
		OUTPUT_VARIABLE INDICATORDIR)
	STRING (REGEX REPLACE "\n" "" INDICATORDIR ${INDICATORDIR})
	set (with_indicator3 yes)
	message (STATUS "  Indicators: with Indicator3 support")

	STRING (REGEX REPLACE "\n" "" INDICATORICONSDIR "${INDICATORICONSDIR}")  # la commande rajoute un retour chariot ...
	if (with_indicator3)
		add_subdirectory (Indicator-applet3)
	endif()
	## DBusMenu is needed for Indicator-applet
	if (DBUSMENU_FOUND AND DBUSMENU_GTK_FOUND)
		add_subdirectory (Indicator-applet)
		set (with_indicator "yes")
	endif()
endif()

if (NOT DBUSMENU_FOUND OR NOT DBUSMENU_GTK_FOUND OR NOT INDICATOR_APPLET_FOUND)
	message (WARNING "These modules are required to compile Indicators applet (Messaging-Menu, Sound-Control, Status-Notifier and Global-Menu): ${INDICATOR_APPLET_MODULE}(-0.4), ${IDO_MODULE}, ${DBUSMENU_MODULE}(-0.4), ${DBUSMENU_GTK_MODULE}(-0.4)")
	set (MODULES_MISSING "${MODULES_MISSING} ${INDICATOR_APPLET_MODULE}(-0.4) (${IDO_MODULE}) ${DBUSMENU_MODULE}(-0.4) ${DBUSMENU_GTK_MODULE}(-0.4)")
endif()

############# UPOWER #####################
message (STATUS "> UPower:")
enable_if_not_defined (enable-upower-support)
if (enable-upower-support)
	pkg_check_modules (UPOWER upower-glib)  # useful for Powermanager too.
endif()
if (UPOWER_FOUND)
	set (with_upower_support yes)
	STRING (REGEX REPLACE "\\..*" "" UPOWER_MAJOR "${UPOWER_VERSION}") # 2.28.3 => 2
	STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" UPOWER_MINOR "${UPOWER_VERSION}")  # 2.28.3 => 2.28
	STRING (REGEX REPLACE "\\.[0-9]*" "" UPOWER_MINOR "${UPOWER_MINOR}") # 2.28 => 28
	if (${UPOWER_MAJOR} GREATER 0 OR ${UPOWER_MINOR} GREATER 89)
		message (STATUS "   Your version of UPower no longer supports suspend/hibernate features")
		set (with_upower_support "yes (0.99+)")
	else()
		set (UPOWER_SUPPORTS_SUSPEND_HIBERNATE 1)
	endif()
else()
	set (with_upower_support no)
	message (STATUS "Could not find upower-glib; Logout and PowerManager plugin won't be built with UPower support.")
	message (WARNING "This module is required to compile LogOut and PowerManager applet with UPower support: upower-glib")
	set (MODULES_MISSING "${MODULES_MISSING} upower-glib")
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

GET_FILENAME_COMPONENT(DEBIAN_VERSION /etc/debian_version ABSOLUTE)
if (EXISTS ${DEBIAN_VERSION})
	set (DEBIAN_PKG_MANAGERS "/usr/bin/apt-get /usr/bin/dpkg /usr/bin/aptitude")
endif()



################################################################################
#                                   PLUG-INS                                   #
################################################################################



############# SHARED LIBRARIES ###########
message (STATUS "> Shared Libraries:")

############# ALSA_MIXER #################
message (STATUS "> AlsaMixer:")
set (with_alsa no)
set (with_soundmenu no)
enable_if_not_defined (enable-alsa-mixer)
if (enable-alsa-mixer)
	pkg_check_modules (ALSA_MIXER_PACKAGE alsa)
	if (NOT ALSA_MIXER_PACKAGE_FOUND)
		message (STATUS "Could not find alsa; Cairo-Dock won't be built with AlsaMixer applet.")
		message (WARNING "This module is required to compile AlsaMixer applet: alsa")
		set (MODULES_MISSING "${MODULES_MISSING} alsa")
	else()
		set (GETTEXT_ALSA_MIXER ${GETTEXT_PLUGINS})
		set (VERSION_ALSA_MIXER "2.1.5")
		set (PACKAGE_ALSA_MIXER "cd-AlsaMixer")
		set (with_alsa yes)
		set (alsa_mixerdatadir "${pluginsdatadir}/AlsaMixer")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/alsaMixer/data/AlsaMixer.conf.in ${CMAKE_CURRENT_BINARY_DIR}/alsaMixer/data/AlsaMixer.conf)
		if (with_indicator3)
			set (INDICATOR_SOUNDMENU_WITH_IND3 TRUE)
			set (with_soundmenu yes)
		endif()
		add_subdirectory ("alsaMixer")
	endif()
endif()

############# ANIMATED ICONS #################
message (STATUS "> Animated Icons:")
set (GETTEXT_ANIMATED_ICONS ${GETTEXT_PLUGINS})
set (VERSION_ANIMATED_ICONS "1.0.13")
set (PACKAGE_ANIMATED_ICONS "cd-Animated-icons")
set (animated_iconsdatadir "${pluginsdatadir}/Animated-icons")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Animated-icons/data/Animated-icons.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Animated-icons/data/Animated-icons.conf)
add_subdirectory (Animated-icons)

############# CAIRO_PENGUIN #################
message (STATUS "> Cairo Penguin:")
set (GETTEXT_CAIRO_PENGUIN ${GETTEXT_PLUGINS})
set (VERSION_CAIRO_PENGUIN "1.1.13")
set (PACKAGE_CAIRO_PENGUIN "cd-Cairo-Penguin")
set (cairo_penguinuserdirname "Cairo-Penguin")
set (cairo_penguindatadir "${pluginsdatadir}/Cairo-Penguin")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Cairo-Penguin/data/Cairo-Penguin.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Cairo-Penguin/data/Cairo-Penguin.conf)
add_subdirectory (Cairo-Penguin)

############# CLIPPER #################
message (STATUS "> Clipper:")
set (GETTEXT_CLIPPER ${GETTEXT_PLUGINS})
set (VERSION_CLIPPER "1.1.9")
set (PACKAGE_CLIPPER "cd-Clipper")
set (Clipperdatadir "${pluginsdatadir}/Clipper")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Clipper/data/Clipper.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Clipper/data/Clipper.conf)
add_subdirectory (Clipper)

############# CLOCK #################
message (STATUS "> Clock:")
enable_if_not_defined (enable-ical-support)
set (with_ical no)
if (enable-ical-support)
	pkg_check_modules ("LIBICAL_PACKAGE" "libical")
	if (NOT LIBICAL_PACKAGE_FOUND)
		message (STATUS "Could not find libical; Clock plugin won't be built with iCal support.")
		message (WARNING "This module is required to compile Clock applet with iCal support: libical")
		set (MODULES_MISSING "${MODULES_MISSING} libical")
	else ()
		set (with_ical yes)
	endif()
endif()
set (GETTEXT_CLOCK ${GETTEXT_PLUGINS})
set (VERSION_CLOCK "2.3.1")
set (PACKAGE_CLOCK "cd-clock")
set (clockuserdirname "clock")
set (clockdatadir "${pluginsdatadir}/clock")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/clock/data/clock.conf.in ${CMAKE_CURRENT_BINARY_DIR}/clock/data/clock.conf)
add_subdirectory (clock)

############# COMPOSITE_MANAGER #################
message (STATUS "> Composite Manager:")
set (GETTEXT_COMPOSITE_MANAGER ${GETTEXT_PLUGINS})
set (VERSION_COMPOSITE_MANAGER "1.0.4")
set (PACKAGE_COMPOSITE_MANAGER "cd-Composite-Manager")
set (composite_managerdatadir "${pluginsdatadir}/Composite-Manager")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Composite-Manager/data/Composite-Manager.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Composite-Manager/data/Composite-Manager.conf)
add_subdirectory (Composite-Manager)

############# DBUS #################
message (STATUS "> DBus:")
set (with_python3 no)
enable_if_not_defined (enable-python-interface)
if (enable-python-interface)
	message (STATUS " * Python:")
	# find python executables
	find_program (PYTHON3_EXECUTABLE python3)
	if (PYTHON3_EXECUTABLE AND EXISTS ${PYTHON3_EXECUTABLE})
		set (PYTHON3_FOUND TRUE)
	endif()
	if (NOT PYTHON3_FOUND)  # if we didn't find one of them, 
		find_program (PYTHON_EXECUTABLE python)
		if (PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})
			EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c
				"from __future__ import print_function; import sys; print(sys.version)"
				OUTPUT_VARIABLE PYTHON_VERSION)
			STRING (REGEX REPLACE "\\..*" "" PYTHON_VERSION ${PYTHON_VERSION})
			if ("${PYTHON_VERSION}" STREQUAL "3")
				set (PYTHON3_EXECUTABLE ${PYTHON_EXECUTABLE})
				set (PYTHON3_FOUND TRUE)
			endif()
		endif()
	endif()
	
	# see if we found anything
	if (NOT PYTHON3_FOUND)
		message (STATUS "Could not find Python, won't install Python interface.")
		message (WARNING "This program is required to compile DBus applet with Python interface: python (version 3)")
		set (PROGRAMS_MISSING "${PROGRAMS_MISSING} python")
	else()
		message (STATUS "   Python executable program: ${PYTHON3_EXECUTABLE}")
#		if (EXISTS ${DEBIAN_VERSION}) -- TODO: deal with debian-specific quirks separately?
#			message (STATUS "   will use '--install-layout deb' with 'python setup.py install'")
#			set (DEBIAN_INSTALL_LAYOUT "--install-layout deb")
#		endif()
#		try to figure out the installation path ourselves -- this is a terrible hack,
#		but see e.g. here https://github.com/mesonbuild/meson/issues/8739
		execute_process(COMMAND "${PYTHON3_EXECUTABLE}" "shared-files/build/detect_python_paths.py" "${prefix}" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE python_destdir RESULT_VARIABLE python_dest_found)
		if (python_dest_found)
			message (STATUS ${python_dest_found})
			message (WARNING "Cannot find Python installation destination!")
		else()
			STRING (REGEX REPLACE "\n" "" python_destdir ${python_destdir})
			message (STATUS "   Python installation destination: ${python_destdir}")
			set (with_python3 yes)
		endif()
	endif()
endif()

enable_if_not_defined (enable-ruby-interface)
if (enable-ruby-interface)
	message (STATUS " * Ruby:")
	# find_package(Ruby) found libs of ruby-dev but we only need a directory where we can install ruby libs.
	find_program (RUBY_EXECUTABLE ruby)
	EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['vendordir']"
		OUTPUT_VARIABLE RUBY_LIB_DIR)
	message (STATUS "   Ruby library dir: ${RUBY_LIB_DIR}")
	if ("${RUBY_LIB_DIR}" STREQUAL "" OR "${RUBY_LIB_DIR}" STREQUAL "nil")
		message (STATUS "Could not find ruby libs, won't install Ruby interface.")
		message (WARNING "This program is required to compile DBus applet with Ruby interface: ruby")
		set (PROGRAMS_MISSING "${PROGRAMS_MISSING} ruby")
		set (with_ruby no)
	else()
		string (FIND ${RUBY_LIB_DIR} ${CMAKE_INSTALL_PREFIX} ruby_prefix_pos)
		if (NOT ${ruby_prefix_pos} EQUAL 0)
			message (STATUS "Ruby library directory not under install prefix (${CMAKE_INSTALL_PREFIX}), not installing ruby interface.")
			set (with_ruby no)
		else()
			message (STATUS "   will be installed in: ${RUBY_LIB_DIR}")
			set (RUBY_FOUND "TRUE")
			set (with_ruby yes)
		endif()
	endif()
endif()

set (with_mono no)
enable_if_not_defined (enable-mono-interface)
if (enable-mono-interface)
	message (STATUS " * Mono:")
	#find_package (Mono)
	find_program (GMCS_EXECUTABLE mcs)
	if (NOT GMCS_EXECUTABLE OR NOT EXISTS ${GMCS_EXECUTABLE})
		message (STATUS "Could not find Mono compiler mcs, won't build Mono interface.")
	else()
		pkg_check_modules (MONO_PACKAGE dbus-sharp-2.0 dbus-sharp-glib-2.0 glib-sharp-2.0)
		if (NOT MONO_PACKAGE_FOUND)
			message (STATUS "Could not find dbus-sharp-2.0, dbus-sharp-glib-2.0 or glib-sharp-2.0; won't be built Mono interface.")
			message (WARNING "These modules are required to compile DBus applet with Mono interface: dbus-sharp-2.0, dbus-sharp-glib-2.0 and glib-sharp-2.0")
			set (MODULES_MISSING "${MODULES_MISSING} dbus-sharp-2.0 dbus-sharp-glib-2.0 glib-sharp-2.0")
		else()
			set (MONO_FOUND TRUE)
			set (with_mono yes)
		endif()
	endif()
endif()

set (with_vala no)
set (with_valac no)
enable_if_not_defined (enable-vala-interface)
if (enable-vala-interface)
	message (STATUS " * Vala:")
	# Valac is only required to convert vala files to C files.
	#  So we can directly use produced files (c, h, vapi) without using valac.
	set (with_vala yes)
	set (with_valac no)
	find_program (VALAC_EXE valac)
	message (STATUS "   Path to valac: ${VALAC_EXE}")
	set (VERSION_VALA "1.0.0")
	message (STATUS "   Cairo-Dock Vala interface's ABI version: ${VERSION_VALA}")

	enable_if_not_defined (enable-vala-support)
	if (NOT enable-vala-support)
		set (with_vala no)
	# Glib < 2.26
	elseif (${GLIB_MAJOR} LESS 3 AND ${GLIB_MINOR} LESS 26)
		set (with_vala no)
	elseif (VALAC_EXE AND EXISTS ${VALAC_EXE}) # now it works with 0.10 and 0.11 (= next 0.12) # $VALAC_EXE may be the correct path, even if valac is not installed !
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
			set (VALAC_FOUND TRUE)
			set (with_valac yes)
		else()
			message (STATUS "   Vala compiler is too old (0.10 required) or too new (> 0.12), won't build Vala interface.")
		endif()
	else()
		message (STATUS "Could not find ValaC, won't build Vala interface with the version of your distribution.")
		message (WARNING "This program is required to compile DBus applet with Vala interface: valac (version > 0.10)")
		set (PROGRAMS_MISSING "${PROGRAMS_MISSING} valac")
	endif()
endif()


set (GETTEXT_DBUS ${GETTEXT_PLUGINS})
set (VERSION_DBUS "1.2.3")
set (PACKAGE_DBUS "cd-Dbus")
set (dbusdatadir "${pluginsdatadir}/Dbus")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Dbus/data/Dbus.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Dbus/data/Dbus.conf)
add_subdirectory (Dbus)

############# DESKLET_RENDERING #################
message (STATUS "> Desklet Rendering:")
set (GETTEXT_DESKLET_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DESKLET_RENDERING "1.5.8")
set (PACKAGE_DESKLET_RENDERING "cd-desklet-rendering")
set (desklet_renderingdatadir "${pluginsdatadir}/desklet-rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/desklet-rendering/data/desklet-rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/desklet-rendering/data/desklet-rendering.conf)
add_subdirectory (desklet-rendering)

############# DIALOG_RENDERING #################
message (STATUS "> Dialog Rendering:")
set (GETTEXT_DIALOG_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DIALOG_RENDERING "0.6.1")
set (PACKAGE_DIALOG_RENDERING "cd-dialog-rendering")
set (dialog_renderingdatadir "${pluginsdatadir}/dialog-rendering")
add_subdirectory (dialog-rendering)

############# DISKS #################
set (with_disks no) # unstable
if (enable-disks)
	message (STATUS "> Disks:")
	set (GETTEXT_DISKS ${GETTEXT_PLUGINS})
	set (VERSION_DISKS "0.0.7")
	set (PACKAGE_DISKS "cd-disks")
	set (with_disks yes)
	set (disksdatadir "${pluginsdatadir}/Disks")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Disks/data/Disks.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Disks/data/Disks.conf)
	add_subdirectory ("Disks")
endif()

############# DND2SHARE #################
message (STATUS "> DND2Share:")
set (GETTEXT_DND2SHARE ${GETTEXT_PLUGINS})
set (VERSION_DND2SHARE "1.0.12")
set (PACKAGE_DND2SHARE "cd-dnd2share")
set (dnd2sharedatadir "${pluginsdatadir}/dnd2share")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dnd2share/data/dnd2share.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dnd2share/data/dnd2share.conf)
add_subdirectory (dnd2share)

############# DOCK RENDERING #################
message (STATUS "> Dock Rendering:")
set (GETTEXT_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_RENDERING "1.5.11")
set (PACKAGE_RENDERING "cd-rendering")
set (renderingdatadir "${pluginsdatadir}/rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dock-rendering/data/rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dock-rendering/data/rendering.conf)
add_subdirectory (dock-rendering)

############# DONCKY #################
set (with_doncky no) # unstable
if (enable-doncky)
	message (STATUS "> Doncky:")
	set (GETTEXT_DONCKY ${GETTEXT_PLUGINS})
	set (VERSION_DONCKY "0.0.9")
	set (PACKAGE_DONCKY "cd-doncky")
	set (with_doncky yes)
	set (donckydatadir "${pluginsdatadir}/Doncky")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doncky/data/Doncky.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Doncky/data/Doncky.conf)
	add_subdirectory (Doncky)
endif()

############# DROP INDICATOR #################
message (STATUS "> Drop Indicator:")
set (GETTEXT_DROP_INDICATOR ${GETTEXT_PLUGINS})
set (VERSION_DROP_INDICATOR "1.1.7")
set (PACKAGE_DROP_INDICATOR "cd-drop_indicator")
set (drop_indicatordatadir "${pluginsdatadir}/drop-indicator")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/drop-indicator/data/drop_indicator.conf.in ${CMAKE_CURRENT_BINARY_DIR}/drop-indicator/data/drop_indicator.conf)
add_subdirectory (drop-indicator)

############# DUSTBIN #################
message (STATUS "> Dustbin:")
set (GETTEXT_DUSTBIN ${GETTEXT_PLUGINS})
set (VERSION_DUSTBIN "2.3.6")
set (PACKAGE_DUSTBIN "cd-dustbin")
set (dustbinuserdirname "dustbin")
set (dustbindatadir "${pluginsdatadir}/dustbin")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dustbin/data/dustbin.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dustbin/data/dustbin.conf)
add_subdirectory (dustbin)

############# FOLDERS #################
message (STATUS "> Folders:")
set (GETTEXT_FOLDERS ${GETTEXT_PLUGINS})
set (VERSION_FOLDERS "0.2.6")
set (PACKAGE_FOLDERS "cd-Folders")
set (foldersdatadir "${pluginsdatadir}/Folders")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Folders/data/Folders.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Folders/data/Folders.conf)
add_subdirectory (Folders)

############# GLOBAL-MENU #################
set (with_global_menu no) # unstable
if (enable-global-menu AND DBUSMENU_FOUND AND DBUSMENU_GTK_FOUND)  # currently only supported with new name of the indicator module and with newer version of dbusmenu
	message (STATUS "> Global-Menu:")
	set (GETTEXT_GLOBAL_MENU ${GETTEXT_PLUGINS})
	set (VERSION_GLOBAL_MENU "0.1.3")
	set (PACKAGE_GLOBAL_MENU "cd-Global-Menu")
	set (with_global_menu yes)
	set (global_menudatadir "${pluginsdatadir}/Global-Menu")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Global-Menu/data/Global-Menu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Global-Menu/data/Global-Menu.conf)
	add_subdirectory (Global-Menu)
endif()

############# GMENU #################
message (STATUS "> GMenu:")
set (with_gmenu no)
enable_if_not_defined (enable-gmenu)
if (enable-gmenu)
	set (GMENU_MODULE "libgnome-menu-3.0")
	pkg_check_modules (GMENU_PACKAGE ${GMENU_MODULE})
	if (NOT GMENU_PACKAGE_FOUND)
		message (STATUS "Could not find ${GMENU_MODULE}; Cairo-Dock won't be built with GMenu applet.")
		message (WARNING "This module is required to compile GMenu applet: ${GMENU_MODULE}")
		set (MODULES_MISSING "${MODULES_MISSING} ${GMENU_MODULE}")
	else()
		set (GETTEXT_GMENU ${GETTEXT_PLUGINS})
		set (VERSION_GMENU "2.0.1")
		set (with_gmenu "yes")
		set (PACKAGE_GMENU "cd-GMenu")
		set (gmenudatadir "${pluginsdatadir}/GMenu")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/GMenu/data/GMenu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/GMenu/data/GMenu.conf)
		add_subdirectory ("GMenu")
	endif()
endif()

############# GNOME-INTEGRATION #################
message (STATUS "> Gnome-Integration:")
set (with_gnome_integration no)
enable_if_not_defined (enable-gnome-integration)
if (enable-gnome-integration)
	pkg_check_modules (GNOME_INTEGRATION gio-2.0)
	if (NOT GNOME_INTEGRATION_FOUND)
		message (STATUS "Could not find gio; Cairo-Dock won't be built with Gnome>=2.22 support.")
		message (STATUS "This module is required to compile Gnome-Integration applet: gio-2.0")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} gio-2.0")
	else()
		set (GETTEXT_GNOME_INTEGRATION ${GETTEXT_PLUGINS})
		set (VERSION_GNOME_INTEGRATION "1.0.5")
		set (PACKAGE_GNOME_INTEGRATION "cd_gnome-integration")
		set (with_gnome_integration yes)
		set (gnome_integrationdatadir "${pluginsdatadir}/gnome-integration")
		add_subdirectory ("gnome-integration")
	endif()
endif()

############# GNOME-INTEGRATION-OLD #################
set (with_gnome_integration_old no) # deprecated
if (enable-old-gnome-integration)
	message (STATUS "> Gnome-Integration:")
	message (WARNING "This applet is deprecated")
	pkg_check_modules (OLD_GNOME_INTEGRATION gnome-vfs-2.0 libgnomeui-2.0)
	if (NOT OLD_GNOME_INTEGRATION_FOUND)
		message (STATUS "Could not find gnome-vfs and/or gnomeui; Cairo-Dock won't be built with Gnome<2.22 support.")
		message (STATUS "These modules are required to compile Gnome-Integration-Old applet: gnome-vfs-2.0 libgnomeui-2.0")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} gnome-vfs-2.0 libgnomeui-2.0")
	else()
		set (GETTEXT_GNOME_INTEGRATION_OLD ${GETTEXT_PLUGINS})
		set (VERSION_GNOME_INTEGRATION_OLD "1.0.6")
		set (PACKAGE_GNOME_INTEGRATION_OLD "cd_gnome-integration-old")
		set (with_gnome_integration_old yes)
		set (gnome_integration_olddatadir "${pluginsdatadir}/gnome-integration-old")
		add_subdirectory ("gnome-integration-old")
	endif()
endif()

############# ICON EFFECTS #################
message (STATUS "> Icon Effects:")
set (GETTEXT_ICON_EFFECTS ${GETTEXT_PLUGINS})
set (VERSION_ICON_EFFECTS "1.2.6")
set (PACKAGE_ICON_EFFECTS "cd-icon-effect")
set (icon_effectsdatadir "${pluginsdatadir}/icon-effect")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/icon-effect/data/icon-effect.conf.in ${CMAKE_CURRENT_BINARY_DIR}/icon-effect/data/icon-effect.conf)
add_subdirectory (icon-effect)

############# IMPULSE #################
set (with_impulse no)
message (STATUS "> Impulse:")
enable_if_not_defined (enable-impulse)
if (enable-impulse)
	pkg_check_modules (LIBPULSE libpulse)
	pkg_check_modules (FFTW3 fftw3)  # optional, not advised for distributions packages (it's not a small dependence...)
	if (NOT LIBPULSE_FOUND)
		message (STATUS "Could not find libpulse; Cairo-Dock won't be built with Impulse applet.")
		message (WARNING "These modules are required to compile Impulse applet: libpulse (and fftw3 - optional)")
		set (MODULES_MISSING "${MODULES_MISSING} libpulse")
	else()
		set (GETTEXT_IMPULSE ${GETTEXT_PLUGINS})
		set (VERSION_IMPULSE "0.0.7")
		set (PACKAGE_IMPULSE "cd-Impulse")
		set (with_impulse yes)
		set (impulsedatadir "${pluginsdatadir}/Impulse")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Impulse/data/Impulse.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Impulse/data/Impulse.conf)
		add_subdirectory (Impulse)
	endif()
endif()

############# INDICATOR_GENERIC #################
set (with_indicator_generic no)
if (with_indicator3)
	message (STATUS "> Indicator-Generic:")
	set (GETTEXT_INDICATOR_GENERIC ${GETTEXT_PLUGINS})
	set (VERSION_INDICATOR_GENERIC "0.0.2")
	set (PACKAGE_INDICATOR_GENERIC "cd-Indicator-Generic")
	set (with_indicator_generic yes)
	set (indicator_genericdatadir "${pluginsdatadir}/Indicator-Generic")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Indicator-Generic/data/Indicator-Generic.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Indicator-Generic/data/Indicator-Generic.conf)
	add_subdirectory (Indicator-Generic)
endif()

############# ILLUSION #################
message (STATUS "> Illusion:")
set (GETTEXT_ILLUSION ${GETTEXT_PLUGINS})
set (VERSION_ILLUSION "1.0.9")
set (PACKAGE_ILLUSION "cd-illusion")
set (illusiondatadir "${pluginsdatadir}/illusion")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/illusion/data/illusion.conf.in ${CMAKE_CURRENT_BINARY_DIR}/illusion/data/illusion.conf)
add_subdirectory (illusion)

############# KDE-INTEGRATION #################
message (STATUS "> KDE-Integration:")
set (with_kde_integration no)
enable_if_not_defined (enable-kde-integration)
set (with_kde_integration2 no) # highly unstable
if (enable-kde-integration2)
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

	if (NOT KDECORE_LIBRARY)
		message (STATUS " * KDECORE Dir: ${KDECORE_INCLUDE_DIR}")
		message (STATUS " * KDECORE Library: ${KDECORE_LIBRARY}")
	else()
		message (STATUS " * KDECORE unavailable")
	endif()
	if (NOT KIO_LIBRARY)
		message (STATUS " * KIO Dir: ${KIO_INCLUDE_DIR}")
		message (STATUS " * KIO Library: ${KIO_LIBRARY}")
	else()
		message (STATUS " * KIO unavailable")
	endif()
	if (NOT KDE_LIBRARY) ## always empty?
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
		set (VERSION_KDE_INTEGRATION "0.0.4")
		set (PACKAGE_KDE_INTEGRATION "cd_kde-integration")
		set (with_kde_integration2 yes)
		set (kde_integrationdatadir "${pluginsdatadir}/kde-integration2")
		add_subdirectory ("kde-integration2")
	else()
		message (STATUS "Could not find kde libs; Cairo-Dock won't be built with KDE support.")
		message (STATUS "These libraries are required to compile KDE experimental applet: kdecore, kio, kde4")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} kdecore, kio, kde4")
	endif()
elseif (enable-kde-integration)
	pkg_check_modules (KDE_INTEGRATION gio-2.0)
	message (STATUS "  KDE_INTEGRATION_FOUND: ${KDE_INTEGRATION_FOUND}")
	if (NOT KDE_INTEGRATION_FOUND)
		message (STATUS "Could not find gio; Cairo-Dock won't be built with KDE support.")
		message (STATUS "This module is required to compile KDE-Integration applet: gio-2.0")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} gio-2.0")
	else()
		set (GETTEXT_KDE_INTEGRATION ${GETTEXT_PLUGINS})
		set (VERSION_KDE_INTEGRATION "1.0.4")
		set (PACKAGE_KDE_INTEGRATION "cd_kde-integration")
		set (with_kde_integration yes)
		set (kde_integrationdatadir "${pluginsdatadir}/kde-integration")
		add_subdirectory ("kde-integration")
	endif()
endif()

############# KEYBOARD_INDICATOR #################
message (STATUS "> Keyboard-Indicator:")
set (with_keyboard_indicator no)
enable_if_not_defined (enable-keyboard-indicator)
if (enable-keyboard-indicator)
	pkg_check_modules (KEYBOARD_INDICATOR_PACKAGE libxklavier)
	if (NOT KEYBOARD_INDICATOR_PACKAGE_FOUND)
		message (STATUS "Could not find libxklavier; Cairo-Dock won't be built with keyboard-indicator applet.")
		message (WARNING "This module is required to compile Keyboard-Indicator applet: libxklavier")
		set (MODULES_MISSING "${MODULES_MISSING} libxklavier")
	else()
		set (GETTEXT_KEYBOARD_INDICATOR ${GETTEXT_PLUGINS})
		set (VERSION_KEYBOARD_INDICATOR "1.2.1")
		set (PACKAGE_KEYBOARD_INDICATOR "cd-keyboard-indicator")
		set (with_keyboard_indicator yes)
		set (keyboard_indicatordatadir "${pluginsdatadir}/keyboard-indicator")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/keyboard-indicator/data/keyboard-indicator.conf.in ${CMAKE_CURRENT_BINARY_DIR}/keyboard-indicator/data/keyboard-indicator.conf)
		add_subdirectory ("keyboard-indicator")
	endif()
endif()

############# LOGOUT #################
message (STATUS "> Logout:")
set (GETTEXT_LOGOUT ${GETTEXT_PLUGINS})
set (VERSION_LOGOUT "2.0.4")
set (PACKAGE_LOGOUT "cd-logout")
set (logoutdatadir "${pluginsdatadir}/logout")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/logout/data/logout.conf.in ${CMAKE_CURRENT_BINARY_DIR}/logout/data/logout.conf)
add_subdirectory (logout)

############# MAIL #################
message (STATUS "> Mail:")
set (with_mail no)
# find the compilation flags
enable_if_not_defined (enable-mail)
if (enable-mail)
	execute_process(
		COMMAND pkg-config libetpan --cflags
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
		COMMAND pkg-config libetpan --libs
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
		set (VERSION_MAIL "1.0.14")
		set (PACKAGE_MAIL "cd-mail")
		set (maildatadir "${pluginsdatadir}/mail")
		set (with_mail yes)
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/mail/data/mail.conf.in ${CMAKE_CURRENT_BINARY_DIR}/mail/data/mail.conf)
		add_subdirectory ("mail")
	endif()
endif()

############# MESSAGING_MENU #################
set (with_messaging_menu no)
if (with_indicator)
	message (STATUS "> Messaging-Menu:")
	if (with_indicator3)
		set (INDICATOR_MESSAGES_WITH_IND3 TRUE) # If it's supported, it's maybe better to use it
	endif()
	set (GETTEXT_MESSAGING_MENU ${GETTEXT_PLUGINS})
	set (VERSION_MESSAGING_MENU "1.0.7")
	set (PACKAGE_MESSAGING_MENU "cd-Messaging-Menu")
	set (with_messaging_menu yes)
	set (messaging_menudatadir "${pluginsdatadir}/Messaging-Menu")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Messaging-Menu/data/Messaging-Menu.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Messaging-Menu/data/Messaging-Menu.conf)
	add_subdirectory (Messaging-Menu)
endif()

############# MOTION BLUR #################
message (STATUS "> Motion Blur:")
set (GETTEXT_MOTION_BLUR ${GETTEXT_PLUGINS})
set (VERSION_MOTION_BLUR "1.0.6")
set (PACKAGE_MOTION_BLUR "cd-motion_blur")
set (motion_blurdatadir "${pluginsdatadir}/motion-blur")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/motion-blur/data/motion_blur.conf.in ${CMAKE_CURRENT_BINARY_DIR}/motion-blur/data/motion_blur.conf)
add_subdirectory (motion-blur)

############# MUSICPLAYER #################
message (STATUS "> MusicPlayer:")
set (GETTEXT_MUSICPLAYER ${GETTEXT_PLUGINS})
set (VERSION_MUSICPLAYER "2.0.4")
set (PACKAGE_MUSICPLAYER "cd-musicPlayer")
set (musicplayerdatadir "${pluginsdatadir}/musicPlayer")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/musicPlayer/data/musicPlayer.conf.in ${CMAKE_CURRENT_BINARY_DIR}/musicPlayer/data/musicPlayer.conf)
add_subdirectory (musicPlayer)

############# NETSPEED #################
message (STATUS "> NetSpeed:")
set (GETTEXT_NETSPEED ${GETTEXT_PLUGINS})
set (VERSION_NETSPEED "1.2.12")
set (PACKAGE_NETSPEED "cd-netspeed")
set (netspeeddatadir "${pluginsdatadir}/netspeed")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/netspeed/data/netspeed.conf.in ${CMAKE_CURRENT_BINARY_DIR}/netspeed/data/netspeed.conf)
add_subdirectory (netspeed)

############# NETWORK_MONITOR #################
set (with_network_monitor no) # unstable
if (enable-network-monitor)
	message (STATUS "> Network Monitor:")
	set (GETTEXT_NETWORK_MONITOR ${GETTEXT_PLUGINS})
	set (VERSION_NETWORK_MONITOR "0.2.10")
	set (PACKAGE_NETWORK_MONITOR "cd-network-monitor")
	set (with_network_monitor yes)
	set (network_monitordatadir "${pluginsdatadir}/Network-Monitor")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Network-Monitor/data/Network-Monitor.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Network-Monitor/data/Network-Monitor.conf)
	add_subdirectory (Network-Monitor)
endif()

############# POWERMANAGER #################
message (STATUS "> PowerManager:")
set (GETTEXT_POWERMANAGER ${GETTEXT_PLUGINS})
set (VERSION_POWERMANAGER "1.3.12")
set (PACKAGE_POWERMANAGER "cd-powermanager")
set (powermanagerdatadir "${pluginsdatadir}/powermanager")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/powermanager/data/powermanager.conf.in ${CMAKE_CURRENT_BINARY_DIR}/powermanager/data/powermanager.conf)
add_subdirectory (powermanager)

############# QUICK BROWSER #################
message (STATUS "> Quick Browser:")
set (GETTEXT_QUICK_BROWSER ${GETTEXT_PLUGINS})
set (VERSION_QUICK_BROWSER "1.0.13")
set (PACKAGE_QUICK_BROWSER "cd-quick-browser")
set (quick_browserdatadir "${pluginsdatadir}/quick_browser")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/quick-browser/data/quick-browser.conf.in ${CMAKE_CURRENT_BINARY_DIR}/quick-browser/data/quick-browser.conf)
add_subdirectory (quick-browser)

############# RECENT-EVENTS #################
message (STATUS "> Recent-Events:")
set (with_recent_events no)
enable_if_not_defined (enable-recent-events)
if (enable-recent-events)
	pkg_check_modules (RECENT_EVENTS zeitgeist-2.0)
	if (NOT RECENT_EVENTS_FOUND)
		message (STATUS "Could not find 'zeitgeist-2.0'; trying with an old version 'zeitgeist-1.0'...")
		pkg_check_modules (RECENT_EVENTS zeitgeist-1.0)
		set (ZEITGEIST_1_0 1)
	endif()
	if (NOT RECENT_EVENTS_FOUND)
		message (STATUS "Could not find libzeitgeist; Cairo-Dock won't be built with Zeitgeist support.")
		message (WARNING "This module is required to compile Recent-Events applet: zeitgeist-x.0")
		set (MODULES_MISSING "${MODULES_MISSING} zeitgeist-x.0")
	else()
		set (GETTEXT_RECENT_EVENTS ${GETTEXT_PLUGINS})
		set (VERSION_RECENT_EVENTS "1.0.4")
		set (PACKAGE_RECENT_EVENTS "cd-Recent-Events")
		if (ZEITGEIST_1_0)
			set (with_recent_events "yes (zeitgeist 1.0)")
		else()
			set (with_recent_events "yes (zeitgeist 2.0)")
		endif()
		set (recent_eventsdatadir "${pluginsdatadir}/Recent-Events")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Recent-Events/data/Recent-Events.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Recent-Events/data/Recent-Events.conf)
		add_subdirectory ("Recent-Events")
	endif()
endif()

############# REMOTE_CONTROL #################
message (STATUS "> Remote Control:")
set (GETTEXT_REMOTE_CONTROL ${GETTEXT_PLUGINS})
set (VERSION_REMOTE_CONTROL "1.0.3")
set (PACKAGE_REMOTE_CONTROL "cd-Remote-Control")
set (remote_controldatadir "${pluginsdatadir}/Remote-Control")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Remote-Control/data/Remote-Control.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Remote-Control/data/Remote-Control.conf)
add_subdirectory (Remote-Control)

############# RSSREADER #################
message (STATUS "> RSSreader:")
set (GETTEXT_RSS_READER ${GETTEXT_PLUGINS})
set (VERSION_RSS_READER "1.0.8")
set (PACKAGE_RSS_READER "cd-rssreader")
set (rss_readerdatadir "${pluginsdatadir}/RSSreader")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/RSSreader/data/RSSreader.conf.in ${CMAKE_CURRENT_BINARY_DIR}/RSSreader/data/RSSreader.conf)
add_subdirectory (RSSreader)

############# SCOOBY_DO #################
set (with_scooby_do no) # unstable
if (enable-scooby-do)
	message (STATUS "> Scooby-Do:")
	set (GETTEXT_SCOOBY_DO ${GETTEXT_PLUGINS})
	set (VERSION_SCOOBY_DO "0.1.4")
	set (PACKAGE_SCOOBY_DO "cd-scooby-do")
	set (with_scooby_do yes)
	set (scooby_dodatadir "${pluginsdatadir}/Scooby-Do")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Scooby-Do/data/Scooby-Do.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Scooby-Do/data/Scooby-Do.conf)
	add_subdirectory (Scooby-Do)
endif()

############# SCREENSHOT #################
message (STATUS "> Screenshot:")
set (GETTEXT_SCREENSHOT ${GETTEXT_PLUGINS})
set (VERSION_SCREENSHOT "0.0.2")
set (PACKAGE_SCREENSHOT "cd-Screenshot")
set (screenshotdatadir "${pluginsdatadir}/Screenshot")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Screenshot/data/Screenshot.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Screenshot/data/Screenshot.conf)
add_subdirectory (Screenshot)

############# SHORTCUTS #################
message (STATUS "> Shortcuts:")
set (GETTEXT_SHORTCUTS ${GETTEXT_PLUGINS})
set (VERSION_SHORTCUTS "1.3.6")
set (PACKAGE_SHORTCUTS "cd-shortcuts")
set (shortcutsdatadir "${pluginsdatadir}/shortcuts")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/data/shortcuts.conf.in ${CMAKE_CURRENT_BINARY_DIR}/shortcuts/data/shortcuts.conf)
add_subdirectory (shortcuts)

############# SHOW DESKTOP #################
message (STATUS "> ShowDesktop:")
enable_if_not_defined (enable-xrandr-support)
if (enable-xrandr-support)
	pkg_check_modules (SHOW_DESKTOP_XRANDR xrandr)
	if (NOT SHOW_DESKTOP_XRANDR_FOUND)
		message (STATUS "Could not find xrandr; ShowDesktop won't be built with screen resolution abilities.")
		message (WARNING "This module is required to compile ShowDesktop applet with xRandr support: xrandr")
		set (MODULES_MISSING "${MODULES_MISSING} xrandr")
		set (with_xrandr no)
	else ()
		set (with_xrandr yes)
	endif()
endif()
set (GETTEXT_SHOW_DESKTOP ${GETTEXT_PLUGINS})
set (VERSION_SHOW_DESKTOP "1.2.9")
set (PACKAGE_SHOW_DESKTOP "cd-showDesktop")
set (show_desktopdatadir "${pluginsdatadir}/showDesktop")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/showDesktop/data/showDesktop.conf.in ${CMAKE_CURRENT_BINARY_DIR}/showDesktop/data/showDesktop.conf)
add_subdirectory (showDesktop)

############# SHOW MOUSE #################
message (STATUS "> Show Mouse:")
set (GETTEXT_SHOW_MOUSE ${GETTEXT_PLUGINS})
set (VERSION_SHOW_MOUSE "1.0.6")
set (PACKAGE_SHOW_MOUSE "cd-show_mouse")
set (show_mousedatadir "${pluginsdatadir}/show_mouse")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/show-mouse/data/show_mouse.conf.in ${CMAKE_CURRENT_BINARY_DIR}/show-mouse/data/show_mouse.conf)
add_subdirectory (show-mouse)

############# SLIDER #################
message (STATUS "> Slider:")
set (with_exif no)
enable_if_not_defined (enable-exif-support)
if (enable-exif-support)
	pkg_check_modules (EXIF libexif)
	if (NOT EXIF_FOUND)
		message (STATUS "Could not find libexif; Slider won't be able to rotate the images accordingly to their exif data.")
		message (WARNING "This module is required to compile Slider applet with Exif support: libexif")
		set (MODULES_MISSING "${MODULES_MISSING} libexif")
	else ()
		set (with_exif yes)
	endif()
endif()
set (GETTEXT_SLIDER ${GETTEXT_PLUGINS})
set (VERSION_SLIDER "2.0.15")
set (PACKAGE_SLIDER "cd-slider")
set (sliderdatadir "${pluginsdatadir}/slider")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/slider/data/slider.conf.in ${CMAKE_CURRENT_BINARY_DIR}/slider/data/slider.conf)
add_subdirectory (slider)

############# SOUND_EFFECTS #################
set (with_sound_effects no)
enable_if_not_defined (enable-sound-effects)
if (enable-sound-effects)
	pkg_check_modules (ALSA_PACKAGE alsa)
	if (NOT ALSA_PACKAGE_FOUND)
		message (STATUS "Could not find alsa; Cairo-Dock won't be built with Sound-Effect applet.")
		message (WARNING "This module is required to compile Sound-Effect applet: alsa")
		set (MODULES_MISSING "${MODULES_MISSING} alsa")
	else()
		message (STATUS "> Sound-Effects:")
		set (GETTEXT_SOUND_EFFECTS ${GETTEXT_PLUGINS})
		set (VERSION_SOUND_EFFECTS "0.0.2")
		set (PACKAGE_SOUND_EFFECTS "cd-Sound-Effects")
		set (with_sound_effects yes)
		set (sound_effectsdatadir "${pluginsdatadir}/Sound-Effects")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Sound-Effects/data/Sound-Effects.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Sound-Effects/data/Sound-Effects.conf)
		add_subdirectory (Sound-Effects)
	endif()
endif()

############# STACK #################
message (STATUS "> Stack:")
set (GETTEXT_STACK ${GETTEXT_PLUGINS})
set (VERSION_STACK "0.3.6")
set (PACKAGE_STACK "cd-stack")
set (stackdatadir "${pluginsdatadir}/stack")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/stack/data/stack.conf.in ${CMAKE_CURRENT_BINARY_DIR}/stack/data/stack.conf)
add_subdirectory (stack)

############# STATUS NOTIFIER #################
message (STATUS "> Status Notifier:")
if (NOT DBUSMENU_GTK_FOUND)
	message (STATUS "Could not find Status-Notifier libs; Cairo-Dock won't be built with Status-Notifier support.")
	message (WARNING "This module is required to compile Status Notifier applet: ${DBUSMENU_GTK_MODULE}(-0.4)")
	# set (MODULES_MISSING "${MODULES_MISSING} dbusmenu-gtk(-0.4)") # it should already be in the list.
	set (with_status_notifier no)
else()
	if (force_indicator_applicationadded_have_hint)
		set (INDICATOR_APPLICATIONADDED_HAS_HINT 1)
	elseif (DEFINED force_indicator_applicationadded_have_hint)
		set (INDICATOR_APPLICATIONADDED_HAS_HINT 0)
	endif()
	message (STATUS "  ApplicationAdded signal has 'Hint' = ${INDICATOR_APPLICATIONADDED_HAS_HINT}")
	message (STATUS "  ApplicationAdded signal has 'Title' = ${INDICATOR_APPLICATIONADDED_HAS_TITLE}")
	set (GETTEXT_STATUS_NOTIFIER ${GETTEXT_PLUGINS})
	set (VERSION_STATUS_NOTIFIER "0.1.9")
	set (PACKAGE_STATUS_NOTIFIER "cd-status-notifier")
	set (with_status_notifier yes)
	set (status_notifierdatadir "${pluginsdatadir}/Status-Notifier")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Status-Notifier/data/Status-Notifier.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Status-Notifier/data/Status-Notifier.conf)
	add_subdirectory (Status-Notifier)
endif()

############# SWITCHER #################
message (STATUS "> Switcher:")
set (GETTEXT_SWITCHER ${GETTEXT_PLUGINS})
set (VERSION_SWITCHER "2.3.0")
set (PACKAGE_SWITCHER "cd-switcher")
set (switcherdatadir "${pluginsdatadir}/switcher")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/switcher/data/switcher.conf.in ${CMAKE_CURRENT_BINARY_DIR}/switcher/data/switcher.conf)
add_subdirectory (switcher)

############# SYSTEM MONITOR #################
message (STATUS "> System Monitor:")
enable_if_not_defined (enable-sensors-support)
if (enable-sensors-support)
	find_package (Sensors)
	if (NOT SENSORS_FOUND)
		message (STATUS "Could not find libsensors; System-Monitor won't be able to monitor fan speed and CPU temperature.")
		message (WARNING "This package is required to compile System Monitor applet with Sensors support: sensors")
		set (PROGRAMS_MISSING "${PROGRAMS_MISSING} sensors")
		set (with_sensors no)
	else ()
		set (with_sensors yes)
	endif()
endif()
set (GETTEXT_SYSTEM_MONITOR ${GETTEXT_PLUGINS})
set (VERSION_SYSTEM_MONITOR "1.0.14")
set (PACKAGE_SYSTEM_MONITOR "cd-system-monitor")
set (system_monitordatadir "${pluginsdatadir}/System-monitor")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/System-Monitor/data/System-Monitor.conf.in ${CMAKE_CURRENT_BINARY_DIR}/System-Monitor/data/System-Monitor.conf)
add_subdirectory (System-Monitor)

############# SYSTRAY #################
message (STATUS "> Systray:")
set (GETTEXT_SYSTRAY ${GETTEXT_PLUGINS})
set (VERSION_SYSTRAY "0.2.7")
set (PACKAGE_SYSTRAY "cd-systray")
set (systraydatadir "${pluginsdatadir}/systray")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/systray/data/systray.conf.in ${CMAKE_CURRENT_BINARY_DIR}/systray/data/systray.conf)
add_subdirectory (systray)

############# TERMINAL #################
message (STATUS "> Terminal:")
enable_if_not_defined (enable-terminal)
if (enable-terminal)
	set (TERMINAL_MODULE vte-2.91)
	pkg_check_modules (VTE ${TERMINAL_MODULE})
	if (NOT VTE_FOUND)
		set (TERMINAL_MODULE vte-2.90)
		pkg_check_modules (VTE ${TERMINAL_MODULE})
	endif()
	if (NOT VTE_FOUND)
		message (STATUS "Could not find ${TERMINAL_MODULE}; Cairo-Dock won't be built with terminal applet.")
		message (WARNING "This module is required to compile Terminal applet: ${TERMINAL_MODULE}")
		set (MODULES_MISSING "${MODULES_MISSING} ${TERMINAL_MODULE}")
		set (with_terminal no)
	else()
		set (GETTEXT_TERMINAL ${GETTEXT_PLUGINS})
		set (VERSION_TERMINAL "1.0.13")
		set (PACKAGE_TERMINAL "cd-terminal")
		set (with_terminal yes)
		set (terminaldatadir "${pluginsdatadir}/terminal")
		STRING (REGEX REPLACE "\\..*" "" VTE_MAJOR "${VTE_VERSION}")
		STRING (REGEX REPLACE "[0-9]*\\.([^ ]+)" "\\1" VTE_MINOR "${VTE_VERSION}")  # 0.1.7 => 1.7
		STRING (REGEX REPLACE "\\.[0-9]*" "" VTE_MINOR "${VTE_MINOR}")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/terminal/data/terminal.conf.in ${CMAKE_CURRENT_BINARY_DIR}/terminal/data/terminal.conf)
		add_subdirectory (terminal)
	endif()
endif()

############# TOMBOY #################
message (STATUS "> Tomboy:")
set (GETTEXT_TOMBOY ${GETTEXT_PLUGINS})
set (VERSION_TOMBOY "1.3.6")
set (PACKAGE_TOMBOY "cd-tomboy")
set (tomboydatadir "${pluginsdatadir}/tomboy")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/tomboy/data/tomboy.conf.in ${CMAKE_CURRENT_BINARY_DIR}/tomboy/data/tomboy.conf)
add_subdirectory (tomboy)

############# TOONS #################
message (STATUS "> Toons:")
set (GETTEXT_TOONS ${GETTEXT_PLUGINS})
set (VERSION_TOONS "1.0.15")
set (PACKAGE_TOONS "cd-Toons")
set (toonsdatadir "${pluginsdatadir}/Toons")
set (toonsuserdirname "Toons")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Toons/data/Toons.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Toons/data/Toons.conf)
add_subdirectory (Toons)

############# WEATHER #################
message (STATUS "> Weather:")
set (with_weather no)
set (WEATHER_MODULE openssl)
pkg_check_modules (WEATHER ${WEATHER_MODULE})
if (NOT WEATHER_FOUND)
	message (STATUS "Could not find ${WEATHER_MODULE}; Cairo-Dock won't be built with Weather applet.")
	message (WARNING "This module is required to compile Weather applet: ${WEATHER_MODULE}")
	set (MODULES_MISSING "${MODULES_MISSING} ${WEATHER_MODULE}")
else()
	set (GETTEXT_WEATHER ${GETTEXT_PLUGINS})
	set (VERSION_WEATHER "2.1.0")
	set (PACKAGE_WEATHER "cd-weather")
	set (weatherdatadir "${pluginsdatadir}/weather")
	set (weatheruserdirname "weather")
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/weather/data/weather.conf.in ${CMAKE_CURRENT_BINARY_DIR}/weather/data/weather.conf)
	add_subdirectory (weather)
	set (with_weather yes)
endif()

############# WEBLETS #################
message (STATUS "> Weblets:")
set (with_weblets no)
enable_if_not_defined (enable-weblets)
if (enable-weblets)
	set (WEBKIT_MODULE webkitgtk-3.0)
	pkg_check_modules (WEBKIT ${WEBKIT_MODULE})
	if (NOT WEBKIT_FOUND)
		message (STATUS "Could not find ${WEBKIT_MODULE}; Cairo-Dock won't be built with Weblets applet.")
		message (WARNING "This module is required to compile Weblets applet: ${WEBKIT_MODULE}")
		set (MODULES_MISSING "${MODULES_MISSING} ${WEBKIT_MODULE}")
	else()
		set (GETTEXT_WEBLETS ${GETTEXT_PLUGINS})
		set (VERSION_WEBLETS "0.0.15")
		set (PACKAGE_WEBLETS "cd-weblets")
		set (webletsdatadir "${pluginsdatadir}/weblets")
		set (with_weblets yes)
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/weblets/data/weblets.conf.in ${CMAKE_CURRENT_BINARY_DIR}/weblets/data/weblets.conf)
		add_subdirectory ("weblets")
	endif()
endif()

############# WIFI #################
message (STATUS "> WiFi:")
set (GETTEXT_WIFI ${GETTEXT_PLUGINS})
set (VERSION_WIFI "1.3.3")
set (PACKAGE_WIFI "cd-wifi")
set (wifidatadir "${pluginsdatadir}/wifi")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/wifi/data/wifi.conf.in ${CMAKE_CURRENT_BINARY_DIR}/wifi/data/wifi.conf)
add_subdirectory (wifi)

############# XFCE-INTEGRATION #################
message (STATUS "> XFCE-Integration:")
set (with_xfce_integration no)
enable_if_not_defined (enable-xfce-integration)
if (enable-xfce-integration)
	pkg_check_modules (XFCE_INTEGRATION_THUNAR QUIET thunar-vfs-1) # thunar-vfs is not used if gvfs (gio) is available
	pkg_check_modules (XFCE_INTEGRATION_GVFS gio-2.0)
	if (XFCE_INTEGRATION_THUNAR_FOUND AND XFCE_INTEGRATION_GVFS_FOUND)
		message (STATUS "Could find neither thunar-vfs or libgio-2.0; Cairo-Dock won't be built with XFCE support.")
		message (STATUS "This module is required to compile XFCE-Integration applet: gio-2.0 (or thunar-vfs-1)")
		set (MODULES_INTEGRATION_MISSING "${MODULES_INTEGRATION_MISSING} gio-2.0")
	else()
		set (GETTEXT_XFCE_INTEGRATION ${GETTEXT_PLUGINS})
		set (VERSION_XFCE_INTEGRATION "1.0.4")
		set (PACKAGE_XFCE_INTEGRATION "cd_xfce-integration")
		set (with_xfce_integration yes)
		set (xfce_integrationdatadir "${pluginsdatadir}/xfce-integration")
		add_subdirectory ("xfce-integration")
	endif()
endif()

############# XGAMMA #################
message (STATUS "> XGamma:")
enable_if_not_defined (enable-xgamma)
if (enable-xgamma)
	pkg_check_modules (XGAMMA_PACKAGE x11 xxf86vm)
	if (NOT XGAMMA_PACKAGE_FOUND)
		message (STATUS "Could not find xxf86vm; Cairo-Dock won't be built with Xgamma applet.")
		message (WARNING "These modules are required to compile XGamma applet: x11 and xxf86vm")
		set (MODULES_MISSING "${MODULES_MISSING} x11 xxf86vm")
		set (with_xgamma no)
	else()
		set (GETTEXT_XGAMMA ${GETTEXT_PLUGINS})
		set (VERSION_XGAMMA "1.2.5")
		set (PACKAGE_XGAMMA "cd-Xgamma")
		set (with_xgamma yes)
		set (xgammadatadir "${pluginsdatadir}/Xgamma")
		configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Xgamma/data/Xgamma.conf.in ${CMAKE_CURRENT_BINARY_DIR}/Xgamma/data/Xgamma.conf)
		add_subdirectory ("Xgamma")
	endif()
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
message (STATUS " - with Indicator-Generic applet:  ${with_indicator_generic}")
message (STATUS " - with Keyboard-indicator applet: ${with_keyboard_indicator}")
message (STATUS " - with Mail applet:               ${with_mail}")
message (STATUS " - with Messaging-Menu applet:     ${with_messaging_menu}")
message (STATUS " - with Recent-Events applet:      ${with_recent_events}")
message (STATUS " - with Sound Effects applet:      ${with_sound_effects}")
message (STATUS " - with Status-Notifier applet:    ${with_status_notifier}")
message (STATUS " - with Terminal applet:           ${with_terminal}")
message (STATUS " - with Weather applet:            ${with_weather}")
message (STATUS " - with Weblets applet:            ${with_weblets}")
message (STATUS " - with Xgamma applet:             ${with_xgamma}")
message (STATUS "Add On:")
message (STATUS " - with Sound-Menu support:        ${with_soundmenu}")
message (STATUS " - with Screen Resolution support: ${with_xrandr}")
message (STATUS " - with Sensors support:           ${with_sensors}")
message (STATUS " - with UPower support:            ${with_upower_support}")
message (STATUS " - with iCal support:              ${with_ical}")
message (STATUS " - with Exif support:              ${with_exif}")
message (STATUS "Third Party Interfaces:")
message (STATUS " - with Python3 interface:         ${with_python3}")
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
