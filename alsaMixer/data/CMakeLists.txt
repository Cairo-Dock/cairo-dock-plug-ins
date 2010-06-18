
########### install files ###############

install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/AlsaMixer.conf
	default.svg
	mute.svg
	broken.svg
	preview.jpg
	icon.png
	DESTINATION ${alsa_mixerdatadir})
