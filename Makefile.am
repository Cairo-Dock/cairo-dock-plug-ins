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


SUBDIRS = . po\
	${alsa_mixer_dir}\
	Cairo-Penguin\
	clock\
	compiz-icon\
	cpusage\
	Dbus\
	dustbin\
	${gnome_dir}\
	${gnome_old_dir}\
	logout\
	netspeed\
	nVidia\
	powermanager\
	rame\
	rendering\
	rhythmbox\
	shortcuts\
	showDesklets\
	showDesktop\
	switcher\
	systray\
	${terminal_dir}\
	tomboy\
	weather\
	wifi\
	${xfce_dir}\
	Xgamma\
	xmms

EXTRA_DIST = \
	intltool-extract.in\
	intltool-merge.in\
	intltool-update.in