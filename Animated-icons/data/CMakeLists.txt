
########### install files ###############

install(FILES  Animated-icons.conf 	preview.jpg 	icon.svg 	texture-chrome.png 	icon-rotation.png 	spot.png 	halo.png 	ray.png 	icon-wave.png DESTINATION ${animated_iconsdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(animated_iconsdatadir)
#
#module_DATA = \
#	Animated-icons.conf\
#	preview.jpg\
#	icon.svg\
#	texture-chrome.png\
#	icon-rotation.png\
#	spot.png\
#	halo.png\
#	ray.png\
#	icon-wave.png
#
#EXTRA_DIST = $(module_DATA)
