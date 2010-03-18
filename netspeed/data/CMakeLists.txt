
########### install files ###############

install(FILES  netspeed.conf 	preview.jpg 	default.png 	icon.png 	ok.png 	unknown.png 	bad.png 	netspeed DESTINATION ${netspeeddatadir})



#original Makefile.am contents follow:

#
#moduledir = $(netspeeddatadir)
#
#module_DATA = \
#	netspeed.conf\
#	preview.jpg\
#	default.png\
#	icon.png\
#	ok.png\
#	unknown.png\
#	bad.png\
#	netspeed
#
#EXTRA_DIST = $(module_DATA)
