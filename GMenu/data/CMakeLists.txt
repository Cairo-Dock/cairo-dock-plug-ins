
########### install files ###############

install(FILES  GMenu.conf 	preview.jpg 	icon.svg 	icon-recent.png DESTINATION ${gmenudatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(gmenudatadir)
#
#module_DATA = \
#	GMenu.conf\
#	preview.jpg\
#	icon.svg\
#	icon-recent.png
#	
#EXTRA_DIST = $(module_DATA)
