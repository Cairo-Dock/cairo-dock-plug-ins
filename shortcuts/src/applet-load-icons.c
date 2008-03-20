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
#include "applet-draw.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

static GList *s_pIconList = NULL;
static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;


static void cd_shortcuts_on_change_drives (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 6);
}
static void cd_shortcuts_on_change_network (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 8);
}


static GList * _load_icons (GError **erreur)
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
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_drives, NULL))
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
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_network, NULL))
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

gpointer cd_shortcuts_threaded_calculation (gpointer data)
{
	GError *erreur = NULL;
	s_pIconList = _load_icons (&erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread");
	return NULL;
}

static gboolean _cd_shortcuts_check_for_redraw (gpointer data)
{
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning)
	{
		s_iSidTimerRedraw = 0;
		if (myIcon == NULL)
		{
			g_print ("annulation du chargement des raccourcis\n");
			g_list_foreach (s_pIconList, cairo_dock_free_icon, NULL);
			g_list_free (s_pIconList);
			s_pIconList = NULL;
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
			myData.iNbIconsInTree = 0;
			//if (myDesklet)
			//	myDesklet->icons = NULL;
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
				if (s_pIconList != NULL)  // l'applet peut faire 'show desktop'.
				{
					cd_message ("  creation du sous-dock des raccourcis");
					myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (s_pIconList, myIcon->acName);
					cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
					cairo_dock_update_dock_size (myIcon->pSubDock);
				}
			}
			else  // on a deja notre sous-dock, on remplace juste ses icones.
			{
				cd_message ("  rechargement du sous-dock des raccourcis");
				if (s_pIconList == NULL)  // inutile de le garder.
				{
					cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
					myIcon->pSubDock = NULL;
				}
				else
				{
					myIcon->pSubDock->icons = s_pIconList;
					cairo_dock_load_buffers_in_one_dock (myIcon->pSubDock);
					cairo_dock_update_dock_size (myIcon->pSubDock);
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
			
			myDesklet->icons = s_pIconList;
			//myData.pDeskletIconList = s_pIconList;
			s_pIconList = NULL;
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
			/*cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
			if (myDesklet->pRenderer == NULL)
			{
				cd_shortcuts_load_tree (s_pIconList, pCairoContext);
				myDesklet->renderer = cd_shortcuts_draw_in_desklet;
				
				GList* ic;
				Icon *icon;
				for (ic = s_pIconList; ic != NULL; ic = ic->next)
				{
					icon = ic->data;
					
					icon->fWidth = 48 * MIN (myData.fTreeWidthFactor, myData.fTreeHeightFactor);
					icon->fHeight = 48 * MIN (myData.fTreeWidthFactor, myData.fTreeHeightFactor);
					
					cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, CAIRO_DOCK_HORIZONTAL, FALSE);
					g_print (" + %.2fx%.2f\n", icon->fWidth, icon->fHeight);
				}
			}
			cairo_destroy (pCairoContext);*/
			
			gtk_widget_queue_draw (myDesklet->pWidget);
		}
		
		s_pIconList = NULL;
		return FALSE;
	}
	return TRUE;
}
void cd_shortcuts_launch_measure (void)
{
	cd_message ("");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1))  // il etait egal a 0, on lui met 1 et on lance le thread.
	{
		cd_message (" ==> lancement du thread de calcul");
		s_pIconList = NULL;
		
		if (s_iSidTimerRedraw == 0)
			s_iSidTimerRedraw = g_timeout_add (200, (GSourceFunc) _cd_shortcuts_check_for_redraw, (gpointer) NULL);
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_shortcuts_threaded_calculation,
			NULL,
			FALSE,
			&erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
}
