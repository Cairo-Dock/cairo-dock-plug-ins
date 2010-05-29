########### requirements ###############

cmake_minimum_required (VERSION 2.6)
find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)

########### project ###############

project ("cairo-dock-plugins")
set (VERSION "2.2.0-0beta0")
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.bzr/;/.deps/;/Makefile$;/Makefile.in$;.conf$;.gmo$;/.libs/;.lo$;.la$;/POTFILES$;/stamp-it$;/aclocal.m4$;/config.guess$;/config.h$;/config.h.in$;/config.log$;/config.status$;/config.sub$;/configure$;/depcomp$;/install-sh$;/libtool$;/ltmain.sh$;/missing$;/stamp-h1$;^compile;/CMakeFiles/;/_CPack_Packages/;/autom4te.cache/;/install_manifest.txt$;.tar.gz$;/CMakeCache.txt$;.cmake$;~$;${CPACK_SOURCE_IGNORE_FILES}")
include (CPack)  # a inclure apres les set.
add_custom_target(dist
    COMMAND bzr export ${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
#add_custom_target( dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)  # pour pouvoir l'appeler avec "make dist"
add_definitions (-std=c99 -Wstrict-prototypes -Wextra -Wwrite-strings -Wuninitialized -Werror-implicit-function-declaration) #-Wunreachable-code -Wno-unused-parameter -Wall
if (NOT ${CMAKE_BUILD_TYPE})
	add_definitions (-O3)
endif()
add_definitions (-DGL_GLEXT_PROTOTYPES="1")
configure_file ("cmake_uninstall.cmake.in" "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target (uninstall "${CMAKE_COMMAND}" -P "cmake_uninstall.cmake")

########### global variables ###############

set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE ${PACKAGE})
execute_process(
    COMMAND pkg-config cairo-dock --variable=pluginsdir
    OUTPUT_VARIABLE pluginsdir)
STRING (REGEX REPLACE "\n" "" pluginsdir ${pluginsdir})  # la commande rajoute un retour chariot ...
execute_process(
    COMMAND pkg-config cairo-dock --variable=pluginsdatadir
    OUTPUT_VARIABLE pluginsdatadir)
STRING (REGEX REPLACE "\n" "" pluginsdatadir ${pluginsdatadir})
execute_process(
    COMMAND pkg-config --modversion cairo-dock
    OUTPUT_VARIABLE dock_version)
STRING (REGEX REPLACE "\n" "" dock_version ${dock_version})
if (NOT "${dock_version}" STREQUAL "${VERSION}")
	MESSAGE (FATAL_ERROR "warning : version mismatch with the core : " ${VERSION} <> ${dock_version})
endif()
set (GETTEXT_PLUGINS "cairo-dock-plugins")

if (NOT "${pluginsdir}" STREQUAL "${CMAKE_INSTALL_PREFIX}/lib/cairo-dock")
	message (STATUS "Warning : plug-ins should be installed in the same place asthe dock, that is to say in ${pluginsdir}")
	STRING (REGEX REPLACE "/lib/cairo-dock" "" CMAKE_INSTALL_PREFIX ${pluginsdir})
endif()
set (localedir "${CMAKE_INSTALL_PREFIX}/share/locale")
set (gaugesdir "${CMAKE_INSTALL_PREFIX}/share/cairo-dock/gauges")


########### dependencies ###############

pkg_check_modules ("PACKAGE" REQUIRED "gtk+-2.0" "gthread-2.0" "cairo" "librsvg-2.0" "dbus-1" "dbus-glib-1" "libxml-2.0" "gtkglext-1.0" "cairo-dock")

############# ALSA_MIXER #################
if (NOT "${enable_alsa_mixer}" STREQUAL "no")
	pkg_check_modules (ALSA_MIXER_PACKAGE alsa)
	if ("${ALSA_MIXER_PACKAGE_FOUND}" STREQUAL "")
		message (STATUS "Could not find alsa; Cairo-Dock won't be built with AlsaMixer applet.")
	else()
		set (GETTEXT_ALSA_MIXER ${GETTEXT_PLUGINS})
		set (VERSION_ALSA_MIXER "1.0.11")
		set (PACKAGE_ALSA_MIXER "cd-AlsaMixer")
		set (alsa_mixerdatadir "${pluginsdatadir}/AlsaMixer")
		configure_file (alsaMixer/data/AlsaMixer.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/alsaMixer/data/AlsaMixer.conf)
		add_subdirectory ("alsaMixer")
	endif()
endif()

############# ANIMATED ICONS #################
set (GETTEXT_ANIMATED_ICONS ${GETTEXT_PLUGINS})
set (VERSION_ANIMATED_ICONS "1.0.8")
set (PACKAGE_ANIMATED_ICONS "cd-Animated-icons")
set (animated_iconsdatadir "${pluginsdatadir}/Animated-icons")
configure_file (Animated-icons/data/Animated-icons.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Animated-icons/data/Animated-icons.conf)
add_subdirectory (Animated-icons)

############# CAIRO_PENGUIN #################
set (GETTEXT_CAIRO_PENGUIN ${GETTEXT_PLUGINS})
set (VERSION_CAIRO_PENGUIN "1.1.7")
set (PACKAGE_CAIRO_PENGUIN "cd-Cairo-Penguin")
set (cairo_penguinuserdirname "Cairo-Penguin")
set (cairo_penguindatadir "${pluginsdatadir}/Cairo-Penguin")
configure_file (Cairo-Penguin/data/Cairo-Penguin.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Cairo-Penguin/data/Cairo-Penguin.conf)
add_subdirectory (Cairo-Penguin)

############# CLIPPER #################
set (GETTEXT_CLIPPER ${GETTEXT_PLUGINS})
set (VERSION_CLIPPER "1.1.3")
set (PACKAGE_CLIPPER "cd-Clipper")
set (Clipperdatadir "${pluginsdatadir}/Clipper")
configure_file (Clipper/data/Clipper.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Clipper/data/Clipper.conf)
add_subdirectory (Clipper)

############# CLOCK #################
pkg_check_modules ("LIBICAL_PACKAGE" "libical")
if ("${LIBICAL_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find libical; Clock plugin won't be built with iCal support.")
endif()
set (GETTEXT_CLOCK ${GETTEXT_PLUGINS})
set (VERSION_CLOCK "2.1.0")
set (PACKAGE_CLOCK "cd-clock")
set (clockuserdirname "clock")
set (clockdatadir "${pluginsdatadir}/clock")
configure_file (clock/data/clock.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/clock/data/clock.conf)
add_subdirectory (clock)

############# COMPIZ_ICON #################
set (GETTEXT_COMPIZ_ICON ${GETTEXT_PLUGINS})
set (VERSION_COMPIZ_ICON "1.1.5")
set (PACKAGE_COMPIZ_ICON "cd-compiz-icon")
set (compiz_icondatadir "${pluginsdatadir}/compiz-icon")
configure_file (compiz-icon/data/compiz-icon.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/compiz-icon/data/compiz-icon.conf)
add_subdirectory (compiz-icon)

############# DBUS #################
set (GETTEXT_DBUS ${GETTEXT_PLUGINS})
set (VERSION_DBUS "1.0.0")
set (PACKAGE_DBUS "cd-Dbus")
set (dbusdatadir "${pluginsdatadir}/Dbus")
configure_file (Dbus/data/Dbus.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Dbus/data/Dbus.conf)
add_subdirectory (Dbus)

############# DESKLET_RENDERING #################
set (GETTEXT_DESKLET_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DESKLET_RENDERING "1.5.5")
set (PACKAGE_DESKLET_RENDERING "cd-desklet-rendering")
set (desklet_renderingdatadir "${pluginsdatadir}/desklet-rendering")
configure_file (desklet-rendering/data/desklet-rendering.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/desklet-rendering/data/desklet-rendering.conf)
add_subdirectory (desklet-rendering)

############# DIALOG_RENDERING #################
set (GETTEXT_DIALOG_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DIALOG_RENDERING "0.4.4")
set (PACKAGE_DIALOG_RENDERING "cd-dialog-rendering")
set (dialog_renderingdatadir "${pluginsdatadir}/dialog-rendering")
configure_file (dialog-rendering/data/dialog-rendering.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/dialog-rendering/data/dialog-rendering.conf)
add_subdirectory (dialog-rendering)

############# DND2SHARE #################
set (GETTEXT_DND2SHARE ${GETTEXT_PLUGINS})
set (VERSION_DND2SHARE "1.0.3")
set (PACKAGE_DND2SHARE "cd-dnd2share")
set (dnd2sharedatadir "${pluginsdatadir}/dnd2share")
configure_file (dnd2share/data/dnd2share.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/dnd2share/data/dnd2share.conf)
add_subdirectory (dnd2share)

############# DOCK RENDERING #################
set (GETTEXT_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_RENDERING "1.5.7")
set (PACKAGE_RENDERING "cd-rendering")
set (renderingdatadir "${pluginsdatadir}/rendering")
configure_file (dock-rendering/data/rendering.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/dock-rendering/data/rendering.conf)
add_subdirectory (dock-rendering)

############# DONCKY #################
if ("${enable-doncky}" STREQUAL "yes")
	set (GETTEXT_DONCKY ${GETTEXT_PLUGINS})
	set (VERSION_DONCKY "0.0.1")
	set (PACKAGE_DONCKY "cd-doncky")
	set (donckydatadir "${pluginsdatadir}/Doncky")
	configure_file (Doncky/data/Doncky.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Doncky/data/Doncky.conf)
	add_subdirectory (Doncky)
endif()

############# DROP INDICATOR #################
set (GETTEXT_DROP_INDICATOR ${GETTEXT_PLUGINS})
set (VERSION_DROP_INDICATOR "1.1.3")
set (PACKAGE_DROP_INDICATOR "cd-drop_indicator")
set (drop_indicatordatadir "${pluginsdatadir}/drop-indicator")
configure_file (drop-indicator/data/drop_indicator.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/drop-indicator/data/drop_indicator.conf)
add_subdirectory (drop-indicator)

############# DUSTBIN #################
set (GETTEXT_DUSTBIN ${GETTEXT_PLUGINS})
set (VERSION_DUSTBIN "2.2.7")
set (PACKAGE_DUSTBIN "cd-dustbin")
set (dustbinuserdirname "dustbin")
set (dustbindatadir "${pluginsdatadir}/dustbin")
configure_file (dustbin/data/dustbin.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/dustbin/data/dustbin.conf)
add_subdirectory (dustbin)

############# GMENU #################
if (NOT "${enable_gmenu}" STREQUAL "no")
	pkg_check_modules (GMENU_PACKAGE libgnome-menu)
	if ("${GMENU_PACKAGE_FOUND}" STREQUAL "")
		message (STATUS "Could not find libgnome-menu; Cairo-Dock won't be built with GMenu applet.")
	else()
		set (GETTEXT_GMENU ${GETTEXT_PLUGINS})
		set (VERSION_GMENU "1.1.5")
		set (PACKAGE_GMENU "cd-GMenu")
		set (gmenudatadir "${pluginsdatadir}/GMenu")
		configure_file (GMenu/data/GMenu.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/GMenu/data/GMenu.conf)
		add_subdirectory ("GMenu")
	endif()
endif()

############# GVFS-INTEGRATION #################
pkg_check_modules ("LIBGIO" "gio-2.0")
add_subdirectory (gvfs-integration)

############# GNOME-INTEGRATION #################
if (NOT "${enable_gnome_integration}" STREQUAL "no")
	pkg_check_modules (GNOME_INTEGRATION gio-2.0)
	if ("${GNOME_INTEGRATION_FOUND}" STREQUAL "")
		message (STATUS "Could not find gio; Cairo-Dock won't be built with Gnome>=2.22 support.")
	else()
		set (VERSION_GNOME_INTEGRATION "1.0.2")
		set (PACKAGE_GNOME_INTEGRATION "cd_gnome-integration")
		set (gnome_integrationdatadir "${pluginsdatadir}/gnome-integration")
		add_subdirectory ("gnome-integration")
	endif()
endif()

############# GNOME-INTEGRATION-OLD #################
if ("${enable_old_gnome_integration}" STREQUAL "yes")
	pkg_check_modules (OLD_GNOME_INTEGRATION gnome-vfs-2.0 libgnomeui-2.0)
	if ("${OLD_GNOME_INTEGRATION_FOUND}" STREQUAL "")
		message (STATUS "Could not find gnome-vfs and/or gnomeui; Cairo-Dock won't be built with Gnome<2.22 support.")
	else()
		set (VERSION_GNOME_INTEGRATION_OLD "1.0.3")
		set (PACKAGE_GNOME_INTEGRATION_OLD "cd_gnome-integration-old")
		set (gnome_integration_olddatadir "${pluginsdatadir}/gnome-integration-old")
		add_subdirectory ("gnome-integration-old")
	endif()
endif()

############# ICON EFFECT #################
set (GETTEXT_ICON_EFFECTS ${GETTEXT_PLUGINS})
set (VERSION_ICON_EFFECTS "1.2.3")
set (PACKAGE_ICON_EFFECTS "cd-icon-effect")
set (icon_effectsdatadir "${pluginsdatadir}/icon-effect")
configure_file (icon-effect/data/icon-effect.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/icon-effect/data/icon-effect.conf)
add_subdirectory (icon-effect)

############# INDICATOR-APPLET #################
pkg_check_modules (DBUSMENU dbusmenu-glib)
pkg_check_modules (INDICATOR_APPLET indicator libido-0.1 dbusmenu-gtk)
if (NOT "${INDICATOR_APPLET_FOUND}" STREQUAL "" AND NOT "${DBUSMENU_FOUND}" STREQUAL "")
	add_subdirectory (Indicator-applet)
endif()

############# ILLUSION #################
set (GETTEXT_ILLUSION ${GETTEXT_PLUGINS})
set (VERSION_ILLUSION "1.0.6")
set (PACKAGE_ILLUSION "cd-illusion")
set (illusiondatadir "${pluginsdatadir}/illusion")
configure_file (illusion/data/illusion.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/illusion/data/illusion.conf)
add_subdirectory (illusion)

############# KDE-INTEGRATION #################
if (NOT "${enable_kde_integration}" STREQUAL "no")
	pkg_check_modules (KDE_INTEGRATION gio-2.0)
	if ("${KDE_INTEGRATION_FOUND}" STREQUAL "")
		message (STATUS "Could not find kde libs; Cairo-Dock won't be built with KDE support.")
	else()
		set (VERSION_KDE_INTEGRATION "0.0.1")
		set (PACKAGE_KDE_INTEGRATION "cd_kde-integration")
		set (kde_integrationdatadir "${pluginsdatadir}/kde-integration")
		add_subdirectory ("kde-integration")
	endif()
endif()

############# KEYBOARD_INDICATOR #################
pkg_check_modules (KEYBOARD_INDICATOR_PACKAGE libxklavier)
if ("${KEYBOARD_INDICATOR_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find libxklavier; Cairo-Dock won't be built with keyboard-indicator applet.")
else()
	set (GETTEXT_KEYBOARD_INDICATOR ${GETTEXT_PLUGINS})
	set (VERSION_KEYBOARD_INDICATOR "1.1.2")
	set (PACKAGE_KEYBOARD_INDICATOR "cd-keyboard-indicator")
	set (keyboard_indicatordatadir "${pluginsdatadir}/keyboard-indicator")
	configure_file (keyboard-indicator/data/keyboard-indicator.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/keyboard-indicator/data/keyboard-indicator.conf)
	add_subdirectory ("keyboard-indicator")
endif()

############# LOGOUT #################
set (GETTEXT_LOGOUT ${GETTEXT_PLUGINS})
set (VERSION_LOGOUT "1.2.9")
set (PACKAGE_LOGOUT "cd-logout")
set (logoutdatadir "${pluginsdatadir}/logout")
configure_file (logout/data/logout.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/logout/data/logout.conf)
add_subdirectory (logout)

############# MAIL #################
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
	set (VERSION_MAIL "1.0.4")
	set (PACKAGE_MAIL "cd-mail")
	set (maildatadir "${pluginsdatadir}/mail")
	configure_file (mail/data/mail.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/mail/data/mail.conf)
	add_subdirectory ("mail")
endif()

############# MEMENU #################
if (NOT "${INDICATOR_APPLET_FOUND}" STREQUAL "" AND NOT "${DBUSMENU_FOUND}" STREQUAL "")
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
	set (VERSION_MEMENU "1.0.0")
	set (PACKAGE_MEMENU "cd-MeMenu")
	set (memenudatadir "${pluginsdatadir}/MeMenu")
	configure_file (MeMenu/data/MeMenu.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/MeMenu/data/MeMenu.conf)
	add_subdirectory (MeMenu)
endif()

############# MESSAGING_MENU #################
if (NOT "${INDICATOR_APPLET_FOUND}" STREQUAL "" AND NOT "${DBUSMENU_FOUND}" STREQUAL "")
	set (GETTEXT_MESSAGING_MENU ${GETTEXT_PLUGINS})
	set (VERSION_MESSAGING_MENU "0.0.1")
	set (PACKAGE_MESSAGING_MENU "cd-Messaging-Menu")
	set (messaging_menudatadir "${pluginsdatadir}/Messaging-Menu")
	configure_file (Messaging-Menu/data/Messaging-Menu.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Messaging-Menu/data/Messaging-Menu.conf)
	add_subdirectory (Messaging-Menu)
endif()

############# MOTION BLUR #################
set (GETTEXT_MOTION_BLUR ${GETTEXT_PLUGINS})
set (VERSION_MOTION_BLUR "1.0.3")
set (PACKAGE_MOTION_BLUR "cd-motion_blur")
set (motion_blurdatadir "${pluginsdatadir}/motion-blur")
configure_file (motion-blur/data/motion_blur.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/motion-blur/data/motion_blur.conf)
add_subdirectory (motion-blur)

############# MUSICPLAYER #################
set (GETTEXT_MUSICPLAYER ${GETTEXT_PLUGINS})
set (VERSION_MUSICPLAYER "1.0.6")
set (PACKAGE_MUSICPLAYER "cd-musicPlayer")
set (musicplayerdatadir "${pluginsdatadir}/musicPlayer")
configure_file (musicPlayer/data/musicPlayer.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/musicPlayer/data/musicPlayer.conf)
add_subdirectory (musicPlayer)

############# NETSPEED #################
set (GETTEXT_NETSPEED ${GETTEXT_PLUGINS})
set (VERSION_NETSPEED "1.2.4")
set (PACKAGE_NETSPEED "cd-netspeed")
set (netspeeddatadir "${pluginsdatadir}/netspeed")
configure_file (netspeed/data/netspeed.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/netspeed/data/netspeed.conf)
add_subdirectory (netspeed)

############# NETWORK_MONITOR #################
if ("${enable-network-monitor}" STREQUAL "yes")
	set (GETTEXT_NETWORK_MONITOR ${GETTEXT_PLUGINS})
	set (VERSION_NETWORK_MONITOR "0.2.2")
	set (PACKAGE_NETWORK_MONITOR "cd-network-monitor")
	set (network_monitordatadir "${pluginsdatadir}/Network-Monitor")
	configure_file (Network-Monitor/data/Network-Monitor.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Network-Monitor/data/Network-Monitor.conf)
	add_subdirectory (Network-Monitor)
endif()

############# POWERMANAGER #################
set (GETTEXT_POWERMANAGER ${GETTEXT_PLUGINS})
set (VERSION_POWERMANAGER "1.3.3")
set (PACKAGE_POWERMANAGER "cd-powermanager")
set (powermanagerdatadir "${pluginsdatadir}/powermanager")
configure_file (powermanager/data/powermanager.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/powermanager/data/powermanager.conf)
add_subdirectory (powermanager)

############# QUICK BROWSER #################
set (GETTEXT_QUICK_BROWSER ${GETTEXT_PLUGINS})
set (VERSION_QUICK_BROWSER "1.0.7")
set (PACKAGE_QUICK_BROWSER "cd-quick-browser")
set (quick_browserdatadir "${pluginsdatadir}/quick_browser")
configure_file (quick-browser/data/quick-browser.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/quick-browser/data/quick-browser.conf)
add_subdirectory (quick-browser)

############# RSSREADER #################
set (GETTEXT_RSS_READER ${GETTEXT_PLUGINS})
set (VERSION_RSS_READER "1.0.2")
set (PACKAGE_RSS_READER "cd-rssreader")
set (rss_readerdatadir "${pluginsdatadir}/RSSreader")
configure_file (RSSreader/data/RSSreader.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/RSSreader/data/RSSreader.conf)
add_subdirectory (RSSreader)

############# SCOOBY_DO #################
if ("${enable-scooby-do}" STREQUAL "yes")
	set (GETTEXT_SCOOBY_DO ${GETTEXT_PLUGINS})
	set (VERSION_SCOOBY_DO "0.1.0")
	set (PACKAGE_SCOOBY_DO "cd-scooby-do")
	set (scooby_dodatadir "${pluginsdatadir}/Scooby-Do")
	configure_file (Scooby-Do/data/Scooby-Do.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Scooby-Do/data/Scooby-Do.conf)
	add_subdirectory (Scooby-Do)
endif()

############# SHORTCUTS #################
set (GETTEXT_SHORTCUTS ${GETTEXT_PLUGINS})
set (VERSION_SHORTCUTS "1.2.4")
set (PACKAGE_SHORTCUTS "cd-shortcuts")
set (shortcutsdatadir "${pluginsdatadir}/shortcuts")
configure_file (shortcuts/data/shortcuts.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/data/shortcuts.conf)
add_subdirectory (shortcuts)

############# SHOW DESKTOP #################
pkg_check_modules (SHOW_DESKTOP_XRANDR xrandr)
if ("${SHOW_DESKTOP_XRANDR_FOUND}" STREQUAL "")
	message (STATUS "Could not find xrandr; ShowDesktop won't be built with screen resolution abilities.")
endif()
set (GETTEXT_SHOW_DESKTOP ${GETTEXT_PLUGINS})
set (VERSION_SHOW_DESKTOP "1.2.2")
set (PACKAGE_SHOW_DESKTOP "cd-showDesktop")
set (show_desktopdatadir "${pluginsdatadir}/showDesktop")
configure_file (showDesktop/data/showDesktop.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/showDesktop/data/showDesktop.conf)
add_subdirectory (showDesktop)

############# SHOW MOUSE #################
set (GETTEXT_SHOW_MOUSE ${GETTEXT_PLUGINS})
set (VERSION_SHOW_MOUSE "1.0.3")
set (PACKAGE_SHOW_MOUSE "cd-show_mouse")
set (show_mousedatadir "${pluginsdatadir}/show_mouse")
configure_file (show-mouse/data/show_mouse.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/show-mouse/data/show_mouse.conf)
add_subdirectory (show-mouse)

############# SLIDER #################
pkg_check_modules (EXIF libexif)
if ("${EXIF_FOUND}" STREQUAL "")
	message (STATUS "Could not find libexif; Slider won't be able to rotate the images accordingly to their exif data.")
endif()
set (GETTEXT_SLIDER ${GETTEXT_PLUGINS})
set (VERSION_SLIDER "2.0.6")
set (PACKAGE_SLIDER "cd-slider")
set (sliderdatadir "${pluginsdatadir}/slider")
configure_file (slider/data/slider.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/slider/data/slider.conf)
add_subdirectory (slider)

############# STACK #################
set (GETTEXT_STACK ${GETTEXT_PLUGINS})
set (VERSION_STACK "0.2.7")
set (PACKAGE_STACK "cd-stack")
set (stackdatadir "${pluginsdatadir}/stack")
configure_file (stack/data/stack.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/stack/data/stack.conf)
add_subdirectory (stack)

############# STATUS NOTIFIER #################
if ("${enable-status-notifier}" STREQUAL "yes")
	set (GETTEXT_STATUS_NOTIFIER ${GETTEXT_PLUGINS})
	set (VERSION_STATUS_NOTIFIER "0.0.1")
	set (PACKAGE_STATUS_NOTIFIER "cd-status-notifier")
	set (status_notifierdatadir "${pluginsdatadir}/Status-Notifier")
	configure_file (Status-Notifier/data/Status-Notifier.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Status-Notifier/data/Status-Notifier.conf)
	add_subdirectory (Status-Notifier)
endif()

############# SWITCHER #################
set (GETTEXT_SWITCHER ${GETTEXT_PLUGINS})
set (VERSION_SWITCHER "2.1.3")
set (PACKAGE_SWITCHER "cd-switcher")
set (switcherdatadir "${pluginsdatadir}/switcher")
configure_file (switcher/data/switcher.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/switcher/data/switcher.conf)
add_subdirectory (switcher)

############# SYSTEM MONITOR #################
set (GETTEXT_SYSTEM_MONITOR ${GETTEXT_PLUGINS})
set (VERSION_SYSTEM_MONITOR "1.0.4")
set (PACKAGE_SYSTEM_MONITOR "cd-system-monitor")
set (system_monitordatadir "${pluginsdatadir}/System-monitor")
configure_file (System-Monitor/data/System-Monitor.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/System-Monitor/data/System-Monitor.conf)
add_subdirectory (System-Monitor)

############# SYSTRAY #################
set (GETTEXT_SYSTRAY ${GETTEXT_PLUGINS})
set (VERSION_SYSTRAY "0.1.11")
set (PACKAGE_SYSTRAY "cd-systray")
set (systraydatadir "${pluginsdatadir}/systray")
configure_file (systray/data/systray.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/systray/data/systray.conf)
add_subdirectory (systray)

############# TERMINAL #################
pkg_check_modules (TERMINAL_PACKAGE vte)
if ("${TERMINAL_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find libvte; Cairo-Dock won't be built with terminal applet.")
else()
	set (GETTEXT_TERMINAL ${GETTEXT_PLUGINS})
	set (VERSION_TERMINAL "1.0.9")
	set (PACKAGE_TERMINAL "cd-terminal")
	set (terminaldatadir "${pluginsdatadir}/terminal")
	configure_file (terminal/data/terminal.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/terminal/data/terminal.conf)
	add_subdirectory (terminal)
endif()

############# TOMBOY #################
set (GETTEXT_TOMBOY ${GETTEXT_PLUGINS})
set (VERSION_TOMBOY "1.2.3")
set (PACKAGE_TOMBOY "cd-tomboy")
set (tomboydatadir "${pluginsdatadir}/tomboy")
configure_file (tomboy/data/tomboy.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/tomboy/data/tomboy.conf)
add_subdirectory (tomboy)

############# TOONS #################
set (GETTEXT_TOONS ${GETTEXT_PLUGINS})
set (VERSION_TOONS "1.0.7")
set (PACKAGE_TOONS "cd-Toons")
set (toonsdatadir "${pluginsdatadir}/Toons")
set (toonsuserdirname "Toons")
configure_file (Toons/data/Toons.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Toons/data/Toons.conf)
add_subdirectory (Toons)

############# WEATHER #################
set (GETTEXT_WEATHER ${GETTEXT_PLUGINS})
set (VERSION_WEATHER "1.2.9")
set (PACKAGE_WEATHER "cd-weather")
set (weatherdatadir "${pluginsdatadir}/weather")
set (weatheruserdirname "weather")
configure_file (weather/data/weather.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/weather/data/weather.conf)
add_subdirectory (weather)

############# WEBLETS #################
pkg_check_modules (WEBKIT webkit-1.0)
if (NOT "${enable_weblets}" STREQUAL "no")
	if ("${WEBKIT_FOUND}" STREQUAL "")
		message (STATUS "Could not find webkit-1.0; Cairo-Dock won't be built with Weblets applet.")
	else()
		set (GETTEXT_WEBLETS ${GETTEXT_PLUGINS})
		set (VERSION_WEBLETS "0.0.9")
		set (PACKAGE_WEBLETS "cd-weblets")
		set (webletsdatadir "${pluginsdatadir}/weblets")
		configure_file (weblets/data/weblets.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/weblets/data/weblets.conf)
		add_subdirectory ("weblets")
	endif()
endif()

############# WIFI #################
set (GETTEXT_WIFI ${GETTEXT_PLUGINS})
set (VERSION_WIFI "1.2.5")
set (PACKAGE_WIFI "cd-wifi")
set (wifidatadir "${pluginsdatadir}/wifi")
configure_file (wifi/data/wifi.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/wifi/data/wifi.conf)
add_subdirectory (wifi)

############# XFCE-INTEGRATION #################
if (NOT "${enable_xfce_integration}" STREQUAL "no")
	pkg_check_modules (XFCE_INTEGRATION_THUNAR thunar-vfs-1)
	pkg_check_modules (XFCE_INTEGRATION_GVFS gio-2.0)
	if ("${XFCE_INTEGRATION_THUNAR_FOUND}" STREQUAL "" AND "${XFCE_INTEGRATION_GVFS_FOUND}" STREQUAL "")
		message (STATUS "Could find neither thunar-vfs or libgio-2.0; Cairo-Dock won't be built with XFCE support.")
	else()
		set (VERSION_XFCE_INTEGRATION "1.0.2")
		set (PACKAGE_XFCE_INTEGRATION "cd_xfce-integration")
		set (xfce_integrationdatadir "${pluginsdatadir}/xfce-integration")
		add_subdirectory ("xfce-integration")
	endif()
endif()

############# XGAMMA #################
pkg_check_modules (XGAMMA_PACKAGE x11 xxf86vm)
if ("${XGAMMA_PACKAGE_FOUND}" STREQUAL "")
	message (STATUS "Could not find xxf86vm; Cairo-Dock won't be built with Xgamma applet.")
else()
	set (GETTEXT_XGAMMA ${GETTEXT_PLUGINS})
	set (VERSION_XGAMMA "1.1.5")
	set (PACKAGE_XGAMMA "cd-Xgamma")
	set (xgammadatadir "${pluginsdatadir}/Xgamma")
	configure_file (Xgamma/data/Xgamma.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/Xgamma/data/Xgamma.conf)
	add_subdirectory ("Xgamma")
endif()

add_subdirectory (po)
