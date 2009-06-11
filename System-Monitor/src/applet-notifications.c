#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-cpusage.h"
#include "applet-rame.h"
#include "applet-top.h"
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		if (myData.pTopDialog != NULL)
			cd_sysmonitor_stop_top_dialog (myApplet);
		else
			cd_sysmonitor_start_top_dialog (myApplet);
	}
	else
		cairo_dock_show_temporary_dialog(D_("Data acquisition has failed"), myIcon, myContainer, 3e3);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		/// afficher : utilisation de chaque coeur, nbre de processus en cours.
		if (myData.pTopDialog != NULL || cairo_dock_remove_dialog_if_any (myIcon))
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		
		gchar *cUpTime = NULL, *cActivityTime = NULL;
		cd_sysmonitor_get_uptime (&cUpTime, &cActivityTime);
		cairo_dock_show_temporary_dialog ("%s : %s\n%s : %d MHz (%d %s)\n%s : %s / %s : %s", myIcon, myContainer, 10e3, D_("Model Name"), myData.cModelName, D_("Frequency"), myData.iFrequency, myData.iNbCPU, D_("core(s)"), D_("Up time"), cUpTime, D_("Activity time"), cActivityTime);
		g_free (cUpTime);
		g_free (cActivityTime);
	}
	else
		cairo_dock_show_temporary_dialog(D_("Data acquisition has failed"), myIcon, myContainer, 4e3);
CD_APPLET_ON_MIDDLE_CLICK_END

static void _show_monitor_system (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	if (myConfig.cSystemMonitorCommand != NULL)
	{
		cairo_dock_launch_command (myConfig.cSystemMonitorCommand);
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		system ("kde-system-monitor");
	}
	else
	{
		cairo_dock_fm_show_system_monitor ();
	}
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU (D_("Monitor System"), _show_monitor_system, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END
