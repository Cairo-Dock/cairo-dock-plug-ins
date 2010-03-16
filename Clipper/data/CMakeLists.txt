
########### install files ###############

install(FILES  Clipper.conf 	preview.jpg 	icon.svg 	Clipper-actions-gnome.conf 	Clipper-actions-xfce.conf 	Clipper-actions-kde.conf DESTINATION ${Clipperdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(Clipperdatadir)
#
#module_DATA = \
#	Clipper.conf\
#	preview.jpg\
#	icon.svg\
#	Clipper-actions-gnome.conf\
#	Clipper-actions-xfce.conf\
#	Clipper-actions-kde.conf
#
#EXTRA_DIST = $(module_DATA)
