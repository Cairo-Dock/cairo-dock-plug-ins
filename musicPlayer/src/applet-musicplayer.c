/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-musicplayer.h"

CD_APPLET_INCLUDE_MY_VARS

MusicPlayerHandeler *cd_musicplayer_get_handeler_by_name (const gchar *cName) {
	GList *ic;
	MusicPlayerHandeler *handeler=NULL;
	for (ic = myData.pHandelers; ic != NULL; ic = ic->next) {
		handeler = ic->data;
		if (strcmp(handeler->name, cName) == 0)
			return handeler;
	}
	
	return NULL;
}

void cd_musicplayer_arm_handeler (void) { //Prépare l'handeler et le lance
	cd_debug ("Arming %s (with class %s)", myData.pCurrentHandeler->name, myData.pCurrentHandeler->appclass);
	myData.pCurrentHandeler->configure;
	myData.pMeasureTimer = cairo_dock_new_measure_timer (1,
		myData.pCurrentHandeler->acquisition,
		myData.pCurrentHandeler->read_data,
		cd_musicplayer_draw_icon);
	cairo_dock_launch_measure (myData.pMeasureTimer);
}

void cd_musicplayer_disarm_handeler (void) { //Arrete l'handeler en nettoyant la mémoire
	cd_debug ("Disarming %s", myData.pCurrentHandeler->name);
	myData.pCurrentHandeler->free_data;
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
}

void cd_musicplayer_register_my_handeler (MusicPlayerHandeler *pHandeler, const gchar *cName) { //Ajout un Handeler a la GList
	MusicPlayerHandeler *handeler = cd_musicplayer_get_handeler_by_name (cName);
	if (handeler == NULL) //Inutile de rajouter un player déjà présent
		myData.pHandelers = g_list_append (myData.pHandelers, pHandeler);
	else
		cd_warning ("Handeler %s already listed", cName);
}

void cd_musicplayer_free_handeler (MusicPlayerHandeler *pHandeler) { //Libère la mémoire de l'handeler
	myData.pHandelers = g_list_remove (myData.pHandelers, pHandeler);
	pHandeler->free_data;
	
	g_free (pHandeler->name);
	pHandeler->name = NULL;
	g_free (pHandeler->appclass);
	pHandeler->appclass = NULL;
	
	g_free (pHandeler);
	pHandeler = NULL;
}
