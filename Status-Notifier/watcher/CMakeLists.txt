configure_file(${CMAKE_CURRENT_SOURCE_DIR}/status-notifier-watcher.py.in ${CMAKE_CURRENT_BINARY_DIR}/status-notifier-watcher.py)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/status-notifier-watcher.py
	DESTINATION ${pluginsdir}
	RENAME status-notifier-watcher
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
