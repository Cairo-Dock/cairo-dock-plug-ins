
#ifndef __FILE_MANAGER_VFS__
#define  __FILE_MANAGER_VFS__


#include <cairo-dock.h>


void _file_manager_get_file_info (gchar *cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountedPoint);


GList *_file_manager_list_directory (gchar *cURI);


void _file_manager_launch_uri (gchar *cURI);


gchar * _file_manager_is_mounting_point (gchar *cURI, gboolean *bIsMounted);

gchar * _file_manager_mount_point_by_id (gchar *cMountPointID);


#endif
