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

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <glib.h>

#include "applet-impulse.h"
#include "Impulse.h"

#define IM_TAB_SIZE 256

extern CairoDockHidingEffect *g_pHidingBackend;  // cairo_dock_is_hidden

////////////////// IMPULSE //////////////////

static void _im_start (void)
{
	cd_debug ("Impulse: start im");
	im_start ();
}

static void _im_stop (void)
{
	cd_debug ("Impulse: stop im");
	im_stop(); // FIXME => if stopped, the client is not stopped and im_getSnapshot(IM_FFT) give always the same thing...
}

void cd_impulse_im_setSourceProperties (void)
{
	cd_debug ("Impulse: iSourceIndex = %d, bUseSink = %d, bUseMonitor = %d",
		myConfig.iSourceIndex, myConfig.bUseSink, myConfig.bUseMonitor);
	im_setSourceProperties (myConfig.iSourceIndex, myConfig.bUseSink, myConfig.bUseMonitor);
}

////////////////// USEFUL FUNCTIONS //////////////////

static void _get_icons_list_without_separators (CDSharedMemory *pSharedMemory)
{
	if (pSharedMemory->pDock == NULL)
	{
		pSharedMemory->pIconsList = NULL;
		return;
	}

	pSharedMemory->bIsUpdatingIconsList = TRUE;

	pSharedMemory->pIconsList = NULL;
	GList *ic;
	Icon *pIcon;
	for (ic = pSharedMemory->pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		// cd_debug ("Impulse: icon name=%s", pIcon->cName);
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			pSharedMemory->pIconsList = g_list_append (pSharedMemory->pIconsList, pIcon);
	}
	g_list_free (ic);
	pSharedMemory->bIsUpdatingIconsList = FALSE;
	cd_debug ("Impulse: updated icons list: %d", g_list_length(pSharedMemory->pIconsList));
}

//static gboolean _animate_the_dock (CDSharedMemory *pSharedMemory)
static gboolean _animate_the_dock (gpointer data)
{
	CD_APPLET_ENTER;
	// cd_debug ("Impulse: in");
	if (myData.pSharedMemory->bIsUpdatingIconsList
		|| cairo_dock_is_hidden (myData.pSharedMemory->pDock)) // not needed for the animations but not for pulse.
		CD_APPLET_LEAVE (TRUE);

	if (myData.pSharedMemory->pIconsList == NULL)
	{
		cd_impulse_stop_animations (TRUE);
		CD_APPLET_LEAVE (FALSE);
	}

	guint iIcons = IM_TAB_SIZE / g_list_length (myData.pSharedMemory->pIconsList); // number of icons (without separators)

	double *array = im_getSnapshot();
	if (!array)
	{
		// backend has been stopped
		cd_impulse_stop_animations (TRUE);
		CD_APPLET_LEAVE (FALSE);
	}

	// we check if there is a signal (most of the time, all values are > 0)
	if (array[0] == 0.0)
	{	// is it really null?
		int j;
		for (j = 1; j < IM_TAB_SIZE; j++)
		{
			if (array[j] != 0)
				break; // there is a signal
		}

		cd_debug ("Impulse: No Signal? %d", j);
		if (j == IM_TAB_SIZE)
			CD_APPLET_LEAVE (TRUE);
		/* TODO?
			* if null, increase the time of the loop?
			* Maybe use: PA_SUBSCRIPTION_EVENT_CLIENT?
			* or something else in pulse?
			*/
	}

	// Computing and animations
	int i;
	double l = 0.0;
	GList *ic = myData.pSharedMemory->pIconsList;
	Icon *pIcon;
	gboolean bHasNotBeenAnimated = TRUE;
	for (i = 0; ic != NULL; i++) // i < 256
	{
		l += array[i]; // a sum for the average
		if (i != 0 && (i % iIcons) == 0)
		{
			pIcon = ic->data;
			// cd_debug ("Impulse: Average: i=%d, l=%f ; I=%d ; l/I=%f ; %s", i, l, iIcons, l/iIcons, pIcon->cName);
			if ((l/iIcons) > myData.pSharedMemory->fMinValueToAnim) // animation
			{
				//cd_debug ("Impulse: animation on this icon=%s", pIcon->cName);
				gldi_icon_request_animation (pIcon,
					myData.pSharedMemory->cIconAnimation,
					myData.pSharedMemory->iNbAnimations);
				bHasNotBeenAnimated = FALSE;
				myData.pSharedMemory->bNeedRefreshIfNotAnimated = TRUE;
			}
			else if (myData.pSharedMemory->bStopAnimations)
				gldi_icon_stop_animation (pIcon);
			l = 0.0;
			ic = ic->next;
		}
	}
	if (bHasNotBeenAnimated && myData.pSharedMemory->bStopAnimations && myData.pSharedMemory->bNeedRefreshIfNotAnimated)
	{
		cd_debug ("Impulse: refresh container");
		cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pSharedMemory->pDock));
		myData.pSharedMemory->bNeedRefreshIfNotAnimated = FALSE; // no need of refresh until the next animation and if bHasNotBeenAnimated
	}

	//cd_debug ("Impulse: out");
	g_list_free (ic);
	CD_APPLET_LEAVE (TRUE);
}

static gboolean _impulse_check_pulse_status (void)
{
	myData.iSidCheckStatus = 0;

	cd_debug ("Impulse: checking PulseAudio Context status");

	if (! myData.bHasBeenStarted && im_context_state () == IM_FAILED)
	{
		cd_impulse_stop_animations (FALSE); // icon will be modified here above
		cd_debug ("Impulse: starting failed");
		gldi_dialogs_remove_on_icon (myIcon);
		gldi_dialog_show_temporary_with_icon (D_("There is something wrong with PulseAudio.\nCan you check its status (installed? running? version?) and report this bug (if any) to forum.glx-dock.org"),
			myIcon,
			myContainer,
			5000,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconImpulseERROR, "impulse-error.svg");
	}

	return FALSE;
}

////////////////// NOTIFICATIONS //////////////////

/*void cd_impulse_start_animations (void)
{
	myData.iSidAnimate = g_timeout_add (myConfig.iLoopTime, (GSourceFunc) _animate_the_dock, NULL);
}*/

void _remove_notifications (void)
{
	gldi_object_remove_notification (&myDockObjectMgr,
		NOTIFICATION_ICON_MOVED,
		(GldiNotificationFunc) cd_impulse_on_icon_changed, NULL);
	gldi_object_remove_notification (&myDockObjectMgr,
		NOTIFICATION_INSERT_ICON,
		(GldiNotificationFunc) cd_impulse_on_icon_changed, NULL);
	gldi_object_remove_notification (&myDockObjectMgr,
		NOTIFICATION_REMOVE_ICON,
		(GldiNotificationFunc) cd_impulse_on_icon_changed, NULL);
}

void _register_notifications (void)
{
	gldi_object_register_notification (&myDockObjectMgr,
		NOTIFICATION_ICON_MOVED,
		(GldiNotificationFunc) cd_impulse_on_icon_changed,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myDockObjectMgr,
		NOTIFICATION_INSERT_ICON,
		(GldiNotificationFunc) cd_impulse_on_icon_changed,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myDockObjectMgr,
		NOTIFICATION_REMOVE_ICON,
		(GldiNotificationFunc) cd_impulse_on_icon_changed,
		GLDI_RUN_FIRST, NULL);
}

////////////////// GENERAL FUNCTIONS //////////////////

void cd_impulse_stop_animations (gboolean bChangeIcon)
{
	//if (myData.pTask != NULL)
	if (myData.iSidAnimate != 0)
	{
		/*gldi_task_discard (myData.pTask);
		myData.pTask = NULL;*/
		g_source_remove (myData.iSidAnimate);
		myData.iSidAnimate = 0;
		// _free_shared_memory (myData.pSharedMemory);
		_remove_notifications ();
	}
	if (myData.bPulseLaunched)
	{
		_im_stop();
		myData.bPulseLaunched = FALSE;
	}
	if (bChangeIcon)
		cd_impulse_draw_current_state ();
	// myData.bPulseLaunched = FALSE; //FIXME => if already started and stopped, it will crash... because not correctly stopped...
}

void cd_impulse_launch_task (void) //(GldiModuleInstance *myApplet)
{
	// if a task is already launching
	/*if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}*/
	if (myData.iSidAnimate != 0)
		cd_impulse_stop_animations (FALSE); // icon will be modified here above

	// PulseAudio Server
	if (! myData.bPulseLaunched)
	{
		_im_start (); // FIXME => if already started and stopped, it will crash... because not correctly stopped...
		myData.bPulseLaunched = TRUE;
	}

	/*myData.pTask = gldi_task_new_full (1,// (SECOND) myConfig.iLoopTime,
		// (GldiGetDataAsyncFunc) _get_icons_list_without_separators,
		NULL,
		(GldiUpdateSyncFunc) _animate_the_dock,
		(GFreeFunc) _free_shared_memory,
		myData.pSharedMemory);
	_get_icons_list_without_separators (myData.pSharedMemory);
	gldi_task_launch (myData.pTask);*/

	_get_icons_list_without_separators (myData.pSharedMemory);
	_register_notifications();

	myData.iSidAnimate = g_timeout_add (myConfig.iLoopTime, (GSourceFunc) _animate_the_dock, NULL); // or into a thread + time?
	cd_debug ("Impulse: animations started (checking status: %d)", myData.iSidCheckStatus);
	cd_impulse_draw_current_state ();

	if (myData.iSidCheckStatus == 0)
		myData.iSidCheckStatus = g_timeout_add_seconds (1, (GSourceFunc) _impulse_check_pulse_status, NULL);
}

gboolean cd_impulse_on_icon_changed (gpointer pUserData, Icon *pIcon, CairoDock *pDock)
{
	// launched and something has changed in the right dock
	//cd_debug ("Impulse: update needed? %d | %d", pDock, myConfig.pDock);
	if (myData.iSidAnimate != 0 && pDock == myConfig.pDock)
	{
		_get_icons_list_without_separators (myData.pSharedMemory);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

void cd_impulse_draw_current_state (void)
{
	if (myData.iSidAnimate != 0)
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconImpulseON, "impulse-running.svg");
	else
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconImpulseOFF, "impulse-stopped.svg");
}

////////////////// DELAY //////////////////

static gboolean _impulse_restart_delayed (void)
{
	myData.iSidRestartDelayed = 0;

	if (! myData.bHasBeenStarted)
	{
		myData.bHasBeenStarted = TRUE;
		cd_message ("Impulse has been started");

		if (myConfig.bFree) // It's maybe a hack but Cairo-Penguin does that :)
		{
			gldi_icon_detach (myIcon);
		}
		else
		{
			gldi_icon_insert_in_container (myIcon, myContainer, CAIRO_DOCK_ANIMATE_ICON);
		}

		cd_impulse_launch_task (); // launched the animations
	}

	return FALSE;
}

void cd_impulse_start_animating_with_delay (void)
{
	if (myData.iSidRestartDelayed != 0)
		return ;

	if (cairo_dock_is_loading ())
		myData.iSidRestartDelayed = g_timeout_add_seconds (2, (GSourceFunc) _impulse_restart_delayed, NULL);  // priority to the loading of the dock
	else
		myData.iSidRestartDelayed = g_timeout_add_seconds (1, (GSourceFunc) _impulse_restart_delayed, NULL);  // if we have to detach the icon
}
