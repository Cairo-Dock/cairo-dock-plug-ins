
########### install files ###############

install(FILES  keyboard-indicator.conf 	preview.jpg 	icon.png DESTINATION ${keyboard_indicatordatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(keyboard_indicatordatadir)
#
#module_DATA = \
#	keyboard-indicator.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
