add_subdirectory(themes)
add_subdirectory(digital)

########### install files ###############

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/clock.conf
	preview.jpg
	dates.svg
	icon.png
	icon-alarm.png
	icon-task.png
	DESTINATION ${clockdatadir})
