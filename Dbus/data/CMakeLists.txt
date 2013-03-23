
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Dbus.conf
	icon.svg
	DESTINATION ${dbusdatadir})

# Unity Launcher API ## need SetMenu => DBusMenu
if (${DBUSMENU_GTK_FOUND})
	install(FILES cairo-dock-launcher-API-daemon.py
		DESTINATION ${pluginsdir}
		RENAME cairo-dock-launcher-API-daemon
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()
