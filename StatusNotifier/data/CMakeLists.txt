
########### install files ###############

install(FILES  StatusNotifier.conf 	preview.jpg 	icon.png DESTINATION ${statusnotifierdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(statusnotifierdatadir)
#
#module_DATA = \
#	StatusNotifier.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
