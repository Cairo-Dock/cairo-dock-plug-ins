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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-composite-manager.h"


  ///////////////////
 /// WM BACKENDS ///
///////////////////

static CDWM *_get_wm_by_name (const gchar *cName)
{
	int i;
	for (i = 0; i < NB_WM; i ++)
	{
		if (strcmp (cName, myData.pWmList[i].cName) == 0)
			return &myData.pWmList[i];
	}
	return NULL;
}
static CDWM *_get_wm_by_index (CDWMIndex n)
{
	if (n < NB_WM)
		return &myData.pWmList[n];
	else
		return NULL;
}

static void _set_metacity_composite (gboolean bActive)
{
	int r;
	if (bActive)
		r = system ("gconftool-2 -s '/apps/metacity/general/compositing_manager' --type bool true");
	else
		r = system ("gconftool-2 -s '/apps/metacity/general/compositing_manager' --type bool false");
	if (r < 0)
		cd_warning ("Not able to launch this command: gconftool-2");
}
static void _set_xfwm_composite (gboolean bActive)
{
	int r;
	if (bActive)
		r = system ("xfconf-query -c xfwm4 -p '/general/use_compositing' -t 'bool' -s 'true'");
	else
		r = system ("xfconf-query -c xfwm4 -p '/general/use_compositing' -t 'bool' -s 'false'");
	if (r < 0)
		cd_warning ("Not able to launch this command: xfconf-query");
}
static void _set_kwin_composite (gboolean bActive)
{
	int r;
	if (bActive)
		r = system ("if test \"`qdbus org.kde.kwin /KWin compositingActive`\" = \"false\";then qdbus org.kde.kwin /KWin toggleCompositing; fi");  // not active, so activating
	else
		r = system ("if test \"`qdbus org.kde.kwin /KWin compositingActive`\" = \"true\"; then qdbus org.kde.kwin /KWin toggleCompositing; fi");  // active, so deactivating
	if (r < 0)
		cd_warning ("Not able to launch this command: qdbus");
}
static void _define_known_wms (void)
{
	myData.pWmList[CD_COMPIZ].cName = "Compiz";
	myData.pWmList[CD_COMPIZ].cCommand = "compiz --replace";
	myData.pWmList[CD_COMPIZ].activate_composite = NULL;
	myData.pWmList[CD_COMPIZ].cConfigTool = "ccsm";
	
	myData.pWmList[CD_KWIN].cName = "KWin";
	myData.pWmList[CD_KWIN].cCommand = "kwin --replace";
	myData.pWmList[CD_KWIN].activate_composite = _set_kwin_composite;
	myData.pWmList[CD_KWIN].cConfigTool = NULL;  /// TODO: find the config tool...
	
	myData.pWmList[CD_XFWM].cName = "Xfwm";
	myData.pWmList[CD_XFWM].cCommand = "xfwm4 --replace";
	myData.pWmList[CD_XFWM].activate_composite = _set_xfwm_composite;
	myData.pWmList[CD_XFWM].cConfigTool = "xfwm4-settings";  // there is also xfwm4-tweaks-settings, wish they merge both ...
	
	myData.pWmList[CD_METACITY].cName = "Metacity";
	myData.pWmList[CD_METACITY].cCommand = "metacity --replace";
	myData.pWmList[CD_METACITY].activate_composite = _set_metacity_composite;
	myData.pWmList[CD_METACITY].cConfigTool = "gconf-editor /apps/metacity";
	
	myData.pWmList[CD_CUSTOM_WMFB].cName = "Fallback";
	myData.pWmList[CD_CUSTOM_WMFB].cCommand = NULL;
	myData.pWmList[CD_CUSTOM_WMFB].activate_composite = NULL;
	myData.pWmList[CD_CUSTOM_WMFB].bIsAvailable = TRUE;
	
	myData.pWmList[CD_CUSTOM_WMC].cName = "Composite";
	myData.pWmList[CD_CUSTOM_WMC].cCommand = NULL;
	myData.pWmList[CD_CUSTOM_WMC].activate_composite = NULL;
	myData.pWmList[CD_CUSTOM_WMC].bIsAvailable = TRUE;
}

static void _check_available_wms (gchar *cWhich)
{
	if (cWhich == NULL)  // no known WM is present, skip the check.
		return;
	
	CDWM *wm;
	wm = _get_wm_by_index (CD_COMPIZ);
	wm->bIsAvailable = (strstr (cWhich, "compiz") != NULL);
	wm = _get_wm_by_index (CD_KWIN);
	wm->bIsAvailable = (strstr (cWhich, "kwin") != NULL);
	wm = _get_wm_by_index (CD_XFWM);
	wm->bIsAvailable = (strstr (cWhich, "xfwm4") != NULL);
	wm = _get_wm_by_index (CD_METACITY);
	wm->bIsAvailable = (strstr (cWhich, "metacity") != NULL);
}

static CDWMIndex _check_current_wm (gchar *cPs)
{
	if (cPs == NULL)  // no known WM is present, skip the check.
		return -1;
	
	if (strstr (cPs, "compiz") != NULL)
		return CD_COMPIZ;
	if (strstr (cPs, "kwin") != NULL)
		return CD_KWIN;
	if (strstr (cPs, "xfwm4") != NULL)
		return CD_XFWM;
	if (strstr (cPs, "metacity") != NULL)
		return CD_METACITY;
	
	return -1;
}


  ////////////////////////
 /// COMPOSITE SIGNAL ///
////////////////////////

static void _on_composited_changed (GdkScreen *pScreen, gpointer data)
{
	myData.bIsComposited = gdk_screen_is_composited (pScreen);
	cd_draw_current_state ();
}
static void _start_watching_composite_state (void)
{
	// get the current state.
	GdkScreen *pScreen = gdk_screen_get_default ();
	myData.bIsComposited = gdk_screen_is_composited (pScreen);
	
	// draw it.
	cd_draw_current_state ();
	
	// listen for future changes.
	g_signal_connect (G_OBJECT (pScreen), "composited-changed", G_CALLBACK(_on_composited_changed), NULL);
}


  ///////////////////
 /// PREFERED WM ///
///////////////////

static CDWM *_get_prefered_wmc (CDWMIndex iCurrentWm)
{
	cd_debug ("%s (%s, %d)", __func__, myConfig.cWmCompositor, iCurrentWm);
	CDWM *wm;
	if (myConfig.cWmCompositor != NULL)  // a composite WM is defined.
	{
		wm = _get_wm_by_name (myConfig.cWmCompositor);
		if (wm == NULL)  // not one of the known WM -> define and take the custom one.
		{
			wm = _get_wm_by_index (CD_CUSTOM_WMC);
			g_free ((gchar*)wm->cCommand);
			wm->cCommand = g_strdup (myConfig.cWmCompositor);
			return wm;
		}
		else if (wm->bIsAvailable)
			return wm;
	}
	
	// no WM defined, or the one defined is not available -> check if a suitable one is running.
	if (iCurrentWm < NB_WM)  // one of the know WM is running
	{
		if (myData.bIsComposited)  // and it provides composite => let's take it!
		{
			wm = _get_wm_by_index (iCurrentWm);
			if (wm->bIsAvailable)  // just to be sure.
				return wm;
		}
	}
	
	// no succes so far, take the most suitable one.
	int index[NB_COMPOSITE_WM] = {CD_COMPIZ, CD_KWIN, CD_XFWM, CD_METACITY};  // in this order by default.
	switch (g_iDesktopEnv)
	{
		case CAIRO_DOCK_GNOME:
			index[1] = CD_METACITY;
			index[3] = CD_KWIN;
		break;
		case CAIRO_DOCK_XFCE:
			index[1] = CD_XFWM;
			index[2] = CD_KWIN;
		break;
		case CAIRO_DOCK_KDE:
		default:
		break;
	}
	int i;
	for (i = 0; i < NB_COMPOSITE_WM; i ++)
	{
		wm = _get_wm_by_index (index[i]);
		cd_debug (" %d) %s, %d", index[i], wm->cName, wm->bIsAvailable);
		if (wm->bIsAvailable)
			return wm;
	}
	return NULL;
}

static CDWM *_get_prefered_wmfb (CDWMIndex iCurrentWm)
{
	cd_debug ("%s (%s, %d)", __func__, myConfig.cWmFallback, iCurrentWm);
	CDWM *wm;
	if (myConfig.cWmFallback != NULL)  // a fallback WM is defined.
	{
		wm = _get_wm_by_name (myConfig.cWmFallback);
		if (wm == NULL)  // not one of the known WM -> define and take the custom one.
		{
			wm = _get_wm_by_index (CD_CUSTOM_WMFB);
			g_free ((gchar*)wm->cCommand);
			wm->cCommand = g_strdup (myConfig.cWmFallback);
			return wm;
		}
		else if (wm->bIsAvailable)
			return wm;
	}
	
	// no WM defined, or the one defined is not available -> check if a suitable one is running.
	if (iCurrentWm < NB_WM)  // one of the know WM is running
	{
		if (!myData.bIsComposited)  // and it is a fallback => let's take it!
		{
			wm = _get_wm_by_index (iCurrentWm);
			cd_debug ("current wm: %d, %d", iCurrentWm, wm->bIsAvailable);
			if (wm->bIsAvailable)  // just to be sure.
				return wm;
		}
	}
	
	// no succes so far, take the most suitable one.
	int index[NB_FALLBACK_WM] = {CD_METACITY, CD_XFWM, CD_KWIN};  // in this order by default.
	switch (g_iDesktopEnv)
	{
		case CAIRO_DOCK_GNOME:
			index[0] = CD_METACITY;
			index[1] = CD_XFWM;
		break;
		case CAIRO_DOCK_XFCE:
			index[0] = CD_XFWM;
			index[1] = CD_METACITY;
		break;
		case CAIRO_DOCK_KDE:
			index[0] = CD_KWIN;
			index[1] = CD_METACITY;
			index[2] = CD_XFWM;
		break;
		case CAIRO_DOCK_UNKNOWN_ENV:
		case CAIRO_DOCK_NB_DESKTOPS:
		break;
	}
	int i;
	for (i = 0; i < NB_FALLBACK_WM; i ++)
	{
		wm = _get_wm_by_index (index[i]);
		cd_debug ("  %s (%d)", wm->cName, wm->bIsAvailable);
		if (wm->bIsAvailable)
			return wm;
	}
	return NULL;
}

static inline gchar *_get_running_wm (void)
{
	return cairo_dock_launch_command_sync ("pgrep -l \"kwin$|compiz$|xfwm4$|metacity$\"");  // -l = write the name of the program (not the command next to the PID in 'ps -ef'. we add a '$' after the names to avoid listing things like compiz-decorator or xfwm4-settings
}
static void _define_prefered_wms (gchar *cPs)
{
	// get the compositor and fallback WMs
	CDWMIndex iCurrentWm = _check_current_wm (cPs);
	myData.wmc = _get_prefered_wmc (iCurrentWm);
	myData.wmfb = _get_prefered_wmfb (iCurrentWm);
	cd_debug ("***** WM: %s / %s", myData.wmc?myData.wmc->cName:NULL, myData.wmfb?myData.wmfb->cName:NULL);
}
void cd_define_prefered_wms (void)
{
	gchar *ps = _get_running_wm ();
	_define_prefered_wms (ps);
	g_free (ps);
}

  /////////////////
 /// INIT/STOP ///
/////////////////

static void _check_wms (CDSharedMemory *pSharedMemory)
{
	pSharedMemory->which = cairo_dock_launch_command_sync ("which compiz kwin xfwm4 metacity");
	
	pSharedMemory->ps = _get_running_wm ();
}
static void _update_from_data (CDSharedMemory *pSharedMemory)
{
	_check_available_wms (pSharedMemory->which);
	
	_define_prefered_wms (pSharedMemory->ps);  // we do it once we know the current state.
	
	gldi_task_discard (myData.pTask);
	myData.pTask = NULL;
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->which);
	g_free (pSharedMemory->ps);
	g_free (pSharedMemory);
}
void cd_init_wms (void)
{
	_define_known_wms ();
	
	_start_watching_composite_state ();
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	myData.pTask = gldi_task_new_full (0,  // one-shot
		(GldiGetDataAsyncFunc) _check_wms,
		(GldiUpdateSyncFunc) _update_from_data,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	gldi_task_launch_delayed (myData.pTask, 3000);  // 3s delay, since we don't need these info right away.
}


void cd_stop_wms (void)
{
	// discard task.
	gldi_task_discard (myData.pTask);
	
	// stop listening
	GdkScreen *pScreen = gdk_screen_get_default ();
	g_signal_handlers_disconnect_by_func (G_OBJECT(pScreen), _on_composited_changed, NULL);
	
	// reset custom WMs.
	CDWM *wm;
	wm = _get_wm_by_index (CD_CUSTOM_WMC);
	g_free ((gchar*)wm->cCommand);
	wm = _get_wm_by_index (CD_CUSTOM_WMFB);
	g_free ((gchar*)wm->cCommand);
}


  ////////////
 /// DRAW ///
////////////

void cd_draw_current_state (void)
{
	cd_debug ("%s (%d)", __func__, myData.bIsComposited);
	if (myData.bIsComposited)
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconCompositeON, "composite-on.png");
	else
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconCompositeOFF, "composite-off.png");
}


  /////////////////////
 /// TOGGLE ON/OFF ///
/////////////////////

static gboolean _activate_composite_delayed (gpointer data)
{
	if (data)
	{
		if (myData.wmc->activate_composite != NULL)
			myData.wmc->activate_composite (TRUE);
	}
	else
	{
		if (myData.wmfb->activate_composite != NULL)
			myData.wmfb->activate_composite (FALSE);
	}
	return FALSE;
}

static gboolean _wm_is_running (CDWM *wm)
{
	const gchar *cCommand = wm->cCommand;
	gchar *cWhich = g_strdup_printf ("pgrep %s$", cCommand);  // see above for the '$' character.
	gchar *str = strchr (cWhich+6, ' ');  // remove any parameter to the command, we just want the program name.
	if (str)  // a space is found.
	{
		*str = '$';
		*(str+1) = '\0';
	}
	gchar *cResult = cairo_dock_launch_command_sync (cWhich);
	gboolean bIsRunning = (cResult != NULL && *cResult != '\0');
	
	g_free (cWhich);
	g_free (cResult);
	return bIsRunning;
}

static void cd_turn_composite_on (void)
{
	if (myData.wmc == NULL)  // no compositor.
	{
		gldi_dialog_show_temporary_with_icon (D_("No compositor is available."), myIcon, myContainer, 6000, "same icon");
		return;
	}
	
	// if not already launched, launch it.
	if (! _wm_is_running (myData.wmc))  // not running
	{
		cairo_dock_launch_command (myData.wmc->cCommand);
		g_timeout_add_seconds (2, _activate_composite_delayed, GINT_TO_POINTER (1));  // let the WM start for 2s.
	}
	else  // already running, just toggle composite ON.
	{
		if (myData.wmc->activate_composite != NULL)
			myData.wmc->activate_composite (TRUE);
		else
			gldi_dialog_show_temporary_with_icon (D_("No compositor is available."), myIcon, myContainer, 6000, "same icon");
	}
}

static void cd_turn_composite_off (void)
{
	if (myData.wmfb == NULL)  // no fallback.
	{
		gldi_dialog_show_temporary_with_icon (D_("No fallback is available."), myIcon, myContainer, 6000, "same icon");
		return;
	}
	
	// if not already launched, launch it.
	if (! _wm_is_running (myData.wmfb))  // not running
	{
		cairo_dock_launch_command (myData.wmfb->cCommand);
		g_timeout_add_seconds (2, _activate_composite_delayed, 0);  // let the WM start for 2s.
	}
	else  // already running, just toggle composite OFF.
	{
		if (myData.wmfb->activate_composite != NULL)
			myData.wmfb->activate_composite (FALSE);
		else
			gldi_dialog_show_temporary_with_icon (D_("No fallback is available."), myIcon, myContainer, 6000, "same icon");
	}
}

void cd_toggle_composite (void)
{
	if (myData.bIsComposited)
		cd_turn_composite_off ();
	else
		cd_turn_composite_on ();
}


  ///////////////////
 /// CONFIG TOOL ///
///////////////////

static const gchar *_get_config_tool (void)
{
	if (myData.bIsComposited && myData.wmc)
	{
		return myData.wmc->cConfigTool;
	}
	else if (!myData.bIsComposited && myData.wmfb)
	{
		return myData.wmfb->cConfigTool;
	}
	return NULL;
}
void cd_open_wm_config (void)
{
	const gchar *cConfigTool = _get_config_tool ();
	
	if (cConfigTool != NULL)
	{
		gchar *cmd = g_strdup_printf ("which %s", cConfigTool);
		gchar *cResult = cairo_dock_launch_command_sync (cmd);
		g_free (cmd);
		if (cResult == NULL || *cResult != '/')
      	{
			gchar *msg = g_strdup_printf (D_("You need to install '%s'"), cConfigTool);
			gldi_dialog_show_temporary_with_icon (msg, myIcon, myContainer, 6000, "same icon");
			g_free (msg);
      	}
		else
		{
			cairo_dock_launch_command (cConfigTool);
		}
	}
	else
	{
		gldi_dialog_show_temporary_with_icon (D_("No configuration tool is available."), myIcon, myContainer, 6000, "same icon");
	}
}


static const gchar *_get_command (void)
{
	if (myData.bIsComposited && myData.wmc)
	{
		return myData.wmc->cCommand;
	}
	else if (!myData.bIsComposited && myData.wmfb)
	{
		return myData.wmfb->cCommand;
	}
	return NULL;
}

void cd_reload_wm (void)
{
	const gchar *cCommand = _get_command ();
	if (cCommand != NULL)
	{
		cairo_dock_launch_command (cCommand);
	}
}
