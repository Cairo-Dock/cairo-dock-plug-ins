
########### install files ###############

install(FILES  weblets.conf 	preview.jpg 	icon.png DESTINATION ${webletsdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(webletsdatadir)
#
#module_DATA = \
#	weblets.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
