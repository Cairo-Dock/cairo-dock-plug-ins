
########### install files ###############

install(FILES  slider.conf 	preview.jpg 	icon.png DESTINATION ${sliderdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(sliderdatadir)
#
#module_DATA = \
#	slider.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
