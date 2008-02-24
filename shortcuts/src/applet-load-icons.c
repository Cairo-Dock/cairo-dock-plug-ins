/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-bookmarks.h"
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
			g_set_error (erreur, 1, 1, "%s () : couldn't detect any drives", __func__);
			return NULL;
		}
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_drives, NULL))
			cd_warning ("Attention : can't monitor drives");
		g_free (cFullURI);
	}
	
	if (myConfig.bListNetwork)
	{
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, 8, FALSE, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		
		if (myConfig.bUseSeparator && pIconList2 != NULL)
		{
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 7;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_network, NULL))
			cd_warning ("Attention : can't monitor network");
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
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 9;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cBookmarkFilePath, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_bookmarks, NULL))
			cd_warning ("Attention : can't monitor bookmarks");
		
		g_free (cBookmarkFilePath);
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
		cd_message ("  chargement du sous-dock des raccourcis");
		
		if (s_pIconList != NULL || ! myConfig.bListDrives)  // l'applet peut faire "show desktop".
		{
			myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (s_pIconList, myIcon->acName);
			cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
			cairo_dock_update_dock_size (myIcon->pSubDock);
		}
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
