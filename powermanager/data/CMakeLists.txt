
########### install files ###############

install(FILES  sector.svg 	default-battery.svg 	default-charge.svg 	powermanager.conf 	preview.jpg 	icon.png 	charge.svg DESTINATION ${powermanagerdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(powermanagerdatadir)
#
#module_DATA = \
#	sector.svg\
#	default-battery.svg\
#	default-charge.svg\
#	powermanager.conf\
#	preview.jpg\
#	icon.png\
#	charge.svg
#
#EXTRA_DIST = $(module_DATA)
