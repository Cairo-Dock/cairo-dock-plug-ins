add_subdirectory(gauges)

########### install files ###############

install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/AlsaMixer.conf
	default.svg
	mute.svg
	emblem-mute.svg
	broken.svg
	preview.jpg
	icon.png
	DESTINATION ${alsa_mixerdatadir})
