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
#include "gdk/gdkx.h"

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-app.h"
#include "applet-notifications.h"

#define FORCE_REMOVE_DOUBLE_SEPARATORS

#ifdef FORCE_REMOVE_DOUBLE_SEPARATORS
/// REMOVE ME WHEN IT'S POSSIBLE! :)
static void _remove_double_separators (GtkWidget *pWidget)
{
	if (pWidget == NULL)
		return;

	gboolean bPrevIsSeparator = TRUE; // to remove the first entry if it's a separator

	GList *pChildren = gtk_container_get_children (GTK_CONTAINER (pWidget));
	GList *ic;
	GtkWidget *pCurrentWidget;
	GtkWidget *pSubMenu;
	for (ic = pChildren; ic != NULL; ic = ic->next)
	{
		pCurrentWidget = ic->data;
		if (GTK_IS_SEPARATOR_MENU_ITEM (pCurrentWidget))
		{
			if (bPrevIsSeparator)
				gtk_widget_destroy (pCurrentWidget); // or ? gtk_container_remove (pContainer, pCurrentWidget);
			bPrevIsSeparator = TRUE;
		}
		else if (GTK_IS_MENU_ITEM (pCurrentWidget))
		{
			pSubMenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (pCurrentWidget));
			if (pSubMenu != NULL)
			{
				bPrevIsSeparator = TRUE;
				_remove_double_separators (pSubMenu);
			}
			else
				bPrevIsSeparator = FALSE;
		}
		else
			bPrevIsSeparator = FALSE;
	}
	g_list_free (pChildren);
}
#endif

static void _show_menu (gboolean bOnMouse)
{
	if (! myConfig.bDisplayMenu)
		return;

	if (myData.pMenu != NULL)
	{
		#ifdef FORCE_REMOVE_DOUBLE_SEPARATORS
		_remove_double_separators (myData.pMenu);
		#endif
		if (bOnMouse)
		{
			gtk_widget_show_all (myData.pMenu);
			gtk_menu_popup_at_pointer (GTK_MENU (myData.pMenu), NULL);
		}
		else
		{
			CD_APPLET_POPUP_MENU_ON_MY_ICON (myData.pMenu);
		}
	}
	else  /// either show a message, or remember the user demand, so that we pop the menu as soon as we get it...
	{
		gldi_dialog_show_temporary_with_icon (D_("The application didn't send its menu to us."), myIcon, myContainer, 4000., "same icon");
	}
}

//\___________ Action on click: show the menu
CD_APPLET_ON_CLICK_BEGIN
	if (myData.pCurrentWindow == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	if (myConfig.bDisplayControls)
	{
		int iNumButton = cd_app_menu_find_button (myApplet);
		if (iNumButton >= 0)
		{
			switch (iNumButton)
			{
				case CD_BUTTON_MENU:
					_show_menu (FALSE);
				break;
				case CD_BUTTON_MINIMIZE:
					if (myData.bCanMinimize)
						gldi_window_minimize (myData.pCurrentWindow);
				break;
				case CD_BUTTON_MAXIMIZE:
					if (myData.bCanMaximize)
					{
						gldi_window_maximize (myData.pCurrentWindow, ! myData.pCurrentWindow->bIsMaximized);
					}
				break;
				case CD_BUTTON_CLOSE:
					if (myData.bCanClose)
						gldi_window_close (myData.pCurrentWindow);
				break;
			}
		}
	}
	else if (myConfig.bDisplayMenu)
		_show_menu (FALSE);
CD_APPLET_ON_CLICK_END


//\___________ Other actions are defined to mime the usual actions available on windows top border
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	// set the window behind all the others.
	GldiWindowActor *actor = gldi_windows_get_active();
	if (actor)
		gldi_window_lower (actor);
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_SCROLL_BEGIN
	// minimize the window (we could also use the scroll to (un)shade the window, but I'm afraid that a maximized shaded window would be too much hidden, users could be confused).
	GldiWindowActor *actor = gldi_windows_get_active();
	if (actor && CD_APPLET_SCROLL_DOWN)
		gldi_window_minimize (actor);
CD_APPLET_ON_SCROLL_END

		
CD_APPLET_ON_DOUBLE_CLICK_BEGIN
	// maximize/restaure the window.
	if (myData.pCurrentWindow != 0)
	{
		gldi_window_maximize (myData.pCurrentWindow, ! myData.pCurrentWindow->bIsMaximized);
	}
CD_APPLET_ON_DOUBLE_CLICK_END


/*
CD_APPLET_ON_BUILD_MENU_BEGIN
	// nothing to do here, since the icon is considered as an appli, the dock will fill it for free !
CD_APPLET_ON_BUILD_MENU_END
*/

void cd_app_menu_on_keybinding_pull (const gchar *keystring, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_show_menu (myConfig.bMenuOnMouse);
	CD_APPLET_LEAVE();
}


//\___________ Other notifications, that are not from the user (but from the Applications-manager)
static void _check_dock_is_active (gchar *cDockName, CairoDock *pDock, gboolean *data)
{
	if (gldi_container_is_active (CAIRO_CONTAINER (pDock)))
		*data = 1;
}
gboolean cd_app_menu_on_active_window_changed (GldiModuleInstance *myApplet, GldiWindowActor *actor)
{
	// check if a dock has the focus (we don't want to control the dock, it wouldn't make sense anyway).
	if (actor)
	{
		gboolean is_dock = FALSE;
		gldi_docks_foreach ((GHFunc) _check_dock_is_active, &is_dock);
		if (is_dock)  // it's a dock, ignore it.
			actor = NULL;
	}
	
	// take this new window (possibly NULL).
	cd_app_menu_set_current_window (actor);
	
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_app_menu_on_state_changed (GldiModuleInstance *myApplet, GldiWindowActor *actor, gboolean bHiddenChanged, gboolean bMaximizedChanged, gboolean bFullScreenChanged)
{
	if (actor == myData.pCurrentWindow)
	{
		if (bMaximizedChanged)
		{
			gldi_window_set_border (actor, ! actor->bIsMaximized);
			cd_app_menu_redraw_buttons ();
		}
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_app_menu_on_name_changed (GldiModuleInstance *myApplet, GldiWindowActor *actor)
{
	if (actor == myData.pCurrentWindow)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (actor->cName);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_app_menu_on_new_appli (GldiModuleInstance *myApplet, GldiWindowActor *actor)
{
	if (actor->bIsMaximized)
	{
		gldi_window_set_border (actor, ! actor->bIsMaximized);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}


//\___________ Other notifications, for animation of the buttons.

gboolean on_mouse_moved (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bStartAnimation)
{
	CD_APPLET_ENTER;
	if (! myIcon->bPointed || ! pContainer->bInside)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	// find the pointed button
	int iNumButton = cd_app_menu_find_button (myApplet);
	if (iNumButton >= 0 && iNumButton < myData.iNbButtons)
	{
		// trigger animation
		*bStartAnimation = TRUE;
	}
	
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}


static gboolean _update_button_image_no_loop (CairoDockImageBuffer *pImage, int *iStep)
{
	gboolean bButtonAnimating = FALSE;
	if (cairo_dock_image_buffer_is_animated (pImage))
	{
		if (pImage->iCurrentFrame != 0)  // in the loop
		{
			gboolean bLastFrame = cairo_dock_image_buffer_next_frame_no_loop (pImage);
			if (bLastFrame)
				pImage->iCurrentFrame = 0;
			else
				bButtonAnimating = TRUE;
		}
	}
	else  // update the step
	{
		if (*iStep != 0)  // in the loop
		{
			++ *iStep;
			if (*iStep >= CD_ANIM_STEPS)
				*iStep = 0;
			else
				bButtonAnimating = TRUE;
		}
	}
	return bButtonAnimating;
}

static void _update_button_image_loop (CairoDockImageBuffer *pImage, int *iStep)
{
	if (cairo_dock_image_buffer_is_animated (pImage))
	{
		cairo_dock_image_buffer_next_frame (pImage);
	}
	else  // update the step
	{
		++ *iStep;
		if (*iStep >= CD_ANIM_STEPS)
			*iStep = 0;
	}
}

static gboolean _update_button_image (CairoDockImageBuffer *pImage, int *iStep, gboolean bLoop)
{
	if (bLoop)
	{
		_update_button_image_loop (pImage, iStep);
		return TRUE;
	}
	else
	{
		return _update_button_image_no_loop (pImage, iStep);
	}
}

gboolean cd_app_menu_on_update_container (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bContinueAnimation)
{
	CD_APPLET_ENTER;
	
	if (! myIcon->bPointed || ! pContainer->bInside)  // our icon is not pointed, only update it to finish the current button animations.
	{
		if (myData.bButtonAnimating)  // one or more button is animating.
		{
			myData.bButtonAnimating = FALSE;
			
			myData.bButtonAnimating |= _update_button_image_no_loop (&myData.minimizeButton, &myData.iAnimIterMin);
			
			myData.bButtonAnimating |= _update_button_image_no_loop (&myData.maximizeButton, &myData.iAnimIterMax);
			
			myData.bButtonAnimating |= _update_button_image_no_loop (&myData.restoreButton, &myData.iAnimIterRestore);
			
			myData.bButtonAnimating |= _update_button_image_no_loop (&myData.closeButton, &myData.iAnimIterClose);
			
			cd_app_menu_redraw_buttons ();
		}
	}
	else  // our button is currently pointed.
	{
		myData.bButtonAnimating = FALSE;
		int iNumButton = cd_app_menu_find_button (myApplet);
		
		myData.bButtonAnimating |= _update_button_image (&myData.minimizeButton, &myData.iAnimIterMin, iNumButton == CD_BUTTON_MINIMIZE);
		
		myData.bButtonAnimating |= _update_button_image (&myData.maximizeButton, &myData.iAnimIterMax, iNumButton == CD_BUTTON_MAXIMIZE);
		
		myData.bButtonAnimating |= _update_button_image (&myData.restoreButton, &myData.iAnimIterRestore, iNumButton == CD_BUTTON_MAXIMIZE);
		
		myData.bButtonAnimating |= _update_button_image (&myData.closeButton, &myData.iAnimIterClose, iNumButton == CD_BUTTON_CLOSE);
		
		cd_app_menu_redraw_buttons ();
	}
	
	if (myData.bButtonAnimating)
		*bContinueAnimation = TRUE;
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}
