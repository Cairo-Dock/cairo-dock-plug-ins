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
#include <cairo-dock-applet-single-instance.h>

#define CD_ITEMS_DELIMITER "=-+-="

typedef enum {
	CD_CLIPPER_NONE=0,
	CD_CLIPPER_CLIPBOARD,
	CD_CLIPPER_PRIMARY,
	CD_CLIPPER_BOTH
	} CDClipperItemType;

typedef struct _CDClipperItem {
	CDClipperItemType iType;
	gchar *cText;
	gchar *cDisplayedText;
	} CDClipperItem;

typedef struct _CDClipperCommand {
	gchar *cDescription;
	gchar *cFormat;
	gchar *cIconFileName;
	} CDClipperCommand;

typedef struct _CDClipperAction {
	gchar *cDescription;
	GRegex *pRegex;
	GList *pCommands;
	} CDClipperAction;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	CDClipperItemType iItemType;
	gint iNbItems[4];
	gboolean bPasteInClipboard;
	gboolean bPasteInPrimary;
	gboolean bEnableActions;
	gboolean bMenuOnMouse;
	gboolean bSeparateSelections;
	gboolean bReplayAction;
	gint iActionMenuDuration;
	gchar *cShortcut;
	gchar **pPersistentItems;
	gboolean bRememberItems;
	gchar *cRememberedItems;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gint iNbItems[4];
	GList *pItems;
	guint iSidClipboardOwnerChange;
	guint iSidPrimaryOwnerChange;
	GList *pActions;
	gboolean bActionsLoaded;
	gboolean bActionBlocked;
	GtkWidget *pActionMenu;
	GldiShortkey *cKeyBinding;
	} ;

#endif
