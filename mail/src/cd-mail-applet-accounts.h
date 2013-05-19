/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __APPLET_ACCOUNTS__
#define  __APPLET_ACCOUNTS__

#include <cairo-dock.h>
#include "cd-mail-applet-struct.h"


void cd_mail_create_pop3_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_imap_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_mbox_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_mh_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_maildir_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_gmail_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_feed_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_yahoo_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_hotmail_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_free_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_neuf_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_sfr_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_orange_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_uclouvain_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );
void cd_mail_create_skynet_params( GKeyFile *pKeyFile, const gchar *pMailAccountName );

void cd_mail_retrieve_pop3_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_imap_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_mbox_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_mh_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_maildir_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_gmail_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_feed_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_yahoo_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_hotmail_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_free_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_neuf_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_sfr_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_orange_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_uclouvain_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
void cd_mail_retrieve_skynet_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);

void cd_mail_init_accounts(GldiModuleInstance *myApplet);
void cd_mail_free_account (CDMailAccount *pMailAccount);
void cd_mail_free_all_accounts (GldiModuleInstance *myApplet);


#endif
