/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>
#include "applet-struct.h"
#include <glib.h>

#include "applet-load-icons.h"
#include "applet-read-data.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;
//extern SwitcherApplet mySwitcher;

static GList *s_pIconList = NULL;
static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;

#define _add_icon(i)\
	pIcon = g_new0 (Icon, 1);\
	if (ScreenCurrentNums == i)\
		{\
	printf("current desks : %d \n", ScreenCurrentNums);\
	pIcon->acName = g_strdup_printf ("Courant %d",i);\
	pIcon->cQuickInfo = g_strdup_printf ("%d",i);;\
	pIcon->acFileName = g_strdup_printf ("workspaces.svg");\
	pIcon->fScale = 1.;\
	pIcon->fAlpha = 1.;\
	pIcon->fWidthFactor = 1.;\
	pIcon->fHeightFactor = 1.;\
	pIcon->acCommand = g_strdup ("none");\
	pIcon->cParentDockName = g_strdup (myIcon->acName);\
}\
else\
		{\
	pIcon->acName = g_strdup_printf ("Bureau %d",i);\
	pIcon->cQuickInfo = g_strdup_printf ("%d",i);\
	pIcon->acFileName = g_strdup_printf ("workspaces.svg");\
	pIcon->fScale = 1.;\
	pIcon->fAlpha = 1.;\
	pIcon->fWidthFactor = 1.;\
	pIcon->fHeightFactor = 1.;\
	pIcon->acCommand = g_strdup ("none");\
	pIcon->cParentDockName = g_strdup (myIcon->acName);\
	}\
	pIconList = g_list_append (pIconList, pIcon);



static GList * _load_icons (void)
{	
	//GList *pDeskList = NULL;

	GList *pIconList = NULL;
	Icon *pIcon;	
	int i;
	int ScreenCurrentSizes=0, ScreenCurrentNums  = 0;
//cairo_dock_get_current_viewport (ScreenCurrentSize, ScreenCurrentNum);
cd_switcher_get_current_desktop (&ScreenCurrentSizes, &ScreenCurrentNums);

//int essai = ScreenCurrentNums+1;
	//printf("current desks : %d \n", essai);
for (i = 0; i < g_iNbDesktops; i ++)
	{

_add_icon (i);
//printf("picon name alpha low : %s \n", pIcon->acName);
//printf("alpha  : %f \n", pIcon->fAlpha);


      }

	
	return pIconList;

}


gpointer cd_switcher_threaded_calculation (gpointer data)
{

GError *erreur = NULL;

//int ScreenCurrentSizes=0, ScreenCurrentNums  = 0;
//cairo_dock_get_current_viewport (ScreenCurrentSize, ScreenCurrentNum);
//cd_switcher_get_current_desktop (&ScreenCurrentSizes, &ScreenCurrentNums);

//guint interval = 4;
//	 g_timeout_add_seconds(interval,(s_pIconList =_load_icons()), NULL);
s_pIconList =_load_icons();
int *iNumberOfDesktops;
int *iDesktopNumber;
//printf("Nb Desktop : %d \n", &iNumberOfDesktops);
//printf("Curr Desktop : %d \n", desknum);
//printf("num Viewport Y : %d \n", ScreenCurrentSize);
//printf("num Viewport X : %d \n", ScreenCurrentNum);
//printf("Num workspace : %d \n", iDesktopNumber);


//printf("Num workspace Y : %d \n", ScreenNbRows);
//printf("Num workspace X : %d \n", ScreenNbCols);

	if (g_iNbDesktops != 0)
	{
	
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
		
	}
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread");
	return NULL;
}




static gboolean _cd_switcher_check_for_redraw (gpointer data)
{
		int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning)
	{
		s_iSidTimerRedraw = 0;
		cd_message ("  chargement du sous-dock des raccourcis");
		
		if (s_pIconList != NULL || ! myData.g_iNbDesktops)  // l'applet peut faire "show desktop".
		{
			myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (s_pIconList, myIcon->acName);
			cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
			cairo_dock_update_dock_size (myIcon->pSubDock);
		}


		return FALSE;
	}
	return TRUE;
}

void cd_switcher_launch_measure (void)
{
gboolean g_bAppliOnCurrentDesktopOnly;
if (g_bAppliOnCurrentDesktopOnly != 0)
{
	cd_message (" ==> lancement du thread de calcul chgt desktop %d:", g_bAppliOnCurrentDesktopOnly);
		s_pIconList = NULL;
		
		if (s_iSidTimerRedraw == 0)
			s_iSidTimerRedraw = g_timeout_add (50, (GSourceFunc) _cd_switcher_check_for_redraw, (gpointer) NULL);
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_switcher_threaded_calculation,
			NULL,
			FALSE,
			&erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
}
	cd_message ("");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1))  // il etait egal a 0, on lui met 1 et on lance le thread.
	{
		cd_message (" ==> lancement du thread de calcul");
		s_pIconList = NULL;
		
		if (s_iSidTimerRedraw == 0)
			s_iSidTimerRedraw = g_timeout_add (50, (GSourceFunc) _cd_switcher_check_for_redraw, (gpointer) NULL);
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_switcher_threaded_calculation,
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



