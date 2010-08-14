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

#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "cairo-dock.h"

#include "applet-struct.h"
#include "applet-trashes-manager.h"


static void cd_dustbin_measure_trash (CairoDockModuleInstance *myApplet)
{
	myData._iMeasure = cairo_dock_fm_measure_diretory (myData.cDustbinPath, (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT ? 1 : 0), (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES), &myData.pTask->bDiscard);
}

static gboolean cd_dustbin_display_result (CairoDockModuleInstance *myApplet)
{
	myData.iMeasure = myData._iMeasure;
	//g_print ("trash measure : %d\n", myData.iMeasure);
	
	if (myData.iMeasure == 0)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		if (myData.bDisplayFullIcon)
		{
			myData.bDisplayFullIcon = FALSE;
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cEmptyUserImage);
		}
	}
	else
	{
		if (! myData.bDisplayFullIcon)
		{
			myData.bDisplayFullIcon = TRUE;
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cFullUserImage);
		}
		
		if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_TRASHES)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%s", myData.iMeasure, (myDesklet ? D_(" trashe(s)") : ""));
		}
		else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%s", myData.iMeasure, (myDesklet ? D_(" file(s)") : ""));
		}
		else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			CD_APPLET_SET_SIZE_AS_QUICK_INFO (myData.iMeasure);
		}
	}
	
	CD_APPLET_REDRAW_MY_ICON;
	return TRUE;
}

static void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	//g_print ("%s (%s, %d)\n", __func__, cURI, myData.iMeasure);
	gchar *cQuickInfo = NULL;
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :
		case CAIRO_DOCK_FILE_CREATED :
			if (cairo_dock_task_is_running (myData.pTask) || cairo_dock_task_is_active (myData.pTask))
			{
				//g_print ("cancel measure\n");
				cairo_dock_discard_task (myData.pTask);
				myData.pTask = cairo_dock_new_task (0,
					(CairoDockGetDataAsyncFunc) cd_dustbin_measure_trash,
					(CairoDockUpdateSyncFunc) cd_dustbin_display_result,
					myApplet);
			}
			else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s...", (myDesklet ? D_("calculating") : ""));
			cairo_dock_launch_task_delayed (myData.pTask, 500);
		break ;
		
		default :
			break;
	}
}

void cd_dustbin_start (CairoDockModuleInstance *myApplet)
{
	if (myData.cDustbinPath == NULL)
		myData.cDustbinPath = cairo_dock_fm_get_trash_path (NULL, NULL);
	if (myData.cDustbinPath != NULL)
	{
		myData.bMonitoringOK = cairo_dock_fm_add_monitor_full (myData.cDustbinPath, TRUE, NULL, (CairoDockFMMonitorCallback) cd_dustbin_on_file_event, myApplet);
		if (! myData.bMonitoringOK)
		{
			cd_message ("dustbin : can't monitor trash folder\n we'll check it periodically");
		}
		
		myData.pTask = cairo_dock_new_task (myData.bMonitoringOK ? 0 : 10,  // si le monitoring de fichiers n'est pas disponible, on execute la tache periodiquement.
			(CairoDockGetDataAsyncFunc) cd_dustbin_measure_trash,
			(CairoDockUpdateSyncFunc) cd_dustbin_display_result,
			myApplet);
		
		cairo_dock_launch_task (myData.pTask);  // on la lance meme si on n'affiche rien, pour savoir si le nombre de fichiers est nul ou non.
		if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)  // operation potentiellement longue => on met un petit message discret.
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s...", (myDesklet ? D_("calculating") : ""));
		}
	}
	else
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cEmptyUserImage);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
	}
}

void cd_dustbin_stop (CairoDockModuleInstance *myApplet)
{
	cairo_dock_free_task (myData.pTask);
	myData.pTask = NULL;
	
	if (myData.bMonitoringOK)
	{
		cairo_dock_fm_remove_monitor_full (myData.cDustbinPath, TRUE, NULL);
	}
}
