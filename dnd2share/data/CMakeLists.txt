
########### install files ###############

install (FILES  dnd2share.conf 	readme 	preview.jpg 	icon.svg DESTINATION ${dnd2sharedatadir})
install (SCRIPT  upload2free.sh DESTINATION ${dnd2sharedatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(dnd2sharedatadir)
#
#module_DATA = \
#	dnd2share.conf\
#	readme\
#	preview.jpg\
#	icon.svg
#
#dist_dnd2sharedata_SCRIPTS = \
#	upload2free.sh
#
#EXTRA_DIST = $(module_DATA)
