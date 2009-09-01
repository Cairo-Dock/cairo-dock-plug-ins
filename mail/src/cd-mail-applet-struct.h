
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
	gboolean bStealTaskBarIcon;
	gboolean bShowMessageContent;
	gboolean bCheckOnStartup;
	guint iNbMaxShown;
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
    CairoDockModuleInstance *pAppletInstance;
    gchar *name;
    struct mailstorage *storage;
    struct mailfolder *folder;
    guint iNbUnseenMails, iPrevNbUnseenMails;
    int driver;
    gchar *server;
    int port;
    int connection_type;
    gchar *user;
    guchar *password;
    int auth_type;
    gchar *path;
    guint timeout;
    CairoDockTask *pAccountMailTimer;
    Icon *icon;
    gboolean bInitialized;
    GList *pUnseenMessageList;  // liste de gchar*
    GList *pUnseenMessageUid;  // liste de gchar*

    gboolean bError;
} CDMailAccount;


#endif
