add_subdirectory(themes)


########### install files ###############

install(FILES  weather.conf 	preview.jpg 	broken.png 	icon.png DESTINATION ${weatherdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . themes
#
#moduledir = $(weatherdatadir)
#
#module_DATA = \
#	weather.conf\
#	preview.jpg\
#	broken.png\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
