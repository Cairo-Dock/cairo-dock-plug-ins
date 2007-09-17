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

Icon *my_fm_pIcon = NULL;
CairoDock *my_fm_pDock = NULL;

FileManagerInitFunc file_manager_init_backend = NULL;
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

FileManagerSortType g_fm_iSortType = FILE_MANAGER_SORT_BY_NAME;

gchar *file_manager_pre_init (void)
{
        return g_strdup_printf ("%s/%s", FILE_MANAGER_SHARE_DATA_DIR, FILE_MANAGER_README_FILE);
}


Icon *file_manager_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	g_print ("%s ()\n", __func__);
	/*gchar *cUserDataDirPath = g_strdup_printf ("%s/plug-in/%s", g_cCurrentThemePath, FILE_MANAGER_USER_DATA_DIR);
	if (! g_file_test (cUserDataDirPath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		g_print ("directory %s doesn't exist, trying to fix it ...\n", cUserDataDirPath);
		
		gchar *command = g_strdup_printf ("mkdir -p %s", cUserDataDirPath);
		system (command);
		g_free (command);
		
		command = g_strdup_printf ("cp %s/%s %s", FILE_MANAGER_SHARE_DATA_DIR, FILE_MANAGER_CONF_FILE, cUserDataDirPath);
		system (command);
		g_free (command);
	}*/
	
	
	//\_______________ On lit le fichier de conf.
	*cConfFilePath = NULL;
	/*cConfFilePath = g_strdup_printf ("%s/%s", cUserDataDirPath, FILE_MANAGER_CONF_FILE);
	int iOriginalWidth = 1, iOriginalHeight = 1;
	gchar *cName = NULL;
	file_manager_read_conf_file (*cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cName);*/
	
	//\_______________ On cree notre icone.
	my_fm_pIcon = NULL;
	my_fm_pDock = pDock;
	
	//\_______________ On charge le backend qui va bien.
	CairoDockDesktopEnv iDesktopEnv = cairo_dock_guess_environment ();
	if (iDesktopEnv == CAIRO_DOCK_UNKNOWN_ENV)
	{
		 g_set_error (erreur, 1, 1, "Attention : couldn't guess desktop environment, this module will not be active");
		return NULL;
	}
	if (iDesktopEnv == CAIRO_DOCK_KDE)  // le backend de KDE n'est pas encore implemente.
	{
		 g_set_error (erreur, 1, 1, "Attention : this module does not yet support KDE virtual file system. Any help is greatly welcome for this !");
		return NULL;
	}
	
	gchar *cBackendPath = NULL;
	cBackendPath = g_strdup_printf ("%s/libfile-manager-%s.so", FILE_MANAGER_BACKEND_DIR, (iDesktopEnv == CAIRO_DOCK_GNOME ? "gnome" : (iDesktopEnv == CAIRO_DOCK_KDE ? "kde" : "xdg")));
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
	g_print ("chargement du backend VFS OK\n");
	
	if (! file_manager_init_backend (file_monitor_action_on_event))
	{
		g_set_error (erreur, 1, 1, "Attention : couldn't initialize the backend correctly.");
		g_module_close (s_fm_pBackendModule);
		s_fm_pBackendModule = NULL;
		return NULL;
	}
	
	cairo_dock_add_uri_func = file_manager_add_desktop_file_from_uri;
	cairo_dock_launch_uri_func = file_manager_launch_icon;
	cairo_dock_load_directory_func = file_manager_create_dock_from_directory;
	
	g_hash_table_foreach (g_hDocksTable, (GHFunc) file_manager_reload_directories, NULL);
	
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) file_manager_notification_build_menu, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA, (CairoDockNotificationFunc) file_manager_notification_drop_data, CAIRO_DOCK_RUN_AFTER);
	
	//g_free (cUserDataDirPath);
	//g_free (cName);
	return my_fm_pIcon;
}

void file_manager_stop (void)
{
	my_fm_pIcon = NULL;
	
	g_module_close (s_fm_pBackendModule);
	s_fm_pBackendModule = NULL;
	
	cairo_dock_add_uri_func = NULL;
	cairo_dock_launch_uri_func = NULL;
	cairo_dock_load_directory_func = NULL;
}

