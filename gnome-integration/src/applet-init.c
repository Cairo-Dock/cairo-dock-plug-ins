
#include "stdlib.h"

#include "applet-gnome-vfs.h"
#include "applet-init.h"


/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include "stdlib.h"

#include "applet-gnome-vfs.h"
#include "applet-init.h"


CairoDockVisitCard *pre_init (void)
{
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
	pVisitCard->cModuleName = g_strdup ("gnome integration");
	pVisitCard->cReadmeFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_README_FILE);
	pVisitCard->iMajorVersionNeeded = 1;
	pVisitCard->iMinorVersionNeeded = 4;
	pVisitCard->iMicroVersionNeeded = 7;
	pVisitCard->cPreviewFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_PREVIEW_FILE);
	pVisitCard->cGettextDomain = g_strdup (MY_APPLET_GETTEXT_DOMAIN);
	pVisitCard->cDockVersionOnCompilation = g_strdup (MY_APPLET_DOCK_VERSION);

	if (g_iDesktopEnv == CAIRO_DOCK_GNOME)
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
			cairo_dock_fm_register_vfs_backend (pVFSBackend);
		}
	}

	return pVisitCard;
}


Icon *init (CairoDock *pDock, CairoDockModule *pModule, GError **erreur)
{
	pModule->cConfFilePath = NULL;
	return NULL;
}

void configure()
{
}

void stop (void)
{
}
