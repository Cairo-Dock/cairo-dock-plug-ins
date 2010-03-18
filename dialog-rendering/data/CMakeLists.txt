
########### install files ###############

install(FILES  dialog-rendering.conf 	icon.svg DESTINATION ${dialog_renderingdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(dialog_renderingdatadir)
#
#module_DATA = \
#	dialog-rendering.conf\
#	icon.svg
#
#EXTRA_DIST = $(module_DATA)
