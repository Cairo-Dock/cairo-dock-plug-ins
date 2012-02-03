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

#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>
#include <gtk/gtk.h>

#if (GTK_MAJOR_VERSION < 3) || defined (DBUSMENU_GTK3_NEW)
#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>
#else
#include <libdbusmenu-gtk3/menuitem.h>
#include <libdbusmenu-gtk3/menu.h>
#endif

typedef enum {
	CD_BUTTON_MENU,
	CD_BUTTON_MINIMIZE,
	CD_BUTTON_MAXIMIZE,
	CD_BUTTON_CLOSE,
	CD_NB_BUTTONS
	} CDButtonEnum;

typedef enum {
	CD_GM_BUTTON_ORDER_AUTO,
	CD_GM_BUTTON_ORDER_RIGHT,
	CD_GM_BUTTON_ORDER_LEFT
	} CDGMReversedMenu; 

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bDisplayControls;  // steal the window top border
	gboolean bDisplayMenu;  // steal the menu from the window
	gboolean bCompactMode;  // TRUE = if bDisplayControls, display all control buttons on the icon
	gint iButtonsOrder;  // Buttons' order (auto / right / left)
	gchar *cShortkey;  // if bDisplayMenu, shortkey to pop up the menu
	gboolean bMenuOnMouse;
	gint iTransitionDuration;  // ms
	gchar *cMinimizeImage;
	gchar *cMaximizeImage;
	gchar *cRestoreImage;
	gchar *cCloseImage;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
#define CD_ANIM_STEPS 15

struct _AppletData {
	DBusGProxy *pProxyRegistrar;
	gboolean bOwnRegistrar;
	Window iPreviousWindow, iCurrentWindow;  // window currently controlled.
	gboolean bCanClose;
	gboolean bCanMinimize;
	gboolean bCanMaximize;
	DbusmenuGtkMenu *pMenu;
	CairoKeyBinding *pKeyBinding;
	CairoDockImageBuffer defaultIcon;
	CairoDockImageBuffer minimizeButton;
	CairoDockImageBuffer maximizeButton;
	CairoDockImageBuffer restoreButton;
	CairoDockImageBuffer closeButton;
	gint iAnimIterMin, iAnimIterMax, iAnimIterClose;
	gboolean bButtonAnimating;
	guint iSidInitIdle;
	guint iSidInitIdle2;
	CairoDockTask *pTask;
	gint iNbButtons;
	GHashTable *windows;
	gboolean bReversedButtonsOrder;
	} ;

#endif
