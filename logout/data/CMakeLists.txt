
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/logout.conf
	icon.svg
	icon-scheduling.svg
	locked.svg
	system-guest.svg
	system-hibernate.svg
	system-log-out.svg
	system-restart.svg
	system-shutdown.svg
	system-suspend.svg
	preview.jpg
	DESTINATION ${logoutdatadir})

install(FILES logout.sh
	DESTINATION ${logoutdatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
