/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)
Inspiration was taken from the "xdg" project :-)

******************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-bookmarks.h"
#include "applet-init.h"

gboolean my_bListDrives;
gboolean my_bListNetwork;
gboolean my_bListBookmarks;
gboolean my_bUseSeparator;
gchar *my_cRenderer = NULL;
GList *my_pBookmarkIconList = NULL;

CD_APPLET_DEFINITION ("shortcuts", 1, 4, 7)


void cd_shortcuts_on_change_drives (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 6);
}
void cd_shortcuts_on_change_network (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 8);
}

CD_APPLET_INIT_BEGIN (erreur)
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;
	
	if (my_bListDrives)
	{
		pIconList = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_VFS_ROOT, CAIRO_DOCK_FM_SORT_BY_NAME, 6, &cFullURI);
		g_print ("  cFullURI : %s\n", cFullURI);
		if (pIconList == NULL)
		{
			g_set_error (erreur, 1, 1, "%s () : couldn't detect any drives", __func__);
			return NULL;
		}
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_drives, NULL))
			g_print ("Attention : can't monitor drives\n");
		g_free (cFullURI);
	}
	
	if (my_bListNetwork)
	{
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, 8, &cFullURI);
		g_print ("  cFullURI : %s\n", cFullURI);
		
		if (my_bUseSeparator && pIconList2 != NULL)
		{
			//Icon *pSeparatorIcon = cairo_dock_create_separator_icon (myDrawContext, CAIRO_DOCK_LAUNCHER, myIcon->pSubDock, CAIRO_DOCK_APPLY_RATIO);
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 7;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_network, NULL))
			g_print ("Attention : can't monitor network\n");
		g_free (cFullURI);
	}
	
	if (my_bListBookmarks)
	{
		gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
		if (! g_file_test (cBookmarkFilePath, G_FILE_TEST_EXISTS))
		{
			FILE *f = fopen (cBookmarkFilePath, "a");
			fclose (f);
		}
		
		GList *pIconList2 = cd_shortcuts_list_bookmarks (cBookmarkFilePath);
		
		if (my_bUseSeparator)
		{
			//Icon *pSeparatorIcon = cairo_dock_create_separator_icon (myDrawContext, CAIRO_DOCK_LAUNCHER, myIcon->pSubDock, CAIRO_DOCK_APPLY_RATIO);
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 9;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cBookmarkFilePath, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_bookmarks, NULL))
			g_print ("Attention : can't monitor bookmarks\n");
		
		g_free (cBookmarkFilePath);
	}
	
	myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
	cairo_dock_set_renderer (myIcon->pSubDock, my_cRenderer);
	cairo_dock_update_dock_size (myIcon->pSubDock);
	
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	
	//\_______________ On libere toutes nos ressources.
	gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
	cairo_dock_fm_remove_monitor_full (cBookmarkFilePath, FALSE, NULL);
	g_free (cBookmarkFilePath);
	
	my_pBookmarkIconList = NULL;  // fait partie de myIcon->pSubDock->icons, donc ne pas desallouer a la main.
	
	cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
	g_print ("  myIcon->pSubDock <- %x\n", myIcon->pSubDock);
	myIcon->pSubDock = NULL;  // normalement inutile.
	
	g_free (my_cRenderer);
	my_cRenderer = NULL;
CD_APPLET_STOP_END
