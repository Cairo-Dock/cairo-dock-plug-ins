
########### install files ###############

install(FILES  show_mouse.conf 	preview.jpg 	icon.png DESTINATION ${show_mousedatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(show_mousedatadir)
#
#module_DATA = \
#	show_mouse.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
