/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#ifndef __FILE_MANAGER_STRUCT__
#define  __FILE_MANAGER_STRUCT__

#include <glib.h>


typedef enum {
	FILE_MANAGER_UNKNOWN=0,
	FILE_MANAGER_GNOME,
	FILE_MANAGER_KDE,
	FILE_MANAGER_XDG
	} FileManagerDesktopEnv;

typedef enum {
	FILE_MANAGER_ICON_MODIFIED=0,
	FILE_MANAGER_ICON_DELETED,
	FILE_MANAGER_ICON_CREATED,
	FILE_MANAGER_NB_EVENT_TYPE
	} FileManagerEventType;

typedef void (*FileManagerOnEventFunc) (FileManagerEventType iEventType, const gchar *cURI, Icon *pIcon);



typedef gboolean (*FileManagerInitFunc) (FileManagerOnEventFunc);


typedef void (*FileManagerGetFileInfoFunc) (gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountPoint);


typedef GList * (*FileManagerListDirectoryFunc) (gchar *cURI);


typedef void (*FileManagerLaunchUriFunc) (gchar *cURI);


typedef gchar * (*FileManagerIsMountingPointFunc) (gchar *cURI, gboolean *bIsMounted);

typedef gchar * (*FileManagerMountFunc) (gchar *cURI);
typedef void (*FileManagerUnmountFunc) (gchar *cURI);


typedef void (*FileManagerAddMonitorFunc) (Icon *pIcon);


#endif
