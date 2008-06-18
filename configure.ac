#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cairo-dock-plugins],[1.6.0],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=cairo-dock-plugins
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)

pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

srcdir="."
AC_SUBST(srcdir)
top_srcdir=".."
AC_SUBST(top_srcdir)
top_builddir=".."
AC_SUBST(top_builddir)

PKG_CHECK_MODULES(PACKAGE, "cairo-dock gtk+-2.0 gthread-2.0 cairo librsvg-2.0 dbus-1 dbus-glib-1 libxml-2.0")


############# COMPILATION CONDITIONNELLE #################
AC_ARG_ENABLE([old-gnome-integration],
	[  --enable-old-gnome-integration : Enable Gnome<2.22 integration (default=yes) ],
	[ with_old_gnome_support=${enableval} ],
	[ with_old_gnome_support=yes ])
AC_ARG_ENABLE(gnome-integration,
	[  --enable-gnome-integration : Enable Gnome>=2.22 integration (default=yes) ],
	[ with_gnome_support=${enableval} ],
	[ with_gnome_support=yes ])
AC_ARG_ENABLE(xfce-integration,
	[  --enable-xfce-integration : Enable XFCE integration (default=yes) ],
	[ with_xfce_support=${enableval} ],
	[ with_xfce_support=yes ])
AC_ARG_ENABLE(alsa-mixer,
	[  --enable-alsa-mixer : Enable Alsa-Mixer applet (default=yes) ],
	[ with_alsa_mixer=${enableval} ],
	[ with_alsa_mixer=yes ])
AC_ARG_ENABLE(terminal,
	[  --enable-terminal : Enable Terminal applet (default=yes) ],
	[ with_terminal=${enableval} ],
	[ with_terminal=yes ])


if test "$with_old_gnome_support" = "yes" ; then
	PKG_CHECK_MODULES(OLD_GNOME_INTEGRATION,gnome-vfs-2.0 libgnomeui-2.0, ,[ old_gnome_detected=false; AC_MSG_WARN(Could not find gnome-vfs and/or gnomeui; Cairo-Dock won't be built with Gnome<2.22 support.) ])
	AC_SUBST(OLD_GNOME_INTEGRATION_CFLAGS)
	AC_SUBST(OLD_GNOME_INTEGRATION_LIBS)
fi
if test "$with_gnome_support" = "yes" ; then
	PKG_CHECK_MODULES(GNOME_INTEGRATION,gio-2.0, ,[ gnome_detected=false; AC_MSG_WARN(Could not find gio; Cairo-Dock won't be built with Gnome>=2.22 support.) ])
	AC_SUBST(GNOME_INTEGRATION_CFLAGS)
	AC_SUBST(GNOME_INTEGRATION_LIBS)
fi
if test "$with_xfce_support" = "yes" ; then
	PKG_CHECK_MODULES(XFCE_INTEGRATION,thunar-vfs-1, ,[ xfce_detected=false; AC_MSG_WARN(Could not find thunar-vfs; Cairo-Dock won't be built with XFCE support.) ])
	AC_SUBST(XFCE_INTEGRATION_CFLAGS)
	AC_SUBST(XFCE_INTEGRATION_LIBS)
fi
if test "$with_alsa_mixer" = "yes" ; then
	PKG_CHECK_MODULES(ALSA_MIXER_PACKAGE,alsa, ,[ alsa_detected=false; AC_MSG_WARN(Could not find alsa; Cairo-Dock won't be built with AlsaMixer applet.) ])
	AC_SUBST(ALSA_MIXER_PACKAGE_CFLAGS)
	AC_SUBST(ALSA_MIXER_PACKAGE_LIBS)
fi
if test "$with_terminal" = "yes" ; then
	PKG_CHECK_MODULES(TERMINAL_PACKAGE,vte, ,[ vte_detected=false; AC_MSG_WARN(Could not find vte; Cairo-Dock won't be built with TERMINAL applet.) ])
	AC_SUBST(TERMINAL_PACKAGE_CFLAGS)
	AC_SUBST(TERMINAL_PACKAGE_LIBS)
fi

AM_CONDITIONAL( [WITH_OLD_GNOME_SUPPORT], [test "$with_old_gnome_support" = "yes"] )
AM_CONDITIONAL( [WITH_GNOME_SUPPORT], [test "$with_gnome_support" = "yes"] )
AM_CONDITIONAL( [WITH_XFCE_SUPPORT], [test "$with_xfce_support" = "yes"] )
AM_CONDITIONAL( [WITH_ALSA_MIXER], [test "$with_alsa_mixer" = "yes"] )
AM_CONDITIONAL( [WITH_TERMINAL], [test "$with_terminal" = "yes"] )



############# ALSA_MIXER #################
ALSA_MIXER_PACKAGE="cd-AlsaMixer"
AC_SUBST(GETTEXT_ALSA_MIXER, "$ALSA_MIXER_PACKAGE")

AC_SUBST(VERSION_ALSA_MIXER, "1.0.2")
AC_SUBST(PACKAGE_ALSA_MIXER, "$ALSA_MIXER_PACKAGE")

AC_SUBST(alsa_mixerdatadir, "${pkgdatadir}/AlsaMixer")

############# CLOCK #################
CLOCK_PACKAGE="cd-clock"
AC_SUBST(GETTEXT_CLOCK, "$CLOCK_PACKAGE")

AC_SUBST(VERSION_CLOCK, "1.3.4")
AC_SUBST(PACKAGE_CLOCK, "$CLOCK_PACKAGE")

AC_SUBST(clockdatadir, "${pkgdatadir}/clock")

############# CAIRO_PENGUIN #################
CAIRO_PENGUIN_PACKAGE="cd-Cairo-Penguin"
AC_SUBST(GETTEXT_CAIRO_PENGUIN, "$CAIRO_PENGUIN_PACKAGE")

AC_SUBST(VERSION_CAIRO_PENGUIN, "1.0.0")
AC_SUBST(PACKAGE_CAIRO_PENGUIN, "$CAIRO_PENGUIN_PACKAGE")

AC_SUBST(cairo_penguindatadir, "${pkgdatadir}/Cairo-Penguin")

############# COMPIZ_ICON #################
COMPIZ_ICON_PACKAGE="cd-compiz-icon"
AC_SUBST(GETTEXT_COMPIZ_ICON, "$COMPIZ_ICON_PACKAGE")

AC_SUBST(VERSION_COMPIZ_ICON, "1.0.0")
AC_SUBST(PACKAGE_COMPIZ_ICON, "$COMPIZ_ICON_PACKAGE")

AC_SUBST(compiz_icondatadir, "${pkgdatadir}/compiz-icon")

############# CPUSAGE #################
CPUSAGE_PACKAGE="cd-cpusage"
AC_SUBST(GETTEXT_CPUSAGE, "$CPUSAGE_PACKAGE")

AC_SUBST(VERSION_CPUSAGE, "1.0.0-rc1")
AC_SUBST(PACKAGE_CPUSAGE, "$CPUSAGE_PACKAGE")

AC_SUBST(cpusagedatadir, "${pkgdatadir}/cpusage")

############# DBUS #################
DBUS_PACKAGE="cd-Dbus"
AC_SUBST(GETTEXT_DBUS, "$DBUS_PACKAGE")

AC_SUBST(VERSION_DBUS, "0.1.0")
AC_SUBST(PACKAGE_DBUS, "$DBUS_PACKAGE")

AC_SUBST(dbusdatadir, "${pkgdatadir}/Dbus")

############# DUSTBIN #################
DUSTBIN_PACKAGE=cd-dustbin
AC_SUBST(GETTEXT_DUSTBIN, "$DUSTBIN_PACKAGE")

AC_SUBST(VERSION_DUSTBIN, "2.1.2")
AC_SUBST(PACKAGE_DUSTBIN, "$DUSTBIN_PACKAGE")

AC_SUBST(dustbindatadir, "${pkgdatadir}/dustbin")

############# GNOME-INTEGRATION #################
GNOME_INTEGRATION_PACKAGE=cd-gnome-integration

AC_SUBST(VERSION_GNOME_INTEGRATION, "1.0.0")
AC_SUBST(PACKAGE_GNOME_INTEGRATION, "$GNOME_INTEGRATION_PACKAGE")

AC_SUBST(gnome_integrationdatadir, "${pkgdatadir}/gnome-integration")

############# GNOME-INTEGRATION-OLD #################
GNOME_INTEGRATION_OLD_PACKAGE=cd-gnome-integration-old

AC_SUBST(VERSION_GNOME_INTEGRATION_OLD, "1.0.1")
AC_SUBST(PACKAGE_GNOME_INTEGRATION_OLD, "$GNOME_INTEGRATION_OLD_PACKAGE")

AC_SUBST(gnome_integration_olddatadir, "${pkgdatadir}/gnome-integration-old")

############# LOGOUT #################
LOGOUT_PACKAGE=cd-logout
AC_SUBST(GETTEXT_LOGOUT, "$LOGOUT_PACKAGE")

AC_SUBST(VERSION_LOGOUT, "1.2.2")
AC_SUBST(PACKAGE_LOGOUT, "$LOGOUT_PACKAGE")

AC_SUBST(logoutdatadir, "${pkgdatadir}/logout")

############# NETSPEED #################
NETSPEED_PACKAGE=cd-netspeed
AC_SUBST(GETTEXT_NETSPEED, "$NETSPEED_PACKAGE")

AC_SUBST(VERSION_NETSPEED, "1.0.0")
AC_SUBST(PACKAGE_NETSPEED, "$NETSPEED_PACKAGE")

AC_SUBST(netspeeddatadir, "${pkgdatadir}/netspeed")

############# NVIDIA #################
NVIDIA_PACKAGE=cd-nVidia
AC_SUBST(GETTEXT_NVIDIA, "$NVIDIA_PACKAGE")

AC_SUBST(VERSION_NVIDIA, "1.0.0-rc1")
AC_SUBST(PACKAGE_NVIDIA, "$NVIDIA_PACKAGE")

AC_SUBST(nvidiadatadir, "${pkgdatadir}/nVidia")

############# POWERMANAGER #################
POWERMANAGER_PACKAGE=cd-powermanager
AC_SUBST(GETTEXT_POWERMANAGER, "$POWERMANAGER_PACKAGE")

AC_SUBST(VERSION_POWERMANAGER, "1.2.6")
AC_SUBST(PACKAGE_POWERMANAGER, "$POWERMANAGER_PACKAGE")

AC_SUBST(powermanagerdatadir, "${pkgdatadir}/powermanager")

############# RAME #################
RAME_PACKAGE=cd-rame
AC_SUBST(GETTEXT_RAME, "$RAME_PACKAGE")

AC_SUBST(VERSION_RAME "1.0.0-beta1")
AC_SUBST(PACKAGE_RAME, "$RAME_PACKAGE")

AC_SUBST(ramedatadir, "${pkgdatadir}/rame")

############# RENDERING #################
RENDERING_PACKAGE=cd-rendering
AC_SUBST(GETTEXT_RENDERING, "$RENDERING_PACKAGE")

AC_SUBST(VERSION_RENDERING "1.4.10")
AC_SUBST(PACKAGE_RENDERING, "$RENDERING_PACKAGE")

AC_SUBST(renderingdatadir, "${pkgdatadir}/rendering")

############# RHYTHMBOX #################
RHYTHMBOX_PACKAGE=cd-rhythmbox
AC_SUBST(GETTEXT_RHYTHMBOX, "$RHYTHMBOX_PACKAGE")

AC_SUBST(VERSION_RHYTHMBOX, "1.2.0")
AC_SUBST(PACKAGE_RHYTHMBOX, "$RHYTHMBOX_PACKAGE")

AC_SUBST(rhythmboxdatadir, "${pkgdatadir}/rhythmbox")

############# SHORTCUTS #################
SHORTCUTS_PACKAGE=cd-shortcuts
AC_SUBST(GETTEXT_SHORTCUTS, "$SHORTCUTS_PACKAGE")

AC_SUBST(VERSION_SHORTCUTS, "1.1.2")
AC_SUBST(PACKAGE_SHORTCUTS, "$SHORTCUTS_PACKAGE")

AC_SUBST(shortcutsdatadir, "${pkgdatadir}/shortcuts")

############# SHOW_DESKLETS #################
SHOW_DESKLETS_PACKAGE=cd-showDesklets
AC_SUBST(GETTEXT_SHOW_DESKLETS, "$SHOW_DESKLETS_PACKAGE")

AC_SUBST(VERSION_SHOW_DESKLETS, "1.0.1")
AC_SUBST(PACKAGE_SHOW_DESKLETS, "$SHOW_DESKLETS_PACKAGE")

AC_SUBST(show_deskletsdatadir, "${pkgdatadir}/showDesklets")

############# SHOW_DESKTOP #################
SHOW_DESKTOP_PACKAGE=cd-showDesktop
AC_SUBST(GETTEXT_SHOW_DESKTOP, "$SHOW_DESKTOP_PACKAGE")

AC_SUBST(VERSION_SHOW_DESKTOP, "1.0.1")
AC_SUBST(PACKAGE_SHOW_DESKTOP, "$SHOW_DESKTOP_PACKAGE")

AC_SUBST(show_desktopdatadir, "${pkgdatadir}/showDesktop")

############# SYSTRAY #################
SYSTRAY_PACKAGE=cd-systray
AC_SUBST(GETTEXT_SYSTRAY, "$SYSTRAY_PACKAGE")

AC_SUBST(VERSION_SYSTRAY, "0.1.3")
AC_SUBST(PACKAGE_SYSTRAY, "$SYSTRAY_PACKAGE")

AC_SUBST(systraydatadir, "${pkgdatadir}/systray")

############# TERMINAL #################
TERMINAL_PACKAGE="cd-terminal"
AC_SUBST(GETTEXT_TERMINAL, "$TERMINAL_PACKAGE")

AC_SUBST(VERSION_TERMINAL, "1.0.1")
AC_SUBST(PACKAGE_TERMINAL, "$TERMINAL_PACKAGE")

AC_SUBST(terminaldatadir, "${pkgdatadir}/terminal")

############# TOMBOY #################
TOMBOY_PACKAGE="cd-tomboy"
AC_SUBST(GETTEXT_TOMBOY, "$TOMBOY_PACKAGE")

AC_SUBST(VERSION_TOMBOY, "1.0.0-rc1")
AC_SUBST(PACKAGE_TOMBOY, "$TOMBOY_PACKAGE")

AC_SUBST(tomboydatadir, "${pkgdatadir}/tomboy")

############# WEATHER #################
WEATHER_PACKAGE="cd-weather"
AC_SUBST(GETTEXT_WEATHER, "$WEATHER_PACKAGE")

AC_SUBST(VERSION_WEATHER, "1.1.4")
AC_SUBST(PACKAGE_WEATHER, "$WEATHER_PACKAGE")

AC_SUBST(weatherdatadir, "${pkgdatadir}/weather")

############# WIFI #################
WIFI_PACKAGE="cd-wifi"
AC_SUBST(GETTEXT_WIFI, "$WIFI_PACKAGE")

AC_SUBST(VERSION_WIFI, "1.0.3")
AC_SUBST(PACKAGE_WIFI, "$WIFI_PACKAGE")

AC_SUBST(wifidatadir, "${pkgdatadir}/wifi")

############# XFCE-INTEGRATION #################
XFCE_INTEGRATION_PACKAGE=cd-xfce-integration

AC_SUBST(VERSION_XFCE_INTEGRATION, "1.0.0")
AC_SUBST(PACKAGE_XFCE_INTEGRATION, "$XFCE_INTEGRATION_PACKAGE")

AC_SUBST(xfce_integrationdatadir, "${pkgdatadir}/xfce-integration")

############# XGAMMA #################
XGAMMA_PACKAGE="cd-Xgamma"
AC_SUBST(GETTEXT_XGAMMA, "$XGAMMA_PACKAGE")

AC_SUBST(VERSION_XGAMMA, "1.0.4")
AC_SUBST(PACKAGE_XGAMMA, "$XGAMMA_PACKAGE")

AC_SUBST(xgammadatadir, "${pkgdatadir}/Xgamma")

############# XMMS #################
XMMS_PACKAGE="cd-xmms"
AC_SUBST(GETTEXT_XMMS, "$XMMS_PACKAGE")

AC_SUBST(VERSION_XMMS, "1.0.4")
AC_SUBST(PACKAGE_XMMS, "$XMMS_PACKAGE")

AC_SUBST(xmmsdatadir, "${pkgdatadir}/xmms")


AC_CONFIG_FILES([
	Makefile
	po/Makefile.in
	
	alsaMixer/Makefile
	alsaMixer/src/Makefile
	alsaMixer/po/Makefile.in
	alsaMixer/data/Makefile
	alsaMixer/data/readme
	alsaMixer/data/AlsaMixer.conf
	
	Cairo-Penguin/Makefile
	Cairo-Penguin/src/Makefile
	Cairo-Penguin/po/Makefile.in
	Cairo-Penguin/data/Makefile
	Cairo-Penguin/data/readme
	Cairo-Penguin/data/Cairo-Penguin.conf
	Cairo-Penguin/data/themes/Makefile
	Cairo-Penguin/data/themes/Classic/Makefile
	Cairo-Penguin/data/themes/Wanda/Makefile
	
	clock/Makefile
	clock/src/Makefile
	clock/po/Makefile.in
	clock/data/Makefile
	clock/data/readme
	clock/data/clock.conf
	clock/data/themes/Makefile
	clock/data/themes/antique/Makefile
	clock/data/themes/default/Makefile
	clock/data/themes/default-24/Makefile
	clock/data/themes/fdo/Makefile
	clock/data/themes/funky/Makefile
	clock/data/themes/glassy/Makefile
	clock/data/themes/ipulse/Makefile
	clock/data/themes/ipulse-24/Makefile
	clock/data/themes/quartz-24/Makefile
	clock/data/themes/radium/Makefile
	clock/data/themes/radium-24/Makefile
	clock/data/themes/silvia/Makefile
	clock/data/themes/silvia-24/Makefile
	clock/data/themes/simple/Makefile
	clock/data/themes/simple-24/Makefile
	clock/data/themes/tango/Makefile
	clock/data/themes/I_Clock/Makefile
	clock/data/themes/BlueSlim/Makefile
	clock/data/themes/zen/Makefile
	
	compiz-icon/Makefile
	compiz-icon/src/Makefile
	compiz-icon/po/Makefile.in
	compiz-icon/data/Makefile
	compiz-icon/data/readme
	compiz-icon/data/compiz-icon.conf
	
	cpusage/Makefile
	cpusage/src/Makefile
	cpusage/po/Makefile.in
	cpusage/data/Makefile
	cpusage/data/readme
	cpusage/data/cpusage.conf
	
	Dbus/Makefile
	Dbus/src/Makefile
	Dbus/po/Makefile.in
	Dbus/data/Makefile
	Dbus/data/readme
	Dbus/data/Dbus.conf
	
	dustbin/Makefile
	dustbin/src/Makefile
	dustbin/po/Makefile.in
	dustbin/data/Makefile
	dustbin/data/dustbin.conf
	dustbin/data/readme
	dustbin/data/themes/Makefile
	dustbin/data/themes/Gion/Makefile
	dustbin/data/themes/Crystal/Makefile
	dustbin/data/themes/NuoveXT-2/Makefile
	dustbin/data/themes/OpenClipArt1/Makefile
	dustbin/data/themes/DroplineNeu/Makefile
	dustbin/data/themes/Human/Makefile
	dustbin/data/themes/Metal/Makefile
	dustbin/data/themes/Silver/Makefile
	dustbin/data/themes/I_Dustbin/Makefile
	dustbin/data/themes/Neon/Makefile
	dustbin/data/themes/Clear/Makefile
	
	gnome-integration/Makefile
	gnome-integration/src/Makefile
	gnome-integration/data/Makefile
	gnome-integration/data/readme
	
	gnome-integration-old/Makefile
	gnome-integration-old/src/Makefile
	gnome-integration-old/data/Makefile
	gnome-integration-old/data/readme
	
	logout/Makefile
	logout/src/Makefile
	logout/po/Makefile.in
	logout/data/Makefile
	logout/data/readme
	logout/data/logout.conf
	
	netspeed/Makefile
	netspeed/src/Makefile
	netspeed/po/Makefile.in
	netspeed/data/Makefile
	netspeed/data/readme
	netspeed/data/netspeed.conf
	
	nVidia/Makefile
	nVidia/src/Makefile
	nVidia/po/Makefile.in
	nVidia/data/Makefile
	nVidia/data/readme
	nVidia/data/nVidia.conf
	
	powermanager/Makefile
	powermanager/src/Makefile
	powermanager/po/Makefile.in
	powermanager/data/Makefile
	powermanager/data/powermanager.conf
	powermanager/data/readme
	
	rame/Makefile
	rame/src/Makefile
	rame/po/Makefile.in
	rame/data/Makefile
	rame/data/readme
	rame/data/rame.conf
	
	rendering/Makefile
	rendering/src/Makefile
	rendering/po/Makefile.in
	rendering/data/Makefile
	rendering/data/readme
	rendering/data/rendering.conf
	
	rhythmbox/Makefile
	rhythmbox/src/Makefile
	rhythmbox/po/Makefile.in
	rhythmbox/data/Makefile
	rhythmbox/data/rhythmbox.conf
	rhythmbox/data/readme
	
	shortcuts/Makefile
	shortcuts/src/Makefile
	shortcuts/po/Makefile.in
	shortcuts/data/Makefile
	shortcuts/data/readme
	shortcuts/data/shortcuts.conf
	
	showDesklets/Makefile
	showDesklets/src/Makefile
	showDesklets/po/Makefile.in
	showDesklets/data/Makefile
	showDesklets/data/readme
	showDesklets/data/showDesklets.conf
	
	showDesktop/Makefile
	showDesktop/src/Makefile
	showDesktop/po/Makefile.in
	showDesktop/data/Makefile
	showDesktop/data/readme
	showDesktop/data/showDesktop.conf
	
	systray/Makefile
	systray/src/Makefile
	systray/po/Makefile.in
	systray/data/Makefile
	systray/data/readme
	systray/data/systray.conf
	
	terminal/Makefile
	terminal/src/Makefile
	terminal/po/Makefile.in
	terminal/data/Makefile
	terminal/data/readme
	terminal/data/terminal.conf
	
	tomboy/Makefile
	tomboy/src/Makefile
	tomboy/po/Makefile.in
	tomboy/data/Makefile
	tomboy/data/tomboy.conf
	tomboy/data/readme
	
	weather/Makefile
	weather/src/Makefile
	weather/po/Makefile.in
	weather/data/Makefile
	weather/data/readme
	weather/data/weather.conf
	weather/data/themes/Makefile
	weather/data/themes/basic/Makefile
	weather/data/themes/Shiny/Makefile
	weather/data/themes/crystal/Makefile
	weather/data/themes/7degrees/Makefile
	weather/data/themes/clear/Makefile
	weather/data/themes/I_Weather/Makefile
	weather/data/themes/WeatherPod/Makefile
	
	wifi/Makefile
	wifi/src/Makefile
	wifi/po/Makefile.in
	wifi/data/Makefile
	wifi/data/readme
	wifi/data/wifi.conf
	
	xfce-integration/Makefile
	xfce-integration/src/Makefile
	xfce-integration/data/Makefile
	xfce-integration/data/readme
	
	Xgamma/Makefile
	Xgamma/src/Makefile
	Xgamma/po/Makefile.in
	Xgamma/data/Makefile
	Xgamma/data/readme
	Xgamma/data/Xgamma.conf
	
	xmms/Makefile
	xmms/src/Makefile
	xmms/po/Makefile.in
	xmms/data/Makefile
	xmms/data/readme
	xmms/data/xmms.conf
	])

AC_OUTPUT([po/Makefile
	alsaMixer/po/Makefile
	Cairo-Penguin/po/Makefile
	clock/po/Makefile
	compiz-icon/po/Makefile
	cpusage/po/Makefile
	Dbus/po/Makefile
	dustbin/po/Makefile
	logout/po/Makefile
	netspeed/po/Makefile
	nVidia/po/Makefile
	powermanager/po/Makefile
	rame/po/Makefile
	rendering/po/Makefile
	rhythmbox/po/Makefile
	shortcuts/po/Makefile
	showDesklets/po/Makefile
	showDesktop/po/Makefile
	systray/po/Makefile
	terminal/po/Makefile
	tomboy/po/Makefile
	weather/po/Makefile
	wifi/po/Makefile
	Xgamma/po/Makefile
	xmms/po/Makefile
	])
