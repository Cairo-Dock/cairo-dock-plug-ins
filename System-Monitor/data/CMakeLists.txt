
########### install files ###############

install(FILES  System-Monitor.conf 	preview.jpg 	icon.png 	button-cpu.png 	button-ram.png DESTINATION ${system_monitordatadir})
install(SCRIPT  nvidia-config DESTINATION ${system_monitordatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(system_monitordatadir)
#
#module_DATA = \
#	System-Monitor.conf\
#	preview.jpg\
#	icon.png\
#	button-cpu.png\
#	button-ram.png
#
#dist_system_monitordata_SCRIPTS = \
#	nvidia-config
#
#EXTRA_DIST = $(module_DATA)
