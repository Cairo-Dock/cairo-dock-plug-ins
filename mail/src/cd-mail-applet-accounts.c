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

#include <string.h>
#include <cairo-dock.h>
#include <libetpan/libetpan.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-etpan.h"
#include "cd-mail-applet-accounts.h"

#define _add_icon(pMailAccount)\
	pIcon = cairo_dock_create_dummy_launcher (g_strdup (pMailAccount->name),\
		g_strdup (myConfig.cNoMailUserImage),\
		g_strdup (pMailAccount->cMailApp),\
		g_strdup ("..."),\
		i);\
	pIcon->cParentDockName = g_strdup (myIcon->cName);\
	pIconList = g_list_append (pIconList, pIcon);\
	pMailAccount->icon = pIcon;

// Translation Hack:
const char *strings_to_translate[20] = {N_("Server address:"), N_("myHost"), N_("Username:"), N_("Password:"), N_("The password will be crypted."), N_("Port:"), N_("Enter 0 to use the default port. Default ports are 110 for POP3 or APOP and 995 for POP3S."), N_("Enter 0 to use the default port. Default ports are 143 for IMAP4 and 993 for IMAP4 over SSL."), N_("Use a secure connection (SSL)"), N_("Refresh time:"), N_("In minutes."), N_("Specific mail application"), N_("Leave empty to use the default mail application."), N_("Directory on server:"), N_("Path of mbox file:"), N_("Path to Mail directory:"), N_("Address of feed:"), N_("Remove this account"), N_("Don't forget to enable IMAP (or POP) service from settings of your mail account.")};

// Default parameters (to not copy these parameters each time)
void _add_default_create_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "username", pMailAccountName); // most of the time, the name of the account is the username
	g_key_file_set_comment (pKeyFile, pMailAccountName, "username", "s0 Username:\n{Don't forget to enable IMAP (or POP) service from settings of your mail account.}", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "password", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "password", "p0 Password:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] Refresh time:\n{In minutes.}", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "mail application", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "mail application", "s0 Specific mail application\n{Leave empty to use the default mail application.}", NULL);
}

void _retrieve_user_password (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{
	gboolean bFlushConfFileNeeded = FALSE;

	if (g_key_file_has_key (pKeyFile, mailbox_name, "username", NULL))
	{
		mailaccount->user = CD_CONFIG_GET_STRING (mailbox_name, "username");
	}
	if (g_key_file_has_key (pKeyFile, mailbox_name, "password", NULL))
	{
		gchar *encryptedPassword = CD_CONFIG_GET_STRING (mailbox_name, "password");
		cairo_dock_decrypt_string( encryptedPassword,  &(mailaccount->password) );

		g_free (encryptedPassword);
	}
}

void cd_mail_create_pop3_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "pop3");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);  // on lui met un widget pour ne pas que la cle se fasse bazarder lors d'une mise a jour du fichier de conf.
	
	g_key_file_set_string (pKeyFile, pMailAccountName, "host", "pop3.myHost.org");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "host", "s0 Server address:", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );

	g_key_file_set_integer (pKeyFile, pMailAccountName, "port", 0);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "port", "i0 Port:\n{Enter 0 to use the default port. Default ports are 110 for POP3 or APOP and 995 for POP3S.}", NULL);

	g_key_file_set_boolean (pKeyFile, pMailAccountName, "use secure connection", FALSE);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "use secure connection", "b0 Use a secure connection (SSL)", NULL);
}

void cd_mail_retrieve_pop3_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{
	if( !mailaccount || !pKeyFile || !mailbox_name ) return;

	gboolean bFlushConfFileNeeded = FALSE;

	mailaccount->driver = POP3_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->auth_type = POP3_AUTH_TYPE_TRY_APOP;
	
	if (g_key_file_has_key (pKeyFile, mailbox_name, "host", NULL))
	{
		mailaccount->server = CD_CONFIG_GET_STRING (mailbox_name, "host");
	}

	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );

	mailaccount->connection_type = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT (
		mailbox_name, "use secure connection", FALSE)
		? CONNECTION_TYPE_TLS
		: CONNECTION_TYPE_PLAIN;
	mailaccount->port = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "port", 0);
}

void cd_mail_create_imap_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "imap");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "host", "imap.myHost.org");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "host", "s0 Server address:", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );

	g_key_file_set_integer (pKeyFile, pMailAccountName, "port", 0);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "port", "i0 Port:\n{Enter 0 to use the default port. Default ports are 143 for IMAP4 and 993 for IMAP4 over SSL.}", NULL);

	g_key_file_set_boolean (pKeyFile, pMailAccountName, "use secure connection", FALSE);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "use secure connection", "b0 Use a secure connection (SSL)", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "server_directory", "Inbox");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "server_directory", "s0 Directory on server:", NULL);
}

void cd_mail_retrieve_imap_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	gboolean bFlushConfFileNeeded = FALSE;

	mailaccount->driver = IMAP_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
	
	if (g_key_file_has_key (pKeyFile, mailbox_name, "host", NULL))
	{
		mailaccount->server = CD_CONFIG_GET_STRING (mailbox_name, "host");
	}

	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );

	mailaccount->port = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "port", 0);

	mailaccount->connection_type = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT (
		mailbox_name, "use secure connection", FALSE)
		? CONNECTION_TYPE_TLS
		: CONNECTION_TYPE_PLAIN;

	/* CONNECTION_TYPE_TLS ? CONNECTION_TYPE_STARTTLS ? */

	if (g_key_file_has_key (pKeyFile, mailbox_name, "server_directory", NULL))
		mailaccount->path = CD_CONFIG_GET_STRING (mailbox_name, "server_directory");
	if (mailaccount->path == NULL)
		mailaccount->path = g_strdup("/");
}

void cd_mail_create_mbox_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "mbox");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "filename", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "filename", "s0 Path of mbox file:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] Refresh time:\n{In minutes.}", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "mail application", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "mail application", "s0 Specific mail application\n{Leave empty to use the default mail application.}", NULL);
}

void cd_mail_retrieve_mbox_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	gboolean bFlushConfFileNeeded = FALSE;

	mailaccount->driver = MBOX_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;

	if (g_key_file_has_key (pKeyFile, mailbox_name, "filename", NULL))
		mailaccount->path = CD_CONFIG_GET_STRING_WITH_DEFAULT (mailbox_name, "filename", "/");
	if (mailaccount->path == NULL)
		mailaccount->path = g_strdup("/");

	//{"filename", "ctime", "size", "interval", NULL, NULL}
}

void cd_mail_create_mh_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "mh");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] Refresh time:\n{In minutes.}", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "mail application", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "mail application", "s0 Specific mail application\n{Leave empty to use the default mail application.}", NULL);
}

void cd_mail_retrieve_mh_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = MH_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;
	mailaccount->path = g_strdup("/");
}

void cd_mail_create_maildir_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "maildir");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "path", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "path", "s0 Path to Mail directory:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] Refresh time:\n{In minutes.}", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "mail application", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "mail application", "s0 Specific mail application\n{Leave empty to use the default mail application.}", NULL);
}

void cd_mail_retrieve_maildir_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	gboolean bFlushConfFileNeeded = FALSE;

	mailaccount->driver = MAILDIR_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;

	if (g_key_file_has_key (pKeyFile, mailbox_name, "path", NULL))
		mailaccount->path = CD_CONFIG_GET_STRING (mailbox_name, "path");
	if (mailaccount->path == NULL)
		mailaccount->path = g_strdup("/");
	//{"path", "mtime", "interval", NULL, NULL, NULL, NULL}
}


void cd_mail_create_feed_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "feed");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "path", "http://identi.ca/api/statuses/user_timeline/cairodock.rss");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "path", "s0 Address of feed:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] Refresh time:\n{In minutes.}", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "mail application", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "mail application", "s0 Specific mail application\n{Leave empty to use the default mail application.}", NULL);
}

void cd_mail_retrieve_feed_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{
	if( !mailaccount || !pKeyFile || !mailbox_name ) return;


	gboolean bFlushConfFileNeeded = FALSE;

	mailaccount->driver = FEED_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->port = 443;
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;
	
	if (g_key_file_has_key (pKeyFile, mailbox_name, "path", NULL))
	{
		mailaccount->path = CD_CONFIG_GET_STRING (mailbox_name, "path");
	}
}

	//  Some servers are pre-configured

void cd_mail_create_gmail_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "gmail");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_gmail_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// FEED ou IMAP
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

#if (1 || __WORDSIZE == 64 )  // RSS authentification seems to have changed, and doesn't work anymore here :-/ so use IMAP by default
/* in 64bit libetpan crashes with RSS, so use the IMAP feature of GMail
 * instead of RSS. */
	mailaccount->driver = IMAP_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("imap.gmail.com");
	mailaccount->port = 993;
	mailaccount->connection_type = CONNECTION_TYPE_TLS;
	mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
	mailaccount->path = g_strdup("Inbox");
	
	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
#else
	mailaccount->driver = FEED_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->port = 443;
	mailaccount->connection_type = CONNECTION_TYPE_STARTTLS;
	mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;
	
	if (g_key_file_has_key (pKeyFile, mailbox_name, "username", NULL))
	{
		mailaccount->user = CD_CONFIG_GET_STRING (mailbox_name, "username");
	}
	if (g_key_file_has_key (pKeyFile, mailbox_name, "password", NULL))
	{
		gchar *encryptedPassword = CD_CONFIG_GET_STRING (mailbox_name, "password");
		cairo_dock_decrypt_string( encryptedPassword,  &(mailaccount->password) );

		if( encryptedPassword ) g_free(encryptedPassword);
	}

	gchar *user_without_column = NULL;
	gchar *password_without_column = NULL;

	if( mailaccount->user )
	{
		gchar **splitString = g_strsplit(mailaccount->user, ":", 0);
		user_without_column = g_strjoinv("%3A", splitString);
		g_strfreev( splitString );
	}
	if( mailaccount->password )
	{
		gchar **splitString = g_strsplit(mailaccount->password, ":", 0);
		password_without_column = g_strjoinv("%3A", splitString);
		g_strfreev( splitString );
	}

	if( user_without_column && password_without_column )
	{
		mailaccount->path = g_strconcat("https://", user_without_column, ":", password_without_column, "@mail.google.com/mail/feed/atom", NULL);
	}
	else
	{
		mailaccount->path = g_strdup( "https://mail.google.com/mail/feed/atom" );
	}

	g_free( user_without_column );
	g_free( password_without_column );
#endif
}

void cd_mail_create_yahoo_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "yahoo");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_yahoo_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// IMAP
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = IMAP_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("imap.mail.yahoo.com");
	mailaccount->port = 993;
	mailaccount->connection_type = CONNECTION_TYPE_TLS;
	mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
	mailaccount->path = g_strdup("Inbox");
	
	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
}

void cd_mail_create_hotmail_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "hotmail");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_hotmail_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// POP3
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = POP3_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("pop3.live.com");
	mailaccount->port = 995;
	mailaccount->connection_type = CONNECTION_TYPE_TLS;
	mailaccount->auth_type = POP3_AUTH_TYPE_TRY_APOP;

	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
}

void cd_mail_create_free_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "free");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_free_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// IMAP
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = IMAP_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("imap.free.fr");
	mailaccount->port = 143;
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
	mailaccount->path = g_strdup("Inbox");
	
	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
}

void cd_mail_create_neuf_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "neuf");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_neuf_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// IMAP
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = IMAP_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("imap.neuf.fr");
	mailaccount->port = 143;
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
	mailaccount->path = g_strdup("Inbox");
	
	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
}

void cd_mail_create_sfr_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "sfr");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_sfr_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// IMAP
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = IMAP_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("imap.sfr.fr");
	mailaccount->port = 143;
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
	mailaccount->path = g_strdup("Inbox");
	
	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
}

void cd_mail_create_orange_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "orange");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_orange_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// IMAP
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = IMAP_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("imap.orange.fr");
	mailaccount->port = 143;
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
	mailaccount->path = g_strdup("Inbox");
	
	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
}

void cd_mail_create_uclouvain_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "uclouvain");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_uclouvain_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// IMAP
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = IMAP_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("mail.sipr.ucl.ac.be");
	mailaccount->port = 993;
	mailaccount->connection_type = CONNECTION_TYPE_TLS;
	mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
	mailaccount->path = g_strdup("Inbox");
	
	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
}

void cd_mail_create_skynet_params( GKeyFile *pKeyFile, const gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "skynet");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "type", ">0 ", NULL);

	_add_default_create_params( pKeyFile, pMailAccountName );
}

void cd_mail_retrieve_skynet_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name)
{	// POP3
	if( !mailaccount || !pKeyFile || !mailbox_name )
		return;

	mailaccount->driver = POP3_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->server = g_strdup("pop.skynet.be");
	mailaccount->port = 110;
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->auth_type = POP3_AUTH_TYPE_TRY_APOP;

	_retrieve_user_password( mailaccount, pKeyFile, mailbox_name );
}

/*{
	{POP3_STORAGE, "pop3", {"host", "username", "password", "auth_type", "timeout mn", "port", NULL}},
	{IMAP_STORAGE, "imap", {"host", "username", "password", "auth_type", "timeout mn", "port", "server_directory"}},
	{NNTP_STORAGE, "nntp", {NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
	{MBOX_STORAGE, "mbox", {"filename", "ctime", "size", "interval", NULL, NULL}},
	{MH_STORAGE, "mh", {"timeout mn", NULL, NULL, NULL, NULL, NULL, NULL}},
	{MAILDIR_STORAGE, "maildir", {"path", "mtime", "interval", NULL, NULL, NULL, NULL}},
	{FEED_STORAGE, "feed", {"username", "password", "timeout mn", NULL, NULL, NULL, NULL}},
	{FEED_STORAGE, "gmail", {"username", "password", "timeout mn", NULL, NULL, NULL, NULL}},
};*/


void cd_mail_init_accounts(GldiModuleInstance *myApplet)
{
	if (myData.pMailAccounts == NULL)
		return ;
	cd_debug ("%s (%d comptes)", __func__, myData.pMailAccounts->len);
	
	//\_______________________ On initialise chaque compte.
	CDMailAccount *pMailAccount;
	GList *pIconList = NULL;
	Icon *pIcon;
	int iNbIcons = 0;
	int r;
	gboolean bIsGettingMail = FALSE;
	guint i;
	for (i = 0; i < myData.pMailAccounts->len; i ++)
	{
		pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
		if( !pMailAccount ) continue;
		
		// init this account
		switch (pMailAccount->driver) {
			case POP3_STORAGE:
				r = pop3_mailstorage_init(pMailAccount->storage, pMailAccount->server, pMailAccount->port,
					NULL, pMailAccount->connection_type,
					pMailAccount->auth_type, pMailAccount->user, pMailAccount->password,
					myData.cWorkingDirPath!=NULL?TRUE:FALSE /*cached*/, myData.cWorkingDirPath /*cache_directory*/, myData.cWorkingDirPath /*flags_directory*/);
			break;

			case IMAP_STORAGE:
				r = imap_mailstorage_init(pMailAccount->storage, pMailAccount->server, pMailAccount->port,
					NULL, pMailAccount->connection_type,
					IMAP_AUTH_TYPE_PLAIN, pMailAccount->user, pMailAccount->password,
					myData.cWorkingDirPath!=NULL?TRUE:FALSE /*cached*/, myData.cWorkingDirPath /*cache_directory*/);
			break;

			case NNTP_STORAGE:
				r = nntp_mailstorage_init(pMailAccount->storage, pMailAccount->server, pMailAccount->port,
					NULL, pMailAccount->connection_type,
					NNTP_AUTH_TYPE_PLAIN, pMailAccount->user, pMailAccount->password,
					myData.cWorkingDirPath!=NULL?TRUE:FALSE /*cached*/, myData.cWorkingDirPath /*cache_directory*/, myData.cWorkingDirPath /*flags_directory*/);
			break;

			case MBOX_STORAGE:
				r = mbox_mailstorage_init(pMailAccount->storage, pMailAccount->path,
					myData.cWorkingDirPath!=NULL?TRUE:FALSE /*cached*/, myData.cWorkingDirPath /*cache_directory*/, myData.cWorkingDirPath /*flags_directory*/);
			break;

			case MH_STORAGE:
				r = mh_mailstorage_init(pMailAccount->storage, pMailAccount->path,
					myData.cWorkingDirPath!=NULL?TRUE:FALSE /*cached*/, myData.cWorkingDirPath /*cache_directory*/, myData.cWorkingDirPath /*flags_directory*/);
			break;
			
			case MAILDIR_STORAGE:
				r = maildir_mailstorage_init(pMailAccount->storage, pMailAccount->path,
					myData.cWorkingDirPath!=NULL?TRUE:FALSE /*cached*/, myData.cWorkingDirPath /*cache_directory*/, myData.cWorkingDirPath /*flags_directory*/);
			break;
			
			case FEED_STORAGE:
				r = feed_mailstorage_init(pMailAccount->storage, pMailAccount->path,
					myData.cWorkingDirPath!=NULL?TRUE:FALSE /*cached*/, myData.cWorkingDirPath /*cache_directory*/, myData.cWorkingDirPath /*flags_directory*/);
			break;
			default :
				r = -1;
		}
		
		// add an icon for this account.
		if (myData.pMailAccounts->len == 1)  // 1 seul compte
		{
			pIcon = myIcon;
		}
		else
		{
			_add_icon (pMailAccount);
		}
		iNbIcons ++;
		
		//  if all is OK, then set a timeout for this mail account
		if (r == MAIL_NO_ERROR)
		{
			gldi_icon_set_quick_info (pIcon, "..."); // on the current icon
			pMailAccount->pAccountMailTimer = cairo_dock_new_task (pMailAccount->timeout * 60,
				(CairoDockGetDataAsyncFunc) cd_mail_get_folder_data,
				(CairoDockUpdateSyncFunc) cd_mail_update_account_status,
				pMailAccount);
			cairo_dock_launch_task (pMailAccount->pAccountMailTimer);
			bIsGettingMail = TRUE;
		}
		else
		{
			cd_warning ("mail : the mail account %s couldn't be initialized !", pMailAccount->name);
			gldi_icon_set_quick_info (pIcon, "N/A");
		}
	}
	
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On charge la nouvelle liste.
	if (iNbIcons > 1)
	{
		gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
		CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, "Caroussel", pConfig);
	}
	
	//\_______________ On dessine l'icone principale initialement.
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cNoMailUserImage);
	if (bIsGettingMail && myData.iPrevNbUnreadMails == G_MAXUINT) // only at the init...
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("...");
}


void cd_mail_free_account (CDMailAccount *pMailAccount)
{
	if (pMailAccount == NULL)
		return ;
	
	cairo_dock_free_task( pMailAccount->pAccountMailTimer );
	
	g_free( pMailAccount->name );
	g_free( pMailAccount->server );
	g_free( pMailAccount->user );
	g_free( pMailAccount->password );
	g_free( pMailAccount->path );
	g_free( pMailAccount->cMailApp );

	if( pMailAccount->folder )
		mailfolder_free(pMailAccount->folder);
	if( pMailAccount->storage )
		mailstorage_free(pMailAccount->storage);
	
	g_list_foreach (pMailAccount->pUnseenMessageList, (GFunc) g_free, NULL);
	g_list_free (pMailAccount->pUnseenMessageList);
	
	g_list_foreach (pMailAccount->pUnseenMessageUid, (GFunc) g_free, NULL);
	g_list_free (pMailAccount->pUnseenMessageUid);
	
	g_free( pMailAccount );
}

void cd_mail_free_all_accounts (GldiModuleInstance *myApplet)
{
	if (myData.pMailAccounts == NULL)
		return ;
	CDMailAccount *pMailAccount;
	guint i;
	for (i = 0; i < myData.pMailAccounts->len; i ++)
	{
		pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
		cd_mail_free_account (pMailAccount);
	}
	g_ptr_array_free (myData.pMailAccounts, TRUE);
	myData.pMailAccounts = NULL;
}
