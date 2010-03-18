
########### install files ###############

install(FILES  motion_blur.conf 	icon.png DESTINATION ${motion_blurdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(motion_blurdatadir)
#
#module_DATA = \
#	motion_blur.conf\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
