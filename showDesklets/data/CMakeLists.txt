
########### install files ###############

install(FILES  showDesklets.conf 	preview.jpg 	show-desklets.svg 	hide-desklets.svg 	icon.png DESTINATION ${show_deskletsdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(show_deskletsdatadir)
#
#module_DATA = \
#	showDesklets.conf\
#	preview.jpg\
#	show-desklets.svg\
#	hide-desklets.svg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
#
