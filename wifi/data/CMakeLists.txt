
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/wifi.conf
	preview.jpg
	link-0.svg
	link-1.svg
	link-2.svg
	link-3.svg
	link-4.svg
	link-5.svg
	icon.png
	DESTINATION ${wifidatadir})

install(FILES wifi
	DESTINATION ${wifidatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
