add_subdirectory(gauges)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/powermanager.conf
	default-battery.svg
	default-charge.svg
	sector.svg
	preview.jpg
	icon.png
	charge.svg
	DESTINATION ${powermanagerdatadir})
