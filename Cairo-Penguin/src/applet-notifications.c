/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-animation.h"
#include "applet-notifications.h"

#define PENGUIN_NB_MESSAGES 13
static const gchar *s_pMessage[PENGUIN_NB_MESSAGES] = {
	N_("Hey, I'm here!"),
	N_("Sorry but I'm busy right now."),
	N_("I don't have time to play with you, I have to dig and mine all these icons."),
	N_("Your dock is so messy! Let me clean it."),
	N_("Admit my superiority on you as a penguin!"),
	N_("Wait, do you want to kill me?!"),
	N_("Do you know how painful it is to be clicked on??"),
	N_("It's my dock now, mwahahaha!"),
	N_("I want to be a pirate!"),
	N_("You shall not pass!"),
	N_("I'm your father!"),
	N_("- Gee, Brain, what do you want to do tonight?\n- The same thing we do every night, Pinky : try to take over the Dock!"),
	N_("For Aiur!")};


CD_APPLET_ON_CLICK_PROTO
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (penguin_is_resting (pAnimation))
		return GLDI_NOTIFICATION_LET_PASS;
	
	if ((myConfig.bFree && pClickedContainer == myContainer && myDock->container.iMouseX >  (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX && myDock->container.iMouseX < (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 +  myData.iCurrentPositionX + pAnimation->iFrameWidth && myDock->container.iMouseY > myContainer->iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight && myDock->container.iMouseY < myContainer->iHeight - myData.iCurrentPositionY) || (! myConfig.bFree && pClickedIcon == myIcon))
	{
		myData.iCurrentPositionY = (myConfig.bFree ? myDocksParam.iDockLineWidth : 0);
		PenguinAnimation *pAnimation = penguin_get_current_animation ();
		int iNewAnimation;
		int iRandom = g_random_int_range (0, 4);
		if (iRandom == 0)  // 1 chance sur 4.
			iNewAnimation = penguin_choose_go_up_animation (myApplet);
		else
			iNewAnimation = penguin_choose_next_animation (myApplet, pAnimation);
		penguin_set_new_animation (myApplet, iNewAnimation);
		
		cairo_dock_redraw_container (myContainer);  // si l'animation etait down, la nouvelle a pu nous placer ailleurs.
		
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
static void _keep_quiet (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	//\_______________ On arrete tout.
	if (myData.iSidRestartDelayed != 0)
	{
		g_source_remove (myData.iSidRestartDelayed);
		myData.iSidRestartDelayed = 0;
	}
	gldi_object_remove_notification (myContainer, NOTIFICATION_UPDATE_SLOW, (GldiNotificationFunc) penguin_update_container, myApplet);
	gldi_object_remove_notification (myIcon, NOTIFICATION_UPDATE_ICON_SLOW, (GldiNotificationFunc) penguin_update_icon, myApplet);
	
	//\_______________ On met l'animation de repos et on la dessine.
	int iNewAnimation = penguin_choose_resting_animation (myApplet);
	penguin_set_new_animation (myApplet, iNewAnimation);
	myData.iCurrentPositionY = (myConfig.bFree ? myDocksParam.iDockLineWidth : 0);
	if (myConfig.bFree)
	{
		penguin_move_in_dock (myApplet);
	}
	else
	{
		penguin_move_in_icon (myApplet);
	}
}
static void _wake_up (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	penguin_start_animating (myApplet);
}
gboolean on_build_container_menu (GldiModuleInstance *myApplet, Icon *pClickedIcon, GldiContainer *pClickedContainer, GtkWidget *pAppletMenu, gboolean *bDiscardMenu)
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if(pAnimation == NULL)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	if ((myConfig.bFree && pClickedContainer == myContainer && myDock->container.iMouseX >  (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX && myDock->container.iMouseX < (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 +  myData.iCurrentPositionX + pAnimation->iFrameWidth && myDock->container.iMouseY > myContainer->iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight && myDock->container.iMouseY < myContainer->iHeight - myData.iCurrentPositionY) || (! myConfig.bFree && pClickedIcon == myIcon))
	{
		if (pClickedIcon != myIcon)
		{
			gldi_object_notify (myContainer, NOTIFICATION_BUILD_CONTAINER_MENU, myIcon, myContainer, pAppletMenu, bDiscardMenu);
			gldi_object_notify (myContainer, NOTIFICATION_BUILD_ICON_MENU, myIcon, myContainer, pAppletMenu);
			return GLDI_NOTIFICATION_INTERCEPT;
		}
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

static gboolean s_bXPenguinsChecked = FALSE, s_bHasXPenguins = FALSE;

CD_APPLET_ON_BUILD_MENU_BEGIN
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if(pAnimation == NULL)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	if ((myConfig.bFree && pClickedContainer == myContainer && myDock->container.iMouseX >  (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX && myDock->container.iMouseX < (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 +  myData.iCurrentPositionX + pAnimation->iFrameWidth && myDock->container.iMouseY > myContainer->iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight && myDock->container.iMouseY < myContainer->iHeight - myData.iCurrentPositionY) || (! myConfig.bFree && pClickedIcon == myIcon))
	{
		if (pClickedIcon != myIcon)
		{
			return GLDI_NOTIFICATION_INTERCEPT;
		}
	}
	
	if (penguin_is_resting (pAnimation))
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Wake up"), MY_APPLET_SHARE_DATA_DIR"/icon.png", _wake_up, CD_APPLET_MY_MENU);
	}
	else
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Keep quiet"), MY_APPLET_SHARE_DATA_DIR"/icon.png",_keep_quiet, CD_APPLET_MY_MENU);
	}

	if (! s_bXPenguinsChecked)
	{
		s_bXPenguinsChecked = TRUE;
		gchar *cResult = cairo_dock_launch_command_sync ("which xpenguins");
		if (cResult != NULL && *cResult == '/')
			s_bHasXPenguins = TRUE;

		g_free (cResult);
	}

	if (s_bHasXPenguins)
	{
		CD_APPLET_ADD_IN_MENU(D_("Start XPenguins"), _start_xpenguins, CD_APPLET_MY_MENU);
		CD_APPLET_ADD_IN_MENU(D_("Stop XPenguins"), _stop_xpenguins, CD_APPLET_MY_MENU);
	}
CD_APPLET_ON_BUILD_MENU_END


gboolean CD_APPLET_ON_MIDDLE_CLICK_FUNC (GldiModuleInstance *myApplet, Icon *pClickedIcon, GldiContainer *pClickedContainer)
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if(pAnimation == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if ((myConfig.bFree && pClickedContainer == myContainer && myDock->container.iMouseX > (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX && myDock->container.iMouseX < (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 +  myData.iCurrentPositionX + pAnimation->iFrameWidth && myDock->container.iMouseY > myContainer->iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight && myDock->container.iMouseY < myContainer->iHeight - myData.iCurrentPositionY) || (! myConfig.bFree && pClickedIcon == myIcon))
	{
		if (myData.pDialog != NULL)
		{
			gldi_object_unref (GLDI_OBJECT(myData.pDialog));
			myData.pDialog = NULL;
		}
		PenguinAnimation *pAnimation = penguin_get_current_animation ();
		if (penguin_is_resting (pAnimation))
		{
			Icon *pIcon = cairo_dock_get_pointed_icon (myDock->icons);
			if (pIcon != NULL)
				myData.pDialog = gldi_dialog_show_temporary (D_("Zzzzz"), pIcon, myContainer, 2000);
			else
				myData.pDialog = gldi_dialog_show_general_message (D_("Zzzzz"), 2000);
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
				myData.pDialog = gldi_dialog_show_temporary ("Olll����� !", myIcon, myContainer, 2500);
			}
			else
			{
				iRandom = g_random_int_range (0, PENGUIN_NB_MESSAGES);  // [a;b[
				Icon *pIcon = cairo_dock_get_pointed_icon (myDock->icons);
				const gchar *cMessage = D_(s_pMessage[iRandom]);
				int iDuration = 2000 + 25 * g_utf8_strlen (cMessage, -1);
				if (pIcon != NULL)
					myData.pDialog = gldi_dialog_show_temporary (cMessage, pIcon, myContainer, iDuration);
				else
					myData.pDialog = gldi_dialog_show_general_message (cMessage, iDuration);
			}
		}
CD_APPLET_ON_MIDDLE_CLICK_END


gboolean cd_on_dock_destroyed (GldiModuleInstance *myApplet, CairoDock *pDock)
{
	gldi_module_delete_instance (myApplet);
	return GLDI_NOTIFICATION_LET_PASS;
}
