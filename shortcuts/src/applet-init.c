/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)
Inspiration was taken from the "xdg" project :-)

******************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-bookmarks.h"
#include "applet-struct.h"
#include "applet-init.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_DEFINITION ("shortcuts", 1, 4, 7)


void cd_shortcuts_on_change_drives (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 6);
}
void cd_shortcuts_on_change_network (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 8);
}


static void _load_icons (GError **erreur)
{
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;
	
	if (myConfig.bListDrives)
	{
		pIconList = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_VFS_ROOT, CAIRO_DOCK_FM_SORT_BY_NAME, 6, &cFullURI);
		cd_message ("  cFullURI : %s\n", cFullURI);
		if (pIconList == NULL)
		{
			g_set_error (erreur, 1, 1, "%s () : couldn't detect any drives", __func__);
			return ;
		}
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_drives, NULL))
			cd_message ("Attention : can't monitor drives\n");
		g_free (cFullURI);
	}
	
	if (myConfig.bListNetwork)
	{
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, 8, &cFullURI);
		cd_message ("  cFullURI : %s\n", cFullURI);
		
		if (myConfig.bUseSeparator && pIconList2 != NULL)
		{
			//Icon *pSeparatorIcon = cairo_dock_create_separator_icon (myDrawContext, CAIRO_DOCK_LAUNCHER, myIcon->pSubDock, CAIRO_DOCK_APPLY_RATIO);
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 7;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_network, NULL))
			cd_message ("Attention : can't monitor network\n");
		g_free (cFullURI);
	}
		
	if (myConfig.bListBookmarks)
	{
		gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
		if (! g_file_test (cBookmarkFilePath, G_FILE_TEST_EXISTS))
		{
			FILE *f = fopen (cBookmarkFilePath, "a");
			fclose (f);
		}
		
		GList *pIconList2 = cd_shortcuts_list_bookmarks (cBookmarkFilePath);
		
		if (myConfig.bUseSeparator)
		{
			//Icon *pSeparatorIcon = cairo_dock_create_separator_icon (myDrawContext, CAIRO_DOCK_LAUNCHER, myIcon->pSubDock, CAIRO_DOCK_APPLY_RATIO);
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 9;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cBookmarkFilePath, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_bookmarks, NULL))
			cd_message ("Attention : can't monitor bookmarks\n");
		
		g_free (cBookmarkFilePath);
	}
	
	if (myConfig.bListDrives || myConfig.bListNetwork || myConfig.bListBookmarks)  // l'applet peut faire "show desktop".
	{
		myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
		cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
		cairo_dock_update_dock_size (myIcon->pSubDock);
	}
}

CD_APPLET_INIT_BEGIN (erreur)
	GError *tmp_erreur = NULL;
	_load_icons (&tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On charge les icones dans un sous-dock.
		reset_data ();
		
		GError *erreur = NULL;
		_load_icons (&erreur);
		if (erreur != NULL)
		{
			cd_message ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return FALSE;
		}
	}
	else
	{
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
CD_APPLET_RELOAD_END
