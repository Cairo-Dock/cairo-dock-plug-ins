/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-animation.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("Cairo-Penguin", 1, 6, 2, CAIRO_DOCK_CATEGORY_ACCESSORY)


CD_APPLET_INIT_BEGIN
	penguin_load_theme (myApplet, myConfig.cThemePath);
	
	penguin_start_animating_with_delay (myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	g_source_remove (myData.iSidAnimation);
	myData.iSidAnimation = 0;
	
	if (myData.iSidRestartDelayed != 0)
	{
		g_source_remove (myData.iSidRestartDelayed);
		myData.iSidRestartDelayed = 0;
	}
	
	gulong iOnExposeCallbackID = g_signal_handler_find (G_OBJECT (myContainer->pWidget),
		G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		0,
		0,
		NULL,
		penguin_draw_on_dock,
		myApplet);
	if (iOnExposeCallbackID > 0)
		g_signal_handler_disconnect (G_OBJECT (myContainer->pWidget), iOnExposeCallbackID);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On stoppe tout.
		g_source_remove (myData.iSidAnimation);
		myData.iSidAnimation = 0;
		if (myData.iSidRestartDelayed != 0)
		{
			g_source_remove (myData.iSidRestartDelayed);
			myData.iSidRestartDelayed = 0;
		}
		
		//\_______________ On efface sa derniere position.
		PenguinAnimation *pAnimation = penguin_get_current_animation ();
		if (pAnimation != NULL)
		{
			GdkRectangle area;
			area.x = (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX;
			area.y = myDock->iCurrentHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight;
			area.width = pAnimation->iFrameWidth;
			area.height = pAnimation->iFrameHeight + myDock->bUseReflect * g_fReflectSize;
#ifdef HAVE_GLITZ
			if (myContainer->pDrawFormat && myContainer->pDrawFormat->doublebuffer)
				gtk_widget_queue_draw (myContainer->pWidget);
			else
#endif
			gdk_window_invalidate_rect (myContainer->pWidget->window, &area, FALSE);
		}
		
		/// Si le dock a change, enlever le redessin sur pOldContainer et le remettre sur le nouveau ...
		
		//\_______________ On recharge tout de zero (changement de theme).
		reset_data (myApplet);  // applet multi-instance => ok.
		
		penguin_load_theme (myApplet, myConfig.cThemePath);
		
		//\_______________ On libere le pingouin ou au contraire on le cloisonne.
		if (myConfig.bFree)
		{
			cairo_dock_detach_icon_from_dock (myIcon, myDock, g_bUseSeparator);
			cairo_dock_update_dock_size (myDock);
		}
		else
		{
			cairo_dock_insert_icon_in_dock (myIcon, myDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
		}
		
		penguin_start_animating (myApplet);
	}
	else
	{
		// rien a faire, la taille du pinguoin est fixe.
	}
CD_APPLET_RELOAD_END
