########### requirements ###############

cmake_minimum_required (VERSION 2.6)
find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)

########### project ###############

project ("cairo-dock-plug-ins")
set (VERSION "2.1.4-0beta0")
set (CPACK_SOURCE_GENERATOR "TBZ2")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.bzr/;~$;${CPACK_SOURCE_IGNORE_FILES}")
include (CPack)  # a inclure apres les set.
add_custom_target( dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)  # pour pouvoir l'appeler avec "make dist"
add_definitions (-std=c99 -Wstrict-prototypes -Wextra -Wwrite-strings -Wuninitialized -Werror-implicit-function-declaration) #-Wunreachable-code -Wno-unused-parameter -Wall
if (NOT ${CMAKE_BUILD_TYPE})
	add_definitions (-O3)
endif()
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

########### global variables ###############

set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE ${PACKAGE})
set (localedir ${CMAKE_INSTALL_PREFIX}/share/locale)
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
if (NOT ${dock_version} STREQUAL ${VERSION})
	MESSAGE (FATAL_ERROR "warning : version mismatch with the core : " ${VERSION} <> ${dock_version})
endif()
set (GETTEXT_PLUGINS "cairo-dock-plugins")

if (NOT ${pluginsdir} STREQUAL ${CMAKE_INSTALL_PREFIX}/lib/cairo-dock)
	message (STATUS "Warning : plug-ins should be installed in the same place asthe dock, that is to say in ${pluginsdir}")
endif()

########### dependencies ###############

pkg_check_modules ("PACKAGE" REQUIRED "gtk+-2.0" "gthread-2.0" "cairo" "librsvg-2.0" "dbus-1" "dbus-glib-1" "libxml-2.0" "gtkglext-1.0" "cairo-dock")

############# ALSA_MIXER #################
if (NOT "${enable_alsa_mixer}" STREQUAL "no")
	pkg_check_modules (ALSA_MIXER_PACKAGE alsa)
	if (NOT ${ALSA_MIXER_PACKAGE_FOUND})
		message (STATUS "Could not find alsa; Cairo-Dock won't be built with AlsaMixer applet.")
	else()
		set (GETTEXT_ALSA_MIXER ${GETTEXT_PLUGINS})
		set (VERSION_ALSA_MIXER "1.0.11")
		set (PACKAGE_ALSA_MIXER "cd-AlsaMixer")
		set (alsa_mixerdatadir "${pluginsdatadir}/AlsaMixer")
		configure_file (alsaMixer/data/AlsaMixer.conf.in alsaMixerdata/data/AlsaMixer.conf)
		add_subdirectory ("alsaMixer")
	endif()
endif()

############# ANIMATED ICONS #################
set (GETTEXT_ANIMATED_ICONS ${GETTEXT_PLUGINS})
set (VERSION_ANIMATED_ICONS "1.0.8")
set (PACKAGE_ANIMATED_ICONS "cd-Animated-icons")
set (animated_iconsdatadir "${pluginsdatadir}/Animated-icons")
configure_file (Animated-icons/data/Animated-icons.conf.in Animated-icons/data/Animated-icons.conf)
add_subdirectory (Animated-icons)

############# CAIRO_PENGUIN #################
set (GETTEXT_CAIRO_PENGUIN ${GETTEXT_PLUGINS})
set (VERSION_CAIRO_PENGUIN "1.1.7")
set (PACKAGE_CAIRO_PENGUIN "cd-Cairo-Penguin")
set (cairo_penguinuserdirname "Cairo-Penguin")
set (cairo_penguindatadir "${pluginsdatadir}/Cairo-Penguin")
configure_file (Cairo-Penguin/data/Cairo-Penguin.conf.in Cairo-Penguin/data/Cairo-Penguin.conf)
add_subdirectory (Cairo-Penguin)

############# CLIPPER #################
set (GETTEXT_CLIPPER ${GETTEXT_PLUGINS})
set (VERSION_CLIPPER "1.1.3")
set (PACKAGE_CLIPPER "cd-Clipper")
set (Clipperdatadir "${pluginsdatadir}/Clipper")
configure_file (Clipper/data/Clipper.conf.in Clipper/data/Clipper.conf)
add_subdirectory (Clipper)

############# CLOCK #################
set (GETTEXT_CLOCK ${GETTEXT_PLUGINS})
set (VERSION_CLOCK "2.0.9")
set (PACKAGE_CLOCK "cd-clock")
set (clockuserdirname "clock")
set (clockdatadir "${pluginsdatadir}/clock")
configure_file (clock/data/clock.conf.in clock/data/clock.conf)
add_subdirectory (clock)

############# COMPIZ_ICON #################
set (GETTEXT_COMPIZ_ICON ${GETTEXT_PLUGINS})
set (VERSION_COMPIZ_ICON "1.1.5")
set (PACKAGE_COMPIZ_ICON "cd-compiz-icon")
set (compiz_icondatadir "${pluginsdatadir}/compiz-icon")
configure_file (compiz-icon/data/compiz-icon.conf.in compiz-icon/data/compiz-icon.conf)
add_subdirectory (compiz-icon)

############# DBUS #################
set (GETTEXT_DBUS ${GETTEXT_PLUGINS})
set (VERSION_DBUS "1.0.0")
set (PACKAGE_DBUS "cd-Dbus")
set (dbusdatadir "${pluginsdatadir}/Dbus")
configure_file (Dbus/data/Dbus.conf.in Dbus/data/Dbus.conf)
add_subdirectory (Dbus)

############# DESKLET_RENDERING #################
set (GETTEXT_DESKLET_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DESKLET_RENDERING "1.5.5")
set (PACKAGE_DESKLET_RENDERING "cd-desklet-rendering")
set (desklet_renderingdatadir "${pluginsdatadir}/desklet-rendering")
configure_file (desklet-rendering/data/desklet-rendering.conf.in desklet-rendering/data/desklet-rendering.conf)
add_subdirectory (desklet-rendering)

############# DIALOG_RENDERING #################
set (GETTEXT_DIALOG_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DIALOG_RENDERING "0.4.4")
set (PACKAGE_DIALOG_RENDERING "cd-dialog-rendering")
set (dialog_renderingdatadir "${pluginsdatadir}/dialog-rendering")
configure_file (dialog-rendering/data/dialog-rendering.conf.in dialog-rendering/data/dialog-rendering.conf)
add_subdirectory (dialog-rendering)

############# DND2SHARE #################
set (GETTEXT_DND2SHARE ${GETTEXT_PLUGINS})
set (VERSION_DND2SHARE "1.0.2")
set (PACKAGE_DND2SHARE "cd-dnd2share")
set (dnd2sharedatadir "${pluginsdatadir}/dnd2share")
configure_file (dnd2share/data/dnd2share.conf.in dnd2share/data/dnd2share.conf)
add_subdirectory (dnd2share)

############# DOCK RENDERING #################
set (GETTEXT_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_RENDERING "1.5.7")
set (PACKAGE_RENDERING "cd-rendering")
set (renderingdatadir "${pluginsdatadir}/rendering")
configure_file (dock-rendering/data/rendering.conf.in dock-rendering/data/rendering.conf)
add_subdirectory (dock-rendering)

############# DONCKY #################
if ("${enable-doncky}" STREQUAL "yes")
	set (GETTEXT_DONCKY ${GETTEXT_PLUGINS})
	set (VERSION_DONCKY "0.0.1")
	set (PACKAGE_DONCKY "cd-doncky")
	set (donckydatadir "${pluginsdatadir}/Doncky")
	configure_file (Doncky/data/Doncky.conf.in Doncky/data/Doncky.conf)
	add_subdirectory (Doncky)
endif()

############# DROP INDICATOR #################
set (GETTEXT_DROP_INDICATOR ${GETTEXT_PLUGINS})
set (VERSION_DROP_INDICATOR "1.1.3")
set (PACKAGE_DROP_INDICATOR "cd-drop_indicator")
set (drop_indicatordatadir "${pluginsdatadir}/drop-indicator")
configure_file (drop-indicator/data/drop_indicator.conf.in drop-indicator/data/drop_indicator.conf)
add_subdirectory (drop-indicator)

############# DUSTBIN #################
set (GETTEXT_DUSTBIN ${GETTEXT_PLUGINS})
set (VERSION_DUSTBIN "2.2.7")
set (PACKAGE_DUSTBIN "cd-dustbin")
set (dustbinuserdirname "dustbin")
set (dustbindatadir "${pluginsdatadir}/dustbin")
configure_file (dustbin/data/dustbin.conf.in dustbin/data/dustbin.conf)
add_subdirectory (dustbin)

############# GNOME-INTEGRATION #################
if (NOT "${enable_gnome_integration}" STREQUAL "no")
	pkg_check_modules (GNOME_INTEGRATION gio-2.0)
	if (NOT ${GNOME_INTEGRATION_FOUND})
		message (STATUS "Could not find gio; Cairo-Dock won't be built with Gnome>=2.22 support.")
	else()
		set (VERSION_GNOME_INTEGRATION "1.0.2")
		set (PACKAGE_GNOME_INTEGRATION "cd_gnome-integration")
		set (gnome_integrationdatadir "${pluginsdatadir}/gnome-integration")
		add_subdirectory ("gnome-integration")
	endif()
endif()

############# GNOME-INTEGRATION-OLD #################
if (NOT "${enable_old_gnome_integration}" STREQUAL "no")
	pkg_check_modules (OLD_GNOME_INTEGRATION gnome-vfs-2.0 libgnomeui-2.0)
	if (NOT ${OLD_GNOME_INTEGRATION_FOUND})
		message (STATUS "Could not find gnome-vfs and/or gnomeui; Cairo-Dock won't be built with Gnome<2.22 support.")
	else()
		set (VERSION_GNOME_INTEGRATION_OLD "1.0.3")
		set (PACKAGE_GNOME_INTEGRATION_OLD "cd_gnome-integration-old")
		set (gnome_integration_olddatadir "${pluginsdatadir}/gnome-integration-old")
		add_subdirectory ("gnome-integration-old")
	endif()
endif()

############# GMENU #################
if (NOT "${enable_gmenu}" STREQUAL "no")
	pkg_check_modules (GMENU_PACKAGE libgnome-menu)
	if (NOT ${GMENU_PACKAGE_FOUND})
		message (STATUS "Could not find libgnome-menu; Cairo-Dock won't be built with GMenu applet.")
	else()
		set (GETTEXT_GMENU ${GETTEXT_PLUGINS})
		set (VERSION_GMENU "1.1.5")
		set (PACKAGE_GMENU "cd-GMenu")
		set (gmenudatadir "${pluginsdatadir}/GMenu")
		configure_file (GMenu/data/GMenu.conf.in GMenu/data/GMenu.conf)
		add_subdirectory ("GMenu")
	endif()
endif()

############# ICON EFFECT #################
set (GETTEXT_ICON_EFFECTS ${GETTEXT_PLUGINS})
set (VERSION_ICON_EFFECTS "1.2.3")
set (PACKAGE_ICON_EFFECTS "cd-icon-effect")
set (icon_effectsdatadir "${pluginsdatadir}/icon-effect")
configure_file (icon-effect/data/icon-effect.conf.in icon-effect/data/icon-effect.conf)
add_subdirectory (icon-effect)

############# ILLUSION #################
set (GETTEXT_ILLUSION ${GETTEXT_PLUGINS})
set (VERSION_ILLUSION "1.0.6")
set (PACKAGE_ILLUSION "cd-illusion")
set (illusiondatadir "${pluginsdatadir}/illusion")
configure_file (illusion/data/illusion.conf.in illusion/data/illusion.conf)
add_subdirectory (illusion)

############# KDE-INTEGRATION #################
if (NOT "${enable_kde_integration}" STREQUAL "no")
	pkg_check_modules (KDE_INTEGRATION gio-2.0)
	if (NOT ${KDE_INTEGRATION_FOUND})
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
if (NOT ${KEYBOARD_INDICATOR_FOUND})
	message (STATUS "Could not find libxklavier; Cairo-Dock won't be built with keyboard-indicator applet.")
else()
	set (GETTEXT_KEYBOARD_INDICATOR ${GETTEXT_PLUGINS})
	set (VERSION_KEYBOARD_INDICATOR "1.1.2")
	set (PACKAGE_KEYBOARD_INDICATOR "cd-keyboard-indicator")
	set (keyboard_indicatordatadir "${pluginsdatadir}/keyboard-indicator")
	configure_file (keyboard-indicator/data/keyboard-indicator.conf.in keyboard-indicator/data/keyboard-indicator.conf)
	add_subdirectory ("keyboard-indicator")
endif()

############# LOGOUT #################
set (GETTEXT_LOGOUT ${GETTEXT_PLUGINS})
set (VERSION_LOGOUT "1.2.9")
set (PACKAGE_LOGOUT "cd-logout")
set (logoutdatadir "${pluginsdatadir}/logout")
configure_file (logout/data/logout.conf.in logout/data/logout.conf)
add_subdirectory (logout)

############# MAIL #################
execute_process(
	COMMAND libetpan-config --cflags
	OUTPUT_VARIABLE MAIL_PACKAGE_CFLAGS)
STRING (REGEX REPLACE "-I" "" MAIL_PACKAGE_CFLAGS ${MAIL_PACKAGE_CFLAGS})  # salete de libetpan ...
execute_process(
	COMMAND libetpan-config --libs
	OUTPUT_VARIABLE MAIL_PACKAGE_LIBS)
if ("${MAIL_PACKAGE_LIBS}" STREQUAL "")
	message (STATUS "warning : Could not find libetpan; Cairo-Dock won't be built with Mail applet.")
else()
	set (GETTEXT_MAIL ${GETTEXT_PLUGINS})
	set (VERSION_MAIL "1.0.3")
	set (PACKAGE_MAIL "cd-mail")
	set (maildatadir "${pluginsdatadir}/mail")
	configure_file (mail/data/mail.conf.in mail/data/mail.conf)
	add_subdirectory ("mail")
endif()

############# MOTION BLUR #################
set (GETTEXT_MOTION_BLUR ${GETTEXT_PLUGINS})
set (VERSION_MOTION_BLUR "1.0.3")
set (PACKAGE_MOTION_BLUR "cd-motion_blur")
set (motion_blurdatadir "${pluginsdatadir}/motion-blur")
configure_file (motion-blur/data/motion_blur.conf.in motion-blur/data/motion_blur.conf)
add_subdirectory (motion-blur)

############# MUSICPLAYER #################
set (GETTEXT_MUSICPLAYER ${GETTEXT_PLUGINS})
set (VERSION_MUSICPLAYER "1.0.3")
set (PACKAGE_MUSICPLAYER "cd-musicPlayer")
set (musicplayerdatadir "${pluginsdatadir}/musicPlayer")
configure_file (musicPlayer/data/musicPlayer.conf.in musicPlayer/data/musicPlayer.conf)
add_subdirectory (musicPlayer)

############# NETSPEED #################
set (GETTEXT_NETSPEED ${GETTEXT_PLUGINS})
set (VERSION_NETSPEED "1.2.3")
set (PACKAGE_NETSPEED "cd-netspeed")
set (netspeeddatadir "${pluginsdatadir}/netspeed")
configure_file (netspeed/data/netspeed.conf.in netspeed/data/netspeed.conf)
add_subdirectory (netspeed)

############# NETWORK_MONITOR #################
if ("${enable-network-monitor}" STREQUAL "yes")
	set (GETTEXT_NETWORK_MONITOR ${GETTEXT_PLUGINS})
	set (VERSION_NETWORK_MONITOR "0.2.1")
	set (PACKAGE_NETWORK_MONITOR "cd-network-monitor")
	set (network_monitordatadir "${pluginsdatadir}/Network-Monitor")
	configure_file (Network-Monitor/data/Network-Monitor.conf.in Network-Monitor/data/Network-Monitor.conf)
	add_subdirectory (Network-Monitor)
endif()

############# POWERMANAGER #################
set (GETTEXT_POWERMANAGER ${GETTEXT_PLUGINS})
set (VERSION_POWERMANAGER "1.3.2")
set (PACKAGE_POWERMANAGER "cd-powermanager")
set (powermanagerdatadir "${pluginsdatadir}/powermanager")
configure_file (powermanager/data/powermanager.conf.in powermanager/data/powermanager.conf)
add_subdirectory (powermanager)

############# QUICK BROWSER #################
set (GETTEXT_QUICK_BROWSER ${GETTEXT_PLUGINS})
set (VERSION_QUICK_BROWSER "1.0.7")
set (PACKAGE_QUICK_BROWSER "cd-quick-browser")
set (quick_browserdatadir "${pluginsdatadir}/quick_browser")
configure_file (quick-browser/data/quick-browser.conf.in quick-browser/data/quick-browser.conf)
add_subdirectory (quick-browser)

############# RSSREADER #################
set (GETTEXT_RSS_READER ${GETTEXT_PLUGINS})
set (VERSION_RSS_READER "1.0.1")
set (PACKAGE_RSS_READER "cd-rssreader")
set (rss_readerdatadir "${pluginsdatadir}/RSSreader")
configure_file (RSSreader/data/RSSreader.conf.in RSSreader/data/RSSreader.conf)
add_subdirectory (RSSreader)

############# SCOOBY_DO #################
if ("${enable-scooby-do}" STREQUAL "yes")
	set (GETTEXT_SCOOBY_DO ${GETTEXT_PLUGINS})
	set (VERSION_SCOOBY_DO "0.1.0")
	set (PACKAGE_SCOOBY_DO "cd-scooby-do")
	set (scooby_dodatadir "${pluginsdatadir}/Scooby-Do")
	configure_file (Scooby-Do/data/Scooby-Do.conf.in Scooby-Do/data/Scooby-Do.conf)
	add_subdirectory (Scooby-Do)
endif()

############# SHORTCUTS #################
set (GETTEXT_SHORTCUTS ${GETTEXT_PLUGINS})
set (VERSION_SHORTCUTS "1.2.4")
set (PACKAGE_SHORTCUTS "cd-shortcuts")
set (shortcutsdatadir "${pluginsdatadir}/shortcuts")
configure_file (shortcuts/data/shortcuts.conf.in shortcuts/data/shortcuts.conf)
add_subdirectory (shortcuts)

############# SHOW DESKTOP #################
pkg_check_modules (SHOW_DESKTOP_XRANDR xrandr)
if (NOT ${SHOW_DESKTOP_XRANDR_FOUND})
	message (STATUS "Could not find xrandr; ShowDesktop won't be built with screen resolution abilities.")
endif()
set (GETTEXT_SHOW_DESKTOP ${GETTEXT_PLUGINS})
set (VERSION_SHOW_DESKTOP "1.2.2")
set (PACKAGE_SHOW_DESKTOP "cd-showDesktop")
set (show_desktopdatadir "${pluginsdatadir}/showDesktop")
configure_file (showDesktop/data/showDesktop.conf.in showDesktop/data/showDesktop.conf)
add_subdirectory (showDesktop)

############# SHOW MOUSE #################
set (GETTEXT_SHOW_MOUSE ${GETTEXT_PLUGINS})
set (VERSION_SHOW_MOUSE "1.0.3")
set (PACKAGE_SHOW_MOUSE "cd-show_mouse")
set (show_mousedatadir "${pluginsdatadir}/show_mouse")
configure_file (show-mouse/data/show_mouse.conf.in show-mouse/data/show_mouse.conf)
add_subdirectory (show-mouse)

############# SLIDER #################
pkg_check_modules (EXIF libexif)
if (NOT ${EXIF_FOUND})
	message (STATUS "Could not find libexif; Slider won't be able to rotate the images accordingly to their exif data.")
endif()
set (GETTEXT_SLIDER ${GETTEXT_PLUGINS})
set (VERSION_SLIDER "2.0.5")
set (PACKAGE_SLIDER "cd-slider")
set (sliderdatadir "${pluginsdatadir}/slider")
configure_file (slider/data/slider.conf.in slider/data/slider.conf)
add_subdirectory (slider)

############# STACK #################
set (GETTEXT_STACK ${GETTEXT_PLUGINS})
set (VERSION_STACK "0.2.7")
set (PACKAGE_STACK "cd-stack")
set (stackdatadir "${pluginsdatadir}/stack")
configure_file (stack/data/stack.conf.in stack/data/stack.conf)
add_subdirectory (stack)

############# SWITCHER #################
set (GETTEXT_SWITCHER ${GETTEXT_PLUGINS})
set (VERSION_SWITCHER "2.1.3")
set (PACKAGE_SWITCHER "cd-switcher")
set (switcherdatadir "${pluginsdatadir}/switcher")
configure_file (switcher/data/switcher.conf.in switcher/data/switcher.conf)
add_subdirectory (switcher)

############# SYSTEM MONITOR #################
set (GETTEXT_SYSTEM_MONITOR ${GETTEXT_PLUGINS})
set (VERSION_SYSTEM_MONITOR "1.0.2")
set (PACKAGE_SYSTEM_MONITOR "cd-system-monitor")
set (system_monitordatadir "${pluginsdatadir}/System-monitor")
configure_file (System-Monitor/data/System-Monitor.conf.in System-Monitor/data/System-Monitor.conf)
add_subdirectory (System-Monitor)

############# SYSTRAY #################
set (GETTEXT_SYSTRAY ${GETTEXT_PLUGINS})
set (VERSION_SYSTRAY "0.1.11")
set (PACKAGE_SYSTRAY "cd-systray")
set (systraydatadir "${pluginsdatadir}/systray")
configure_file (systray/data/systray.conf.in systray/data/systray.conf)
add_subdirectory (systray)

############# TERMINAL #################
pkg_check_modules (TERMINAL_PACKAGE vte)
if (NOT ${TERMINAL_FOUND})
	message (STATUS "Could not find libvte; Cairo-Dock won't be built with terminal applet.")
else()
	set (GETTEXT_TERMINAL ${GETTEXT_PLUGINS})
	set (VERSION_TERMINAL "1.0.9")
	set (PACKAGE_TERMINAL "cd-terminal")
	set (terminaldatadir "${pluginsdatadir}/terminal")
	configure_file (terminal/data/terminal.conf.in terminal/data/terminal.conf)
	add_subdirectory (terminal)
endif()

############# TOMBOY #################
set (GETTEXT_TOMBOY ${GETTEXT_PLUGINS})
set (VERSION_TOMBOY "1.2.3")
set (PACKAGE_TOMBOY "cd-tomboy")
set (tomboydatadir "${pluginsdatadir}/tomboy")
configure_file (tomboy/data/tomboy.conf.in tomboy/data/tomboy.conf)
add_subdirectory (tomboy)

############# TOONS #################
set (GETTEXT_TOONS ${GETTEXT_PLUGINS})
set (VERSION_TOONS "1.0.6")
set (PACKAGE_TOONS "cd-Toons")
set (toonsdatadir "${pluginsdatadir}/Toons")
set (toonsuserdirname "Toons")
configure_file (Toons/data/Toons.conf.in Toons/data/Toons.conf)
add_subdirectory (Toons)

############# WEATHER #################
set (GETTEXT_WEATHER ${GETTEXT_PLUGINS})
set (VERSION_WEATHER "1.2.9")
set (PACKAGE_WEATHER "cd-weather")
set (weatherdatadir "${pluginsdatadir}/weather")
set (weatheruserdirname "weather")
configure_file (weather/data/weather.conf.in weather/data/weather.conf)
add_subdirectory (weather)

############# WEBLETS #################
pkg_check_modules (WEBKIT webkit-1.0)
if (NOT ${WEBKIT_FOUND})
	message (STATUS "Could not find webkit-1.0; Cairo-Dock won't be built with Weblets applet.")
else()
	set (GETTEXT_WEBLETS ${GETTEXT_PLUGINS})
	set (VERSION_WEBLETS "0.0.9")
	set (PACKAGE_WEBLETS "cd-weblets")
	set (webletsdatadir "${pluginsdatadir}/weblets")
	configure_file (weblets/data/weblets.conf.in weblets/data/weblets.conf)
	add_subdirectory ("weblets")
endif()

############# WIFI #################
set (GETTEXT_WIFI ${GETTEXT_PLUGINS})
set (VERSION_WIFI "1.2.3")
set (PACKAGE_WIFI "cd-wifi")
set (wifidatadir "${pluginsdatadir}/wifi")
configure_file (wifi/data/wifi.conf.in wifi/data/wifi.conf)
add_subdirectory (wifi)

############# XFCE-INTEGRATION #################
if (NOT "${enable_xfce_integration}" STREQUAL "no")
	pkg_check_modules (XFCE_INTEGRATION_THUNAR thunar-vfs-1)
	pkg_check_modules (XFCE_INTEGRATION_GVFS gio-2.0)
	if (NOT "${XFCE_INTEGRATION_THUNAR_FOUND}" AND NOT "${XFCE_INTEGRATION_GVFS_FOUND}")
#	if (NOT ${XFCE_INTEGRATION_THUNAR_FOUND})
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
if (NOT ${XGAMMA_PACKAGE_FOUND})
	message (STATUS "Could not find xxf86vm; Cairo-Dock won't be built with Xgamma applet.")
else()
	set (GETTEXT_XGAMMA ${GETTEXT_PLUGINS})
	set (VERSION_XGAMMA "1.1.5")
	set (PACKAGE_XGAMMA "cd-Xgamma")
	set (xgammadatadir "${pluginsdatadir}/Xgamma")
	configure_file (Xgamma/data/Xgamma.conf.in Xgamma/data/Xgamma.conf)
	add_subdirectory ("Xgamma")
endif()

add_subdirectory (po)


#original Makefile.am contents follow:

##dnl Process this file with automake to produce Makefile.in
#
#if WITH_OLD_GNOME_SUPPORT
#gnome_old_dir=gnome-integration-old
#endif
#
#if WITH_GNOME_SUPPORT
#gnome_dir=gnome-integration
#endif
#
#if WITH_XFCE_SUPPORT
#xfce_dir=xfce-integration
#endif
#
#if WITH_KDE_SUPPORT
#kde_dir=kde-integration
#endif
#
#if WITH_ALSA_MIXER
#alsa_mixer_dir=alsaMixer
#endif
#
#if WITH_TERMINAL
#terminal_dir=terminal
#endif
#
#if WITH_POWERMANAGER
#powermanager_dir=powermanager
#endif
#
#if WITH_WIFI
#wifi_dir=wifi
#endif
#
#if WITH_TOMBOY
#tomboy_dir=tomboy
#endif
#
#if WITH_COMPIZ_ICON
#compiz_icon_dir=compiz-icon
#endif
#
#if WITH_XGAMMA
#xgamma_dir=Xgamma
#endif
#
#if WITH_GMENU
#gmenu_dir=GMenu
#endif
#
#if WITH_MAIL
#mail_dir=mail
#endif
#
#if WITH_WEBLETS
#weblets_dir=weblets
#endif
#
#if WITH_KEYBOARD_INDICATOR
#keyboard_indicator_dir=keyboard-indicator
#endif
#
#if WITH_DND2SHARE
#dnd2share_dir=dnd2share
#endif
#
#if WITH_MUSICPLAYER
#musicplayer_dir=musicPlayer
#endif
#
#if WITH_NETWORK_MONITOR
#network_monitor_dir=Network-Monitor
#endif
#
#if WITH_SCOOBY_DO
#scooby_do_dir=Scooby-Do
#endif
#
#if WITH_RSSREADER
#rssreader_dir=RSSreader
#endif
#
#if WITH_DONCKY
#doncky_dir=Doncky
#endif
#
#SUBDIRS = . po\
#	${alsa_mixer_dir}\
#	Animated-icons\
#	Cairo-Penguin\
#	Clipper\
#	clock\
#	${compiz_icon_dir}\
#	Dbus\
#	desklet-rendering\
#	dialog-rendering\
#	${doncky_dir}\
#	${dnd2share_dir}\
#	dock-rendering\
#	drop-indicator\
#	dustbin\
#	${gmenu_dir}\
#	${gnome_dir}\
#	${gnome_old_dir}\
#	icon-effect\
#	illusion\
#	${kde_dir}\
#	${keyboard_indicator_dir}\
#	logout\
#	${mail_dir}\
#	motion-blur\
#	$(musicplayer_dir)\
#	netspeed\
#	$(network_monitor_dir)\
#	${powermanager_dir}\
#	quick-browser\
#	${rssreader_dir}\
#	${scooby_do_dir}\
#	shortcuts\
#	showDesktop\
#	show-mouse\
#	slider\
#	stack\
#	switcher\
#	System-Monitor\
#	systray\
#	${terminal_dir}\
#	${tomboy_dir}\
#	Toons\
#	weather\
#	${weblets_dir}\
#	${wifi_dir}\
#	${xfce_dir}\
#	${xgamma_dir}
#
#EXTRA_DIST = \
#	intltool-extract.in\
#	intltool-merge.in\
#	intltool-update.in\
#	LICENSE\
#	LGPL-2\
#	ChangeLog\
#	copyright
#
