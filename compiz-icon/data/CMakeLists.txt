
########### install files ###############

install(FILES  compiz-kill 	compiz DESTINATION ${compiz_icondatadir})
install(FILES  compiz-icon.conf 	preview.jpg 	default.svg 	broken.svg 	other.svg 	0.svg 	1.svg 	2.svg 	3.svg 	4.svg 	icon.png DESTINATION ${compiz_icondatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = .
#
#moduledir = $(compiz_icondatadir)
#
#module_DATA = \
#	compiz-icon.conf\
#	preview.jpg\
#	default.svg\
#	broken.svg\
#	other.svg\
#	0.svg\
#	1.svg\
#	2.svg\
#	3.svg\
#	4.svg\
#	icon.png
#
#dist_compiz_icondata_SCRIPTS = \
#	compiz-kill\
#	compiz
#
#EXTRA_DIST = $(module_DATA)
