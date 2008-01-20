
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-init.h"

gboolean my_bListDrives;
gboolean my_bListNetwork;
gboolean my_bListBookmarks;
gboolean my_bUseSeparator;
gchar *my_cRenderer = NULL;

CD_APPLET_DEFINITION ("shortcuts", 1, 4, 7)


void cd_shortcuts_on_change_bookmarks (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data)
{
	g_print ("%s () : un signet en plus ou en moins\n", __func__);
	
}

CD_APPLET_INIT_BEGIN (erreur)
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;
	
	if (my_bListDrives)
	{
		pIconList = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_VFS_ROOT, CAIRO_DOCK_FM_SORT_BY_NAME, &cFullURI);
		g_print ("  cFullURI : %s\n", cFullURI);
		g_free (cFullURI);
		if (pIconList == NULL)
		{
			g_set_error (erreur, 1, 1, "%s () : couldn't detect any drives", __func__);
			return NULL;
		}
	}
	
	if (my_bListNetwork)
	{
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, &cFullURI);
		g_print ("  cFullURI : %s\n", cFullURI);
		g_free (cFullURI);
		pIconList = g_list_concat (pIconList, pIconList2);
	}
	
	if (my_bListBookmarks)
	{
		gchar *cBookmarkFilsPath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
		if (! g_file_test (cBookmarkFilsPath, G_FILE_TEST_EXISTS))
		{
			FILE *f = fopen (cBookmarkFilsPath, "a");
			fclose (f);
		}
		
		gchar *cContent = NULL;
		gsize length=0;
		GError *tmp_erreur = NULL;
		g_file_get_contents  (cBookmarkFilsPath, &cContent, &length, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_print ("Attention : %s\n  no bookmark will be available\n", tmp_erreur->message);
			g_error_free (tmp_erreur);
		}
		else
		{
			GList *pIconList2 = NULL;
			gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
			gchar *cOneBookmark;
			Icon *pNewIcon;
			gchar *cName, *cRealURI, *cIconName;
			gboolean bIsDirectory;
			int iVolumeID;
			double fOrder;
			int i = 0;
			for (i = 0; cBookmarksList[i] != NULL; i ++)
			{
				cOneBookmark = cBookmarksList[i];
				if (cairo_dock_fm_get_file_info (cOneBookmark, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, g_iFileSortType))
				{
					pNewIcon = g_new0 (Icon, 1);
					pNewIcon->iType = CAIRO_DOCK_LAUNCHER;
					pNewIcon->cBaseURI = cOneBookmark;
					pNewIcon->acName = cName;
					pNewIcon->acCommand = cRealURI;
					pNewIcon->acFileName = cIconName;
					pNewIcon->iVolumeID = iVolumeID;
					pNewIcon->fOrder = fOrder;
					pIconList = g_list_append (pIconList, pNewIcon);
				}
				else
				{
					g_free (cOneBookmark);
				}
			}
			g_free (cBookmarksList);
			
			if (! cairo_dock_fm_add_monitor_full (cBookmarkFilsPath, FALSE, NULL, (CairoDockFMMonitorCallback) cairo_dock_fm_action_on_file_event, NULL))
				g_print ("Attention : can't monitor bookmarks\n");
		}
		g_free (cBookmarkFilsPath);
		g_free (cContent);
	}
	
	myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
	cairo_dock_set_renderer (myIcon->pSubDock, my_cRenderer);
	cairo_dock_update_dock_size (myIcon->pSubDock);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	
	//\_______________ On libere toutes nos ressources.
	cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
	
	g_free (my_cRenderer);
CD_APPLET_STOP_END
