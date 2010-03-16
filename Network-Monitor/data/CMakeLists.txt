
########### install files ###############

install(FILES  Network-Monitor.conf 	preview.jpg 	icon.png 	link-0.svg 	link-1.svg 	link-2.svg 	link-3.svg 	link-4.svg 	link-5.svg 	network-wired.png 	network-not-connected.png DESTINATION ${network_monitordatadir})
install(SCRIPT  wifi DESTINATION ${network_monitordatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(network_monitordatadir)
#
#module_DATA = \
#	Network-Monitor.conf\
#	preview.jpg\
#	icon.png\
#	link-0.svg\
#	link-1.svg\
#	link-2.svg\
#	link-3.svg\
#	link-4.svg\
#	link-5.svg\
#	network-wired.png\
#	network-not-connected.png
#
#dist_network_monitordata_SCRIPTS = \
#	wifi
#	
#EXTRA_DIST = $(module_DATA)
