
########### install files ###############

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/dnd2share.conf
	preview.jpg
	icon.svg
	DESTINATION ${dnd2sharedatadir})

install (FILES upload2free.sh
	DESTINATION ${dnd2sharedatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
