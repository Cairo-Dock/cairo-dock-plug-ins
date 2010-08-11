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

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bCompactMode;
	gint iItemMinSize;
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
	gchar *cId;
	gchar *cTitle;
	CDStatusEnum iStatus;
	CDCategoryEnum iCategory;
	Window iWindowId;
	gchar *cIconName;
	gchar *cIconThemePath;
	gchar *cAttentionIconName;
	gchar *cAttentionMovieName;
	gchar *cOverlayIconName;
	CDToolTip *pToolTip;
	guint iSidPopupTooltip;
	// data
	DBusGProxy *pProxyProps;
	DBusGProxy *pProxy;
	cairo_surface_t *pSurface;
	GLuint iTexture;
	Icon *pIcon; 
} CDStatusNotifierItem;


//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	DBusGProxy *pProxyWatcher;
	gchar *cHostName;
	GList *pItems;
	guint iSidGetWatcher;
	} ;


#endif
