
########### install files ###############

install(FILES  terminal.conf 	icon.png 	preview.jpg DESTINATION ${terminaldatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(terminaldatadir)
#
#module_DATA = \
#	terminal.conf\
#	icon.png\
#	preview.jpg
#
#EXTRA_DIST = $(module_DATA)
