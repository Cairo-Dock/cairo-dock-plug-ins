/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "applet-gnome-vfs.h"
#include "applet-utils.h"
#include "applet-init.h"


CD_APPLET_PRE_INIT_BEGIN("gnome integration old", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)
	//if (g_iDesktopEnv == CAIRO_DOCK_GNOME)  ///  && glib_major_version == 2 && glib_minor_version < 16  <--- quand il y'aura les 2...
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME && glib_major_version == 2 && glib_minor_version < 16)
	{
		if (init_vfs_backend ())
		{
			CairoDockVFSBackend *pVFSBackend = g_new0 (CairoDockVFSBackend, 1);
			
			pVFSBackend->get_file_info = vfs_backend_get_file_info;
			pVFSBackend->get_file_properties = vfs_backend_get_file_properties;
			pVFSBackend->list_directory = vfs_backend_list_directory;
			pVFSBackend->launch_uri = vfs_backend_launch_uri;
			pVFSBackend->is_mounted = vfs_backend_is_mounted;
			pVFSBackend->mount = vfs_backend_mount;
			pVFSBackend->unmount = vfs_backend_unmount;
			pVFSBackend->add_monitor = vfs_backend_add_monitor;
			pVFSBackend->remove_monitor = vfs_backend_remove_monitor;
			pVFSBackend->delete = vfs_backend_delete_file;
			pVFSBackend->rename = vfs_backend_rename_file;
			pVFSBackend->move = vfs_backend_move_file;
			pVFSBackend->get_trash_path = vfs_backend_get_trash_path;
			pVFSBackend->get_desktop_path = vfs_backend_get_desktop_path;
			pVFSBackend->logout = env_backend_logout;
			pVFSBackend->setup_time = env_backend_setup_time;
			cairo_dock_fm_register_vfs_backend (pVFSBackend);
		}
	}
CD_APPLET_PRE_INIT_END


void init (GKeyFile *pKeyFile, Icon *pIcon, CairoContainer *pContainer, gchar *cConfFilePath, GError **erreur)
{
	
}

gboolean reload (GKeyFile *pKeyFile, gchar *cConfFilePath, CairoContainer *pNewContainer)
{
	return TRUE;
}

void stop (void)
{
}
