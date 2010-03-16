
########### install files ###############

install(FILES  CD_APPLET_NAME.conf 	preview.jpg 	icon.png DESTINATION ${pkgdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(pkgdatadir)
#
#module_DATA = \
#	CD_APPLET_NAME.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
