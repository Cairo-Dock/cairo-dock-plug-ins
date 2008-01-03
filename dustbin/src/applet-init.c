/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "applet-draw.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-init.h"

double my_fCheckInterval;
int my_iSidCheckTrashes = 0;
gchar **my_cTrashDirectoryList = NULL;
int *my_pTrashState = NULL;
int my_iNbTrash = 0;
cairo_surface_t *my_pEmptyBinSurface = NULL;
cairo_surface_t *my_pFullBinSurface = NULL;
//GHashTable *my_pThemeTable = NULL;
//gchar *my_theme = NULL;
gchar *my_cThemePath = NULL;
int my_iState = -1;
gchar *my_cBrowser = NULL;


CD_APPLET_DEFINITION ("dustbin", 1, 4, 7)


CD_APPLET_INIT_BEGIN (erreur)
	//\_______________ On met a jour la liste des themes disponibles.
	/*if (my_pThemeTable != NULL)
		cairo_dock_update_conf_file_with_hash_table (CD_APPLET_MY_CONF_FILE, my_pThemeTable, "MODULE", "theme", NULL, (GHFunc) cairo_dock_write_one_theme_name, TRUE, FALSE);*/
	
	
	//\_______________ On charge le theme choisi.
	//g_print ("theme : %s\n", cThemeName);
	/*if (my_theme != NULL)
	{
		gchar *cThemePath = g_hash_table_lookup (my_pThemeTable, my_theme);
		if (cThemePath == NULL)
			cThemePath = g_hash_table_lookup (my_pThemeTable, "Gion");
		g_return_val_if_fail (cThemePath != NULL, NULL);*/
	if (my_cThemePath != NULL)
	{
		GError *tmp_erreur = NULL;
		GDir *dir = g_dir_open (my_cThemePath, 0, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
		
		double fImageWidth, fImageHeight;
		const gchar *cElementName;
		gchar *cElementPath;
		while ((cElementName = g_dir_read_name (dir)) != NULL)
		{
			cElementPath = g_strdup_printf ("%s/%s", my_cThemePath, cElementName);
			//g_print ("  %s\n", cElementPath);
			if (strncmp (cElementName, "trashcan_full", 13) == 0)
				my_pFullBinSurface = cairo_dock_create_surface_for_icon (cElementPath,
					myDrawContext,
					myIcon->fWidth * (1 + g_fAmplitude),
					myIcon->fHeight * (1 + g_fAmplitude));
			else if (strncmp (cElementName, "trashcan_empty", 14) == 0)
				my_pEmptyBinSurface = cairo_dock_create_surface_for_icon (cElementPath,
					myDrawContext,
					myIcon->fWidth * (1 + g_fAmplitude),
					myIcon->fHeight * (1 + g_fAmplitude));
			g_free (cElementPath);
		}
		g_dir_close (dir);
	}
	if (my_pFullBinSurface == NULL || my_pFullBinSurface == NULL)
	{
		g_print ("Attention : couldn't find images, this theme is not valid");
	}
	
	
	//\_______________ On enregistre nos notifications.
	cairo_dock_register_first_notifications (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_CLICK,
		CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU,
		-1);
	
	
	//\_______________ On initialise l'etat des poubelles.
	int i = 0;
	if (my_cTrashDirectoryList != NULL)
	{
		while (my_cTrashDirectoryList[i] != NULL)
			i ++;
	}
	my_iNbTrash = i;
	my_pTrashState = g_new0 (int, i);
	my_iState = -1;
	
	//\_______________ On lance le timer.
	cd_dustbin_check_trashes (myIcon);
	my_iSidCheckTrashes = g_timeout_add ((int) (1000 * my_fCheckInterval), (GSourceFunc) cd_dustbin_check_trashes, (gpointer) myIcon);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	cairo_dock_remove_notification_funcs (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_CLICK,
		CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU,
		-1);
	
	//\_______________ On stoppe le timer.
	g_source_remove (my_iSidCheckTrashes);
	my_iSidCheckTrashes = 0;
	
	//\_______________ On libere toutes nos ressources.
	my_iNbTrash = 0;
	
	g_free (my_pTrashState);
	my_pTrashState = NULL;
	
	/*g_hash_table_destroy (my_pThemeTable);
	my_pThemeTable = NULL;
	g_free (my_theme);
	my_theme = NULL;*/
	g_free (my_cThemePath);
	my_cThemePath = NULL;
	
	if (my_pEmptyBinSurface != NULL)
		cairo_surface_destroy (my_pEmptyBinSurface);
	my_pEmptyBinSurface = NULL;
	if (my_pFullBinSurface != NULL)
		cairo_surface_destroy (my_pFullBinSurface);
	my_pFullBinSurface = NULL;
	
	g_free (my_cBrowser);
	my_cBrowser = NULL;
CD_APPLET_STOP_END
