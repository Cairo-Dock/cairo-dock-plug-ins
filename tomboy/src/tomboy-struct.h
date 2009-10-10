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

#ifndef __TOMBOY_STRUCT__
#define  __TOMBOY_STRUCT__

#include <cairo-dock.h>
#include <cairo-dock-applet-single-instance.h>

struct _AppletConfig {
	gchar *defaultTitle;
	gchar *cIconDefault;
	gchar *cIconClose;
	gchar *cIconBroken;
	gchar *cIconEmpty;
	gboolean bNoDeletedSignal;
	gint iAppControlled;
	gchar *cRenderer;
	gboolean bDrawContent;
	gboolean bPopupContent;
	gchar *cDateFormat;
	gboolean bAutoNaming;
	gboolean bAskBeforeDelete;
	gdouble fTextColor[3];
	gint iDialogDuration;
	} ;

struct _AppletData {
	cairo_surface_t *pSurfaceDefault;
	cairo_surface_t *pSurfaceNote;
	gboolean dbus_enable;
	gboolean opening;
	guint iSidCheckNotes;
	GHashTable *hNoteTable;
	CairoDockTask *pTask;
	guint iSidResetQuickInfo;
	guint iSidPopupDialog;
	} ;

#endif
