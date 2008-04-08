#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (_D("This is the switcher applet\n made by Cchumi for Cairo-Dock"))

static void _cd_switcher_reload (GtkMenuItem *menu_item, gpointer *data)
{
	g_source_remove (myData.loadaftercompiz);
	myData.loadaftercompiz = 0;
	
	cd_switcher_launch_measure ();  // asynchrone
}

void _cd_switcher_cairo_main_icon (int iMouseX, int iMouseY)
{
		double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);
		double fMaxSScale = (myIcon->pSubDock ? 1 + g_fAmplitude : 1);
		int fDrawX =  (int) myIcon->fDrawX;
		int fDrawY =  (int) myIcon->fDrawY;
		int MaxWidth = (int) myData.switcher.MaxWidthIcon;
		int MaxHeightbyLine = (int) myData.switcher.MaxNbLigne;
		int Maxdeskligne= (int) myData.switcher.NumDeskbyLigne;
		int i;

cd_message (" Bureau : %d",myData.switcher.iNbViewportX);
for (i = 0; i < myData.switcher.iNbViewportX; i ++)
	{
if (iMouseY - fDrawY > 0 && iMouseY - fDrawY < MaxHeightbyLine)		
		{
			if (iMouseX - fDrawX > 0 && i * MaxWidth < MaxWidth)
			{
cd_message (" 1ere Ligne   ");
cd_message (" Bureau : %d",i);

myData.switcher.iDesktopViewportX = i;
cd_message ("myData.switcher.iDesktopViewportX : %d",myData.switcher.iDesktopViewportX);
cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
			}
			else
			if (iMouseX - fDrawX >= i * MaxWidth && i * MaxWidth < i+i* MaxWidth)
			{
cd_message (" 1ere Ligne   ");
cd_message (" Bureau : %d",i);

myData.switcher.iDesktopViewportX = i;
cd_message ("myData.switcher.iDesktopViewportX : %d",myData.switcher.iDesktopViewportX);
cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
			}	
	}
	else
	{
		if (iMouseY - fDrawY>=MaxHeightbyLine && iMouseY - fDrawY<= 2*MaxHeightbyLine)
		
			{			if (iMouseX - fDrawX > 0 && i * MaxWidth < MaxWidth)
						{
cd_message (" 2eme Ligne   ");
cd_message (" Bureau  : %d",i);

myData.switcher.iDesktopViewportX = i +Maxdeskligne;
cd_message ("myData.switcher.iDesktopViewportX : %d",myData.switcher.iDesktopViewportX);
cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
						}
						else
						if (iMouseX - fDrawX >= i * MaxWidth && i * MaxWidth < i+i* MaxWidth)
						{
cd_message (" 2eme Ligne   ");
cd_message (" Bureau : %d",i);


myData.switcher.iDesktopViewportX = i +Maxdeskligne;
cd_message ("myData.switcher.iDesktopViewportX : %d",myData.switcher.iDesktopViewportX);
cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
						}
			}
	}
	}

}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myDock)
	{	
		gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
		g_print ("bDesktopIsVisible : %d\n", bDesktopIsVisible);
		cairo_dock_show_hide_desktop (! bDesktopIsVisible);
	}
CD_APPLET_ON_MIDDLE_CLICK_END

CD_APPLET_ON_CLICK_BEGIN


cairo_dock_get_nb_viewports (&myData.switcher.iNbViewportX, &myData.switcher.iNbViewportY);
myData.switcher.iDesktopViewportY = myData.switcher.iNbViewportY;


if (myConfig.bCurrentView && myDesklet == NULL) 
	{
			cd_message (" Main Icon :");
		int iMouseX =  myDock->iMouseX;
		int iMouseY =  myDock->iMouseY;
		_cd_switcher_cairo_main_icon(iMouseX, iMouseY);
		
		}
		else if (myConfig.bCurrentView && myDesklet != NULL) 
		{
			
cd_message (" Desklet :");
int iMouseX = - (int) myDesklet->diff_x;
		int iMouseY = - (int) myDesklet->diff_y;
_cd_switcher_cairo_main_icon(iMouseX, iMouseY);

		}
		else if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_DOCK_CONTAINER (myIcon->pSubDock))  // on a clique sur une icone du sous-dock.
				{

		cd_debug (" clic sur %s", pClickedIcon->acName);
myData.switcher.iDesktopViewportX = atoi (pClickedIcon->cQuickInfo);
//myData.switcher.iDesktopViewportX = myData.switcher.ScreenCurrentNums +1;
cd_message ("myData.switcher.iDesktopViewportX : %d",myData.switcher.iDesktopViewportX);
cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);



				}

else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;


CD_APPLET_ON_CLICK_END	

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("switcher", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_("Reload now"), _cd_switcher_reload, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END


