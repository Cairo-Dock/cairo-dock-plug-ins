
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/netspeed.conf
	preview.jpg
	default.png
	icon.png
	ok.png
	unknown.png
	bad.png
	DESTINATION ${netspeeddatadir})

install(FILES netspeed
	DESTINATION ${netspeeddatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
