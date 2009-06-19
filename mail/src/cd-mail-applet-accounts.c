/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Christophe Chapuis (for any bug report, please mail me to tofe@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>
#include <libetpan/libetpan.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-etpan.h"
#include "cd-mail-applet-accounts.h"

void cd_mail_create_pop3_params( GKeyFile *pKeyFile, gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "pop3");

	g_key_file_set_string (pKeyFile, pMailAccountName, "host", "pop3.myhost.org");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "host", "s0 server address:", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "username", "myLogin");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "username", "s0 username:", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "password", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "password", "p0 password:\n{The password will be crypted.}", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "port", 0);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "port", "i0 port:\n{Enter 0 to use the default port. Default ports are 110 for POP3 or APOP and 995 for POP3S.}", NULL);

	g_key_file_set_boolean (pKeyFile, pMailAccountName, "use secure connection", FALSE);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "use secure connection", "b0 use secure connection (SSL)", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] timeout:\n{In minutes.}", NULL);
}

void cd_mail_retrieve_pop3_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name)
{
  if( !mailaccount || !pKeyFile || !mailbox_name ) return;

  gboolean bFlushConfFileNeeded = FALSE;

  mailaccount->driver = POP3_STORAGE;
  mailaccount->storage = mailstorage_new(NULL);
  mailaccount->folder = NULL;
  mailaccount->server = NULL;
  mailaccount->port = 0;
  mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
  mailaccount->user = NULL;
  mailaccount->password = NULL;
  mailaccount->auth_type = POP3_AUTH_TYPE_TRY_APOP;
  mailaccount->path = NULL;
  mailaccount->timeout = 0;
  
  if (g_key_file_has_key (pKeyFile, mailbox_name, "host", NULL))
  {
    mailaccount->server = CD_CONFIG_GET_STRING (mailbox_name, "host");
  }
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
  mailaccount->connection_type = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT (mailbox_name, "use secure connection", FALSE)?CONNECTION_TYPE_TLS:CONNECTION_TYPE_PLAIN;
  mailaccount->port = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "port", 0);

  mailaccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "timeout mn", 10);
}

void cd_mail_create_imap_params( GKeyFile *pKeyFile, gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "imap");

	g_key_file_set_string (pKeyFile, pMailAccountName, "host", "imap.myhost.org");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "host", "s0 server address:", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "username", "myLogin");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "username", "s0 username:", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "password", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "password", "p0 password:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "port", 0);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "port", "i0 port:\n{Enter 0 to use the default port. Default ports are 143 for IMAP4 and 993 for IMAP4 over SSL.}", NULL);

	g_key_file_set_boolean (pKeyFile, pMailAccountName, "use secure connection", FALSE);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "use secure connection", "b0 use secure connection (SSL)", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "server_directory", "Inbox");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "server_directory", "s0 directory on server:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] timeout:\n{In minutes.}", NULL);
}

void cd_mail_retrieve_imap_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name)
{
  if( !mailaccount || !pKeyFile || !mailbox_name ) return;

  gboolean bFlushConfFileNeeded = FALSE;

  mailaccount->driver = IMAP_STORAGE;
  mailaccount->storage = mailstorage_new(NULL);
  mailaccount->folder = NULL;
  mailaccount->server = NULL;
  mailaccount->port = 0;
  mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
  mailaccount->user = NULL;
  mailaccount->password = NULL;
  mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
  mailaccount->path = g_strdup("/");
  mailaccount->timeout = 0;
  
  if (g_key_file_has_key (pKeyFile, mailbox_name, "host", NULL))
  {
    mailaccount->server = CD_CONFIG_GET_STRING (mailbox_name, "host");
  }
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
  mailaccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "timeout mn", 10);
  mailaccount->port = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "port", 0);

  mailaccount->connection_type = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT (mailbox_name, "use secure connection", FALSE)?CONNECTION_TYPE_TLS:CONNECTION_TYPE_PLAIN;

  /* CONNECTION_TYPE_TLS ? CONNECTION_TYPE_STARTTLS ? */

  if (g_key_file_has_key (pKeyFile, mailbox_name, "server_directory", NULL))
  {
    mailaccount->path = CD_CONFIG_GET_STRING (mailbox_name, "server_directory");
  }
}

void cd_mail_create_mbox_params( GKeyFile *pKeyFile, gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "mbox");

	g_key_file_set_string (pKeyFile, pMailAccountName, "filename", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "filename", "s0 path of mbox file:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] timeout:\n{In minutes.}", NULL);
}

void cd_mail_retrieve_mbox_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name)
{
	if( !mailaccount || !pKeyFile || !mailbox_name ) return;

	gboolean bFlushConfFileNeeded = FALSE;

	mailaccount->driver = MBOX_STORAGE;
	mailaccount->storage = mailstorage_new(NULL);
	mailaccount->folder = NULL;
	mailaccount->server = NULL;
	mailaccount->port = 0;
	mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
	mailaccount->user = NULL;
	mailaccount->password = NULL;
	mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;
	mailaccount->timeout = 0;
	if (g_key_file_has_key (pKeyFile, mailbox_name, "filename", NULL))
		mailaccount->path = CD_CONFIG_GET_STRING_WITH_DEFAULT (mailbox_name, "filename", "/");
	if (mailaccount->path == NULL)
		mailaccount->path = g_strdup("/");
	mailaccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "timeout mn", 10);

	//{"filename", "ctime", "size", "interval", NULL, NULL}
}

void cd_mail_create_mh_params( GKeyFile *pKeyFile, gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "mh");

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] timeout:\n{In minutes.}", NULL);
}

void cd_mail_retrieve_mh_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name)
{
  if( !mailaccount || !pKeyFile || !mailbox_name ) return;

  gboolean bFlushConfFileNeeded = FALSE;

  mailaccount->driver = MH_STORAGE;
  mailaccount->storage = mailstorage_new(NULL);
  mailaccount->folder = NULL;
  mailaccount->server = NULL;
  mailaccount->port = 0;
  mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
  mailaccount->user = NULL;
  mailaccount->password = NULL;
  mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;
  mailaccount->path = g_strdup("/");
  mailaccount->timeout = 0;

  mailaccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "timeout mn", 10);
}

void cd_mail_create_maildir_params( GKeyFile *pKeyFile, gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "maildir");

	g_key_file_set_string (pKeyFile, pMailAccountName, "path", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "path", "s0 path to mail directory:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] timeout:\n{In minutes.}", NULL);
}

void cd_mail_retrieve_maildir_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name)
{
  if( !mailaccount || !pKeyFile || !mailbox_name ) return;

  gboolean bFlushConfFileNeeded = FALSE;

  mailaccount->driver = MAILDIR_STORAGE;
  mailaccount->storage = mailstorage_new(NULL);
  mailaccount->folder = NULL;
  mailaccount->server = NULL;
  mailaccount->port = 0;
  mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
  mailaccount->user = NULL;
  mailaccount->password = NULL;
  mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;
  mailaccount->path = g_strdup("/");
  mailaccount->timeout = 0;

  if (g_key_file_has_key (pKeyFile, mailbox_name, "path", NULL))
  {
    mailaccount->path = CD_CONFIG_GET_STRING (mailbox_name, "path");
  }
  mailaccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "timeout mn", 10);

  //{"path", "mtime", "interval", NULL, NULL, NULL, NULL}
}

void cd_mail_create_gmail_params( GKeyFile *pKeyFile, gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "gmail");

	g_key_file_set_string (pKeyFile, pMailAccountName, "username", "myLogin");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "username", "s0 username:", NULL);

	g_key_file_set_string (pKeyFile, pMailAccountName, "password", "");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "password", "p0 password:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] timeout:\n{In minutes.}", NULL);
}

void cd_mail_retrieve_gmail_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name)
{
  if( !mailaccount || !pKeyFile || !mailbox_name ) return;

  gboolean bFlushConfFileNeeded = FALSE;

#if ( __WORDSIZE == 64 )
/* in 64bit libetpan crashes with RSS, so use the IMAP feature of GMail
 * instead of RSS. */
  mailaccount->driver = IMAP_STORAGE;
  mailaccount->storage = mailstorage_new(NULL);
  mailaccount->folder = NULL;
  mailaccount->server = g_strdup("imap.gmail.com");
  mailaccount->port = 993;
  mailaccount->connection_type = CONNECTION_TYPE_TLS;
  mailaccount->user = NULL;
  mailaccount->password = NULL;
  mailaccount->auth_type = IMAP_AUTH_TYPE_PLAIN;
  mailaccount->path = g_strdup("Inbox");
  mailaccount->timeout = 0;
  
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
  mailaccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "timeout mn", 10);
#else
  mailaccount->driver = FEED_STORAGE;
  mailaccount->storage = mailstorage_new(NULL);
  mailaccount->folder = NULL;
  mailaccount->server = NULL;
  mailaccount->port = 443;
  mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
  mailaccount->user = NULL;
  mailaccount->password = NULL;
  mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;
  mailaccount->path = NULL;
  mailaccount->timeout = 0;
  
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
  mailaccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "timeout mn", 10);

  g_free( user_without_column );
  g_free( password_without_column );
#endif
}

void cd_mail_create_feed_params( GKeyFile *pKeyFile, gchar *pMailAccountName )
{
	g_key_file_set_string (pKeyFile, pMailAccountName, "type", "feed");

	g_key_file_set_string (pKeyFile, pMailAccountName, "path", "http://www.cairo-dock.org/rss/cd_svn.xml");
	g_key_file_set_comment (pKeyFile, pMailAccountName, "path", "s0 address of feed:", NULL);

	g_key_file_set_integer (pKeyFile, pMailAccountName, "timeout mn", 10);
	g_key_file_set_comment (pKeyFile, pMailAccountName, "timeout mn", "I0[1;30] timeout:\n{In minutes.}", NULL);
}

void cd_mail_retrieve_feed_params (CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name)
{
  if( !mailaccount || !pKeyFile || !mailbox_name ) return;

  extern int mailstream_debug;
  mailstream_debug = 1;

  gboolean bFlushConfFileNeeded = FALSE;

  mailaccount->driver = FEED_STORAGE;
  mailaccount->storage = mailstorage_new(NULL);
  mailaccount->folder = NULL;
  mailaccount->server = NULL;
  mailaccount->port = 443;
  mailaccount->connection_type = CONNECTION_TYPE_PLAIN;
  mailaccount->user = NULL;
  mailaccount->password = NULL;
  mailaccount->auth_type = POP3_AUTH_TYPE_PLAIN;
  mailaccount->path = NULL;
  mailaccount->timeout = 0;
  
  if (g_key_file_has_key (pKeyFile, mailbox_name, "path", NULL))
  {
    mailaccount->path = CD_CONFIG_GET_STRING (mailbox_name, "path");
  }
  mailaccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (mailbox_name, "timeout mn", 10);
}

/*
{
  {POP3_STORAGE, "pop3", {"host", "username", "password", "auth_type", "timeout mn", "port", NULL}},
  {IMAP_STORAGE, "imap", {"host", "username", "password", "auth_type", "timeout mn", "port", "server_directory"}},
  {NNTP_STORAGE, "nntp", {NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
  {MBOX_STORAGE, "mbox", {"filename", "ctime", "size", "interval", NULL, NULL}},
  {MH_STORAGE, "mh", {"timeout mn", NULL, NULL, NULL, NULL, NULL, NULL}},
  {MAILDIR_STORAGE, "maildir", {"path", "mtime", "interval", NULL, NULL, NULL, NULL}},
  {FEED_STORAGE, "feed", {"username", "password", "timeout mn", NULL, NULL, NULL, NULL}},
  {FEED_STORAGE, "gmail", {"username", "password", "timeout mn", NULL, NULL, NULL, NULL}},
};
*/


void cd_mail_init_accounts(CairoDockModuleInstance *myApplet)
{	
	if (myData.pMailAccounts == NULL)
		return ;
	g_print ("%s (%d comptes)\n", __func__, myData.pMailAccounts->len);
	
	CDMailAccount *pMailAccount;
  	guint i;
	int r;
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
					FALSE /*cached*/, NULL /*cache_directory*/, NULL /*flags_directory*/);
			break;

			case IMAP_STORAGE:
				r = imap_mailstorage_init(pMailAccount->storage, pMailAccount->server, pMailAccount->port,
					NULL, pMailAccount->connection_type,
					IMAP_AUTH_TYPE_PLAIN, pMailAccount->user, pMailAccount->password,
					FALSE /*cached*/, NULL /*cache_directory*/);
			break;

			case NNTP_STORAGE:
				r = nntp_mailstorage_init(pMailAccount->storage, pMailAccount->server, pMailAccount->port,
					NULL, pMailAccount->connection_type,
					NNTP_AUTH_TYPE_PLAIN, pMailAccount->user, pMailAccount->password,
					FALSE /*cached*/, NULL /*cache_directory*/, NULL /*flags_directory*/);
			break;

			case MBOX_STORAGE:
				r = mbox_mailstorage_init(pMailAccount->storage, pMailAccount->path,
					FALSE /*cached*/, NULL /*cache_directory*/, NULL /*flags_directory*/);
			break;

			case MH_STORAGE:
				r = mh_mailstorage_init(pMailAccount->storage, pMailAccount->path,
					FALSE /*cached*/, NULL /*cache_directory*/, NULL /*flags_directory*/);
			break;
		        
			case MAILDIR_STORAGE:
				r = maildir_mailstorage_init(pMailAccount->storage, pMailAccount->path,
					FALSE /*cached*/, NULL /*cache_directory*/, NULL /*flags_directory*/);
			break;
		        
			case FEED_STORAGE:
				r = feed_mailstorage_init(pMailAccount->storage, pMailAccount->path,
					FALSE /*cached*/, NULL /*cache_directory*/, NULL /*flags_directory*/);
			break;
			default :
				r = -1;
		}

		//  if all is OK, then set a timeout for this mail account
		if (r == MAIL_NO_ERROR)
		{
			pMailAccount->pAccountMailTimer = cairo_dock_new_task (pMailAccount->timeout * 60,
				cd_mail_acquire_folder_data,
				cd_mail_read_folder_data,
				cd_mail_update_account_status,
				pMailAccount);
			cairo_dock_launch_task (pMailAccount->pAccountMailTimer);
		}
		else
		{
			cd_warning ("mail : the mail account %s couldn't be initialized !", pMailAccount->name);
		}
	}
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

	if( pMailAccount->folder )
		mailfolder_free(pMailAccount->folder);
	if( pMailAccount->storage )
		mailstorage_free(pMailAccount->storage);
	
	g_list_foreach (pMailAccount->pUnseenMessageList, (GFunc) g_free, NULL);
	g_list_free (pMailAccount->pUnseenMessageList);
	
	g_free( pMailAccount );
}

void cd_mail_free_all_accounts (CairoDockModuleInstance *myApplet)
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
