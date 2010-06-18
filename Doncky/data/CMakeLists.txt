
########### install files ###############

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/Doncky.conf
	preview.jpg
	icon.png
	documentation.txt
	default.xml
	DESTINATION ${donckydatadir})

install (FILES nvidia-config
	DESTINATION ${donckydatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
