
########### install files ###############

install(FILES  has_mail.svg 	no_mail.svg 	new_mail_sound.wav DESTINATION ${maildatadir}/themes/Default)



#original Makefile.am contents follow:

#themedir = $(maildatadir)/themes/Default
#
#theme_DATA =	    \
#	has_mail.svg\
#	no_mail.svg\
#	new_mail_sound.wav
#
#EXTRA_DIST = $(theme_DATA)
