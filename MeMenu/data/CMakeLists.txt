########### subdirs if any ###############
add_subdirectory (status-icons)

########### install files ###############

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/MeMenu.conf
	preview.jpg
	icon.svg
	DESTINATION ${memenudatadir})

