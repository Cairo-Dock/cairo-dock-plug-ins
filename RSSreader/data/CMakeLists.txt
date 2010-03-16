
########### install files ###############

install(FILES  RSSreader.conf 	preview.jpg 	icon.svg DESTINATION ${rss_readerdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(rss_readerdatadir)
#
#module_DATA = \
#	RSSreader.conf\
#	preview.jpg\
#	icon.svg
#
#EXTRA_DIST = $(module_DATA)
