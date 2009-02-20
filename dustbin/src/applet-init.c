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


CD_APPLET_DEFINITION ("dustbin", 1, 6, 2, CAIRO_DOCK_CATEGORY_DESKTOP)


static void _load_theme (GError **erreur)
{
	//\_______________ On charge en priorite les images utilisateur.
	if (myConfig.cEmptyUserImage != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cEmptyUserImage);
		myData.pEmptyBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	if (myConfig.cFullUserImage != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cFullUserImage);
		myData.pFullBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
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
				myData.pFullBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cElementPath);
			}
			else
			{
				if (strncmp (cElementName, "trashcan_empty", 14) == 0 && myData.pEmptyBinSurface == NULL)
					myData.pEmptyBinSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cElementPath);
				g_free (cElementPath);
			}
		}
		g_dir_close (dir);
	}
	if (myData.pFullBinSurface == NULL || myData.pFullBinSurface == NULL)
	{
		cd_warning ("dustbin : couldn't find images, check the existence of te files in %s", myConfig.cThemePath);
	}
}

static void _cd_dusbin_start (CairoDockModuleInstance *myApplet)
{
	//\_______________ On commence a surveiller les repertoires.
	gboolean bMonitoringOK = FALSE;
	gchar *cDustbinPath = cairo_dock_fm_get_trash_path (g_getenv ("HOME"), NULL);
	if (cDustbinPath != NULL)
		bMonitoringOK = cd_dustbin_add_one_dustbin (cDustbinPath, 0);  // cDustbinPath ne nous appartient plus.
	
	if (myConfig.cAdditionnalDirectoriesList != NULL)
	{
		int i = 0;
		while (myConfig.cAdditionnalDirectoriesList[i] != NULL)
		{
			if (*myConfig.cAdditionnalDirectoriesList[i] == '\0' || *myConfig.cAdditionnalDirectoriesList[i] == ' ')
			{
				cd_warning ("dustbin : this directory (%s) is not valid, be careful with it !", myConfig.cAdditionnalDirectoriesList[i]);
				i ++;
				continue ;
			}
			if (*myConfig.cAdditionnalDirectoriesList[i] == '~')
				bMonitoringOK |= cd_dustbin_add_one_dustbin (g_strdup_printf ("%s%s", getenv ("HOME"), myConfig.cAdditionnalDirectoriesList[i]+1), 0);
			else
				bMonitoringOK |= cd_dustbin_add_one_dustbin (g_strdup (myConfig.cAdditionnalDirectoriesList[i]), 0);
			i ++;
		}
		cd_message ("  %d dossier(s) poubelle", i);
	}
	cd_message ("  %d dechet(s) actuellement (%d)", myData.iNbTrashes, bMonitoringOK);
	
	//\_______________ On met l'icone qui va bien.
	if (myData.iNbTrashes <= 0)
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pEmptyBinSurface);
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pFullBinSurface);
	}
	
	//\_______________ On lance le comptage de nos poubelles.
	if (bMonitoringOK)
	{
		if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			cd_dustbin_add_message (NULL, NULL);
		}
		else
		{
			cd_dustbin_draw_quick_info (FALSE);
		}
	}
	else  // methode par defaut.
	{
		if (myConfig.cAdditionnalDirectoriesList != NULL)
		{
			cd_message ("***mode degrade");
			cd_dustbin_check_trashes (myIcon);
			myData.iSidCheckTrashes = g_timeout_add_seconds ((int) (myConfig.fCheckInterval), (GSourceFunc) cd_dustbin_check_trashes, (gpointer) myIcon);
		}
	}
}

CD_APPLET_INIT_BEGIN
	//\_______________ On charge le theme choisi.
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	GError *erreur = NULL;
	_load_theme (&erreur);
	if (erreur != NULL)
	{
		cd_warning ("dustbin : %s", erreur->message);
		g_error_free (erreur);
		return;
	}
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	
	//\_______________ On d�marre la surveillance des nos poubelles.
	_cd_dusbin_start (myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	
	//\_______________ On stoppe la surveillance.
	cd_dustbin_remove_all_dustbins ();
	
	if (myData.iSidCheckTrashes != 0)
	{
		g_source_remove (myData.iSidCheckTrashes);
		myData.iSidCheckTrashes = 0;
	}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge notre theme.
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
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
		cd_warning ("dustbin : %s", erreur->message);
		g_error_free (erreur);
		return TRUE;
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
		
		//\_______________ On la redemarre.
		_cd_dusbin_start (myApplet);
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
				CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pEmptyBinSurface);
			}
			else
			{
				CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pFullBinSurface);
			}
		}
	}
CD_APPLET_RELOAD_END
