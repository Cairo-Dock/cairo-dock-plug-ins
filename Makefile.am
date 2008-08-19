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

if WITH_RHYTHMBOX
rhythmbox_dir=rhythmbox
endif

if WITH_NVIDIA
nvidia_dir=nVidia
endif

if WITH_COMPIZ_ICON
compiz_icon_dir=compiz-icon
endif

SUBDIRS = . po\
	${alsa_mixer_dir}\
	Cairo-Penguin\
	clock\
	${compiz_icon_dir}\
	cpusage\
	Dbus\
	dustbin\
	${gnome_dir}\
	${gnome_old_dir}\
	logout\
	netspeed\
	${nvidia_dir}\
	${powermanager_dir}\
	rame\
	rendering\
	${rhythmbox_dir}\
	shortcuts\
	showDesklets\
	showDesktop\
	slider\
	stack\
	switcher\
	systray\
	${terminal_dir}\
	${tomboy_dir}\
	weather\
	${wifi_dir}\
	${xfce_dir}\
	Xgamma\
	xmms

EXTRA_DIST = \
	intltool-extract.in\
	intltool-merge.in\
	intltool-update.in
