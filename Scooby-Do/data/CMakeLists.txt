
########### install files ###############

install (FILES calc.sh 	updatedb.sh
	DESTINATION ${scooby_dodatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install (FILES Scooby-Do.conf preview.jpg icon.png arrows.svg active-button.svg inactive-button.svg internet.png google.png wikipedia.png yahoo.png amazon.png mediadico.png files.png firefox.png recent.png
	DESTINATION ${scooby_dodatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(scooby_dodatadir)
#
#module_DATA = \
#	Scooby-Do.conf\
#	preview.jpg\
#	icon.png\
#	arrows.svg\
#	active-button.svg\
#	inactive-button.svg\
#	internet.png\
#	google.png\
#	wikipedia.png\
#	yahoo.png\
#	amazon.png\
#	mediadico.png\
#	files.png\
#	firefox.png\
#	recent.png
#
#
#dist_scooby_dodata_SCRIPTS = \
#	calc.sh\
#	updatedb.sh
#
#EXTRA_DIST = $(module_DATA)
