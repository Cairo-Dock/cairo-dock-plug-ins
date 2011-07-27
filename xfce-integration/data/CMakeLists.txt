
########### install files ###############

install(FILES icon.png
	DESTINATION ${xfce_integrationdatadir})

install(FILES lock-screen.sh
	DESTINATION ${xfce_integrationdatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
