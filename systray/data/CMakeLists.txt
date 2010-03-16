
########### install files ###############

install(FILES  systray.conf 	preview.jpg 	icon.png DESTINATION ${systraydatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(systraydatadir)
#
#module_DATA = \
#	systray.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
