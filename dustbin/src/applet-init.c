/**********************************************************************************

This file is a part of the cairo-dock project,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "applet-draw.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-trashes-manager.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("dustbin", 1, 4, 7)

AppletConfig myConfig;
AppletData myData;

static void _load_theme (GError **erreur)
{
	//\_______________ On charge en priorite les images utilisateur.
	if (myConfig.cEmptyUserImage != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cEmptyUserImage);
		myData.pEmptyBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath)
		g_free (cUserImagePath);
	}
	if (myConfig.cFullUserImage != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cFullUserImage);
		myData.pFullBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath)
		g_free (cUserImagePath);
	}
	
	//\_______________ On charge le theme si necessaire.
	if (myConfig.cThemePath != NULL && (myData.pEmptyBinSurface == NULL || myData.pFullBinSurface == NULL))
	{
		GError *tmp_erreur = NULL;
		GDir *dir = g_dir_open (myConfig.cThemePath, 0, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return ;
		}
		
		const gchar *cElementName;
		gchar *cElementPath;
		while ((cElementName = g_dir_read_name (dir)) != NULL)
		{
			cElementPath = g_strdup_printf ("%s/%s", myConfig.cThemePath, cElementName);
			cd_message ("  %s\n", cElementPath);
			if (strncmp (cElementName, "trashcan_full", 13) == 0 && myConfig.cFullUserImage == NULL)
			{
				myData.cDialogIconPath = cElementPath;
				myData.pFullBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cElementPath)
			}
			else
			{
				if (strncmp (cElementName, "trashcan_empty", 14) == 0 && myData.pEmptyBinSurface == NULL)
					myData.pEmptyBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cElementPath)
				g_free (cElementPath);
			}
		}
		g_dir_close (dir);
	}
	if (myData.pFullBinSurface == NULL || myData.pFullBinSurface == NULL)
	{
		cd_message ("Attention : couldn't find images, this theme is not valid");
	}
}

CD_APPLET_INIT_BEGIN (erreur)
	//\_______________ On charge le theme choisi.
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, myDesklet->iWidth - 2 * g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - 2 * g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius;
		myIcon->fDrawY = g_iDockRadius;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	GError *tmp_erreur = NULL;
	_load_theme (&tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return;
	}
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	
	//\_______________ On commence a surveiller les repertoires.
	myData.iNbTrashes = -1;
	gchar *cDustbinPath = cairo_dock_fm_get_trash_path (g_getenv ("HOME"), TRUE);
	gboolean bMonitoringOK = cd_dustbin_add_one_dustbin (cDustbinPath, 0);  // cDustbinPath ne nous appartient plus.
	
	if (myConfig.cAdditionnalDirectoriesList != NULL)
	{
		int i = 0;
		while (myConfig.cAdditionnalDirectoriesList[i] != NULL)
		{
			if (*myConfig.cAdditionnalDirectoriesList[i] == '~')
				bMonitoringOK |= cd_dustbin_add_one_dustbin (g_strdup_printf ("%s%s", getenv ("HOME"), myConfig.cAdditionnalDirectoriesList[i]+1), 0);
			else
				bMonitoringOK |= cd_dustbin_add_one_dustbin (g_strdup (myConfig.cAdditionnalDirectoriesList[i]), 0);
			i ++;
		}
		cd_message ("  %d dossier(s) poubelle\n", i);
	}
	cd_message ("  %d dechets actuellement (%d)\n", myData.iNbTrashes, bMonitoringOK);
	
	
	if (myData.iNbTrashes == 0)
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pEmptyBinSurface)
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pFullBinSurface)
	}
	
	//\_______________ On lance la surveillancce de nos poubelles.
	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	if (bMonitoringOK)
	{
		if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			cd_dustbin_add_message (NULL, NULL);
		}
	}
	else  // methode par defaut.
	{
		if (myConfig.cAdditionnalDirectoriesList != NULL)
		{
			cd_message ("***utilisation par defaut\n");
			cd_dustbin_check_trashes (myIcon);
			myData.iSidCheckTrashes = g_timeout_add ((int) (1000 * myConfig.fCheckInterval), (GSourceFunc) cd_dustbin_check_trashes, (gpointer) myIcon);
		}
	}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	
	//\_______________ On stoppe la surveillance.
	cd_dustbin_remove_all_dustbins ();
	
	if (myData.iSidCheckTrashes != 0)
	{
		g_source_remove (myData.iSidCheckTrashes);
		myData.iSidCheckTrashes = 0;
	}
	
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge notre theme.
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, myDesklet->iWidth - 2 * g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - 2 * g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius;
		myIcon->fDrawY = g_iDockRadius;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	
	if (myData.pEmptyBinSurface != NULL)
	{
		cairo_surface_destroy (myData.pEmptyBinSurface);
		myData.pEmptyBinSurface = NULL;
	}
	if (myData.pFullBinSurface != NULL)
	{
		cairo_surface_destroy (myData.pFullBinSurface);
		myData.pFullBinSurface = NULL;
	}
	GError *erreur = NULL;
	_load_theme (&erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On stoppe la surveillance.
		cd_dustbin_remove_all_dustbins ();
		
		if (myData.iSidCheckTrashes != 0)
		{
			g_source_remove (myData.iSidCheckTrashes);
			myData.iSidCheckTrashes = 0;
		}
		
		//\_______________ On commence a surveiller les repertoires.
		myData.iNbTrashes = -1;
		gchar *cDustbinPath = cairo_dock_fm_get_trash_path (g_getenv ("HOME"), TRUE);
		gboolean bMonitoringOK = cd_dustbin_add_one_dustbin (cDustbinPath, 0);  // cDustbinPath ne nous appartient plus.
		
		if (myConfig.cAdditionnalDirectoriesList != NULL)
		{
			int i = 0;
			while (myConfig.cAdditionnalDirectoriesList[i] != NULL)
			{
				if (*myConfig.cAdditionnalDirectoriesList[i] == '~')
					bMonitoringOK |= cd_dustbin_add_one_dustbin (g_strdup_printf ("%s%s", getenv ("HOME"), myConfig.cAdditionnalDirectoriesList[i]+1), 0);
				else
					bMonitoringOK |= cd_dustbin_add_one_dustbin (g_strdup (myConfig.cAdditionnalDirectoriesList[i]), 0);
				i ++;
			}
			cd_message ("  %d dossier(s) poubelle\n", i);
		}
		cd_message ("  %d dechets actuellement (%d)\n", myData.iNbTrashes, bMonitoringOK);
		
		if (myData.iNbTrashes == 0)
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pEmptyBinSurface)
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pFullBinSurface)
		}
		
		if (bMonitoringOK)
		{
			if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
			{
				cd_dustbin_add_message (NULL, NULL);
			}
		}
		else  // methode par defaut.
		{
			if (myConfig.cAdditionnalDirectoriesList != NULL)
			{
				cd_message ("***utilisation par defaut\n");
				myData.iNbTrashes = -1;
				cd_dustbin_check_trashes (myIcon);
				myData.iSidCheckTrashes = g_timeout_add ((int) (1000 * myConfig.fCheckInterval), (GSourceFunc) cd_dustbin_check_trashes, (gpointer) myIcon);
			}
		}
	}
	else  // on redessine
	{
		if (myData.iSidCheckTrashes != 0)
			myData.iNbTrashes = -1;
		else
		{
			cd_dustbin_draw_quick_info (FALSE);
			if (myData.iNbTrashes == 0)
			{
				CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pEmptyBinSurface)
			}
			else
			{
				CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pFullBinSurface)
			}
		}
	}
CD_APPLET_RELOAD_END
