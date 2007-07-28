
#include "stdlib.h"
#include "string.h"

#include "dustbin-draw.h"
#include "dustbin-config.h"
#include "dustbin-menu-functions.h"
#include "dustbin-init.h"


#define CD_DUSTBIN_CONF_FILE "dustbin.conf"
#define CD_DUSTBIN_USER_DATA_DIR "dustbin"


gchar *my_dustbin_cConfFilePath = NULL;
Icon *my_dustbin_pIcon = NULL;
cairo_t *my_dustbin_pCairoContext = NULL;
double my_dustbin_fCheckInterval;
int my_dustbin_iSidCheckTrashes = 0;
gchar **my_dustbin_cTrashDirectoryList = NULL;
int *my_dustbin_pTrashState = NULL;
int my_dustbin_iNbTrash = 0;
GtkWidget **my_dustbin_pShowToggleList = NULL;
GtkWidget **my_dustbin_pDeleteToggleList = NULL;
cairo_surface_t *my_dustbin_pEmptyBinSurface = NULL;
cairo_surface_t *my_dustbin_pFullBinSurface = NULL;
GHashTable *my_dustbin_pThemeTable = NULL;
int my_dustbin_iState = -1;


Icon *cd_dustbin_init (cairo_t *pSourceContext, GError **erreur)
{
	//g_print ("%s ()\n", __func__);
	
	gchar *cUserDataDirPath = g_strdup_printf ("%s/plug-in/%s", g_cCairoDockDataDir, CD_DUSTBIN_USER_DATA_DIR);
	if (! g_file_test (cUserDataDirPath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		g_print ("directory %s doesn't exist, trying to fix it ...\n", cUserDataDirPath);
		
		gchar *command = g_strdup_printf ("mkdir -p %s", cUserDataDirPath);
		system (command);
		g_free (command);
		
		command = g_strdup_printf ("cp %s/%s %s", CD_DUSTBIN_SHARE_DATA_DIR, CD_DUSTBIN_CONF_FILE, cUserDataDirPath);
		system (command);
		g_free (command);
	}
	
	//\_______________ On charge la liste des themes disponibles.
	GError *tmp_erreur = NULL;
	gchar *cThemesDir = g_strdup_printf ("%s/themes", CD_DUSTBIN_SHARE_DATA_DIR);
	my_dustbin_pThemeTable = cairo_dock_list_themes (cThemesDir, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	my_dustbin_cConfFilePath = g_strdup_printf ("%s/plug-in/%s/%s", g_cCairoDockDataDir, CD_DUSTBIN_USER_DATA_DIR, CD_DUSTBIN_CONF_FILE);
	cairo_dock_update_conf_file_with_hash_table (my_dustbin_cConfFilePath, my_dustbin_pThemeTable, "MODULE", "theme", 1, "Theme :");
	
	
	//\_______________ On lit le fichier de conf.
	int iOriginalWidth = 1, iOriginalHeight = 1;
	gchar *cName = NULL, *cThemeName = NULL;
	cd_dustbin_read_conf_file (my_dustbin_cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cName, &cThemeName);
	
	//\_______________ On cree nos entrees dans le menu qui sera appele lors d'un clic droit.
	GtkWidget *pModuleMenu = gtk_menu_new ();
	GtkWidget *pModuleSubMenu;
	GtkWidget *menu_item;
	int i = 0;
	while (my_dustbin_cTrashDirectoryList[i] != NULL)
		i ++;
	my_dustbin_iNbTrash = i;
	
	my_dustbin_pShowToggleList = g_new0 (GtkWidget *, my_dustbin_iNbTrash);
	my_dustbin_pDeleteToggleList = g_new0 (GtkWidget *, my_dustbin_iNbTrash);
	
	GString *sLabel = g_string_new ("");
	
	menu_item = gtk_menu_item_new_with_label ("Show Trash");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleMenu), menu_item);
	pModuleSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pModuleSubMenu);
	
	i = 0;
	while (my_dustbin_cTrashDirectoryList[i] != NULL)
	{
		g_string_printf (sLabel, "Show %s", my_dustbin_cTrashDirectoryList[i]);
		
		//menu_item = gtk_menu_item_new_with_label (sLabel->str);
		menu_item = gtk_check_menu_item_new_with_label (sLabel->str);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), FALSE);
		
		gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_dustbin_show_trash), my_dustbin_cTrashDirectoryList[i]);
		
		my_dustbin_pShowToggleList[i] = menu_item;
		
		i ++;
	}
	menu_item = gtk_menu_item_new_with_label ("Show All");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_dustbin_show_trash), NULL);
	
	
	menu_item = gtk_menu_item_new_with_label ("Delete Trash");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleMenu), menu_item);
	pModuleSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pModuleSubMenu);
	i = 0;
	while (my_dustbin_cTrashDirectoryList[i] != NULL)
	{
		g_string_printf (sLabel, "Delete %s", my_dustbin_cTrashDirectoryList[i]);
		
		//menu_item = gtk_menu_item_new_with_label (sLabel->str);
		menu_item = gtk_check_menu_item_new_with_label (sLabel->str);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), FALSE);
		
		gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_dustbin_delete_trash), my_dustbin_cTrashDirectoryList[i]);
		
		my_dustbin_pDeleteToggleList[i] = menu_item;
		
		i ++;
	}
	menu_item = gtk_menu_item_new_with_label ("Delete All");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_dustbin_delete_trash), NULL);
	
	
	g_string_free (sLabel, TRUE);
	menu_item = gtk_menu_item_new_with_label ("About");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_dustbin_about), NULL);
	
	
	//\_______________ On cree notre icone.
	my_dustbin_pIcon = cairo_dock_create_icon_for_applet (pSourceContext, iOriginalWidth, iOriginalHeight, cName, pModuleMenu);
	my_dustbin_pCairoContext = cairo_create (my_dustbin_pIcon->pIconBuffer);  // le 'pSourceContext' ne nous appartient pas !
	g_return_val_if_fail (my_dustbin_pCairoContext != NULL, NULL);
	
	
	//\_______________ On charge le theme choisi.
	//g_print ("theme : %s\n", cThemeName);
	if (cThemeName != NULL)
	{
		gchar *cThemePath = g_hash_table_lookup (my_dustbin_pThemeTable, cThemeName);
		if (cThemePath == NULL)
			cThemePath = g_hash_table_lookup (my_dustbin_pThemeTable, "Gion");
		g_return_val_if_fail (cThemePath != NULL, NULL);
		
		GError *erreur = NULL;
		GDir *dir = g_dir_open (cThemePath, 0, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return NULL;
		}
		
		double fImageWidth, fImageHeight;
		const gchar *cElementName;
		gchar *cElementPath;
		while ((cElementName = g_dir_read_name (dir)) != NULL)
		{
			cElementPath = g_strdup_printf ("%s/%s", cThemePath, cElementName);
			//g_print ("  %s\n", cElementPath);
			if (strncmp (cElementName, "trashcan_full", 13) == 0)
				my_dustbin_pFullBinSurface = cairo_dock_create_surface_from_image (cElementPath,
					my_dustbin_pCairoContext,
					1 + g_fAmplitude,
					(int) my_dustbin_pIcon->fWidth,
					(int) my_dustbin_pIcon->fHeight,
					(int) my_dustbin_pIcon->fWidth,
					(int) my_dustbin_pIcon->fHeight,
					&fImageWidth,
					&fImageHeight,
					0);
			else if (strncmp (cElementName, "trashcan_empty", 14) == 0)
				my_dustbin_pEmptyBinSurface = cairo_dock_create_surface_from_image (cElementPath,
					my_dustbin_pCairoContext,
					1 + g_fAmplitude,
					(int) my_dustbin_pIcon->fWidth,
					(int) my_dustbin_pIcon->fHeight,
					(int) my_dustbin_pIcon->fWidth,
					(int) my_dustbin_pIcon->fHeight,
					&fImageWidth,
					&fImageHeight,
					0);
			g_free (cElementPath);
		}
		g_dir_close (dir);
	}
	if (my_dustbin_pFullBinSurface == NULL || my_dustbin_pFullBinSurface == NULL)
	{
		g_print ("Attention : couldn't find images, this theme is not valid");
	}
	
	//\_______________ On lance le timer.
	my_dustbin_pTrashState = g_new0 (int, i);
	my_dustbin_iState = -1;
	cd_dustbin_check_trashes (my_dustbin_pIcon);
	my_dustbin_iSidCheckTrashes = g_timeout_add ((int) (1000 * my_dustbin_fCheckInterval), (GSourceFunc) cd_dustbin_check_trashes, (gpointer) my_dustbin_pIcon);
	
	
	g_free (cName);
	return my_dustbin_pIcon;
}

void cd_dustbin_stop (void)
{
	//g_print ("%s ()\n", __func__);
	
	g_source_remove (my_dustbin_iSidCheckTrashes);
	my_dustbin_iSidCheckTrashes = 0;
	my_dustbin_pIcon = NULL;
	
	cairo_destroy (my_dustbin_pCairoContext);
	my_dustbin_pCairoContext = NULL;
	
	g_free (my_dustbin_cConfFilePath);
	my_dustbin_cConfFilePath = NULL;
	
	g_free (my_dustbin_pTrashState);
	my_dustbin_pTrashState = NULL;
	
	g_hash_table_destroy (my_dustbin_pThemeTable);
	my_dustbin_pThemeTable = NULL;
	
	if (my_dustbin_pEmptyBinSurface != NULL)
		cairo_surface_destroy (my_dustbin_pEmptyBinSurface);
	my_dustbin_pEmptyBinSurface = NULL;
	if (my_dustbin_pFullBinSurface != NULL)
		cairo_surface_destroy (my_dustbin_pFullBinSurface);
	my_dustbin_pFullBinSurface = NULL;
}

gboolean cd_dustbin_config (void)
{
	gchar *cConfFilePath = g_strdup_printf ("%s/plug-in/%s/%s", g_cCairoDockDataDir, CD_DUSTBIN_USER_DATA_DIR, CD_DUSTBIN_CONF_FILE);
	
	cairo_dock_edit_conf_file (NULL, cConfFilePath, "Dustbin appet's config.");
	return TRUE;
}

gboolean cd_dustbin_action (void)
{
	g_print ("%s ()\n", __func__);
	return TRUE;
}

