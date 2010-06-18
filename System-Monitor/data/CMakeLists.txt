
########### install files ###############

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/System-Monitor.conf
	preview.jpg
	icon.png
	button-cpu.png
	button-ram.png
	DESTINATION ${system_monitordatadir})

install (FILES nvidia-config
	DESTINATION ${system_monitordatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
