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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <time.h>

#include <libetpan/libetpan.h>
#include <cairo-dock.h>

#define MAIL_DEFAULT_NAME "_Mail_"

typedef enum {
  POP3_STORAGE = 1,
  IMAP_STORAGE,
  NNTP_STORAGE,
  MBOX_STORAGE,
  MH_STORAGE,
  MAILDIR_STORAGE,
  FEED_STORAGE
} CDMailAccountType;


struct _AppletConfig {
	gchar *cNoMailUserImage;
	gchar *cHasMailUserImage;
	gchar *cNewMailUserSound;
	gchar *cThemePath;
	gchar *cRenderer;
	gchar *cMailApplication;
	gchar *cMailClass;
	gchar *cAnimation;
	gint iAnimationDuration;
	gboolean bPlaySound;
	gboolean bStealTaskBarIcon;
	gboolean bShowMessageContent;
	gboolean bCheckOnStartup;
	guint iNbMaxShown;
	gint iDialogDuration;
	gboolean bAlwaysShowMailCount;
} ;

struct _AppletData {
	GPtrArray *pMailAccounts;
	guint iNbUnreadMails, iPrevNbUnreadMails;
	gchar *cWorkingDirPath;
	time_t timeEndOfSound;
	
	GLuint iNoMailTexture;
	GLuint iHasMailTexture;
	GLuint iCubeCallList;

	gdouble current_rotX;
	gdouble current_rotY;

	CairoDialog *pMessagesDialog;
	GtkTextBuffer *pTextBuffer;
	GtkWidget *pPrevButton;
	GtkWidget *pNextButton;
	gint iCurrentlyShownMail;
} ;


typedef struct {
    GldiModuleInstance *pAppletInstance;
    gchar *name;
    struct mailstorage *storage;
    struct mailfolder *folder;
    guint iNbUnseenMails, iPrevNbUnseenMails;
    int driver;
    gchar *server;
    int port;
    int connection_type;
    gchar *user;
    gchar *password;
    int auth_type;
    gchar *path;
    guint timeout;
    CairoDockTask *pAccountMailTimer;
    Icon *icon;
    gboolean bInitialized;
    GList *pUnseenMessageList;  // liste de gchar*
    GList *pUnseenMessageUid;  // liste de gchar*

    gchar *cMailApp;

    gboolean bError;
} CDMailAccount;


#endif
