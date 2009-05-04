
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <time.h>

#include <libetpan/libetpan.h>
#include <cairo-dock.h>

#define MAIL_DEFAULT_NAME "_Mail_"

struct _AppletConfig {
    gchar *cNoMailUserImage;
    gchar *cHasMailUserImage;
    gchar *cNewMailUserSound;
    gchar *cMailApplication;
    gchar *cThemePath;
} ;

struct _AppletData {
    GPtrArray *pMailAccounts;
    guint iNbUnreadMails;
    gboolean bNewMailFound;
    time_t timeEndOfSound; 
    CairoDockMeasure *pGlobalMailTimer;

    GLuint iNoMailTexture;
    GLuint iHasMailTexture;
    GLuint iCubeCallList;
    GLuint iCapsuleCallList;

    guint current_rotX;
    guint current_rotY;
} ;

typedef struct {
        gboolean dirtyfied;
        CairoDockModuleInstance *pAppletInstance;
		
        gchar *name;
        struct mailstorage *storage;
        struct mailfolder *folder;
        guint iNbUnseenMails;
        int driver;
        gchar *server;
        int port;
        int connection_type;
        gchar *user;
        gchar *password;
        int auth_type;
        gchar *path;
        guint timeout;
        CairoDockMeasure *pAccountMailTimer;
      } CDMailAccount;


void cd_mail_read_folder_data(CDMailAccount *pMailAccount);

#endif
