
#ifndef __APPLET_GNOME_VFS__
#define  __APPLET_GNOME_VFS__


#include <cairo-dock.h>


gboolean init_vfs_backend (void);
void stop_vfs_backend (void);


void vfs_backend_get_file_info (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType);


GList *vfs_backend_list_directory (const gchar *cBaseURI, CairoDockFMSortType iSortType, gchar **cFullURI);


void vfs_backend_launch_uri (const gchar *cURI);


gchar * vfs_backend_is_mounted (const gchar *cURI, gboolean *bIsMounted);

void vfs_backend_mount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock);

void vfs_backend_unmount (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock);


void vfs_backend_add_monitor (const gchar *cURI, gboolean bDirectory, CairoDockFMMonitorCallback pCallback, gpointer data);
void vfs_backend_remove_monitor (const gchar *cURI);


gboolean vfs_backend_delete_file (const gchar *cURI);

gboolean vfs_backend_rename_file (const gchar *cOldURI, const gchar *cNewName);

gboolean vfs_backend_move_file (const gchar *cURI, const gchar *cDirectoryURI);


void vfs_backend_get_file_properties (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask);

gchar *vfs_backend_get_trash_path (const gchar *cNearURI, gboolean bCreateIfNecessary);

gchar *vfs_backend_get_desktop_path (void);


#endif
