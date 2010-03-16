
########### install files ###############

install(FILES  Dbus.conf 	icon.svg DESTINATION ${dbusdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(dbusdatadir)
#
#module_DATA = \
#	Dbus.conf\
#	icon.svg
#
#EXTRA_DIST = $(module_DATA)
