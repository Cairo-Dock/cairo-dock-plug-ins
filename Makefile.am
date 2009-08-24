#dnl Process this file with automake to produce Makefile.in

if WITH_OLD_GNOME_SUPPORT
gnome_old_dir=gnome-integration-old
endif

if WITH_GNOME_SUPPORT
gnome_dir=gnome-integration
endif

if WITH_XFCE_SUPPORT
xfce_dir=xfce-integration
endif

if WITH_ALSA_MIXER
alsa_mixer_dir=alsaMixer
endif

if WITH_TERMINAL
terminal_dir=terminal
endif

if WITH_POWERMANAGER
powermanager_dir=powermanager
endif

if WITH_WIFI
wifi_dir=wifi
endif

if WITH_TOMBOY
tomboy_dir=tomboy
endif

if WITH_COMPIZ_ICON
compiz_icon_dir=compiz-icon
endif

if WITH_XGAMMA
xgamma_dir=Xgamma
endif

if WITH_GMENU
gmenu_dir=GMenu
endif

if WITH_MAIL
mail_dir=mail
endif

if WITH_WEBLETS
weblets_dir=weblets
endif

if WITH_KEYBOARD_INDICATOR
keyboard_indicator_dir=keyboard-indicator
endif

if WITH_DND2SHARE
dnd2share_dir=dnd2share
endif

if WITH_MUSICPLAYER
musicplayer_dir=musicPlayer
endif

SUBDIRS = . po\
	${alsa_mixer_dir}\
	Animated-icons\
	Cairo-Penguin\
	Clipper\
	clock\
	${compiz_icon_dir}\
	Dbus\
	desklet-rendering\
	dialog-rendering\
	${dnd2share_dir}\
	dock-rendering\
	drop-indicator\
	dustbin\
	${gmenu_dir}\
	${gnome_dir}\
	${gnome_old_dir}\
	icon-effect\
	illusion\
	${keyboard_indicator_dir}\
	logout\
	${mail_dir}\
	motion-blur\
	$(musicplayer_dir)\
	netspeed\
	${powermanager_dir}\
	quick-browser\
	shortcuts\
	showDesklets\
	showDesktop\
	show-mouse\
	slider\
	stack\
	switcher\
	System-Monitor\
	systray\
	${terminal_dir}\
	${tomboy_dir}\
	Toons\
	weather\
	${weblets_dir}\
	${wifi_dir}\
	${xfce_dir}\
	${xgamma_dir}

EXTRA_DIST = \
	intltool-extract.in\
	intltool-merge.in\
	intltool-update.in\
	LICENSE

