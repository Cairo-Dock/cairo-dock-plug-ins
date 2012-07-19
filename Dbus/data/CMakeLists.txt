
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Dbus.conf
	icon.svg
	DESTINATION ${dbusdatadir})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cairo-dock-unity-bridge.py
	DESTINATION ${pluginsdir}
	RENAME cairo-dock-unity-bridge
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
