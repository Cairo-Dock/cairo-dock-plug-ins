
########### install files ###############

install(FILES  shortcuts.conf 	icon.png 	preview.jpg DESTINATION ${shortcutsdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(shortcutsdatadir)
#
#module_DATA = \
#	shortcuts.conf\
#	icon.png\
#	preview.jpg
#
#EXTRA_DIST = $(module_DATA)
