/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>
#include <glib.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
#include "applet-draw.h"
#include "applet-init.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;
//extern SwitcherApplet mySwitcher;

static GList *s_pIconList = NULL;
static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;

#define _add_icon(i)\
	pIcon = g_new0 (Icon, 1);\
	if (myData.switcher.ScreenCurrentNums == i)\
		{\
	printf("current desks : %d \n", myData.switcher.ScreenCurrentNums);\
	pIcon->acName = g_strdup_printf ("Courant %d",i);\
	pIcon->acFileName = g_strdup_printf ("%s/workspaces.svg", MY_APPLET_SHARE_DATA_DIR);\
	pIcon->cQuickInfo = g_strdup_printf ("%d",i);\
	pIcon->fScale = 1.;\
	pIcon->fAlpha = .1;\
	pIcon->fWidthFactor = 1.;\
	pIcon->fHeightFactor = 1.;\
	pIcon->acCommand = g_strdup ("none");\
	pIcon->cParentDockName = g_strdup (myIcon->acName);\
}\
else\
		{\
	pIcon->acName = g_strdup_printf ("Bureau %d",i);\
	pIcon->acFileName = g_strdup_printf ("%s/workspaces.svg", MY_APPLET_SHARE_DATA_DIR);\
	pIcon->cQuickInfo = g_strdup_printf ("%d",i);\
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
	
	GList *pIconList = NULL;
	Icon *pIcon;	
	int i;

myData.switcher.ScreenCurrentSizes=0;
myData.switcher.ScreenCurrentNums  = 0;

cd_switcher_get_current_desktop (&myData.switcher.ScreenCurrentSizes, &myData.switcher.ScreenCurrentNums);

for (i = 0; i < g_iNbDesktops; i ++)
	{

_add_icon (i);



      }

	
	return pIconList;

}

gboolean cd_switcher_timer (gpointer data)
{
	cd_switcher_launch_measure ();
	return TRUE;
}


static gboolean _cd_switcher_check_for_redraw (gpointer data)
{
	
		//\_______________________ On recharge l'icone principale.
		if (myConfig.bCurrentView)
		{

					cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
					myIcon->pSubDock = NULL;
				
						cd_message ("  chargement de l'icone Switcher sans sous dock");



			switcher_draw_main_dock_icon (myData.pSurface);

			if (myConfig.bDisplayNumDesk)
			{

				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", myData.switcher.ScreenCurrentNums)
	cd_message ("  chargement de quick info %d ", myData.switcher.ScreenCurrentNums);

			}
			else
			{
cd_message ("  chargement de quick info NULL");
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)

	
			}
}
else
{
	
		//\_______________________ On cree la liste des icones de prevision.
		GList *pIconList = _load_icons ();
		
		//\_______________________ On efface l'ancienne liste.
		if (myData.pDeskletIconList != NULL)
		{
			g_list_foreach (myData.pDeskletIconList, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myData.pDeskletIconList);
			myData.pDeskletIconList = NULL;
			myData.iNbIcons = 0;
			myData.iMaxIconWidth = 0;
			myDesklet->icons = NULL;
		}
		if (myIcon->pSubDock != NULL)
		{
			g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
		}
		
		//\_______________________ On charge la nouvelle liste.
		if (myDock != NULL)  
		{
			if (myIcon->pSubDock == NULL)
			{	
				
					if (pIconList != NULL) 
					{
					
					cd_message ("  creation du sous-dock Switcher");
					myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
					cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
					cairo_dock_update_dock_size (myIcon->pSubDock);
					}
				
			}
			else  // on a deja notre sous-dock, on remplace juste ses icones.
			{
				cd_message ("  rechargement du sous-dock Switcher");
				if (pIconList == NULL)  // inutile de le garder.
				{
					cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
					myIcon->pSubDock = NULL;
				}
				else
				{
					myIcon->pSubDock->icons = pIconList;
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
			myData.pDeskletIconList = pIconList;
			myDesklet->icons = pIconList;
			myData.iNbIcons = g_list_length (myData.pDeskletIconList);
			GList* ic;
			Icon *icon;
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
			for (ic = pIconList; ic != NULL; ic = ic->next)
			{
				icon = ic->data;
				if (myConfig.bDesklet3D)
				{
					icon->fWidth = 0;
					icon->fHeight = 0;
				}
				else
				{
					icon->fWidth = MAX (1, .2 * myDesklet->iWidth - g_iLabelSize);
					icon->fHeight = MAX (1, .2 * myDesklet->iHeight - g_iLabelSize);
				}
				cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, CAIRO_DOCK_HORIZONTAL, myConfig.bDesklet3D);
				myData.iMaxIconWidth = MAX (myData.iMaxIconWidth, icon->fWidth);
			}
			//cairo_destroy (pCairoContext);
			gtk_widget_queue_draw (myDesklet->pWidget);
		}
		
		//\_______________________ On lance le timer si necessaire.
		if (myData.iSidTimer == 0)
			myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_switcher_timer, NULL);
		return FALSE;
}

	return TRUE;
}

void cd_switcher_launch_measure (void)
{

cd_message ("dfdfdfdfdf");

_cd_switcher_check_for_redraw (NULL);
	
}
