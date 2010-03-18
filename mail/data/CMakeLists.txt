add_subdirectory(themes)


########### install files ###############

install(FILES  mail.conf 	cd_mail_nomail.svg  	cd_mail_newmail.svg  	cd_mail_icon.png  	preview.jpg DESTINATION ${maildatadir})



#original Makefile.am contents follow:

#
#SUBDIRS = . themes
#
#moduledir = $(maildatadir)
#
#module_DATA = \
#	mail.conf\
#	cd_mail_nomail.svg \
#	cd_mail_newmail.svg \
#	cd_mail_icon.png \
#	preview.jpg
#
#EXTRA_DIST = $(module_DATA)
