add_subdirectory (themes)


########### install files ###############

install(FILES  Cairo-Penguin.conf 	preview.jpg 	icon.png DESTINATION ${cairo_penguindatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . themes
#
#moduledir = $(cairo_penguindatadir)
#
#module_DATA = \
#	Cairo-Penguin.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
