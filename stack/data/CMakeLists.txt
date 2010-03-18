
########### install files ###############

install(FILES  stack.conf 	preview.jpg 	icon.png 	text-icon.svg 	url-icon.svg DESTINATION ${stackdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(stackdatadir)
#
#module_DATA = \
#	stack.conf\
#	preview.jpg\
#	icon.png\
#	text-icon.svg\
#	url-icon.svg
#
#EXTRA_DIST = $(module_DATA)
