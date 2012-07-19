
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Dbus.conf
	icon.svg
	DESTINATION ${dbusdatadir})

# Unity API ## need SetMenu => DBusMenu
if (${DBUSMENU_GTK_FOUND})
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cairo-dock-unity-bridge.py.in ${CMAKE_CURRENT_BINARY_DIR}/cairo-dock-unity-bridge.py)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cairo-dock-unity-bridge.py
		DESTINATION ${pluginsdir}
		RENAME cairo-dock-unity-bridge
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()
