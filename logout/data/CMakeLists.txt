
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/logout.conf
	icon.svg
	icon-scheduling.png
	icon-lock.png
	system-hibernate.svg
	system-restart.svg
	system-shutdown.svg
	system-suspend.svg
	system-switch-user.svg
	system-users.svg
	emblem-reboot.png
	DESTINATION ${logoutdatadir})

install(FILES logout.sh
	DESTINATION ${logoutdatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
