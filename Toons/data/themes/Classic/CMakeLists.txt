
########### install files ###############

install(FILES  theme.conf 	readme 	toon.svg 	pupil.svg 	eyelid.svg DESTINATION ${toonsdatadir}/themes/Classic)



#original Makefile.am contents follow:

#themedir = $(toonsdatadir)/themes/Classic
#
#theme_DATA =	    \
#	theme.conf\
#	readme\
#	toon.svg\
#	pupil.svg\
#	eyelid.svg
#
#EXTRA_DIST = $(theme_DATA)
