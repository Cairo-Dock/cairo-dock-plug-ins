#include <string.h>
#include <dirent.h>
#include <dbus/dbus-glib.h>

#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-dbus.h"

#define MY_BATTERY_DIR "/proc/acpi/battery"
#define MY_DEFAULT_BATTERY_NAME "BAT0"

static DBusGProxy *dbus_proxy_power = NULL;
///static DBusGProxy *dbus_proxy_stats = NULL;
///static DBusGProxy *dbus_proxy_battery = NULL;

CD_APPLET_INCLUDE_MY_VARS

static gboolean cd_powermanager_find_battery (void) {
	GError *erreur = NULL;
	GDir *dir = g_dir_open (MY_BATTERY_DIR, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("powermanager : %s", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	
	GString *sBatteryStateFilePath = g_string_new ("");
	gchar *cContent = NULL, *cPresentLine;
	gsize length=0;
	const gchar *cBatteryName;
	gboolean bBatteryFound = FALSE;
	do
	{
		cBatteryName = g_dir_read_name (dir);
		if (cBatteryName == NULL)
			break ;
		
		g_string_printf (sBatteryStateFilePath, "%s/%s/info", MY_BATTERY_DIR, cBatteryName);
		length=0;
		cd_debug ("  examen de la batterie '%s' ...", sBatteryStateFilePath->str);
		g_file_get_contents(sBatteryStateFilePath->str, &cContent, &length, &erreur);
		if (erreur != NULL)
		{
			cd_warning("powermanager : %s", erreur->message);
			g_error_free(erreur);
			erreur = NULL;
		}
		else
		{
			gchar *str = strchr (cContent, '\n');  // "present:    yes"
			if (str != NULL)
			{
				*str = '\0';
				
				if (g_strstr_len (cContent, -1, "yes") != NULL)  // on l'a trouvee !
				{
					bBatteryFound = TRUE;
					myData.cBatteryStateFilePath = g_strdup_printf ("%s/%s/state", MY_BATTERY_DIR, cBatteryName);
					
					str ++;
					gchar *str2 = strchr (str, ':');
					if (str2 != NULL)
					{
						str2 ++;
						myData.iCapacity = atoi (str2);
						g_print ("capacite de la batterie : %d mWsh\n", myData.iCapacity);
					}
				}
				else
				{
					g_print ("cette batterie (%s) n'est pas presente.\n", cBatteryName);
				}
			}
		}
		g_free (cContent);
	}
	while (! bBatteryFound);
	g_dir_close (dir);
	return bBatteryFound;
}

gboolean dbus_connect_to_bus (void)
{
	cd_message ("");
	
	if (cairo_dock_bdus_is_enabled ())
	{
		dbus_proxy_power = cairo_dock_create_new_session_proxy (
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement",
			"org.freedesktop.PowerManagement"
		);

		/**dbus_proxy_stats = cairo_dock_create_new_session_proxy (
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement/Statistics",
			"org.freedesktop.PowerManagement.Statistics"
		);*/
		
		dbus_g_proxy_add_signal(dbus_proxy_power, "OnBatteryChanged",
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);
			
		dbus_g_proxy_connect_signal(dbus_proxy_power, "OnBatteryChanged",
			G_CALLBACK(on_battery_changed), NULL, NULL);
		
		gboolean bBatteryFound = cd_powermanager_find_battery();
		if (! bBatteryFound)  // on n'a pas trouve de batterie nous-meme.
		{
			gchar *cBatteryName = MY_DEFAULT_BATTERY_NAME;  // utile ? si on a rien trouve, c'est surement qu'il n'y a pas de batterie non ?
			cd_warning ("No battery were found, trying with default one : %s, with DBus", cBatteryName);
			
			/**cd_message ("Battery Name : %s", cBatteryName);
			gchar *batteryPath = g_strdup_printf ("/org/freedesktop/Hal/devices/acpi_%s", cBatteryName);
			cd_debug ("  batteryPath : %s", batteryPath);
			dbus_proxy_battery = cairo_dock_create_new_system_proxy (
				"org.freedesktop.Hal",
				batteryPath,
				"org.freedesktop.Hal.Device"
			);
			cd_debug ("  acquisition de la batterie -> %x", dbus_proxy_battery);
			myData.battery_present = (dbus_proxy_battery != NULL);  // a priori toujours vrai.
			g_free (batteryPath);
			
			detect_battery ();*/
		}
		else
		{
			myData.battery_present = TRUE;  // a verifier mais ca parait logique.
			g_print ("batterie presente\n");
		}
		
		return TRUE;
	}
	return FALSE;
}

void dbus_disconnect_from_bus (void)
{
	cd_message ("");
	if (dbus_proxy_power != NULL)
	{
		dbus_g_proxy_disconnect_signal(dbus_proxy_power, "OnBatteryChanged",
			G_CALLBACK(on_battery_changed), NULL);
		cd_message ("OnBatteryChanged deconnecte");
		g_object_unref (dbus_proxy_power);
		dbus_proxy_power = NULL;
	}
	/**if (dbus_proxy_battery != NULL)
	{
		g_object_unref (dbus_proxy_battery);
		dbus_proxy_battery = NULL;
	}
	if (dbus_proxy_stats != NULL)
	{
		g_object_unref (dbus_proxy_stats);
		dbus_proxy_stats = NULL;
	}*/
}


void on_battery_changed(DBusGProxy *proxy, gboolean onBattery, gpointer data)
{
	g_print ("Dbus : batery changed\n");
	if (myData.on_battery != onBattery)
	{
		update_stats();
		update_icon();
	}
}


#define go_to_next_line \
	cCurLine = strchr (cCurVal, '\n'); \
	g_return_val_if_fail (cCurLine != NULL, FALSE); \
	cCurLine ++; \
	cCurVal = cCurLine;

#define jump_to_value \
	cCurVal = strchr (cCurLine, ':'); \
	g_return_val_if_fail (cCurVal != NULL, FALSE); \
	cCurVal ++; \
	while (*cCurVal == ' ') \
		cCurVal ++;

/*void get_on_battery(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "GetOnBattery", NULL,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &myData.on_battery,
		G_TYPE_INVALID);
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(myData.cBatteryStateFilePath, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning("powermanager : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.on_battery = FALSE;
	}
	else
	{
		gchar *cCurLine = cContent, *cCurVal = cContent;
		
		go_to_next_line
		go_to_next_line
		jump_to_value
		myData.on_battery = (*cCurVal == 'd');  // discharging
		
		g_free (cContent);
	}
}*/

gboolean update_stats(void)
{
	/**if(myData.battery_present)
	{
		get_on_battery();
		myData.battery_charge = get_stats("charge");
		myData.battery_time = get_stats("time");
		cd_debug ("PowerManager [On Battery]: %d", myData.on_battery); 
		update_icon();
	}*/
	if (myData.cBatteryStateFilePath == NULL)
		cd_powermanager_find_battery ();
	if (myData.cBatteryStateFilePath == NULL)
		return TRUE;
	
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(myData.cBatteryStateFilePath, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning("powermanager : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		return FALSE;
	}
	else
	{
		gchar *cCurLine = cContent, *cCurVal = cContent;
		
		jump_to_value
		gboolean bBatteryPresent = (*cCurVal == 'y');
		if (bBatteryPresent != myData.battery_present)
		{
			myData.battery_present = bBatteryPresent;
			if (! bBatteryPresent)
			{
				g_print ("la batterie a ete enlevee\n");
				g_free (cContent);
				update_icon();
				return TRUE;
			}
			else
			{
				g_print ("la batterie a ete connectee\n");
				myData.previous_battery_time = 0;
				myData.previous_battery_charge = 0;
			}
		}
		
		go_to_next_line  // "present: yes"
		
		go_to_next_line  // capacity state: ok
		
		jump_to_value
		myData.on_battery = (*cCurVal == 'd');  // discharging
		
		go_to_next_line  // charging state: discharging
		
		jump_to_value
		int iPresentRate = atoi (cCurVal);  // 15000 mW OU 1400 mA
		/*cCurVal ++;
		while (*cCurVal != ' ')
			cCurVal ++;
		while (*cCurVal == ' ')
			cCurVal ++;
		if (*cCurVal != 'm')
			cd_warning ("PowerManager : expecting mA or mW as the present rate unit");
		cCurVal ++;
		if (*cCurVal == 'W')
			bWatt = TRUE;
		else if (*cCurVal == 'A')
			bWatt = FALSE;
		else
			cd_warning ("PowerManager : expecting A or W as the present rate unit");*/
		
		go_to_next_line  // present rate: 15000 mW
		
		jump_to_value
		int iRemainingCapacity = atoi (cCurVal);  // 47040 mWh
		
		go_to_next_line  // remaining capacity: 47040 mWh
		
		jump_to_value
		int iPresentVoltage = atoi (cCurVal);  // 15000 mV
		
		if (iPresentRate != 0)
			myData.battery_time = 3600. * iRemainingCapacity / iPresentRate;
		else
			myData.battery_time = 0;
		
		myData.battery_charge = 100. * iRemainingCapacity / myData.iCapacity;
		
		g_print ("PowerManager : On Battery:%d ; iRemainingCapacity:%dmWh ; iPresentRate:%dmW ; iPresentVoltage:%dmV\n", myData.on_battery, iRemainingCapacity, iPresentRate, iPresentVoltage); 
		g_free (cContent);
		
		update_icon();
	}
	/*present: yes
	capacity state: ok
	charging state: discharging
	present rate: 15000 mW
	remaining capacity: 47040 mWh
	present voltage: 15000 mV*/
	return TRUE;
}

/**int get_stats(gchar *dataType)
{
	GValueArray *gva;
	GValue *gv;
	GPtrArray *ptrarray = NULL;
	GType g_type_ptrarray;
	int i;
	int x, y, col;  /// mettre des nom comprehensibles...

	g_type_ptrarray = dbus_g_type_get_collection ("GPtrArray",
		dbus_g_type_get_struct("GValueArray",
			G_TYPE_INT,
			G_TYPE_INT,
			G_TYPE_INT,
			G_TYPE_INVALID));
	
	dbus_g_proxy_call (dbus_proxy_stats, "GetData", NULL,
		 G_TYPE_STRING, dataType,
		 G_TYPE_INVALID,
		 g_type_ptrarray, &ptrarray,
		 G_TYPE_INVALID);
	g_return_val_if_fail (ptrarray != NULL, 0);
	
	for (i=0; i< ptrarray->len; i++)  /// il semble que seule la derniere valeur ait de l'interet ....
	{
		gva = (GValueArray *) g_ptr_array_index (ptrarray, i);
		gv = g_value_array_get_nth (gva, 0);
		x = g_value_get_int (gv);
		g_value_unset (gv);
		gv = g_value_array_get_nth (gva, 1);
		y = g_value_get_int (gv);
		g_value_unset (gv);
		gv = g_value_array_get_nth (gva, 2);
		col = g_value_get_int (gv);
		g_value_unset (gv);
		g_value_array_free (gva);
	}
	g_ptr_array_free (ptrarray, TRUE);
	
	cd_debug ("PowerManager [%s]: %d", dataType, y);
	return y;  /// a quoi servent x et col alors ??
}*/

/**void detect_battery(void)
{
	if (dbus_proxy_battery != NULL)
		dbus_g_proxy_call (dbus_proxy_battery, "GetPropertyBoolean", NULL,
			G_TYPE_STRING,"battery.present",
			G_TYPE_INVALID,
			G_TYPE_BOOLEAN, &myData.battery_present,
			G_TYPE_INVALID);
}*/

void power_halt(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "Shutdown", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_hibernate(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "Hibernate", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_suspend(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "Suspend", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
void power_reboot(void)
{
	dbus_g_proxy_call (dbus_proxy_power, "Reboot", NULL,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}
