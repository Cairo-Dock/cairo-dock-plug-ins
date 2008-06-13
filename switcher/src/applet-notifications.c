#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (_D("This is the switcher applet\n made by Cchumi for Cairo-Dock"))


/* Fonction maitre de recuperation des coordonnées de clique pour la vue simplifiée.*/
/*void _cd_switcher_cairo_main_icon (int iMouseX, int iMouseY)
{
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	double fMaxSScale = (myIcon->pSubDock ? 1 + g_fAmplitude : 1);
	int fDrawX =  (int) myIcon->fDrawX;
	int fDrawY =  (int) myIcon->fDrawY;
	int MaxWidth = (int) myData.switcher.fOneViewportWidth;
	int MaxHeightbyLine = (int) myData.switcher.fOneViewportHeight;
	int Maxdeskligne= (int) myData.switcher.iNbDesktopByLine;
	int i;
	
	for (i = 0; i < myData.switcher.iNbViewportX; i ++)
	{
		if (iMouseY - fDrawY > 0 && iMouseY - fDrawY < MaxHeightbyLine)		
		{
			if (iMouseX - fDrawX > 0 && i * MaxWidth < MaxWidth)
			{
				cd_message ("SWITCHER : 1ere Ligne, Bureau : %d",i);
				
				myData.switcher.iDesktopViewportX = i;
				
				cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
			}
			else if (iMouseX - fDrawX >= i * MaxWidth && i * MaxWidth < i+i* MaxWidth)
			{
				cd_message ("SWITCHER : 1ere Ligne, Bureau : %d",i);
				
				myData.switcher.iDesktopViewportX = i;
				
				cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
			}
		}
		else
		{
			if (iMouseY - fDrawY>=MaxHeightbyLine && iMouseY - fDrawY<= 2*MaxHeightbyLine)
			{
				if (iMouseX - fDrawX > 0 && i * MaxWidth < MaxWidth)
				{
					cd_message ("SWITCHER : 2eme Ligne, Bureau  : %d",i);
					
					myData.switcher.iDesktopViewportX = i +Maxdeskligne;
					
					cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
				}
				else if (iMouseX - fDrawX >= i * MaxWidth && i * MaxWidth < i+i* MaxWidth)
				{
					cd_message ("SWITCHER : 2eme Ligne, Bureau : %d",i);
					
					
					myData.switcher.iDesktopViewportX = i +Maxdeskligne;
					
					cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
				}
			}
		}
	}
CD_APPLET_REDRAW_MY_ICON
}*/

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myDock)
	{
		gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
		cairo_dock_show_hide_desktop (! bDesktopIsVisible);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_CLICK_BEGIN
	
	int iNumLine, iNumColumn;
	int iNumDesktop, iNumViewportX, iNumViewportY;
	int iMouseX, iMouseY;
	if (myDock)
	{
		iMouseX = myDock->iMouseX - myIcon->fDrawX;
		iMouseY = myDock->iMouseY - myIcon->fDrawY;
	}
	else
	{
		iMouseX = - myDesklet->diff_x - myIcon->fDrawX;
		iMouseY = - myDesklet->diff_y - myIcon->fDrawY;
	}
	
	if (myConfig.bCompactView && pClickedIcon == myIcon)
	{
		if (iMouseX < 0)
			iMouseX = 0;
		if (iMouseY < 0)
			iMouseY = 0;
		if (iMouseX > myIcon->fWidth * myIcon->fScale)
			iMouseX = myIcon->fWidth * myIcon->fScale;
		if (iMouseY > myIcon->fHeight * myIcon->fScale)
			iMouseY = myIcon->fHeight * myIcon->fScale;
		iNumLine = (int) (iMouseY / (myIcon->fHeight * myIcon->fScale) * myData.switcher.iNbLines);
		iNumColumn = (int) (iMouseX / (myIcon->fWidth * myIcon->fScale) * myData.switcher.iNbColumns);
		cd_switcher_compute_desktop_from_coordinates (iNumLine, iNumColumn, &iNumDesktop, &iNumViewportX, &iNumViewportY);
		myIcon->iCount = 0;
	}
	else if (pClickedIcon != NULL)
	{
		int iIndex = pClickedIcon->fOrder;
		cd_switcher_compute_viewports_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
		pClickedIcon->iCount = 0;
	}
	
	if (iNumDesktop != myData.switcher.iCurrentDesktop)
		cairo_dock_set_current_desktop (iNumDesktop);
	if (iNumViewportX != myData.switcher.iCurrentViewportX || iNumViewportY != myData.switcher.iCurrentViewportY)
		cairo_dock_set_current_viewport (iNumViewportX, iNumViewportY);
	
	/*myData.switcher.iDesktopViewportY = myData.switcher.iNbViewportY;
	
	if (myConfig.bCompactView && myDesklet == NULL) 
	{
		cd_message ("SWITCHER : Main Icon :");
		int iMouseX =  myDock->iMouseX;
		int iMouseY =  myDock->iMouseY;
		_cd_switcher_cairo_main_icon(iMouseX, iMouseY);
		myIcon->iCount = 0;
	}
	else if (myDesklet != NULL)
	{
		if (myConfig.bCompactView)
		{
			cd_message ("SWITCHER : Desklet :");
			int iMouseX = - (int) myDesklet->diff_x;
			int iMouseY = - (int) myDesklet->diff_y;
			_cd_switcher_cairo_main_icon(iMouseX, iMouseY);
		}
		else
		{
			cd_debug ("SWITCHER : clic sur %s", pClickedIcon->fOrder);
			
			myData.switcher.iDesktopViewportX = (int)pClickedIcon->fOrder;
			printf("myIcon->fOrder %f \n",pClickedIcon->fOrder);
			printf("myData.switcher.iDesktopViewportX %d \n",myData.switcher.iDesktopViewportX);
			
			cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
			CD_APPLET_REDRAW_MY_ICON
		}
	}
	else if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_CONTAINER (myIcon->pSubDock))  // on a clique sur une icone du sous-dock.
	{
		cd_debug ("SWITCHER : clic sur %s", pClickedIcon->fOrder);
		
		myData.switcher.iDesktopViewportX = (int)pClickedIcon->fOrder; //on recupere son ordre et on switch
		printf("myIcon->fOrder %f \n",pClickedIcon->fOrder);
		printf("myData.switcher.iDesktopViewportX %d \n",myData.switcher.iDesktopViewportX);
		cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;*/
	
CD_APPLET_ON_CLICK_END	


static void _cd_switcher_add_desktop (GtkMenuItem *menu_item, Icon *pIcon)
{
	cd_switcher_add_a_desktop ();
}
static void _cd_switcher_remove_last_desktop (GtkMenuItem *menu_item, Icon *pIcon)
{
	cd_switcher_remove_last_desktop ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("switcher", pSubMenu, CD_APPLET_MY_MENU)
		//CD_APPLET_ADD_IN_MENU (_("Reload now"), _cd_switcher_reload, pSubMenu)
		CD_APPLET_ADD_IN_MENU (_("Add a desktop"), _cd_switcher_add_desktop, pSubMenu)
		CD_APPLET_ADD_IN_MENU (_("Remove last desktop"), _cd_switcher_remove_last_desktop, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END


