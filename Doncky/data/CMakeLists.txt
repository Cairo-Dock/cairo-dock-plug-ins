
########### install files ###############

install(FILES  Doncky.conf 	preview.jpg 	icon.png 	documentation.txt 	default.xml DESTINATION ${donckydatadir})
install(SCRIPT  nvidia-config DESTINATION ${donckydatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(donckydatadir)
#
#module_DATA = \
#	Doncky.conf\
#	preview.jpg\
#	icon.png\
#	documentation.txt\
#	default.xml
#
#dist_donckydata_SCRIPTS = \
#	nvidia-config
#
#EXTRA_DIST = $(module_DATA)
