
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rendering.conf
	icon.svg
	preview-caroussel.jpg
		readme-caroussel-view
	preview-parabolic.jpg
		readme-parabolic-view
	preview-3D-plane.jpg
		readme-3D-plane-view
	preview-rainbow.jpg
		readme-rainbow-view
	preview-diapo-simple.jpg
		readme-diapo-simple-view
	preview-curve.jpg
		readme-curve-view
	preview-panel.jpg
		readme-panel-view
	DESTINATION ${renderingdatadir})
