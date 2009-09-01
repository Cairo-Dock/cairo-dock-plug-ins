
#ifndef __APPLET_ACCOUNTS__
#define  __APPLET_ACCOUNTS__

#include <cairo-dock.h>
#include "cd-mail-applet-struct.h"


void cd_mail_create_pop3_params( GKeyFile *pKeyFile, gchar *pMailAccountName );
void cd_mail_create_imap_params( GKeyFile *pKeyFile, gchar *pMailAccountName );
void cd_mail_create_mbox_params( GKeyFile *pKeyFile, gchar *pMailAccountName );
void cd_mail_create_mh_params( GKeyFile *pKeyFile, gchar *pMailAccountName );
void cd_mail_create_maildir_params( GKeyFile *pKeyFile, gchar *pMailAccountName );
void cd_mail_create_gmail_params( GKeyFile *pKeyFile, gchar *pMailAccountName );
void cd_mail_create_feed_params( GKeyFile *pKeyFile, gchar *pMailAccountName );

void cd_mail_retrieve_pop3_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name);
void cd_mail_retrieve_imap_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name);
void cd_mail_retrieve_mbox_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name);
void cd_mail_retrieve_mh_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name);
void cd_mail_retrieve_maildir_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name);
void cd_mail_retrieve_gmail_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name);
void cd_mail_retrieve_feed_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name);

void cd_mail_init_accounts(CairoDockModuleInstance *myApplet);
void cd_mail_free_account (CDMailAccount *pMailAccount);
void cd_mail_free_all_accounts (CairoDockModuleInstance *myApplet);


#endif
