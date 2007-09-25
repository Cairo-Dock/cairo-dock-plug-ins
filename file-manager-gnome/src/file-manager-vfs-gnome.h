
#ifndef __FILE_MANAGER_VFS_GNOME__
#define  __FILE_MANAGER_VFS_GNOME__


#include <glib.h>
#include <file-manager-struct.h>


gboolean _file_manager_init_backend (FileManagerOnEventFunc pCallback);
void _file_manager_stop_backend (void);


void _file_manager_get_file_info (gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, FileManagerSortType iSortType);


GList *_file_manager_list_directory (gchar *cBaseURI, FileManagerSortType iSortType, gchar **cFullURI);


void _file_manager_launch_uri (gchar *cURI);


gchar * _file_manager_is_mounting_point (gchar *cURI, gboolean *bIsMounted);

void _file_manager_mount (int iVolumeID, FileManagerMountCallback pOnSuccessCallback, gpointer *data);

void _file_manager_unmount (gchar *cURI, FileManagerMountCallback pOnSuccessCallback, gpointer *data);


void _file_manager_add_monitor (Icon *pIcon);
void _file_manager_remove_monitor (Icon *pIcon);


void _file_manager_delete_file (gchar *cURI);

void _file_manager_rename_file (gchar *cOldURI, gchar *cNewName);

void _file_manager_move_file (gchar *cURI, gchar *cDirectoryURI);

void _file_manager_get_file_properties (gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask);


#endif
