/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-animation.h"
#include "applet-notifications.h"

#define PENGUIN_NB_MESSAGES 13
static gchar *s_pMessage[PENGUIN_NB_MESSAGES] = {
	N_("Hey, I'm here !"),
	N_("Sorry but I'm busy right now."),
	N_("I don't have time to play with you, I have to dig and mine all these icons."),
	N_("Your dock is so messy ! Let me clean it."),
	N_("Admit my superiority on you as a penguin !"),
	N_("Wait, do you want to kill me ?!"),
	N_("Do you know how much painful it is to be clicked on ??"),
	N_("It's my dock now, mwahahaha !"),
	N_("I want to be a pirate !"),
	N_("You shall not pass !"),
	N_("I'm your father !"),
	N_("- What will we do tonight Cortex ?\n- The same thing as every nights, Minus. Try to take over the Dock !"),
	N_("For Aiur !")};


CD_APPLET_ON_CLICK_PROTO
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (penguin_is_resting (pAnimation))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if ((myConfig.bFree && pClickedContainer == myContainer && myDock->iMouseX >  (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX && myDock->iMouseX < (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 +  myData.iCurrentPositionX + pAnimation->iFrameWidth && myDock->iMouseY > myContainer->iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight && myDock->iMouseY < myContainer->iHeight - myData.iCurrentPositionY) || (! myConfig.bFree && pClickedIcon == myIcon))
	{
		myData.iCurrentPositionY = (myConfig.bFree ? g_iDockLineWidth : 0);
		PenguinAnimation *pAnimation = penguin_get_current_animation ();
		int iNewAnimation;
		int iRandom = g_random_int_range (0, 4);
		if (iRandom == 0)  // 1 chance sur 4.
			iNewAnimation = penguin_choose_go_up_animation (myApplet);
		else
			iNewAnimation = penguin_choose_next_animation (myApplet, pAnimation);
		penguin_set_new_animation (myApplet, iNewAnimation);
		
		cairo_dock_stop_icon_animation (pClickedIcon);
CD_APPLET_ON_CLICK_END


static void _start_xpenguins (GtkMenuItem *menu_item, gpointer *data)
{
	cairo_dock_launch_command ("xpenguins");
}
static void _stop_xpenguins (GtkMenuItem *menu_item, gpointer *data)
{
	cairo_dock_launch_command ("xpenguins-stop");
}
static void _keep_quiet (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	//\_______________ On arrete tout.
	if (myData.iSidRestartDelayed != 0)
	{
		g_source_remove (myData.iSidRestartDelayed);
		myData.iSidRestartDelayed = 0;
	}
	cairo_dock_remove_notification_func_on_container (myContainer, CAIRO_DOCK_UPDATE_DOCK_SLOW, (CairoDockNotificationFunc) penguin_update_container, myApplet);
	cairo_dock_remove_notification_func_on_icon (myIcon, CAIRO_DOCK_UPDATE_ICON_SLOW, (CairoDockNotificationFunc) penguin_update_icon, myApplet);
	
	//\_______________ On met l'animation de repos et on la dessine.
	int iNewAnimation = penguin_choose_resting_animation (myApplet);
	penguin_set_new_animation (myApplet, iNewAnimation);
	myData.iCurrentPositionY = (myConfig.bFree ? g_iDockLineWidth : 0);
	if (myConfig.bFree)
	{
		penguin_move_in_dock (myApplet);
	}
	else
	{
		penguin_move_in_icon (myApplet);
	}
}
static void _wake_up (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	penguin_start_animating (myApplet);
}
CD_APPLET_ON_BUILD_MENU_PROTO
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if(pAnimation == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if ((myConfig.bFree && pClickedContainer == myContainer && myDock->iMouseX >  (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX && myDock->iMouseX < (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 +  myData.iCurrentPositionX + pAnimation->iFrameWidth && myDock->iMouseY > myContainer->iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight && myDock->iMouseY < myContainer->iHeight - myData.iCurrentPositionY) || (! myConfig.bFree && pClickedIcon == myIcon))
	{
		if (pClickedIcon != myIcon && ! (CAIRO_DOCK_IS_APPLET (pClickedIcon) && pClickedIcon->pModuleInstance->pModule == myIcon->pModuleInstance->pModule))
		{
			g_print ("%s\n", myApplet->cConfFilePath);
			cairo_dock_notify (CAIRO_DOCK_BUILD_MENU, myIcon, myContainer, CD_APPLET_MY_MENU);
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		}
		
		GtkWidget *pMenuItem, *image;
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
		
		GtkWidget *pModuleSubMenu = CD_APPLET_ADD_SUB_MENU (D_("Hey, you there !"), CD_APPLET_MY_MENU);
		//GtkWidget *pModuleSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		if (penguin_is_resting (pAnimation))
		{
			CD_APPLET_ADD_IN_MENU(D_("Wake up"), _wake_up, pModuleSubMenu);
		}
		else
		{
			CD_APPLET_ADD_IN_MENU(D_("Keep quiet"), _keep_quiet, pModuleSubMenu);
		}
		
		CD_APPLET_ADD_IN_MENU(D_("Start XPenguins"), _start_xpenguins, pModuleSubMenu);
		CD_APPLET_ADD_IN_MENU(D_("Stop XPenguins"), _stop_xpenguins, pModuleSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pModuleSubMenu);
CD_APPLET_ON_BUILD_MENU_END


gboolean CD_APPLET_ON_MIDDLE_CLICK_FUNC (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer)
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if(pAnimation == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if ((myConfig.bFree && pClickedContainer == myContainer && myDock->iMouseX >  (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX && myDock->iMouseX < (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 +  myData.iCurrentPositionX + pAnimation->iFrameWidth && myDock->iMouseY > myContainer->iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight && myDock->iMouseY < myContainer->iHeight - myData.iCurrentPositionY) || (! myConfig.bFree && pClickedIcon == myIcon))
	{
		if (myData.pDialog != NULL)
		{
			cairo_dock_dialog_unreference (myData.pDialog);
			myData.pDialog = NULL;
		}
		PenguinAnimation *pAnimation = penguin_get_current_animation ();
		if (penguin_is_resting (pAnimation))
		{
			Icon *pIcon = cairo_dock_get_pointed_icon (myDock->icons);
			if (pIcon != NULL)
				myData.pDialog = cairo_dock_show_temporary_dialog (D_("Zzzzz"), pIcon, myContainer, 2000);
			else
				myData.pDialog = cairo_dock_show_general_message (D_("Zzzzz"), 2000);
		}
		else if (! pAnimation->bEnding && myData.iSidRestartDelayed == 0)
		{
			int iRandom = g_random_int_range (0, 5);  // [a;b[
			if (iRandom == 0)  // 1 chance sur 5.
			{
				int iNewAnimation = penguin_choose_ending_animation (myApplet);
				penguin_set_new_animation (myApplet, iNewAnimation);
			}
			else if (iRandom == 1 && ! myConfig.bFree)
			{
				CD_APPLET_ANIMATE_MY_ICON ("bounce", 3);
				myData.pDialog = cairo_dock_show_temporary_dialog ("Olllééééé !", myIcon, myContainer, 2500);
			}
			else
			{
				iRandom = g_random_int_range (0, PENGUIN_NB_MESSAGES);  // [a;b[
				Icon *pIcon = cairo_dock_get_pointed_icon (myDock->icons);
				const gchar *cMessage = D_(s_pMessage[iRandom]);
				int iDuration = 1000 + 25 * g_utf8_strlen (cMessage, -1);
				if (pIcon != NULL)
					myData.pDialog = cairo_dock_show_temporary_dialog (cMessage, pIcon, myContainer, iDuration);
				else
					myData.pDialog = cairo_dock_show_general_message (cMessage, iDuration);
			}
		}
CD_APPLET_ON_MIDDLE_CLICK_END
