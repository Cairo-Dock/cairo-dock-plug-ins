add_subdirectory(gauges)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/wifi.conf
	preview.jpg
	default.svg
	no-signal.svg
	icon.png
	DESTINATION ${wifidatadir})
