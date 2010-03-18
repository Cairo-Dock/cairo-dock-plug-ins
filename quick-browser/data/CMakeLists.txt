
########### install files ###############

install(FILES  quick-browser.conf 	preview.jpg 	icon.svg DESTINATION ${quick_browserdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(quick_browserdatadir)
#
#module_DATA = \
#	quick-browser.conf\
#	preview.jpg\
#	icon.svg
#
#EXTRA_DIST = $(module_DATA)
