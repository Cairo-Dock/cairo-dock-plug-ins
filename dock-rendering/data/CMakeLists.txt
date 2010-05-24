
########### install files ###############

install(FILES
rendering.conf 	icon.svg
	preview-caroussel.jpg 		readme-caroussel-view
	preview-parabolic.jpg 		readme-parabolic-view
	preview-3D-plane.jpg 		readme-3D-plane-view
	preview-rainbow.jpg 		readme-rainbow-view
	preview-diapo-simple.jpg 	readme-diapo-simple-view
	preview-curve.jpg 			readme-curve-view
	preview-panel.jpg 			readme-panel-view
	DESTINATION ${renderingdatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . 
#
#moduledir = $(renderingdatadir)
#
#module_DATA = \
#	rendering.conf\
#	icon.svg\
#	preview-caroussel.jpg\
#	preview-parabolic.jpg\
#	preview-3D-plane.jpg\
#	preview-rainbow.jpg\
#	preview-diapo.jpg\
#	preview-diapo-simple.jpg\
#	preview-curve.jpg\
#	readme-3D-plane-view\
#	readme-caroussel-view\
#	readme-curve-view\
#	readme-diapo-simple-view\
#	readme-diapo-view\
#	readme-parabolic-view\
#	readme-rainbow-view
#
#EXTRA_DIST = $(module_DATA)
