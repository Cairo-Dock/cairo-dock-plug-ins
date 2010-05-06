
########### install files ###############

install(FILES  Status-Notifier.conf 	preview.jpg 	icon.png DESTINATION ${status_notifierdatadir})



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
