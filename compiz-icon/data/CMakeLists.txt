
########### install files ###############

install(FILES compiz-kill compiz
	DESTINATION ${compiz_icondatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/compiz-icon.conf
	preview.jpg
	default.svg
	broken.svg
	other.svg
	0.svg
	1.svg
	2.svg
	3.svg
	4.svg
	icon.png
	DESTINATION ${compiz_icondatadir})
