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

#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>

typedef enum {
	CD_BUTTON_MENU,
	CD_BUTTON_MINIMIZE,
	CD_BUTTON_MAXIMIZE,
	CD_BUTTON_CLOSE,
	CD_NB_BUTTONS
	} CDButtonEnum;

typedef enum {
	CD_GM_BUTTON_ORDER_AUTO = 0,
	CD_GM_BUTTON_ORDER_RIGHT,
	CD_GM_BUTTON_ORDER_LEFT
	} CDGMReversedMenu; 

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bDisplayControls;  // steal the window top border
	gboolean bDisplayMenu;  // steal the menu from the window
	gboolean bCompactMode;  // TRUE = if bDisplayControls, display all control buttons on the icon
	CDGMReversedMenu iButtonsOrder;  // Buttons' order (auto / right / left)
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

typedef struct _AppChildWatchData
{
	AppletData *pApplet; // set to NULL if not valid anymore
} AppChildWatchData;

struct _AppletData {
	GDBusProxy *pProxyRegistrar;
	GCancellable *pCancel;
	GCancellable *pCancelMenu;
	guint uRegWatch;
	gboolean bOwnRegistrar;
	GPid pidOwnRegistrar;
	AppChildWatchData *pChildWatch;
	GldiWindowActor *pPreviousWindow, *pCurrentWindow;  // window currently controlled.
	gboolean bCanClose;
	gboolean bCanMinimize;
	gboolean bCanMaximize;
	GtkWidget *pMenu;
	GldiShortkey *pKeyBinding;
	CairoDockImageBuffer defaultIcon;
	CairoDockImageBuffer minimizeButton;
	CairoDockImageBuffer maximizeButton;
	CairoDockImageBuffer restoreButton;
	CairoDockImageBuffer closeButton;
	gint iAnimIterMin, iAnimIterMax, iAnimIterClose, iAnimIterRestore;
	gboolean bButtonAnimating;
	guint iSidInitIdle;
	guint iSidInitIdle2;
	// GldiTask *pTask;
	gint iNbButtons;
	GHashTable *windows;
	gboolean bReversedButtonsOrder;
	} ;

#endif
