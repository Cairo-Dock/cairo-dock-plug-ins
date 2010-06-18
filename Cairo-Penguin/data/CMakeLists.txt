add_subdirectory (themes)


########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cairo-Penguin.conf
	preview.jpg
	icon.png
	DESTINATION ${cairo_penguindatadir})
