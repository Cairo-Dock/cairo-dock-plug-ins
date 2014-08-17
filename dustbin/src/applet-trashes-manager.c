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


static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->cDustbinPath);
	g_free (pSharedMemory);
}

static void cd_dustbin_measure_trash (CDSharedMemory *pSharedMemory)
{
	pSharedMemory->iMeasure = cairo_dock_fm_measure_diretory (pSharedMemory->cDustbinPath,
		(pSharedMemory->iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT ? 1 : 0),
		(pSharedMemory->iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT || pSharedMemory->iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES),
		pSharedMemory->bDiscard);
}

static gboolean cd_dustbin_display_result (CDSharedMemory *pSharedMemory)
{
	myData.iMeasure = pSharedMemory->iMeasure;
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
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%zd%s", myData.iMeasure, (myDesklet ? D_(" trashe(s)") : ""));
		}
		else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%zd%s", myData.iMeasure, (myDesklet ? D_(" file(s)") : ""));
		}
		else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			CD_APPLET_SET_SIZE_AS_QUICK_INFO (myData.iMeasure);
		}
		else  // on vire les "..."
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		}
	}
	
	CD_APPLET_REDRAW_MY_ICON;
	return TRUE;
}

static void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, GldiModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	//g_print ("%s (%s, %d)\n", __func__, cURI, myData.iMeasure);
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :
		case CAIRO_DOCK_FILE_CREATED :
			if (gldi_task_is_running (myData.pTask))  // task is running, cancel it since files count has changed, no need to finish this measure.
			{
				//g_print ("cancel measure\n");
				gldi_task_discard (myData.pTask);
				
				CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
				pSharedMemory->cDustbinPath = g_strdup (myData.cDustbinPath);
				pSharedMemory->iQuickInfoType = myConfig.iQuickInfoType;
				myData.pTask = gldi_task_new_full (0,
					(GldiGetDataAsyncFunc) cd_dustbin_measure_trash,
					(GldiUpdateSyncFunc) cd_dustbin_display_result,
					(GFreeFunc) _free_shared_memory,
					pSharedMemory);
				pSharedMemory->bDiscard = &myData.pTask->bDiscard;
			}
			else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)  // task was not running, so no waiting message; let's add it before we launch the task.
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s...", (myDesklet ? D_("calculating") : ""));
			}
			gldi_task_launch_delayed (myData.pTask, 500);
		break ;
		
		default :
			break;
	}
}

void cd_dustbin_start (GldiModuleInstance *myApplet)
{
	// get the trash folder if not already done.
	if (myData.cDustbinPath == NULL)
		myData.cDustbinPath = cairo_dock_fm_get_trash_path (NULL, NULL);
	// monitor this folder.
	if (myData.cDustbinPath != NULL)
	{
		// try monitoring the trash folder.
		myData.bMonitoringOK = cairo_dock_fm_add_monitor_full (myData.cDustbinPath, TRUE, NULL, (CairoDockFMMonitorCallback) cd_dustbin_on_file_event, myApplet);
		if (! myData.bMonitoringOK)
		{
			cd_message ("dustbin : can't monitor trash folder\n we'll check it periodically");
		}
		
		// measure the trash content once, to get the initial stats.
		CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
		pSharedMemory->cDustbinPath = g_strdup (myData.cDustbinPath);
		pSharedMemory->iQuickInfoType = myConfig.iQuickInfoType;
		myData.pTask = gldi_task_new_full (myData.bMonitoringOK ? 0 : 10,  // si le monitoring de fichiers n'est pas disponible, on execute la tache periodiquement.
			(GldiGetDataAsyncFunc) cd_dustbin_measure_trash,
			(GldiUpdateSyncFunc) cd_dustbin_display_result,
			(GFreeFunc) _free_shared_memory,
			pSharedMemory);
		pSharedMemory->bDiscard = &myData.pTask->bDiscard;
		
		gldi_task_launch (myData.pTask);  // on la lance meme si on n'affiche rien, pour savoir si le nombre de fichiers est nul ou non.
		if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)  // operation potentiellement longue => on met un petit message discret.
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s...", (myDesklet ? D_("calculating") : ""));
		}
	}
	else  // no trash, set a N/A icon.
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cEmptyUserImage);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
	}
}

void cd_dustbin_stop (GldiModuleInstance *myApplet)
{
	gldi_task_discard (myData.pTask);
	myData.pTask = NULL;
	
	if (myData.bMonitoringOK)
	{
		cairo_dock_fm_remove_monitor_full (myData.cDustbinPath, TRUE, NULL);
	}
	
	gldi_object_unref (GLDI_OBJECT(myData.pInfoDialog));
	myData.pInfoDialog = NULL;
}
