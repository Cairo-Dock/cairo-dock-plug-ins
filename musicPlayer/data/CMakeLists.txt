add_subdirectory(themes)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/musicPlayer.conf
	broken.svg
	pause.svg
	play.svg
	stop.svg
	default.svg
	broken.jpg
	pause.jpg
	play.jpg
	stop.jpg
	default.jpg
	0.svg
	1.svg
	2.svg
	3.svg
	preview.jpg
	icon.png
	DESTINATION ${musicplayerdatadir})
