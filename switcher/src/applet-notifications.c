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
#include <glib/gstdio.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-draw.h"
#include "applet-notifications.h"

static unsigned int _expose_windows_timeout = 0;

static void _cd_expose_windows (void)
{
	// note: myDock will be NULL if we are not in a dock (i.e. we are in a desklet)
	// but it is not needed in that case (the parameter is only used on Wayfire to
	// lose keyboard focus from layer-shell surfaces)
	gldi_desktop_present_windows (myDock ? &myDock->container : NULL);
}
static void _cd_expose_desktops (void)
{
	gldi_desktop_present_desktops ();
}
static gboolean _cd_expose_windows_idle (G_GNUC_UNUSED gpointer dummy)
{
	_cd_expose_windows ();
	_expose_windows_timeout = 0;
	return FALSE;
}
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	switch (myConfig.iActionOnMiddleClick)
	{
		case SWICTHER_WINDOWS_LIST:
		default:
		{
			GtkWidget *pMenu = gldi_menu_new (myIcon);
			cd_switcher_build_windows_list (pMenu);
			CD_APPLET_POPUP_MENU_ON_MY_ICON (pMenu);
		}
		break;
		case SWICTHER_SHOW_DESKTOP:
		{
			gboolean bDesktopIsVisible = gldi_desktop_is_visible ();
			gldi_desktop_show_hide (! bDesktopIsVisible);
		}
		break;
		case SWICTHER_EXPOSE_DESKTOPS:
		{
			_cd_expose_desktops ();
		}
		break;
		case SWICTHER_EXPOSE_WINDOWS:
		{
			// ok this is just crazy: if you call the Scale dbus method of Compiz before the middle button is released, it doesn't work.
			if (!_expose_windows_timeout)
				_expose_windows_timeout = g_timeout_add (300, _cd_expose_windows_idle, NULL);
		}
		break;
	}
CD_APPLET_ON_MIDDLE_CLICK_END


static gboolean _cd_switcher_get_viewport_from_clic (Icon *pClickedIcon, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY)
{
	if (myConfig.bCompactView && pClickedIcon == myIcon)
	{
		int iMouseX, iMouseY;
		if (myDesklet)
		{
			if (g_bUseOpenGL)
			{
				iMouseX = 0;
				iMouseY = 0;
				cd_switcher_extract_viewport_coords_from_picked_object (myDesklet, &iMouseX, &iMouseY);
			}
			else
			{
				iMouseX = myDesklet->iMouseX2d;
				iMouseY = myDesklet->iMouseY2d;
			}
			//g_print ("on cherche le bureau en (%d;%d)\n", iMouseX, iMouseY);
		}
		else
		{
			iMouseX = myContainer->iMouseX - myIcon->fDrawX;
			iMouseY = myContainer->iMouseY - myIcon->fDrawY;
		}
		
		double w, h;
		if (myContainer->bIsHorizontal)
		{
			w = myIcon->fWidth * myIcon->fScale;
			h = myIcon->fHeight * myIcon->fScale;
		}
		else
		{
			h = myIcon->fWidth * myIcon->fScale;
			w = myIcon->fHeight * myIcon->fScale;
			double tmp = iMouseX;
			iMouseX = iMouseY;
			iMouseY = tmp;
		}
		double fMaxScale = cairo_dock_get_icon_max_scale (myIcon);
		double dx = myData.switcher.fOffsetX / fMaxScale * myIcon->fScale;
		double dy = myData.switcher.fOffsetY / fMaxScale * myIcon->fScale;
		w -= 2 * dx;
		h -= 2 * dy;
		iMouseX -= dx;
		iMouseY -= dy;
		
		/**if (iMouseX < 0 || iMouseX > w || iMouseY < 0 || iMouseY > h)
			return FALSE;*/
		if (iMouseX < 0)
			iMouseX = 0;
		if (iMouseY < 0)
			iMouseY = 0;
		if (iMouseX > w)
			iMouseX = w;
		if (iMouseY > h)
			iMouseY = h;
		
		int iNumLine = (int) (iMouseY / (h) * myData.switcher.iNbLines);
		int iNumColumn = (int) (iMouseX / (w) * myData.switcher.iNbColumns);
		cd_switcher_compute_desktop_from_coordinates (iNumLine, iNumColumn, iNumDesktop, iNumViewportX, iNumViewportY);
		return TRUE;
	}
	else if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		int iIndex = pClickedIcon->fOrder;
		cd_switcher_compute_desktop_from_index (iIndex, iNumDesktop, iNumViewportX, iNumViewportY);
		return TRUE;
	}
	else
		return FALSE;
}

CD_APPLET_ON_CLICK_BEGIN
	int iNumDesktop, iNumViewportX, iNumViewportY;
	if (! _cd_switcher_get_viewport_from_clic (pClickedIcon, &iNumDesktop, &iNumViewportX, &iNumViewportY))
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	if (iNumViewportX != myData.switcher.iCurrentViewportX
	|| iNumViewportY != myData.switcher.iCurrentViewportY
	|| iNumDesktop != myData.switcher.iCurrentDesktop)
		gldi_desktop_set_current (iNumDesktop, iNumViewportX, iNumViewportY);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_SCROLL_BEGIN  // Merci ChangFu !
	int iIndex = cd_switcher_compute_index_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	int iNumDesktop, iNumViewportX, iNumViewportY;
	cd_debug ("Switcher: current %d", iIndex);
	if (CD_APPLET_SCROLL_DOWN)
	{
		iIndex++;
		if (iIndex >= myData.switcher.iNbViewportTotal)
			iIndex = 0;
		cd_switcher_compute_desktop_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	}
	else if (CD_APPLET_SCROLL_UP)
	{
		iIndex = iIndex - 1;
		if (iIndex < 0)
			iIndex = myData.switcher.iNbViewportTotal - 1;
		cd_switcher_compute_desktop_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	}
	else
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	cd_debug ("Switcher: switching to %d", iIndex);
	if (iNumViewportX != myData.switcher.iCurrentViewportX
	|| iNumViewportY != myData.switcher.iCurrentViewportY
	|| iNumDesktop != myData.switcher.iCurrentDesktop)
		gldi_desktop_set_current (iNumDesktop, iNumViewportX, iNumViewportY);
CD_APPLET_ON_SCROLL_END


static void _cd_switcher_add_desktop (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gldi_desktop_add_workspace ();
}
static void _cd_switcher_remove_last_desktop (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gldi_desktop_remove_last_workspace ();
}
static void _cd_switcher_refresh (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	cd_switcher_refresh_desktop_values (myApplet);
}
static void _cd_switcher_move_to_desktop (GtkMenuItem *menu_item, gpointer data)
{
	int iIndex = GPOINTER_TO_INT (data);
	int iNumDesktop, iNumViewportX, iNumViewportY;
	cd_switcher_compute_desktop_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	cd_switcher_move_current_desktop_to (iNumDesktop, iNumViewportX, iNumViewportY);
}

static void _on_got_workspace_name (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		const gchar *cNewName = gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget));
		if (cNewName != NULL)
		{
			int iIndex = GPOINTER_TO_INT (data);
			// store the new name inside the array.
			if (iIndex >= myData.iNbNames)
			{
				myData.cDesktopNames = g_realloc (myData.cDesktopNames, (iIndex + 2) * sizeof (gchar*));  // NULL-terminated.
				int i;
				for (i = myData.iNbNames; i < iIndex; i ++)  // on met des noms par defaut aux nouveaux, sauf a celui choisi.
					myData.cDesktopNames[i] = g_strdup_printf ("%s %d", D_("Desktop"), i+1);
				myData.cDesktopNames[iIndex] = NULL;
				myData.cDesktopNames[iIndex+1] = NULL;  // NULL-terminated.
				myData.iNbNames = iIndex + 1;
			}
			
			g_free (myData.cDesktopNames[iIndex]);
			myData.cDesktopNames[iIndex] = g_strdup (cNewName);  // donc ne pas liberer 'cNewName'.
			
			// apply the change on X (it will trigger the notification, which will store the names in config).
			gldi_desktop_set_names (myData.cDesktopNames);
		}
	}
	CD_APPLET_LEAVE ();
}
static void _cd_switcher_rename_desktop (GtkMenuItem *menu_item, gpointer data)
{
	// on demande le nouveau nom.
	int iIndex = GPOINTER_TO_INT (data);
	gchar *cName = (iIndex < myData.iNbNames ? g_strdup (myData.cDesktopNames[iIndex]) : g_strdup_printf ("%s %d", D_("Desktop"), iIndex+1));
	gldi_dialog_show_with_entry (D_("Rename this workspace"),
		myIcon, myContainer, "same icon",
		cName,
		(CairoDockActionOnAnswerFunc)_on_got_workspace_name, data, (GFreeFunc)NULL);
	g_free (cName);
}
static void _cd_switcher_show_desktop (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gboolean bDesktopIsVisible = gldi_desktop_is_visible ();
	gldi_desktop_show_hide (! bDesktopIsVisible);
}
static void _cd_switcher_expose_windows (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	_cd_expose_windows ();
}
static void _cd_switcher_expose_desktops (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	_cd_expose_desktops ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	// Workspaces
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Add a workspace"),
		GLDI_ICON_NAME_ADD,
		_cd_switcher_add_desktop,
		CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Remove last workspace"),
		GLDI_ICON_NAME_REMOVE,
		_cd_switcher_remove_last_desktop,
		CD_APPLET_MY_MENU);
	
	int iNumDesktop, iNumViewportX, iNumViewportY;
	if (_cd_switcher_get_viewport_from_clic (pClickedIcon, &iNumDesktop, &iNumViewportX, &iNumViewportY))
	{
		int iIndex = cd_switcher_compute_index_from_desktop (iNumDesktop, iNumViewportX, iNumViewportY);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Rename this workspace"),
			GLDI_ICON_NAME_EDIT,
			_cd_switcher_rename_desktop,
			CD_APPLET_MY_MENU,
			GINT_TO_POINTER (iIndex));
		if (gldi_window_manager_can_move_to_desktop () && (iNumDesktop != myData.switcher.iCurrentDesktop
			|| iNumViewportX != myData.switcher.iCurrentViewportX || iNumViewportY != myData.switcher.iCurrentViewportY))
		{
			GtkWidget *pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Move current workspace to this workspace"),
				GLDI_ICON_NAME_JUMP_TO,
				_cd_switcher_move_to_desktop,
				CD_APPLET_MY_MENU,
				GINT_TO_POINTER (iIndex));
			gtk_widget_set_tooltip_text (pMenuItem, D_("This will move all windows from the current desktop to the one you clicked on."));
		}
	}
	
	// separator
	CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
	gchar *cLabel;
	
	// desktop actions
	cLabel = (myConfig.iActionOnMiddleClick == SWICTHER_WINDOWS_LIST ? g_strdup_printf ("%s (%s)", D_("Windows List"), D_("middle-click")) : g_strdup (D_("Windows List")));
	GtkWidget *pWindowsListMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (cLabel, CD_APPLET_MY_MENU, GLDI_ICON_NAME_SORT_DESCENDING);
	g_free (cLabel);
	cd_switcher_build_windows_list (pWindowsListMenu);

	cLabel = (myConfig.iActionOnMiddleClick == SWICTHER_SHOW_DESKTOP ? g_strdup_printf ("%s (%s)", D_("Show the desktop"), D_("middle-click")) : g_strdup (D_("Show the desktop")));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
		MY_APPLET_SHARE_DATA_DIR"/../shared-files/images/show-desktop.svg",
		_cd_switcher_show_desktop,
		CD_APPLET_MY_MENU);
	g_free (cLabel);
	
	if (gldi_desktop_can_present_desktops ())
	{
		cLabel = (myConfig.iActionOnMiddleClick == SWICTHER_EXPOSE_DESKTOPS ? g_strdup_printf ("%s (%s)", D_("Expose all the desktops"), D_("middle-click")) : g_strdup (D_("Expose all the desktops")));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			MY_APPLET_SHARE_DATA_DIR"/../shared-files/images/expose-desktops.svg",
			_cd_switcher_expose_desktops,
			CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	if (gldi_desktop_can_present_windows ())
	{
		cLabel = (myConfig.iActionOnMiddleClick == SWICTHER_EXPOSE_WINDOWS ? g_strdup_printf ("%s (%s)", D_("Expose all the windows"), D_("middle-click")) : g_strdup (D_("Expose all the windows")));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			MY_APPLET_SHARE_DATA_DIR"/../shared-files/images/expose-windows.svg",
			_cd_switcher_expose_windows,
			CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	
	CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Refresh"),
		GLDI_ICON_NAME_REFRESH,
		_cd_switcher_refresh,
		CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END



static gboolean _cd_switcher_redraw_main_icon_idle (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	if (myData.switcher.iNbColumns == 0)
	{
		cd_switcher_compute_nb_lines_and_columns ();
		cd_switcher_get_current_desktop ();
	}
	cd_switcher_draw_main_icon ();
	myData.iSidRedrawMainIconIdle = 0;
	CD_APPLET_LEAVE (FALSE);
	//return FALSE;
}
static void _cd_switcher_trigger_redraw (GldiModuleInstance *myApplet)
{
	if (myData.iSidRedrawMainIconIdle == 0 && myData.iSidUpdateIdle == 0)
	{
		myData.iSidRedrawMainIconIdle = g_idle_add ((GSourceFunc) _cd_switcher_redraw_main_icon_idle, myApplet);
	}
}

gboolean on_change_desktop (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("");
	int iPreviousIndex = cd_switcher_compute_index_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	cd_switcher_get_current_desktop ();
	int iIndex = cd_switcher_compute_index_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	if (myConfig.bDisplayNumDesk)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1);
	}
	
	if (myConfig.bCompactView)
	{
		_cd_switcher_trigger_redraw (myApplet);
	}
	else
	{
		GldiContainer *pContainer = (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer);
		CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL, GLDI_NOTIFICATION_LET_PASS);
		//g_return_val_if_fail (pContainer != NULL, GLDI_NOTIFICATION_LET_PASS);
		
		if (myDock && myConfig.bDisplayNumDesk)
			CD_APPLET_REDRAW_MY_ICON;
		
		// On redessine les 2 icones du sous-dock impactees.
		GList *pIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
		Icon *icon;
		GList *ic;
		for (ic = pIconList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->fOrder == iPreviousIndex)  // l'ancienne icone du bureau courant.
			{
				if (iPreviousIndex < myData.iNbNames)
					gldi_icon_set_name (icon, myData.cDesktopNames[iPreviousIndex]);
				else
					gldi_icon_set_name_printf (icon, "%s %d", D_("Desktop"), iPreviousIndex+1);
				icon->bHasIndicator = FALSE;
				icon->fAlpha = 1.;
				if (myDock)
					cairo_dock_redraw_icon (icon);
			}
			if (icon->fOrder == iIndex)  // c'est l'icone du bureau courant.
			{
				gldi_icon_set_name_printf (icon, "%s (%d)", D_("Current"), iIndex+1);
				icon->bHasIndicator = TRUE;
				icon->fAlpha = .7;
				if (myDock)
					cairo_dock_redraw_icon (icon);
			}
		}
		if (myDesklet)
			gtk_widget_queue_draw (myDesklet->container.pWidget);
	}
	
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

gboolean on_change_screen_geometry (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_switcher_trigger_update_from_screen_geometry (TRUE);  // TRUE = now
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

gboolean on_change_wallpaper (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_switcher_trigger_update_from_wallpaper ();
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

gboolean on_window_size_position_changed (GldiModuleInstance *myApplet, GldiWindowsManager *actor)
{
	CD_APPLET_ENTER;
	_cd_switcher_trigger_redraw (myApplet);
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

gboolean on_change_window_state (GldiModuleInstance *myApplet, G_GNUC_UNUSED GldiWindowActor *actor, G_GNUC_UNUSED gboolean bHiddenChanged, G_GNUC_UNUSED gboolean bMaximizedChanged, G_GNUC_UNUSED gboolean bFullScreenChanged)
{
	CD_APPLET_ENTER;
	_cd_switcher_trigger_redraw (myApplet);
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}


gboolean on_change_window_order (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_cd_switcher_trigger_redraw (myApplet);
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

static void _save_desktop_names (void)
{
	GString *sNames = g_string_new ("");
	int i;
	for (i = 0; i < myData.iNbNames; i ++)
	{
		g_string_append_printf (sNames, "%s;", myData.cDesktopNames[i]);
	}
	sNames->str[sNames->len-1] = '\0';
	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
		G_TYPE_STRING, "Configuration", "desktop names", sNames->str,
		G_TYPE_INVALID);
	g_string_free (sNames, TRUE);
}
gboolean on_change_desktop_names (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	// retrieve the desktop names
	if (myData.cDesktopNames != NULL)
		g_strfreev (myData.cDesktopNames);
	myData.cDesktopNames = gldi_desktop_get_names ();
	myData.iNbNames = g_strv_length (myData.cDesktopNames);
	// store them in config, to be able to set them on startup if noone has done it.
	_save_desktop_names ();
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}


gboolean on_mouse_moved (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bStartAnimation)
{
	CD_APPLET_ENTER;
	if (! myIcon->bPointed || ! pContainer->bInside)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	int iNumDesktop, iNumViewportX, iNumViewportY;
	if (! _cd_switcher_get_viewport_from_clic (myIcon, &iNumDesktop, &iNumViewportX, &iNumViewportY))
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	int iIndex = cd_switcher_compute_index_from_desktop (iNumDesktop, iNumViewportX, iNumViewportY);
	if (iIndex != myData.iPrevIndexHovered)
	{
		myData.iPrevIndexHovered = iIndex;
		myData.fDesktopNameAlpha = 0.;
		if (iIndex < myData.iNbNames)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myData.cDesktopNames[iIndex]);
		}
		else
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("%s %d", D_("Desktop"), iIndex+1);
		}
		if (myDock)
			CAIRO_DOCK_REDRAW_MY_CONTAINER;
		else
			*bStartAnimation = TRUE;
	}
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

gboolean on_update_desklet (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bContinueAnimation)
{
	CD_APPLET_ENTER;
	if (! myIcon->bPointed || ! pContainer->bInside)
	{
		myData.fDesktopNameAlpha -= .07;
		if (myData.fDesktopNameAlpha < .01)
			myData.fDesktopNameAlpha = 0;
		if (myData.fDesktopNameAlpha != 0)
			*bContinueAnimation = TRUE;
	}
	else
	{
		myData.fDesktopNameAlpha += .07;
		if (myData.fDesktopNameAlpha > .99)
			myData.fDesktopNameAlpha = 1;
		if (myData.fDesktopNameAlpha != 1)
			*bContinueAnimation = TRUE;
	}
	CAIRO_DOCK_REDRAW_MY_CONTAINER;
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

gboolean on_render_desklet (GldiModuleInstance *myApplet, GldiContainer *pContainer, cairo_t *pCairoContext)
{
	if (myContainer != pContainer)
		return GLDI_NOTIFICATION_LET_PASS;
	CD_APPLET_ENTER;
	int x, y;  // text center (middle of the icon).
	x = myIcon->fDrawX + myIcon->fWidth * myIcon->fScale / 2;
	y = myIcon->fDrawY + myIcon->fHeight * myIcon->fScale / 2;
	if (x - myIcon->label.iWidth/2 < 0)
	{
		x += (myIcon->label.iWidth/2 - x);
	}
	if (pCairoContext != NULL)
	{
		if (myIcon->label.pSurface != NULL)
		{
			/**cairo_save (pCairoContext);
			cairo_translate (pCairoContext, x, y);
			cairo_set_source_surface (pCairoContext, myIcon->pTextBuffer, - myIcon->iTextWidth/2, - myIcon->iTextHeight/2);
			cairo_paint_with_alpha (pCairoContext, myData.fDesktopNameAlpha);
			cairo_restore (pCairoContext);*/
			cairo_dock_apply_image_buffer_surface_with_offset (&myIcon->label, pCairoContext,
				x - myIcon->label.iWidth/2, y - myIcon->label.iHeight/2, myData.fDesktopNameAlpha);
		}
	}
	else
	{
		if (myIcon->label.iTexture != 0)
		{
			glPushMatrix ();
			glTranslatef (-myContainer->iWidth/2, -myContainer->iHeight/2, -myContainer->iHeight*(sqrt(3)/2));
			/**glTranslatef (x - ((myIcon->iTextWidth & 1) ? 0.5 : 0.),
				y - ((myIcon->iTextHeight & 1) ? 0.5 : 0.),
				0);
			cairo_dock_draw_texture_with_alpha (myIcon->iLabelTexture, myIcon->iTextWidth, myIcon->iTextHeight, myData.fDesktopNameAlpha);*/
			_cairo_dock_enable_texture ();
			_cairo_dock_set_blend_alpha ();
			_cairo_dock_set_alpha (myData.fDesktopNameAlpha);
			cairo_dock_apply_image_buffer_texture_with_offset (&myIcon->label,
				x - ((myIcon->label.iWidth & 1) ? 0.5 : 0.),
				y - ((myIcon->label.iHeight & 1) ? 0.5 : 0.));
			_cairo_dock_disable_texture ();
			glPopMatrix ();
		}
	}
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

gboolean on_leave_desklet (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bStartAnimation)
{
	*bStartAnimation = TRUE;
	myData.iPrevIndexHovered = -1;
	return GLDI_NOTIFICATION_LET_PASS;
}
