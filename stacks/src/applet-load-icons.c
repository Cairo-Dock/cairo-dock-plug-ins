/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-stacks.h"
 
CD_APPLET_INCLUDE_MY_VARS

void cd_stacks_build_icons (void) {
	if (myConfig.bLocalDir) {
		myConfig.cMonitoredDirectory = g_strdup_printf("/home/%s/.cairo-dock/stacks", g_getenv ("USER"));
	}
	
	if (myConfig.cMonitoredDirectory == NULL)
    return;
	
  cd_message("Stacks - Now Listing: %s", myConfig.cMonitoredDirectory);  
  
	GList *pIconList = NULL;  // ne nous appartiendra plus, donc ne pas desallouer.
	gchar *cFullURI = NULL;	
	Icon *pDirIcon = NULL;
	
	//On liste le dossier a surveiller
	pIconList = cairo_dock_fm_list_directory (myConfig.cMonitoredDirectory, CAIRO_DOCK_FM_SORT_BY_NAME, 9, myConfig.bHiddenFiles, &cFullURI);
	
	g_list_foreach (pIconList, (GFunc) cd_stacks_debug_icon, NULL);
	
	if (myConfig.bFilter)
		pIconList = cd_stacks_mime_filter(pIconList);
	
	if (myDock) {
		CD_APPLET_CREATE_MY_SUBDOCK (pIconList, myConfig.cRenderer)
	}
	else {
		myDesklet->icons = pIconList;
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);  // on n'a pas besoin du context sur myIcon.
		gtk_widget_queue_draw (myDesklet->pWidget);  // utile ?
	}
	
	if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_stacks_update, NULL))
		cd_warning ("Attention : can't monitor files");
		
	CD_APPLET_REDRAW_MY_ICON
}

void cd_stacks_destroy_icons (void) {
	if (myDock && myIcon->pSubDock != NULL) {
		cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
		myIcon->pSubDock = NULL;
	}
	else if (myDesklet && myDesklet->icons != NULL) {
		g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myDesklet->icons);
		myDesklet->icons = NULL;
	}
}

void cd_stacks_debug_icon(Icon *pIcon) {
	pIcon->cWorkingDirectory = NULL;
}

//A optimiser comme shortcuts
void cd_stacks_update (void) {
	cd_debug("");
	cd_stacks_destroy_icons();  /// comme c'est bourrin ! (cf les signets de shortcuts pour un truc plus optimise).
	cd_stacks_build_icons();
}

void cd_stacks_reload (void) {
	cd_debug("");
	cd_stacks_destroy_icons();
	cd_stacks_build_icons();
}
