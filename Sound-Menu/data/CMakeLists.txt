
########### install files ###############

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/Sound-Menu.conf
	preview.jpg
	icon.svg
	default.svg
	mute.svg
	audio-volume-off.svg
	audio-volume-low.svg
	audio-volume-medium.svg
	audio-volume-high.svg
	audio-volume-muted.svg
	DESTINATION ${sound_menudatadir})
