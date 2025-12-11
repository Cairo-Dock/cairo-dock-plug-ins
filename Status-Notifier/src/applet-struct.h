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

#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>

typedef enum {
	CD_MODE_COMPACT=0,
	CD_MODE_SUB_DOCK,
	CD_NB_MODES
	} CDDisplayMode;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bCompactMode;  // les items sur l'icone principale ou dans un sous-dock.
	gboolean bResizeIcon;  // si compact, redimensionner l'icone principale automatiquement.
	gint iNbLines;  // si compact et redimensionnement auto, nbre de lignes (colonnes en mode vertical).
	gboolean bHideInactive;
	gboolean bMenuOnLeftClick;
	} ;


typedef enum {
	CD_STATUS_PASSIVE=0,
	CD_STATUS_ACTIVE,
	CD_STATUS_NEEDS_ATTENTION,
	CD_NB_STATUS
	} CDStatusEnum;

typedef enum {
	CD_CATEGORY_APPLICATION_STATUS=0,
	CD_CATEGORY_COMMUNICATIONS,
	CD_CATEGORY_SYSTEM_SERVICES,
	CD_CATEGORY_HARDWARE,
	CD_NB_CATEGORIES
	} CDCategoryEnum;

typedef struct {
	gchar *cIconName;
	GPtrArray *pIconData;  // array of (INT, INT, ARRAY BYTE)
	gchar *cTitle;
	gchar *cMessage;  // can contain a subset of the HTML markup language
	} CDToolTip;

typedef struct {
	// props
	gchar *cService;
	gchar *cObjectPath;
	gchar *cId;
	CDCategoryEnum iCategory;
	CDStatusEnum iStatus;
	gchar *cIconName;
	gchar *cIconThemePath;
	gchar *cAttentionIconName;
	gchar *cTitle;
	// additionnal props supported by Ubuntu
	gchar *cLabel;
	gchar *cLabelGuide;
	gchar *cMenuPath;
	gchar *cAccessibleDesc;
	// additionnal props supported by KDE
	guint iWindowId;
	gchar *cAttentionMovieName;
	gchar *cOverlayIconName;
	CDToolTip *pToolTip;
	gboolean bItemIsMenu;
	gint iPosition;  // donnee par l'indicator service
	guint iSidPopupTooltip;
	// data
	GDBusProxy *pProxyProps;
	GDBusProxy *pProxy;
	DbusmenuGtkMenu *pMenu;
	cairo_surface_t *pSurface; // surface used when drawing in compact mode
	cairo_surface_t *pFallbackIcon; // IconPixmap property, used when no IconName is available
	cairo_surface_t *pFallbackIconAttention; // AttentionIconPixmap property, used when no IconName is available
	guint iSidUpdateIcon;
	GCancellable *pCancel;
	// menu redraw
	int iMenuWidth;
	Icon *pIcon; // last assigned icon to this item
} CDStatusNotifierItem;


//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gchar *cHostName;
	GDBusProxy *pProxyWatcher; // DBus proxy for the KDE SNI watcher interface
	GCancellable *pCancellableWatcher; // cancellable for the above proxy
	GDBusProxy *pProxyIndicatorApplicationService;
	GCancellable *pCancellableIAS;
	gboolean bIASWatched;
	gboolean bBrokenWatcher;
	gboolean bHaveIAS;
	gboolean bHaveWatcher;
	GList *pItems;  // list of all items.
	GHashTable *pThemePaths;
	gint iNbLines, iNbColumns, iItemSize;  // agencement compact.
	CDStatusNotifierItem *pCurrentlyHoveredItem;  // in compact mode, item currently hovered.
	gdouble fDesktopNameAlpha;  // in compact desklet mode, alpha for the currently hovered item title.
	gint iDefaultWidth;  // in compact mode, initial icon size.
	gint iDefaultHeight;
	guint uBusNameID; // ID returned by g_bus_own_name () -- only used by the KDE SNI implementation
	} ;


#endif
