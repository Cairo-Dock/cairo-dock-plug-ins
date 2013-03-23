
########### install files ###############

install(FILES appmenu-registrar.py
	DESTINATION ${pluginsdir}
	RENAME appmenu-registrar
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
