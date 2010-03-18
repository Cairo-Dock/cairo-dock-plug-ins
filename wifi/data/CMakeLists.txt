
########### install files ###############

install(FILES  wifi.conf 	preview.jpg 	link-0.svg 	link-1.svg 	link-2.svg 	link-3.svg 	link-4.svg 	link-5.svg 	icon.png DESTINATION ${wifidatadir})
install(FILES  wifi DESTINATION ${wifidatadir})



#original Makefile.am contents follow:

#
#moduledir = $(wifidatadir)
#
#module_DATA = \
#	wifi.conf\
#	preview.jpg\
#	link-0.svg\
#	link-1.svg\
#	link-2.svg\
#	link-3.svg\
#	link-4.svg\
#	link-5.svg\
#	icon.png
#
#dist_wifidata_SCRIPTS = \
#	wifi
#
#EXTRA_DIST = $(module_DATA)
