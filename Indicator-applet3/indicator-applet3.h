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

#ifndef __CD_INDICATOR_APPLET3__
#define  __CD_INDICATOR_APPLET3__

#include <glib.h>
#include <gtk/gtk.h>
#include <libayatana-indicator/indicator-object.h>

#define INDICATOR_SERVICE_DIR "/usr/share/unity/indicators"

typedef void (* CairoDockIndicator3Func ) (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet);
typedef void (* CairoDockIndicator3FuncMenu ) (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, guint32 timestamp, GldiModuleInstance *myApplet);

IndicatorObject * cd_indicator3_load (const gchar *cName,
	CairoDockIndicator3Func entry_added,
	CairoDockIndicator3Func entry_removed,
	CairoDockIndicator3Func accessible_desc_update,
	CairoDockIndicator3FuncMenu menu_show,
	GldiModuleInstance *myApplet);

void cd_indicator3_unload (IndicatorObject *pIndicator,
	CairoDockIndicator3Func entry_added,
	CairoDockIndicator3Func entry_removed,
	CairoDockIndicator3Func accessible_desc_update,
	CairoDockIndicator3FuncMenu menu_show,
	GldiModuleInstance *myApplet);

const gchar * cd_indicator3_get_label (IndicatorObjectEntry *pEntry);

GtkImage * cd_indicator3_get_image (IndicatorObjectEntry *pEntry);

GtkMenu * cd_indicator3_get_menu (IndicatorObjectEntry *pEntry);

const gchar * cd_indicator3_get_accessible_desc (IndicatorObjectEntry *pEntry);

const gchar * cd_indicator3_get_name_hint (IndicatorObjectEntry *pEntry);

const gchar * cd_indicator3_get_directory_path (void);

#endif /* __CD_INDICATOR_APPLET3__ */
