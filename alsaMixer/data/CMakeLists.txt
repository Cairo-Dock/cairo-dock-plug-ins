
########### install files ###############

install(FILES  AlsaMixer.conf 	default.svg 	mute.svg 	broken.svg 	preview.jpg 	icon.png DESTINATION ${alsa_mixerdatadir})

#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(alsa_mixerdatadir)
#
#module_DATA = \
#	AlsaMixer.conf\
#	default.svg\
#	mute.svg\
#	broken.svg\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
