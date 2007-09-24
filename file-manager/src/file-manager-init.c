/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include "stdlib.h"

#include "file-manager-config.h"
#include "file-manager-menu-functions.h"
#include "file-manager-add-desktop-file.h"
#include "file-manager-struct.h"
#include "file-manager-load-directory.h"
#include "file-manager-init.h"


#define FILE_MANAGER_CONF_FILE "file-manager.conf"
#define FILE_MANAGER_USER_DATA_DIR "file-manager"

static GModule *s_fm_pBackendModule = NULL;

FileManagerInitFunc file_manager_init_backend = NULL;
FileManagerStopFunc file_manager_stop_backend = NULL;
FileManagerGetFileInfoFunc file_manager_get_file_info = NULL;
FileManagerListDirectoryFunc file_manager_list_directory = NULL;
FileManagerLaunchUriFunc file_manager_launch_uri = NULL;
FileManagerIsMountingPointFunc file_manager_is_mounting_point = NULL;
FileManagerMountFunc file_manager_mount = NULL;
FileManagerUnmountFunc file_manager_unmount = NULL;
FileManagerAddMonitorFunc file_manager_add_monitor;
FileManagerAddMonitorFunc file_manager_remove_monitor;
FileManagerDeleteFileFunc file_manager_delete_file;
FileManagerRenameFileFunc file_manager_rename_file;
FileManagerMoveFileFunc file_manager_move_file;
FileManagerFilePropertiesFunc file_manager_get_file_properties;


gchar *my_fm_cIconFileName = NULL;
FileManagerSortType my_fm_iSortType = FILE_MANAGER_SORT_BY_NAME;
gboolean my_fm_bShowVolumes;
gboolean my_fm_bShowNetwork;
CairoDockDesktopEnv my_fm_iDesktopEnv;


gchar *file_manager_pre_init (void)
{
        return g_strdup_printf ("%s/%s", FILE_MANAGER_SHARE_DATA_DIR, FILE_MANAGER_README_FILE);
}


Icon *file_manager_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	//g_print ("%s ()\n", __func__);
	*cConfFilePath = cairo_dock_check_conf_file_exists (FILE_MANAGER_USER_DATA_DIR, FILE_MANAGER_SHARE_DATA_DIR, FILE_MANAGER_CONF_FILE);
	
	
	//\_______________ On lit le fichier de conf.
	*cConfFilePath = NULL;
	/*cConfFilePath = g_strdup_printf ("%s/%s", cUserDataDirPath, FILE_MANAGER_CONF_FILE);
	int iOriginalWidth = 1, iOriginalHeight = 1;
	gchar *cName = NULL;
	file_manager_read_conf_file (*cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cName);*/
	
	
	//\_______________ On charge le backend qui va bien.
	my_fm_iDesktopEnv = cairo_dock_guess_environment ();
	if (my_fm_iDesktopEnv == CAIRO_DOCK_UNKNOWN_ENV)
	{
		 g_set_error (erreur, 1, 1, "couldn't guess desktop environment, the file-manager will not be active");
		return NULL;
	}
	if (my_fm_iDesktopEnv == CAIRO_DOCK_KDE)  // le backend de KDE n'est pas encore implemente.
	{
		 g_set_error (erreur, 1, 1, "the file-manager plug-in does not yet support KDE virtual file system. Any help is greatly welcome for this !");
		return NULL;
	}
	
	gchar *cBackendPath = NULL;
	cBackendPath = g_strdup_printf ("%s/libfile-manager-%s.so", FILE_MANAGER_BACKEND_DIR, (my_fm_iDesktopEnv == CAIRO_DOCK_GNOME ? "gnome" : (my_fm_iDesktopEnv == CAIRO_DOCK_KDE ? "kde" : "xdg")));
	s_fm_pBackendModule = g_module_open (cBackendPath, G_MODULE_BIND_LAZY);
	if (s_fm_pBackendModule == NULL)
	{
		g_set_error (erreur, 1, 1, "Attention : while opening backend '%s' : (%s)", cBackendPath, g_module_error ());
		g_free (cBackendPath);
		return NULL;
	}
	g_free (cBackendPath);
	
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_init_backend", (gpointer) &file_manager_init_backend))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_stop_backend", (gpointer) &file_manager_stop_backend))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_get_file_info", (gpointer) &file_manager_get_file_info))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_list_directory", (gpointer) &file_manager_list_directory))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_launch_uri", (gpointer) &file_manager_launch_uri))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_is_mounting_point", (gpointer) &file_manager_is_mounting_point))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_mount", (gpointer) &file_manager_mount))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_unmount", (gpointer) &file_manager_unmount))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_add_monitor", (gpointer) &file_manager_add_monitor))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_remove_monitor", (gpointer) &file_manager_remove_monitor))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_delete_file", (gpointer) &file_manager_delete_file))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_rename_file", (gpointer) &file_manager_rename_file))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_move_file", (gpointer) &file_manager_move_file))
		return NULL;
	if (! g_module_symbol (s_fm_pBackendModule, "_file_manager_get_file_properties", (gpointer) &file_manager_get_file_properties))
		return NULL;
	//g_print ("chargement du backend VFS OK\n");
	
	if (! file_manager_init_backend (file_monitor_action_on_event))
	{
		g_set_error (erreur, 1, 1, "Attention : couldn't initialize the backend correctly.");
		g_module_close (s_fm_pBackendModule);
		s_fm_pBackendModule = NULL;
		return NULL;
	}
	
	cairo_dock_add_uri_func = file_manager_add_desktop_file_from_uri;
	cairo_dock_load_directory_func = file_manager_create_dock_from_directory;
	
	g_hash_table_foreach (g_hDocksTable, (GHFunc) file_manager_reload_directories, NULL);
	
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) file_manager_notification_build_menu, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA, (CairoDockNotificationFunc) file_manager_notification_drop_data, CAIRO_DOCK_RUN_AFTER);
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) file_manager_notification_click_icon, CAIRO_DOCK_RUN_FIRST);
	
	//g_free (cUserDataDirPath);
	//g_free (cName);
	return NULL;
}

void file_manager_stop (void)
{
	file_manager_stop_backend ();
	
	cairo_dock_add_uri_func = NULL;
	cairo_dock_load_directory_func = NULL;
	
	g_module_close (s_fm_pBackendModule);
	s_fm_pBackendModule = NULL;
	
	cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) file_manager_notification_build_menu);
	cairo_dock_remove_notification_func (CAIRO_DOCK_DROP_DATA, (CairoDockNotificationFunc) file_manager_notification_drop_data);
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) file_manager_notification_click_icon);
	
	g_hash_table_foreach (g_hDocksTable, (GHFunc) file_manager_unload_directories, NULL);
	
	g_free (my_fm_cIconFileName);
	my_fm_cIconFileName = NULL;
}
