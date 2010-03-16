
########### install files ###############

install(FILES  drop_indicator.conf 	preview.jpg 	icon.svg 	default-drop-indicator.svg 	default-hover-indicator.svg DESTINATION ${drop_indicatordatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(drop_indicatordatadir)
#
#module_DATA = \
#	drop_indicator.conf\
#	preview.jpg\
#	icon.svg\
#	default-drop-indicator.svg\
#	default-hover-indicator.svg
#
#EXTRA_DIST = $(module_DATA)
