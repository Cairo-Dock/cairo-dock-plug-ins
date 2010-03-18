
########### install files ###############

install (FILES System-Monitor.conf 	preview.jpg icon.png button-cpu.png button-ram.png
	DESTINATION ${system_monitordatadir})

install (FILES nvidia-config
	DESTINATION ${system_monitordatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)



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
