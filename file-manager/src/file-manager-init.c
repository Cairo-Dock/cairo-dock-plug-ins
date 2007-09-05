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


Icon *file_manager_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	//g_print ("%s ()\n", __func__);
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
	
	
	//\_______________ On cree nos entrees dans le menu qui sera appele lors d'un clic droit.
	/*GtkWidget *pMenu = gtk_menu_new ();
	GtkWidget *menu_item;
	
	menu_item = gtk_menu_item_new_with_label ("About");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (file_manager_about), NULL);*/
	
	
	//\_______________ On cree notre icone.
	my_fm_pIcon = NULL;
	my_fm_pDock = pDock;
	
	//\_______________ On charge le backend qui va bien.
	FileManagerDesktopEnv iDesktopEnv = FILE_MANAGER_UNKNOWN;
	const gchar * cEnv = g_getenv ("GNOME_DESKTOP_SESSION_ID");
	if (cEnv == NULL || *cEnv == '\0')
	{
		cEnv = g_getenv ("KDE_FULL_SESSION");
		if (cEnv == NULL || *cEnv == '\0')
		{
			iDesktopEnv = FILE_MANAGER_UNKNOWN;
		}
		else
			iDesktopEnv = FILE_MANAGER_KDE;
	}
	else
	{
		iDesktopEnv = FILE_MANAGER_GNOME;
	}
	
	if (iDesktopEnv == FILE_MANAGER_UNKNOWN)
	{
		 g_set_error (erreur, 1, 1, "Attention : couldn't guesse desktop environment, this module will not be active");
		return NULL;
	}
	if (iDesktopEnv == FILE_MANAGER_KDE)  // le backend de KDE n'est pas encore implemente.
		iDesktopEnv = FILE_MANAGER_XDG;
	
	gchar *cBackendPath = NULL;
	cBackendPath = g_strdup_printf ("%s/libfile-manager-%s.so", FILE_MANAGER_BACKEND_DIR, (iDesktopEnv == FILE_MANAGER_GNOME ? "gnome" : (iDesktopEnv == FILE_MANAGER_KDE ? "kde" : "xdg")));
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
	
	//g_free (cUserDataDirPath);
	//g_free (cName);
	return my_fm_pIcon;
}

void file_manager_stop (void)
{
	g_print ("OK a plus !\n");
	
	g_module_close (s_fm_pBackendModule);
	s_fm_pBackendModule = NULL;
	my_fm_pIcon = NULL;
	cairo_dock_add_uri_func = NULL;
	cairo_dock_launch_uri_func = NULL;
	cairo_dock_load_directory_func = NULL;
}

