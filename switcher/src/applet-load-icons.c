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

static GList *s_pIconList = NULL;





static GList * _load_icons (void)
{	
	
	myData.pIconList = NULL;
	Icon *pIcon;	
	int i;

cd_switcher_get_current_desktop (&myData.switcher.ScreenCurrentSizes, &myData.switcher.ScreenCurrentNums);

for (i = 0; i < myData.switcher.iNbViewportX; i ++)
	{
cd_message ("  myData.switcher.iNbViewportX : %d",myData.switcher.iNbViewportX);
	pIcon = g_new0 (Icon, 1);
	if (myData.switcher.ScreenCurrentNums == i)
		{
	pIcon->acName = g_strdup_printf ("Courant %d",i);
	pIcon->acFileName = g_strdup_printf ("%s/workspaces.svg", MY_APPLET_SHARE_DATA_DIR);
	pIcon->cQuickInfo = g_strdup_printf ("%d",i);
	pIcon->fScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->acCommand = g_strdup ("none");
	pIcon->cParentDockName = g_strdup (myIcon->acName);
}
else
		{
	pIcon->acName = g_strdup_printf ("Bureau %d",i);
	pIcon->acFileName = g_strdup_printf ("%s/workspaces.svg", MY_APPLET_SHARE_DATA_DIR);
	pIcon->cQuickInfo = g_strdup_printf ("%d",i);
	pIcon->fScale = 1.;
	pIcon->fAlpha = 0.3;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->acCommand = g_strdup ("none");
	pIcon->cParentDockName = g_strdup (myIcon->acName);
	}
	myData.pIconList = g_list_append (myData.pIconList, pIcon);
//_add_icon (i);



      }

	
	return myData.pIconList;

}

gboolean cd_switcher_timer (gpointer data)
{
	cd_switcher_launch_measure ();
	return TRUE;
}


static gboolean _cd_switcher_check_for_redraw (gpointer data)
{
	cd_switcher_get_current_desktop (&myData.switcher.ScreenCurrentSizes, &myData.switcher.ScreenCurrentNums);
		//\_______________________ On recharge l'icone principale.
		if (myConfig.bCurrentView)
		{
			
if (myIcon->pSubDock != NULL)
{
g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;

cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
					myIcon->pSubDock = NULL;

					}
				
						cd_message ("SWITCHER : chargement de l'icone Switcher sans sous dock");

_cd_switcher_check_for_redraw_cairo(NULL);

}
else
{
	
		//\_______________________ On cree la liste des icones de prevision.
		GList *pIconList = _load_icons ();
		
		//\_______________________ On efface l'ancienne liste.

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
					
					cd_message ("SWITCHER : creation du sous-dock Switcher");
					CD_APPLET_CREATE_MY_SUBDOCK (pIconList, myConfig.cRenderer)
					}
				
			}
			else  // on a deja notre sous-dock, on remplace juste ses icones.
			{
				cd_message ("SWITCHER : rechargement du sous-dock Switcher");
				if (pIconList == NULL)  // inutile de le garder.
				{
					CD_APPLET_DESTROY_MY_SUBDOCK
				}
				else
				{
					CD_APPLET_LOAD_ICONS_IN_MY_SUBDOCK (pIconList)
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
				cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, myContainer->bIsHorizontal, myConfig.bDesklet3D, myContainer->bDirectionUp);
				myData.iMaxIconWidth = MAX (myData.iMaxIconWidth, icon->fWidth);
				}

			}

	if (myData.LoadAfterCompiz != 0)//\_______________________ On Tue le Timer.
{
cd_message ("SWITCHER : timer à 0 ");
		g_source_remove (myData.LoadAfterCompiz);
	myData.LoadAfterCompiz = 0;
}
	//}
}
if (myConfig.bDisplayNumDesk)
			{

				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.switcher.ScreenCurrentNums)
	cd_message ("SWITCHER : chargement de quick info %d ", myData.switcher.ScreenCurrentNums);

			}
			else
			{

cd_message ("SWITCHER : chargement de quick info NULL");
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL)

	
			}

CD_APPLET_REDRAW_MY_ICON

}

void cd_switcher_launch_measure (void)
{

_cd_switcher_check_for_redraw (NULL);
	
}
