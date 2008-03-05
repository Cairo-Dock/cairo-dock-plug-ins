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
		
		//\_______________________ On efface l'ancienne liste.
		if (myData.pDeskletIconList != NULL)
		{
			g_list_foreach (myData.pDeskletIconList, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myData.pDeskletIconList);
			myData.pDeskletIconList = NULL;
			myData.iNbIconsInTree = 0;
			myDesklet->icons = NULL;
		}
		if (myIcon->pSubDock != NULL)
		{
			g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
		}
		
		//\_______________________ On charge la nouvelle liste.
		if (myDock != NULL)  // en mode 'dock', on affiche la meteo dans un sous-dock.
		{
			if (myIcon->pSubDock == NULL)
			{
				if (s_pIconList != NULL)  // l'applet peut montrer les conditions courantes.
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
			
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
			cd_shortcuts_load_tree (s_pIconList, pCairoContext);
			
			GList* ic;
			Icon *icon;
			for (ic = s_pIconList; ic != NULL; ic = ic->next)
			{
				icon = ic->data;
				
				icon->fWidth = 0;  // fixer leur taille en fonction de celle du desklet ?...
				icon->fHeight = 0;
				
				cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, CAIRO_DOCK_HORIZONTAL, TRUE);
			}
			cairo_destroy (pCairoContext);
			gtk_widget_queue_draw (myDesklet->pWidget);
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


#define TREE_WIDTH 150
#define TREE_HEIGHT 161
static int s_iLeafPosition[2][3*3] = {{-30,40,1 , 60,110,0 , -45,120,1},{-60,70,0 , 55,115,1 , -30,115,0}};

void cd_shortcuts_load_tree (GList *pIconsList, cairo_t *pCairoContext)
{
	g_print ("%s ()\n", __func__);
	myData.pDeskletIconList = pIconsList;
	myDesklet->icons = pIconsList;
	
	int iNbIcons = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_IS_SEPARATOR (pIcon))
			iNbIcons ++;
	}
	myData.iNbIconsInTree = iNbIcons;
	
	myData.iNbBranches = (int) ceil (1.*iNbIcons/3.);
	if (myData.iNbBranches == 0)
		return;
	
	double h = myDesklet->iHeight, w = myDesklet->iWidth;
	myData.fTreeWidthFactor = (w > TREE_WIDTH ? 1 : w / TREE_WIDTH);
	myData.fTreeHeightFactor = h / (myData.iNbBranches * TREE_HEIGHT);
	
	g_print (" -> %d icones, %d branches, proportions : %.2fx%.2f\n", myData.iNbIconsInTree, myData.iNbBranches, myData.fTreeWidthFactor, myData.fTreeHeightFactor);
	double fImageWidth = TREE_WIDTH * myData.fTreeWidthFactor, fImageHeight = TREE_HEIGHT * myData.fTreeHeightFactor;
	gchar *cImageFilePath = g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/branche1.svg", NULL);
	myData.pBrancheSurface[0] = cairo_dock_load_image (pCairoContext,
		cImageFilePath,
		&fImageWidth,
		&fImageHeight,
		0.,
		1.,
		FALSE);
	g_free (cImageFilePath);
	
	cImageFilePath = g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/branche2.svg", NULL);
	myData.pBrancheSurface[1] = cairo_dock_load_image (pCairoContext,
		cImageFilePath,
		&fImageWidth,
		&fImageHeight,
		0.,
		1.,
		FALSE);
	g_free (cImageFilePath);
}

void cd_shortcuts_draw_in_desklet (cairo_t *pCairoContext, gpointer data)
{
	double h = myDesklet->iHeight, w = myDesklet->iWidth;
	int i;
	for (i = 0; i < myData.iNbBranches; i ++)
	{
		cairo_save (pCairoContext);
		//g_print (" branche %d en (%.2f;%.2f)\n", i, (w - myData.fTreeWidthFactor * TREE_WIDTH) / 2, h - i * TREE_HEIGHT * myData.fTreeHeightFactor);
		cairo_translate (pCairoContext, (w - myData.fTreeWidthFactor * TREE_WIDTH) / 2, h - (i + 1) * TREE_HEIGHT * myData.fTreeHeightFactor);
		cairo_set_source_surface (pCairoContext, myData.pBrancheSurface[i%2], 0, 0);
		cairo_paint (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	
	int iBrancheNumber=0, iBrancheType=0, iLeafNumber=0;
	double x, y;
	int sens;
	Icon *pIcon;
	GList *ic;
	for (ic = myData.pDeskletIconList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_IS_SEPARATOR (pIcon))
		{
			x = s_iLeafPosition[iBrancheType][3*iLeafNumber];
			y = s_iLeafPosition[iBrancheType][3*iLeafNumber+1];
			sens = s_iLeafPosition[iBrancheType][3*iLeafNumber+2];
			
			pIcon->fDrawX = w / 2 + x * myData.fTreeWidthFactor - pIcon->fWidth / 2;
			pIcon->fDrawY = h - (iBrancheNumber * TREE_HEIGHT + y) * myData.fTreeHeightFactor - sens * pIcon->fHeight;
			pIcon->fScale = 1;
			pIcon->fAlpha = 1;
			pIcon->fWidthFactor = 1;
			pIcon->fHeightFactor = 1;
			
			cairo_save (pCairoContext);
			cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, myDesklet->iWidth);
			cairo_restore (pCairoContext);
			
			iLeafNumber ++;
			if (iLeafNumber == 3)
			{
				iLeafNumber = 0;
				iBrancheNumber ++;
				iBrancheType = iBrancheNumber % 2;
			}
		}
	}
}
