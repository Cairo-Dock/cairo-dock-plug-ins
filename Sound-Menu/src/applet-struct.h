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
#include "indicator-applet.h"


typedef enum {
	VOLUME_EFFECT_ICONS,
	VOLUME_EFFECT_BAR,
	VOLUME_EFFECT_GAUGE,
	VOLUME_NB_EFFECTS
	} VolumeTypeEffect;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	VolumeTypeEffect iVolumeEffect;
	gchar *cDefaultIcon;
	gchar *cMuteIcon;
	gchar *cGThemePath;
	RendererRotateTheme iRotateTheme;
	gchar *cShortkey;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	CDAppletIndicator *pIndicator;
	GtkWidget* volume_widget;
	GList *transport_widgets_list;
	GtkWidget* voip_widget;
	gint iCurrentState;
	cairo_surface_t *pSurface;
	CairoKeyBinding *pKeyBinding;
	} ;

#endif
