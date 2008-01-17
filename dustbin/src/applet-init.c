/**********************************************************************************

This file is a part of the cairo-dock project, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "applet-draw.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-trashes-manager.h"
#include "applet-struct.h"
#include "applet-init.h"

double my_fCheckInterval;
int my_iSidCheckTrashes = 0;
GList *my_pTrashDirectoryList = NULL;
gchar **my_cAdditionnalDirectoriesList = NULL;
int *my_pTrashState = NULL;
cairo_surface_t *my_pEmptyBinSurface = NULL;
cairo_surface_t *my_pFullBinSurface = NULL;
gchar *my_cThemePath = NULL;
int my_iState = -1;
int my_iNbTrashes = 0, my_iNbFiles = 0, my_iSize = 0;
CdDustbinInfotype my_iQuickInfoType;
int my_iQuickInfoValue = 0;
gchar *my_cDefaultBrowser = NULL;
gchar *my_cEmptyUserImage = NULL;
gchar *my_cFullUserImage = NULL;
int my_iGlobalSizeLimit, my_iSizeLimit;
gchar *my_cDialogIconPath = NULL;

CD_APPLET_DEFINITION ("dustbin", 1, 4, 7)


CD_APPLET_INIT_BEGIN (erreur)
	//\_______________ On charge le theme choisi.
	if (my_cEmptyUserImage != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (my_cEmptyUserImage);
		my_pEmptyBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath)
		g_free (cUserImagePath);
	}
	if (my_cFullUserImage != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (my_cFullUserImage);
		my_pEmptyBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath)
		g_free (cUserImagePath);
	}
	
	if (my_cThemePath != NULL)
	{
		GError *tmp_erreur = NULL;
		GDir *dir = g_dir_open (my_cThemePath, 0, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
		
		const gchar *cElementName;
		gchar *cElementPath;
		while ((cElementName = g_dir_read_name (dir)) != NULL)
		{
			cElementPath = g_strdup_printf ("%s/%s", my_cThemePath, cElementName);
			g_print ("  %s\n", cElementPath);
			if (strncmp (cElementName, "trashcan_full", 13) == 0)
			{
				my_cDialogIconPath = cElementPath;
				my_pFullBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cElementPath)
			}
			else
			{
				if (strncmp (cElementName, "trashcan_empty", 14) == 0)
					my_pEmptyBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cElementPath)
				g_free (cElementPath);
			}
		}
		g_dir_close (dir);
	}
	if (my_pFullBinSurface == NULL || my_pFullBinSurface == NULL)
	{
		g_print ("Attention : couldn't find images, this theme is not valid");
	}
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
	
	//\_______________ On initialise l'etat des poubelles.
	my_iState = -1;
	
	//\_______________ On commence a surveiller les repertoires.
	gchar *cDustbinPath = cairo_dock_fm_get_trash_path (g_getenv ("HOME"), TRUE);
	gboolean bMonitoringOK = cd_dustbin_add_one_dustbin (cDustbinPath, 0);
	
	if (my_cAdditionnalDirectoriesList != NULL)
	{
		int i = 0;
		while (my_cAdditionnalDirectoriesList[i] != NULL)
		{
			if (*my_cAdditionnalDirectoriesList[i] == '~')
				bMonitoringOK |= cd_dustbin_add_one_dustbin (g_strdup_printf ("%s%s", getenv ("HOME"), my_cAdditionnalDirectoriesList[i]+1), 0);
			else
				bMonitoringOK |= cd_dustbin_add_one_dustbin (g_strdup (my_cAdditionnalDirectoriesList[i]), 0);
			i ++;
		}
		g_print ("  %d dossier(s) poubelle\n", i);
		g_strfreev (my_cAdditionnalDirectoriesList);
		my_cAdditionnalDirectoriesList = NULL;
	}
	g_print ("  %d dechets actuellement (%d)\n", my_iNbTrashes, bMonitoringOK);
	
	
	cd_dustbin_draw_quick_info (FALSE);
	if (my_iNbTrashes == 0)
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (my_pEmptyBinSurface)
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (my_pFullBinSurface)
	}
	
	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	if (bMonitoringOK)
	{
		if (my_iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || my_iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			cd_dustbin_add_message (NULL, NULL);
		}
	}
	else  // methode par defaut.
	{
		if (my_pTrashDirectoryList != NULL)
		{
			g_print ("***utilisation par defaut\n");
			cd_dustbin_check_trashes (myIcon);
			my_iSidCheckTrashes = g_timeout_add ((int) (1000 * my_fCheckInterval), (GSourceFunc) cd_dustbin_check_trashes, (gpointer) myIcon);
		}
	}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	
	//\_______________ On stoppe la surveillance.
	cd_dustbin_remove_all_dustbins ();
	
	if (my_iSidCheckTrashes != 0)
	{
		g_source_remove (my_iSidCheckTrashes);
		my_iSidCheckTrashes = 0;
	}
	
	//\_______________ On libere toutes nos ressources.
	g_atomic_int_set (&my_iQuickInfoValue, 0);
	my_iNbTrashes = 0, my_iNbFiles = 0, my_iSize = 0;
	
	g_free (my_pTrashState);
	my_pTrashState = NULL;
	
	g_free (my_cThemePath);
	my_cThemePath = NULL;
	
	if (my_pEmptyBinSurface != NULL)
		cairo_surface_destroy (my_pEmptyBinSurface);
	my_pEmptyBinSurface = NULL;
	if (my_pFullBinSurface != NULL)
		cairo_surface_destroy (my_pFullBinSurface);
	my_pFullBinSurface = NULL;
	
	g_free (my_cDefaultBrowser);
	my_cDefaultBrowser = NULL;
	
	g_free (my_cEmptyUserImage);
	my_cEmptyUserImage = NULL;
	g_free (my_cFullUserImage);
	my_cFullUserImage = NULL;
	g_free (my_cDialogIconPath);
	
CD_APPLET_STOP_END
