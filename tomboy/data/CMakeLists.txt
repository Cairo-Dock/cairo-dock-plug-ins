
########### install files ###############

install(FILES  default.svg 	close.svg 	broken.svg 	note.svg 	tomboy.conf 	icon.png 	preview.jpg DESTINATION ${tomboydatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . 
#
#moduledir = $(tomboydatadir)
#
#module_DATA = \
#	default.svg\
#	close.svg\
#	broken.svg\
#	note.svg\
#	tomboy.conf\
#	icon.png\
#	preview.jpg
#
#EXTRA_DIST = $(module_DATA)
