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

#include <math.h>
#include <string.h>
#include <dirent.h>
#include <dbus/dbus-glib.h>

#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-proc-acpi.h"
#include "powermanager-sys-class.h"
#include "powermanager-dbus.h"
/* KDE:
$ qdbus org.kde.powerdevil /modules/powerdevil # tab tab
[...]
org.kde.PowerDevil.brightnessChanged
org.kde.PowerDevil.getSupportedSuspendMethods
org.kde.PowerDevil.lidClosed
org.kde.PowerDevil.profileChanged
org.kde.PowerDevil.refreshStatus
org.kde.PowerDevil.reloadAndStream
org.kde.PowerDevil.setBrightness
org.kde.PowerDevil.setPowerSave
org.kde.PowerDevil.setProfile
org.kde.PowerDevil.stateChanged
org.kde.PowerDevil.streamData
org.kde.PowerDevil.suspend
org.kde.PowerDevil.turnOffScreen */

/*
get /org/freedesktop/PowerManagement -> ko => display broken.svg, abort
get /org/freedesktop/PowerManagement/Widget -> ko => display ourselves
get /org/freedesktop/PowerManagement/Statistics -> ko => if Widget: use it, else: display broken.svg, abort (possibly use a fallback low-level method)

representation: gauge/graph/icons/Widget

title: Widget, else: "time until discharge/charge: xxmn / xhyy (xx%)"
quick-info: none/charge "xx%"/time "xhyy"
*/

#define CD_POWER_MANAGER_ADDR "org.freedesktop.PowerManagement"
#define CD_POWER_MANAGER_OBJ "org/freedesktop/PowerManagement"
#define CD_POWER_MANAGER_IFACE "org.freedesktop.PowerManagement"
#define CD_POWER_MANAGER_STATS_OBJ "org/freedesktop/PowerManagement/Statistics"
#define CD_POWER_MANAGER_STATS_IFACE "org.freedesktop.PowerManagement.Statistics"
#define CD_POWER_MANAGER_WIDGET_IFACE "org.freedesktop.PowerManagement.Widget	"
#define CD_POWER_MANAGER_WIDGET_OBJ "/org/freedesktop/PowerManagement/Widget"

static DBusGProxyCall *s_pDetectPMCall = NULL;
static DBusGProxyCall *s_pGetStateCall = NULL;
static DBusGProxyCall *s_pGetStatsCall = NULL;
static DBusGProxyCall *s_pGetDescriptionCall = NULL;
static DBusGProxyCall *s_pGetIconCall = NULL;
static void on_battery_changed(DBusGProxy *proxy, gboolean onBattery, gpointer data);


  ///////////////
 /// SIGNALS ///
///////////////

static void on_battery_changed (DBusGProxy *proxy, gboolean bOnBattery, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("%s (%d)", __func__, bOnBattery);
	
	// store the new state
	myData.bOnBattery = bOnBattery;
	
	// update the icon.
	update_icon ();
	
	CD_APPLET_LEAVE ();
}

static void on_description_changed (DBusGProxy *proxy, const gchar *cDescription, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("%s (%s)", __func__, cDescription);
	CD_APPLET_SET_NAME_FOR_MY_ICON (cDescription);
	CD_APPLET_LEAVE ();
}

static void on_icon_changed (DBusGProxy *proxy, const gchar *cImage, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("%s (%s)", __func__, cImage);
	if (myData.pProxyStats == NULL)
		CD_APPLET_SET_IMAGE_ON_MY_ICON (cImage);  // the image represents both the current charge and the state.
	CD_APPLET_LEAVE ();
}


  ////////////////
 /// GET DATA ///
////////////////

static void _on_get_state (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("%s ()", __func__);
	s_pGetStateCall = NULL;
	gboolean bOnBattery;
	GError *erreur = NULL;
	
	// fetch the state from the request result.
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_BOOLEAN, &bOnBattery,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_debug (" couldn't get the current state (%s)", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		myData.bOnBattery = FALSE;
	}
	else
	{
		myData.bOnBattery = bOnBattery;
	}
	
	// get the stats
	update_stats ();
	
	// update the icon.
	update_icon ();
	
	CD_APPLET_LEAVE ();
}
void cd_get_current_state (void)
{
	if (s_pGetStateCall != NULL)
		return;
	cd_debug ("");
	s_pGetStateCall = dbus_g_proxy_begin_call (myData.pProxyPower,
		"GetOnBattery",
		(DBusGProxyCallNotify)_on_get_state,
		myApplet,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}

static void _on_get_description (DBusGProxy *proxy, DBusGProxyCall *call_id, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("%s ()", __func__);
	s_pGetDescriptionCall = NULL;
	gchar *cDescription = NULL;
	GError *erreur = NULL;
	
	// fetch the state from the request result.
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_STRING, &cDescription,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_debug (" couldn't get the current description (%s)", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		// don't try this method any more.
		if (myData.pProxyWidget)
		{
			g_object_unref (myData.pProxyWidget);
			myData.pProxyWidget = NULL;
		}
	}
	else
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (cDescription);
	}
	
	g_free (cDescription);
	CD_APPLET_LEAVE ();
}
void cd_get_widget_description (void)
{
	if (s_pGetDescriptionCall != NULL || myData.pProxyWidget == NULL)
		return;
	s_pGetDescriptionCall = dbus_g_proxy_begin_call (myData.pProxyWidget,
		"GetDescription",
		(DBusGProxyCallNotify)_on_get_description,
		NULL,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}

static void _on_get_icon (DBusGProxy *proxy, DBusGProxyCall *call_id, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("%s ()", __func__);
	s_pGetIconCall = NULL;
	gchar *cImage = NULL;
	GError *erreur = NULL;
	
	// fetch the state from the request result.
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_STRING, &cImage,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_debug (" couldn't get the current image (%s)", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		// don't try this method any more.
		if (myData.pProxyWidget)
		{
			g_object_unref (myData.pProxyWidget);
			myData.pProxyWidget = NULL;
		}
	}
	else
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (cImage);
	}
	
	g_free (cImage);
	CD_APPLET_LEAVE ();
}
void cd_get_widget_icon (void)
{
	if (s_pGetIconCall != NULL || myData.pProxyWidget == NULL)
		return;
	s_pGetIconCall = dbus_g_proxy_begin_call (myData.pProxyWidget,
		"GetIcon",
		(DBusGProxyCallNotify)_on_get_icon,
		NULL,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}

static void _cd_get_one_stat (const gchar *cDataType, gboolean bFirst);
static void _on_get_data (DBusGProxy *proxy, DBusGProxyCall *call_id, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("%s ()", __func__);
	gboolean bFirstRound = (data != NULL);
	s_pGetStatsCall = NULL;
	
	GError *erreur = NULL;
	
	// fetch the data from the request result.
	int val = 0;
	GType g_type_ptrarray = dbus_g_type_get_collection ("GPtrArray",
		dbus_g_type_get_struct("GValueArray",
			G_TYPE_INT,
			G_TYPE_INT,
			G_TYPE_INT,
			G_TYPE_INVALID));
	GPtrArray *ptrarray = NULL;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		g_type_ptrarray, &ptrarray,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		cd_debug (" couldn't get the current data (%s)", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		// don't try this method any more.
		g_object_unref (myData.pProxyStats);
		myData.pProxyStats = NULL;
		// instead, rely on the widget icon.
		cd_get_widget_icon ();
		CD_APPLET_LEAVE ();
	}
	else
	{
		cd_debug (" got %d values", ptrarray->len);
		GValueArray *va = (GValueArray *) g_ptr_array_index (ptrarray, ptrarray->len-1);  // get the latest data
		GValue *v = g_value_array_get_nth (va, 2);  // (x, y, data)
		if (v && G_VALUE_HOLDS_INT (v))
			val = g_value_get_int (v);
		cd_debug (" data: %d", val);
		
		g_ptr_array_foreach (ptrarray, (GFunc)g_value_array_free, NULL);
		g_ptr_array_free (ptrarray, TRUE);
	}
	
	if (bFirstRound)
	{
		cd_debug ("  got percentage: %d%%\n", (int)val);
		myData.iPercentage = val;
		_cd_get_one_stat ("time", FALSE);
	}
	else
	{
		cd_debug ("  got time: %d%%\n", (int)val);
		myData.iTime = val;
		update_icon ();
	}
	
	CD_APPLET_LEAVE ();
}
static void _cd_get_one_stat (const gchar *cDataType, gboolean bFirst)
{
	if (s_pGetStatsCall != NULL || myData.pProxyStats == NULL)
		return;
	s_pGetStatsCall = dbus_g_proxy_begin_call (myData.pProxyStats,
		"GetData",
		(DBusGProxyCallNotify)_on_get_data,
		GINT_TO_POINTER (bFirst),
		(GDestroyNotify) NULL,
		G_TYPE_UINT, 0,  // we just want the latest data
		G_TYPE_STRING, cDataType,
		G_TYPE_INVALID);
}
void cd_get_stats_from_bus (void)
{
	cd_debug ("");
	_cd_get_one_stat ("percentage", TRUE);
}


  ////////////////
 /// SERVICE ///
////////////////

static void _on_start_service (DBusGProxy *proxy, guint status, GError *error, gpointer data)  // just for info
{
	if (status != DBUS_START_REPLY_SUCCESS && status != DBUS_START_REPLY_ALREADY_RUNNING)  // service is not started.
	{
		if (error != NULL)  // couldn't start the service
			cd_debug ("Unable to start the Power-Manager service (%s)", error->message);
		else
			cd_debug ("Unable to start the Power-Manager service (got status %d)", status);
		return;
	}
	cd_debug ("Power-Manager service has started");
}

static void _on_power_manager_owner_changed (gboolean bOwned, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("Power-Manager service is on the bus (%d)", bOwned);
	if (bOwned)
	{
		// set up a proxy to the Service
		myData.pProxyPower = cairo_dock_create_new_session_proxy (
			CD_POWER_MANAGER_ADDR,
			CD_POWER_MANAGER_OBJ,
			CD_POWER_MANAGER_IFACE);
		
		dbus_g_proxy_add_signal(myData.pProxyPower, "OnBatteryChanged",
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.pProxyPower, "OnBatteryChanged",
			G_CALLBACK(on_battery_changed), NULL, NULL);
		
		// set up a proxy to the optionnal Statistics object
		myData.pProxyStats = cairo_dock_create_new_session_proxy (
			CD_POWER_MANAGER_ADDR,
			CD_POWER_MANAGER_STATS_OBJ,
			CD_POWER_MANAGER_STATS_IFACE);
		
		// set up a proxy to the optionnal Widget object
		myData.pProxyWidget = cairo_dock_create_new_session_proxy (
			CD_POWER_MANAGER_ADDR,
			CD_POWER_MANAGER_WIDGET_OBJ,
			CD_POWER_MANAGER_WIDGET_IFACE);
		
		dbus_g_proxy_add_signal(myData.pProxyWidget, "DescriptionChanged",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.pProxyPower, "DescriptionChanged",
			G_CALLBACK(on_description_changed), NULL, NULL);
		
		dbus_g_proxy_add_signal(myData.pProxyWidget, "IconChanged",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(myData.pProxyPower, "IconChanged",
			G_CALLBACK(on_icon_changed), NULL, NULL);
		
		// get the current state.
		if (myData.cBatteryStateFilePath == NULL)  // couldn't get the state from files, so get it now from dbus.
			cd_get_current_state ();
		
		// get the current description.
		cd_get_widget_description ();
	}
	else  // no more service on the bus.
	{
		g_object_unref (myData.pProxyPower);
		myData.pProxyPower = NULL;
		
		if (myData.pProxyStats)
		{
			g_object_unref (myData.pProxyStats);
			myData.pProxyStats = NULL;
		}
		
		if (myData.pProxyWidget)
		{
			g_object_unref (myData.pProxyWidget);
			myData.pProxyWidget = NULL;
		}
	}
	CD_APPLET_LEAVE ();
}
static void _on_detect_power_manager (gboolean bPresent, gpointer data)
{
	CD_APPLET_ENTER;
	cd_debug ("Power-Manager is present: %d", bPresent);
	s_pDetectPMCall = NULL;
	if (bPresent)
	{
		_on_power_manager_owner_changed (TRUE, NULL);
	}
	else  // not present, maybe the service is not started => try starting it.
	{
		cd_debug ("  try to start the Power-Manager service...");
		DBusGProxy *dbus_proxy = cairo_dock_get_main_proxy ();
		org_freedesktop_DBus_start_service_by_name_async (dbus_proxy,
			CD_POWER_MANAGER_ADDR,
			0,
			_on_start_service,
			myApplet);
		
		// until it's launched, get the current values from files.
		update_stats ();
		update_icon ();
	}
	
	// now that we know if we can use Dbus or not, get the current values with what we have found since the beginning.
	update_stats ();
	update_icon();
	
	// and periodically update the data.
	if (myData.checkLoop == 0)
		myData.checkLoop = g_timeout_add_seconds (myConfig.iCheckInterval, (GSourceFunc) update_stats_loop, (gpointer) NULL);
	
	// watch whenever the Service goes up or down.
	cairo_dock_watch_dbus_name_owner (CD_POWER_MANAGER_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_power_manager_owner_changed,
		NULL);
	CD_APPLET_LEAVE ();
}
void cd_detect_power_manager_on_bus (void)
{
	s_pDetectPMCall = cairo_dock_dbus_detect_application_async (CD_POWER_MANAGER_ADDR,
		(CairoDockOnAppliPresentOnDbus) _on_detect_power_manager,
		NULL);
}


void cd_disconnect_from_bus (void)
{
	// cancel current calls
	if (s_pDetectPMCall != NULL)
	{
		dbus_g_proxy_cancel_call (myData.pProxyPower, s_pDetectPMCall);
		s_pDetectPMCall = NULL;
		
	}
	if (s_pGetStateCall != NULL)
	{
		dbus_g_proxy_cancel_call (myData.pProxyPower, s_pGetStateCall);
		s_pGetStateCall = NULL;
		
	}
	if (s_pGetStatsCall != NULL)
	{
		dbus_g_proxy_cancel_call (myData.pProxyStats, s_pGetStatsCall);
		s_pGetStatsCall = NULL;
		
	}
	if (s_pGetDescriptionCall != NULL)
	{
		dbus_g_proxy_cancel_call (myData.pProxyWidget, s_pGetDescriptionCall);
		s_pGetDescriptionCall = NULL;
		
	}
	if (s_pGetIconCall != NULL)
	{
		dbus_g_proxy_cancel_call (myData.pProxyWidget, s_pGetIconCall);
		s_pGetIconCall = NULL;
		
	}
	
	// destroy proxies
	g_object_unref (myData.pProxyPower);
	myData.pProxyPower = NULL;

	if (myData.pProxyStats)
	{
		g_object_unref (myData.pProxyStats);
		myData.pProxyStats = NULL;
	}

	if (myData.pProxyWidget)
	{
		g_object_unref (myData.pProxyWidget);
		myData.pProxyWidget = NULL;
	}
}

gboolean update_stats (void)
{
	if (myData.cBatteryStateFilePath != NULL)  // found the battery, get the info from files and compute ourselves.
	{
		if (myData.bProcAcpiFound)
			cd_get_stats_from_proc_acpi ();
		else
			cd_get_stats_from_sys_class ();
	}
	else if (myData.pProxyStats != NULL)
	{
		cd_get_stats_from_bus ();
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

gboolean update_stats_loop (void)
{
	CD_APPLET_ENTER;
	
	gboolean bContinue = update_stats ();
	
	update_icon ();
	
	if (! bContinue)
		myData.checkLoop = 0;
	CD_APPLET_LEAVE (bContinue);
}


  ///////////////
 /// ACTIONS ///
///////////////

void power_halt(void)
{
	dbus_g_proxy_call (myData.pProxyPower, "Shutdown", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_hibernate(void)
{
	dbus_g_proxy_call (myData.pProxyPower, "Hibernate", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_suspend(void)
{
	dbus_g_proxy_call (myData.pProxyPower, "Suspend", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_reboot(void)
{
	dbus_g_proxy_call (myData.pProxyPower, "Reboot", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
