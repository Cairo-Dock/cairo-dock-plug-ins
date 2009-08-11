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


MusicPlayerHandeler *cd_musicplayer_get_handler_by_name (const gchar *cName) {
	g_return_val_if_fail (cName != NULL, NULL);
	GList *ic;
	MusicPlayerHandeler *handler = NULL;
	for (ic = myData.pHandelers; ic != NULL; ic = ic->next) {
		handler = ic->data;
		if (strcmp(handler->name, cName) == 0)
			return handler;
	}
	return NULL;
}

static void _cd_musicplayer_get_data_async (gpointer data) {
	if (myData.pCurrentHandeler->acquisition)
		myData.pCurrentHandeler->acquisition();
	if (myData.pCurrentHandeler->read_data)
		myData.pCurrentHandeler->read_data();
}

static gboolean _cd_musicplayer_get_data_and_update (gpointer data) {
	if (myData.pCurrentHandeler->acquisition)
		myData.pCurrentHandeler->acquisition();
	if (myData.pCurrentHandeler->read_data)
		myData.pCurrentHandeler->read_data();
	return cd_musicplayer_draw_icon (data);
}

/* Initialise le backend et lance la tache periodique si necessaire.
 */
void cd_musicplayer_launch_handler (void)
{ 
	//cd_debug ("MP : Arming %s (with class %s)", myData.pCurrentHandeler->name, myData.pCurrentHandeler->appclass);
	if (myData.pCurrentHandeler->configure != NULL)
		myData.pCurrentHandeler->configure();
	
	if ((myData.pCurrentHandeler->acquisition || myData.pCurrentHandeler->read_data) && (myData.pCurrentHandeler->iLevel == PLAYER_BAD || (myData.pCurrentHandeler->iLevel == PLAYER_GOOD && (myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT))))  // il y'a de l'acquisition de donnees periodique a faire.
	{
		if (myData.pCurrentHandeler->bSeparateAcquisition == TRUE)  // Utilisation du thread pour les actions longues
		{
  			myData.pTask = cairo_dock_new_task (1,
  				(CairoDockGetDataAsyncFunc) _cd_musicplayer_get_data_async,
  				(CairoDockUpdateSyncFunc) cd_musicplayer_draw_icon,
  				NULL);
		}
		else
		{
  			myData.pTask = cairo_dock_new_task (1,
  				NULL,
  				(CairoDockUpdateSyncFunc) _cd_musicplayer_get_data_and_update,
  				NULL);
		}
		cairo_dock_launch_task (myData.pTask);
	}  // else tout est fait par signaux.
}

/* Arrete le backend en nettoyant la memoire
 */
void cd_musicplayer_stop_handler (void)
{
	if (myData.pCurrentHandeler == NULL)
		return ;
	cd_debug ("MP : stopping %s", myData.pCurrentHandeler->name);
	myData.pCurrentHandeler->free_data();
	cairo_dock_free_task (myData.pTask);
	myData.pTask = NULL;
	myData.dbus_enable = FALSE;
	myData.bIsRunning = FALSE;
	myData.iPlayingStatus = PLAYER_NONE;
}


/* Ajout d'un backend a la liste
 */
void cd_musicplayer_register_my_handler (MusicPlayerHandeler *pHandeler, const gchar *cName)
{
	MusicPlayerHandeler *handler = cd_musicplayer_get_handler_by_name (cName);  // un peu paranoiaque ...
	if (handler == NULL) { //Inutile de rajouter un player déjà présent
		myData.pHandelers = g_list_prepend (myData.pHandelers, pHandeler);
	}
	else
		cd_warning ("MP : Handeler %s already listed", cName);
}


/* Detruit un backend
 */
void cd_musicplayer_free_handler (MusicPlayerHandeler *pHandeler)
{
	if (pHandeler == NULL)
		return ;
	
	g_free (myData.pCurrentHandeler->cCoverDir);
	
	g_free (pHandeler);
}
