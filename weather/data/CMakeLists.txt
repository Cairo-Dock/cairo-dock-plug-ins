add_subdirectory(themes)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/weather.conf
	preview.jpg
	broken.png
	icon.png
	DESTINATION ${weatherdatadir})
