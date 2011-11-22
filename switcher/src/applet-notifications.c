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


static void _cd_expose_windows (void)
{
	cairo_dock_wm_present_windows ();
}
static void _cd_expose_desktops (void)
{
	cairo_dock_wm_present_desktops ();
}
static gboolean _cd_expose_windows_idle (gpointer data)
{
	_cd_expose_windows ();
	return FALSE;
}
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	switch (myConfig.iActionOnMiddleClick)
	{
		case SWICTHER_WINDOWS_LIST:
		default:
		{
			GtkWidget *pMenu = gtk_menu_new ();
			cd_switcher_build_windows_list (pMenu);
			CD_APPLET_POPUP_MENU_ON_MY_ICON (pMenu);
		}
		break;
		case SWICTHER_SHOW_DESKTOP:
		{
			gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
			cairo_dock_show_hide_desktop (! bDesktopIsVisible);
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
			g_timeout_add (300, _cd_expose_windows_idle, NULL);
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
		double fMaxScale = cairo_dock_get_max_scale (myContainer);
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
		//g_print ("(%d;%d) --> (%d;%d;%d)\n", iNumLine, iNumColumn, *iNumDesktop, *iNumViewportX, *iNumViewportY);
		return TRUE;
	}
	else if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		int iIndex = pClickedIcon->fOrder;
		cd_switcher_compute_viewports_from_index (iIndex, iNumDesktop, iNumViewportX, iNumViewportY);
		return TRUE;
	}
	else
		return FALSE;
}

CD_APPLET_ON_CLICK_BEGIN
	int iNumDesktop, iNumViewportX, iNumViewportY;
	if (! _cd_switcher_get_viewport_from_clic (pClickedIcon, &iNumDesktop, &iNumViewportX, &iNumViewportY))
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	if (iNumDesktop != myData.switcher.iCurrentDesktop)
		cairo_dock_set_current_desktop (iNumDesktop);
	if (iNumViewportX != myData.switcher.iCurrentViewportX || iNumViewportY != myData.switcher.iCurrentViewportY)
		cairo_dock_set_current_viewport (iNumViewportX, iNumViewportY);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_SCROLL_BEGIN  // Merci ChangFu !
	int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	int iNumDesktop, iNumViewportX, iNumViewportY;
	cd_debug ("Switcher: current %d", iIndex);
	if (CD_APPLET_SCROLL_DOWN)
	{
		iIndex++;
		if (iIndex >= myData.switcher.iNbViewportTotal)
			iIndex = 0;
		cd_switcher_compute_viewports_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	}
	else if (CD_APPLET_SCROLL_UP)
	{
		iIndex = iIndex - 1;
		if (iIndex < 0)
			iIndex = myData.switcher.iNbViewportTotal - 1;
		cd_switcher_compute_viewports_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	}
	else
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	cd_debug ("Switcher: switching to %d", iIndex);
	if (iNumDesktop != myData.switcher.iCurrentDesktop)
		cairo_dock_set_current_desktop (iNumDesktop);
	if (iNumViewportX != myData.switcher.iCurrentViewportX || iNumViewportY != myData.switcher.iCurrentViewportY)
		cairo_dock_set_current_viewport (iNumViewportX, iNumViewportY);
CD_APPLET_ON_SCROLL_END


static void _cd_switcher_add_desktop (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_switcher_add_a_desktop ();
}
static void _cd_switcher_remove_last_desktop (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_switcher_remove_last_desktop ();
}
static void _cd_switcher_refresh (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_switcher_refresh_desktop_values (myApplet);
}
static void _cd_switcher_move_to_desktop (GtkMenuItem *menu_item, gpointer data)
{
	int iIndex = GPOINTER_TO_INT (data);
	int iNumDesktop, iNumViewportX, iNumViewportY;
	cd_switcher_compute_viewports_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	cd_switcher_move_current_desktop_to (iNumDesktop, iNumViewportX, iNumViewportY);
}
static void _cd_switcher_rename_desktop (GtkMenuItem *menu_item, gpointer data)
{
	// on demande le nouveau nom.
	int iIndex = GPOINTER_TO_INT (data);
	gchar *cName = (iIndex < myConfig.iNbNames ? g_strdup (myConfig.cDesktopNames[iIndex]) : g_strdup_printf ("%s %d", D_("Desktop"), iIndex+1));
	gchar *cNewName = cairo_dock_show_demand_and_wait (D_("Rename this workspace"), myIcon, myContainer, cName);
	g_free (cName);
	
	if (cNewName != NULL)
	{
		// on augmente la taille du tableau si necessaire.
		if (iIndex >= myConfig.iNbNames)
		{
			myConfig.cDesktopNames = g_realloc (myConfig.cDesktopNames, (iIndex + 2) * sizeof (gchar*));  // NULL-terminated.
			int i;
			for (i = myConfig.iNbNames; i < iIndex; i ++)  // on met des noms par defaut aux nouveaux, sauf a celui choisi.
				myConfig.cDesktopNames[i] = g_strdup_printf ("%s %d", D_("Desktop"), i+1);
			myConfig.cDesktopNames[iIndex] = NULL;
			myConfig.cDesktopNames[iIndex+1] = NULL;  // NULL-terminated.
			myConfig.iNbNames = iIndex + 1;
		}
		
		g_free (myConfig.cDesktopNames[iIndex]);
		myConfig.cDesktopNames[iIndex] = cNewName;  // donc ne pas liberer 'cNewName'.
		
		// on sauvegarde le nom dans le fichier de conf.
		GString *sNames = g_string_new ("");
		int i;
		for (i = 0; i < myConfig.iNbNames; i ++)
		{
			g_string_append_printf (sNames, "%s;", myConfig.cDesktopNames[i]);
		}
		sNames->str[sNames->len-1] = '\0';
		cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
			G_TYPE_STRING, "Configuration", "desktop names", sNames->str,
			G_TYPE_INVALID);
		g_string_free (sNames, TRUE);
	}
}
static void _cd_switcher_show_desktop (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
}
static void _cd_switcher_expose_windows (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	_cd_expose_windows ();
}
static void _cd_switcher_expose_desktops (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	_cd_expose_desktops ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	// Workspaces
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Add a workspace"),
		GTK_STOCK_ADD,
		_cd_switcher_add_desktop,
		CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Remove last workspace"),
		GTK_STOCK_REMOVE,
		_cd_switcher_remove_last_desktop,
		CD_APPLET_MY_MENU);
	
	int iNumDesktop, iNumViewportX, iNumViewportY;
	if (_cd_switcher_get_viewport_from_clic (pClickedIcon, &iNumDesktop, &iNumViewportX, &iNumViewportY))
	{
		int iIndex = cd_switcher_compute_index (iNumDesktop, iNumViewportX, iNumViewportY);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Rename this workspace"),
			GTK_STOCK_EDIT,
			_cd_switcher_rename_desktop,
			CD_APPLET_MY_MENU,
			GINT_TO_POINTER (iIndex));
		if (iNumDesktop != myData.switcher.iCurrentDesktop || iNumViewportX != myData.switcher.iCurrentViewportX || iNumViewportY != myData.switcher.iCurrentViewportY)
		{
			GtkWidget *pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Move current workspace to this workspace"),
				GTK_STOCK_JUMP_TO,
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
	GtkWidget *pWindowsListMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (cLabel, CD_APPLET_MY_MENU, GTK_STOCK_DND_MULTIPLE);
	g_free (cLabel);
	cd_switcher_build_windows_list (pWindowsListMenu);

	cLabel = (myConfig.iActionOnMiddleClick == SWICTHER_SHOW_DESKTOP ? g_strdup_printf ("%s (%s)", D_("Show the desktop"), D_("middle-click")) : g_strdup (D_("Show the desktop")));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
		MY_APPLET_SHARE_DATA_DIR"/../shared-files/images/show-desktop.svg",
		_cd_switcher_show_desktop,
		CD_APPLET_MY_MENU);
	g_free (cLabel);
	
	if (cairo_dock_wm_can_present_desktops ())
	{
		cLabel = (myConfig.iActionOnMiddleClick == SWICTHER_EXPOSE_DESKTOPS ? g_strdup_printf ("%s (%s)", D_("Expose all the desktops"), D_("middle-click")) : g_strdup (D_("Expose all the desktops")));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			MY_APPLET_SHARE_DATA_DIR"/../shared-files/images/expose-desktops.svg",
			_cd_switcher_expose_desktops,
			CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	if (cairo_dock_wm_can_present_windows ())
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
		GTK_STOCK_REFRESH,
		_cd_switcher_refresh,
		CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END



static gboolean _cd_switcher_redraw_main_icon_idle (CairoDockModuleInstance *myApplet)
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
static void _cd_switcher_trigger_redraw (CairoDockModuleInstance *myApplet)
{
	if (myData.iSidRedrawMainIconIdle == 0 && myData.iSidUpdateIdle == 0)
	{
		myData.iSidRedrawMainIconIdle = g_idle_add ((GSourceFunc) _cd_switcher_redraw_main_icon_idle, myApplet);
	}
}

gboolean on_change_active_window (CairoDockModuleInstance *myApplet, Window *XActiveWindow)
{
	CD_APPLET_ENTER;
	_cd_switcher_trigger_redraw (myApplet);
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_change_desktop (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("");
	int iPreviousIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	cd_switcher_get_current_desktop ();
	int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
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
		CairoContainer *pContainer = (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer);
		CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		//g_return_val_if_fail (pContainer != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		
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
				if (iPreviousIndex < myConfig.iNbNames)
					cairo_dock_set_icon_name (myConfig.cDesktopNames[iPreviousIndex], icon, pContainer);
				else
					cairo_dock_set_icon_name_printf (icon, pContainer, "%s %d", D_("Desktop"), iPreviousIndex+1);
				icon->bHasIndicator = FALSE;
				icon->fAlpha = 1.;
				if (myDock)
					cairo_dock_redraw_icon (icon, pContainer);
			}
			if (icon->fOrder == iIndex)  // c'est l'icone du bureau courant.
			{
				cairo_dock_set_icon_name_printf (icon, pContainer, "%s (%d)", D_("Current"), iIndex+1);
				icon->bHasIndicator = TRUE;
				icon->fAlpha = .7;
				if (myDock)
					cairo_dock_redraw_icon (icon, pContainer);
			}
		}
		if (myDesklet)
			gtk_widget_queue_draw (myDesklet->container.pWidget);
	}
	
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_change_screen_geometry (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("");
	cd_switcher_trigger_update_from_screen_geometry (TRUE);  // TRUE = now
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_window_configured (CairoDockModuleInstance *myApplet, Window Xid, XConfigureEvent *xconfigure)
{
	CD_APPLET_ENTER;
	cd_debug ("");
	_cd_switcher_trigger_redraw (myApplet);
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}


gboolean on_mouse_moved (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	CD_APPLET_ENTER;
	if (! myIcon->bPointed || ! pContainer->bInside)
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	int iNumDesktop, iNumViewportX, iNumViewportY;
	if (! _cd_switcher_get_viewport_from_clic (myIcon, &iNumDesktop, &iNumViewportX, &iNumViewportY))
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	int iIndex = cd_switcher_compute_index (iNumDesktop, iNumViewportX, iNumViewportY);
	if (iIndex != myData.iPrevIndexHovered)
	{
		myData.iPrevIndexHovered = iIndex;
		myData.fDesktopNameAlpha = 0.;
		if (iIndex < myConfig.iNbNames)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDesktopNames[iIndex]);
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
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_update_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bContinueAnimation)
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
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_render_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, cairo_t *pCairoContext)
{
	CD_APPLET_ENTER;
	int x, y;  // centre du texte.
	x = myIcon->fDrawX + myIcon->fWidth * myIcon->fScale / 2;
	y = myIcon->fDrawY + myIcon->fHeight * myIcon->fScale / 2;
	if (x - myIcon->iTextWidth/2 < 0)
	{
		x -= myIcon->iTextWidth/2;
	}
	if (pCairoContext != NULL)
	{
		if (myIcon->pTextBuffer == NULL)
			CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
		cairo_save (pCairoContext);
		cairo_translate (pCairoContext, x, y);
		cairo_set_source_surface (pCairoContext, myIcon->pTextBuffer, - myIcon->iTextWidth/2, - myIcon->iTextHeight/2);
		cairo_paint_with_alpha (pCairoContext, myData.fDesktopNameAlpha);
		cairo_restore (pCairoContext);
	}
	else
	{
		if (myIcon->iLabelTexture == 0)
			CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
		glPushMatrix ();
		if (myDesklet)
			glTranslatef (-myDesklet->container.iWidth/2, -myDesklet->container.iHeight/2, -myDesklet->container.iHeight*(sqrt(3)/2));
		glTranslatef (x - ((myIcon->iTextWidth & 1) ? 0.5 : 0.),
			y - ((myIcon->iTextHeight & 1) ? 0.5 : 0.),
			0);
		cairo_dock_draw_texture_with_alpha (myIcon->iLabelTexture, myIcon->iTextWidth, myIcon->iTextHeight, myData.fDesktopNameAlpha);
		glPopMatrix ();
	}
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_leave_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	*bStartAnimation = TRUE;
	myData.iPrevIndexHovered = -1;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
