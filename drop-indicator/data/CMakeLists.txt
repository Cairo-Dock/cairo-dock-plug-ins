
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/drop_indicator.conf
	preview.jpg
	icon.svg
	default-drop-indicator.svg
	default-hover-indicator.svg
	DESTINATION ${drop_indicatordatadir})
