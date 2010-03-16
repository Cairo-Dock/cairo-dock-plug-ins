
########### install files ###############

install(FILES  switcher.conf 	broken.svg 	default.svg 	workspaces.svg 	preview.jpg DESTINATION ${switcherdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(switcherdatadir)
#
#module_DATA = \
#	switcher.conf\
#	broken.svg\
#	default.svg\
#	workspaces.svg\
#	preview.jpg
#
#EXTRA_DIST = $(module_DATA)
