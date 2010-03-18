
########### install files ###############

install(FILES  icon-effect.conf 	preview.jpg 	icon.png 	star.png 	snow.png 	rain.png 	icon-fire.svg 	icon-storm.png 	icon-firework.png DESTINATION ${icon_effectsdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(icon_effectsdatadir)
#
#module_DATA = \
#	icon-effect.conf\
#	preview.jpg\
#	icon.png\
#	star.png\
#	snow.png\
#	rain.png\
#	icon-fire.svg\
#	icon-storm.png\
#	icon-firework.png
#
#EXTRA_DIST = $(module_DATA)
