/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-bookmarks.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS

/*static GList *s_pIconList = NULL;
static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;*/


static void cd_shortcuts_on_change_drives (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 6);
	
	GList *ic;
	Icon *icon;
	gboolean bIsMounted;
	gchar *cTargetURI = cairo_dock_fm_is_mounted (cURI, &bIsMounted);
	if (cTargetURI == NULL)  // version bourrinne.
	{
		cd_shortcuts_on_change_bookmarks (CAIRO_DOCK_FILE_MODIFIED, NULL, NULL);
	}
	else  // version optimisee.
	{
		for (ic = (myDock ? myIcon->pSubDock->icons : myDesklet->icons); ic != NULL; ic = ic->next)
		{
			icon =ic->data;
			if (icon->iType == 10)
			{
				if (strncmp (cTargetURI, icon->cBaseURI, strlen (cTargetURI)) == 0)
				{
					cd_message ("le signet %s est situe sur un point de montage ayant change (%s)", icon->cBaseURI, cTargetURI);
					gchar *cName = NULL, *cRealURI = NULL, *cIconName = NULL, *cUserName = NULL;
					int iVolumeID = 0;
					gboolean bIsDirectory = FALSE;
					double fOrder;
					if (cairo_dock_fm_get_file_info (icon->cBaseURI, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, g_iFileSortType))
					{
						g_print (" -> %s (%d)\n", cIconName, bIsMounted);
						g_free (icon->acName);
						if (bIsMounted || cIconName == NULL)
							icon->acName = cName;
						else
						{
							icon->acName = g_strdup_printf ("%s\n[%s]", cName, D_("Unmounted"));
							g_free (cName);
						}
						g_free (icon->acCommand);
						icon->acCommand = cRealURI;
						g_free (icon->acFileName);
						icon->acFileName = cIconName;
						icon->iVolumeID = iVolumeID;
						cairo_dock_load_one_icon_from_scratch (icon, (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer));
					}
				}
			}
		}
		g_free (cTargetURI);
	}
}
static void cd_shortcuts_on_change_network (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 8);
}


static GList * _load_icons (void)
{
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;
	
	if (myConfig.bListDrives)
	{
		pIconList = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_VFS_ROOT, CAIRO_DOCK_FM_SORT_BY_NAME, 6, FALSE, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		if (pIconList == NULL)
		{
			cd_warning ("couldn't detect any drives");  // on decide de poursuivre malgre tout, pour les signets.
		}
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_drives, myIcon))
			cd_warning ("Attention : can't monitor drives");
		myData.cDisksURI = cFullURI;
	}
	
	if (myConfig.bListNetwork)
	{
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, 8, FALSE, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		
		if (myConfig.bUseSeparator && pIconList2 != NULL && pIconList != NULL)
		{
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 7;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_network, NULL))
			cd_warning ("Attention : can't monitor network");
		myData.cNetworkURI = cFullURI;
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
		
		if (myConfig.bUseSeparator && pIconList2 != NULL && pIconList != NULL)
		{
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 9;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cBookmarkFilePath, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_bookmarks, NULL))
			cd_warning ("Attention : can't monitor bookmarks");
		
		myData.cBookmarksURI = cBookmarkFilePath;
	}
	
	return pIconList;
}


void cd_shortcuts_get_shortcuts_data (void)
{
	myData.pIconList = _load_icons ();
}


gboolean cd_shortcuts_build_shortcuts_from_data (void)
{
	if (myIcon == NULL)
	{
		g_print ("annulation du chargement des raccourcis\n");
		g_list_foreach (myData.pIconList, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myData.pIconList);
		myData.pIconList = NULL;
		return FALSE;
	}
	cd_message ("  chargement du sous-dock des raccourcis");
	
	//\_______________________ On efface l'ancienne liste.
	//if (myData.pDeskletIconList != NULL)
	if (myDesklet && myDesklet->icons != NULL)
	{
		g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myDesklet->icons);
		myDesklet->icons = NULL;
	}
	if (myIcon->pSubDock != NULL)
	{
		g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myIcon->pSubDock->icons);
		myIcon->pSubDock->icons = NULL;
	}
	
	//\_______________________ On charge la nouvelle liste.
	if (myDock)  // en mode 'dock', on affiche les raccourcis dans un sous-dock.
	{
		if (myIcon->pSubDock == NULL)
		{
			if (myData.pIconList != NULL)  // l'applet peut faire 'show desktop'.
			{
				cd_message ("  creation du sous-dock des raccourcis");
				CD_APPLET_CREATE_MY_SUBDOCK (myData.pIconList, myConfig.cRenderer)
				myData.pIconList = NULL;
			}
		}
		else  // on a deja notre sous-dock, on remplace juste ses icones.
		{
			cd_message ("  rechargement du sous-dock des raccourcis");
			if (myData.pIconList == NULL)  // inutile de le garder.
			{
				CD_APPLET_DESTROY_MY_SUBDOCK
			}
			else
			{
				CD_APPLET_LOAD_ICONS_IN_MY_SUBDOCK (myData.pIconList)
			}
		}
	}
	else
	{
		if (myIcon->pSubDock != NULL)
		{
			cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
			myIcon->pSubDock = NULL;
		}
		
		myDesklet->icons = myData.pIconList;
		myData.pIconList = NULL;
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);  // on n'a pas besoin du context sur myIcon.
		
		gtk_widget_queue_draw (myDesklet->pWidget);
	}
	
	myData.pIconList = NULL;
	return TRUE;
}
