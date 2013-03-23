
########### install files ###############

install(FILES status-notifier-watcher.py
	DESTINATION ${pluginsdir}
	RENAME status-notifier-watcher
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
