
########### install files ###############

install (FILES calc.sh
	updatedb.sh
	DESTINATION ${scooby_dodatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/Scooby-Do.conf
	preview.jpg
	icon.png
	arrows.svg
	active-button.svg
	inactive-button.svg
	internet.png
	google.png
	wikipedia.png
	yahoo.png
	amazon.png
	mediadico.png
	files.png
	firefox.png
	recent.png
	DESTINATION ${scooby_dodatadir})
