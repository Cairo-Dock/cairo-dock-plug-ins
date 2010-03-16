add_subdirectory(themes)


########### install files ###############

install(FILES  dustbin.conf 	preview.jpg 	icon.png DESTINATION ${dustbindatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . themes
#
#moduledir = $(dustbindatadir)
#
#module_DATA = \
#	dustbin.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
