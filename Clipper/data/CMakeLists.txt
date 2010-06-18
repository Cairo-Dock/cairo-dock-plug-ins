
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Clipper.conf
	preview.jpg
	icon.svg
	Clipper-actions-gnome.conf
	Clipper-actions-xfce.conf
	Clipper-actions-kde.conf
	DESTINATION ${Clipperdatadir})
