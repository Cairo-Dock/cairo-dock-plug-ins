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

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"
 

#define MY_NB_ICON_STATE 3
#define MY_NB_SUB_ICONS 5

static const gchar *s_iconName[MY_NB_SUB_ICONS] = {N_("Configure Compiz"), N_("Emerald Manager"), N_("Reload WM"), N_("Exposition"), N_("Widget Layer")};

static const gchar *s_iconClass[MY_NB_SUB_ICONS] = {"ccsm", "emerald-theme-manager", NULL, NULL, NULL};

static const gchar *s_iconFile[MY_NB_ICON_STATE] = {"default.svg", "broken.svg", "other.svg"};


static GList * _list_icons (void) {
	GList *pIconList = NULL;
	
	Icon *pIcon;
	int i, j=3;
	if (myConfig.bScriptSubDock)
		j = 5;
  
	for (i = 0; i < j; i ++)
	{
		if (i == 1 && ! myConfig.bEmeraldIcon)
			continue;
		pIcon = cairo_dock_create_dummy_launcher (g_strdup (D_(s_iconName[i])),
			(myConfig.cUserImage[i+MY_NB_ICON_STATE] != NULL ?
				cairo_dock_generate_file_path (myConfig.cUserImage[i+MY_NB_ICON_STATE]) :
				g_strdup_printf ("%s/%d.svg", MY_APPLET_SHARE_DATA_DIR, i)),
			(s_iconClass[i] != NULL ? g_strdup (s_iconClass[i]) : g_strdup ("none")),
			NULL,
			2*i);
		pIcon->cParentDockName = g_strdup (myIcon->cName);
		pIconList = g_list_append (pIconList, pIcon);
		if (myConfig.bStealTaskBarIcon && s_iconClass[i] != NULL)
		{
			cairo_dock_inhibate_class (s_iconClass[i], pIcon);
		}
	}
	
	return pIconList;
}

void cd_compiz_update_main_icon (void) {
	gboolean bNeedsRedraw = FALSE;
	if (myData.bAcquisitionOK) {
		if (myData.bCompizIsRunning && myData.iCompizIcon != COMPIZ_DEFAULT) {
// 			//cd_debug ("COMPIZ_DEFAULT");
			myData.iCompizIcon = COMPIZ_DEFAULT;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_DEFAULT], "default.svg");
			bNeedsRedraw = TRUE;
		}
		else if (! myData.bCompizIsRunning && myData.iCompizIcon != COMPIZ_OTHER) {
			//cd_debug ("COMPIZ_OTHER");
			myData.iCompizIcon = COMPIZ_OTHER;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_OTHER], "other.svg");
			bNeedsRedraw = TRUE;
		}
	}
	else {
		if (myData.iCompizIcon != COMPIZ_BROKEN) {
			//cd_debug ("COMPIZ_BROKEN");
			myData.iCompizIcon = COMPIZ_BROKEN;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_BROKEN], "broken.svg");
			bNeedsRedraw = TRUE;
		}
	}
	if (bNeedsRedraw)
		CD_APPLET_REDRAW_MY_ICON;
}


void cd_compiz_build_icons (void) {
	if (myIcon->cName == NULL && myDock)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (COMPIZ_DEFAULT_NAME);
	}
	GList *pIconList = _list_icons ();  // ne nous appartiendra plus, donc ne pas desallouer.
	
	gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
	CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, "Caroussel", pConfig);
}
