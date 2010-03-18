add_subdirectory(themes)
add_subdirectory(digital)


########### install files ###############

install(FILES  clock.conf 	preview.jpg 	dates.svg 	icon.png 	icon-alarm.png DESTINATION ${clockdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . themes digital
#
#moduledir = $(clockdatadir)
#
#module_DATA = \
#	clock.conf\
#	preview.jpg\
#	dates.svg\
#	icon.png\
#	icon-alarm.png
#
#EXTRA_DIST = $(module_DATA)
