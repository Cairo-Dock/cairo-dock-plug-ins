
########### install files ###############

install(FILES  Xgamma.conf 	preview.jpg 	icon.png DESTINATION ${xgammadatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(xgammadatadir)
#
#module_DATA = \
#	Xgamma.conf\
#	preview.jpg\
#	icon.png
#
#EXTRA_DIST = $(module_DATA)
