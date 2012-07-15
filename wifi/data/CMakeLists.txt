add_subdirectory(gauges)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/wifi.conf
	preview.jpg
	default.svg
	no-signal.svg
	icon.png
	DESTINATION ${wifidatadir})

#install(FILES wifi
#	DESTINATION ${wifidatadir}
#	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
