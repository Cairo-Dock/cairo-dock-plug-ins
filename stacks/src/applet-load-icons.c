/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
 
CD_APPLET_INCLUDE_MY_VARS

void cd_stacks_build_icons (void) {
  if (myConfig.cMonitoredDirectory == NULL)
    return;

  cd_message("Stacks - Now Listing: %s",myConfig.cMonitoredDirectory);  
  
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;	
	Icon *pDirIcon = NULL;
	
	//On liste le dossier a surveiller
	pIconList = cairo_dock_fm_list_directory (myConfig.cMonitoredDirectory, CAIRO_DOCK_FM_SORT_BY_NAME, 1, myConfig.bHiddenFiles, &cFullURI);
	
	if (myDock) {
		myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
		cairo_dock_set_renderer (myIcon->pSubDock, "Parabolic");
		cairo_dock_update_dock_size (myIcon->pSubDock);
	}
	else {
		myDesklet->icons = pIconList;
  	gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
  	cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
  	myDrawContext = cairo_create (myIcon->pIconBuffer);
  	gtk_widget_queue_draw (myDesklet->pWidget);
	}
	
	if (! cairo_dock_fm_add_monitor_full (cFullURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_stacks_update, NULL))
			cd_warning ("Attention : can't monitor files");
	
}


//La fonction pose probleme, elle segfault lors des free.
void cd_stacks_destroy_icons (void) {
	if (myIcon->pSubDock != NULL) {
		//cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
		myIcon->pSubDock = NULL;
	}
	if (myDesklet && myDesklet->icons != NULL) {
		//g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		//g_list_free (myDesklet->icons);
		myDesklet->icons = NULL;
	}
}

//A retravailler
void cd_stacks_update (void) {
	cd_stacks_destroy_icons();
	cd_stacks_build_icons();
}
