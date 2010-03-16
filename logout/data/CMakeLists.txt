
########### install files ###############

install(FILES  logout.conf 	icon.svg 	icon-scheduling.png DESTINATION ${logoutdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . 
#
#moduledir = $(logoutdatadir)
#
#module_DATA = \
#	logout.conf\
#	icon.svg\
#	icon-scheduling.png
#
#EXTRA_DIST = $(module_DATA)
