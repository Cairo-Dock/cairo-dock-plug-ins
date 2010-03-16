
########### install files ###############

install (FILES Doncky.conf preview.jpg icon.png documentation.txt default.xml
	DESTINATION ${donckydatadir})

install (FILES nvidia-config
	DESTINATION ${donckydatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)



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
