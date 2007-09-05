
#ifndef __FILE_MANAGER_VFS_GNOME__
#define  __FILE_MANAGER_VFS_GNOME__


#include <glib.h>
#include <file-manager-struct.h>


gboolean _file_manager_init_backend (FileManagerOnEventFunc pCallback);


void _file_manager_get_file_info (gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountPoint);


GList *_file_manager_list_directory (gchar *cURI);


void _file_manager_launch_uri (gchar *cURI);


gchar * _file_manager_is_mounting_point (gchar *cURI, gboolean *bIsMounted);

gchar * _file_manager_mount (gchar *cURI);

void _file_manager_unmount (gchar *cURI);


void _file_manager_add_monitor (Icon *pIcon);


#endif
