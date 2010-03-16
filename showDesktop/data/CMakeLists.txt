
########### install files ###############

install(FILES  showDesktop.conf 	icon.png 	icon-active.png DESTINATION ${show_desktopdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(show_desktopdatadir)
#
#module_DATA = \
#	showDesktop.conf\
#	icon.png\
#	icon-active.png
#
#EXTRA_DIST = $(module_DATA)
