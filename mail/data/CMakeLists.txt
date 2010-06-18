add_subdirectory(themes)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mail.conf
	cd_mail_nomail.svg
	cd_mail_newmail.svg
	cd_mail_icon.png
	preview.jpg
	DESTINATION ${maildatadir})
