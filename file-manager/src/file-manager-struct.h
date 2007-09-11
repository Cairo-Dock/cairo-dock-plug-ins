/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#ifndef __FILE_MANAGER_STRUCT__
#define  __FILE_MANAGER_STRUCT__

#include <glib.h>
#include <cairo-dock.h>

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

typedef enum {
	FILE_MANAGER_SORT_BY_NAME=0,
	FILE_MANAGER_SORT_BY_DATE,
	FILE_MANAGER_SORT_BY_SIZE,
	FILE_MANAGER_SORT_BY_TYPE
	} FileManagerSortType;

#define FILE_MANAGER_VFS_ROOT "_vfsroot_"
#define FILE_MANAGER_NETWORK "_network_"

typedef void (*FileManagerOnEventFunc) (FileManagerEventType iEventType, const gchar *cURI, Icon *pIcon);



typedef gboolean (*FileManagerInitFunc) (FileManagerOnEventFunc);


typedef void (*FileManagerGetFileInfoFunc) (gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountPoint, double *fOrder, FileManagerSortType iSortType);


typedef GList * (*FileManagerListDirectoryFunc) (gchar *cURI, FileManagerSortType g_fm_iSortType);


typedef void (*FileManagerLaunchUriFunc) (gchar *cURI);


typedef gchar * (*FileManagerIsMountingPointFunc) (gchar *cURI, gboolean *bIsMounted);

typedef gchar * (*FileManagerMountFunc) (gchar *cURI);
typedef void (*FileManagerUnmountFunc) (gchar *cURI);


typedef void (*FileManagerAddMonitorFunc) (Icon *pIcon);


typedef void (*FileManagerDeleteFileFunc) (gchar *cURI);
typedef void (*FileManagerRenameFileFunc) (gchar *cOldURI, gchar *cNewName);
typedef void (*FileManagerMoveFileFunc) (gchar *cURI, gchar *cDirectoryURI);
typedef void (*FileManagerFilePropertiesFunc) (gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask);

#endif
