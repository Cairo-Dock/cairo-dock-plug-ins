########### subdirs if any ###############
add_subdirectory (status-icons)

########### install files ###############

install (FILES
	MeMenu.conf
	preview.jpg
	icon.png
	DESTINATION ${memenudatadir})

