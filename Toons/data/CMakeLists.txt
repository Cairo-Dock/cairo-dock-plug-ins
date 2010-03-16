add_subdirectory(themes)


########### install files ###############

install(FILES  Toons.conf 	preview.jpg 	icon.png DESTINATION ${toonsdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . themes
#
#moduledir = $(toonsdatadir)
#
#module_DATA = \
#	Toons.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
