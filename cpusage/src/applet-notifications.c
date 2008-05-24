#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the cpusage applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


static gboolean _cd_cpusage_top_timer (GTimer *pClock)
{
	g_timer_stop (pClock);
	double fTimeElapsed = g_timer_elapsed (pClock, NULL);
	g_timer_start (pClock);
	GTimeVal time_val;
	g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
	double fTime = time_val.tv_sec + time_val.tv_usec * 1e-6;
	cd_cpusage_get_process_times (fTime, fTimeElapsed);
	
	CDProcess *pProcess;
	int i;
	
	GString *sTopInfo = g_string_new ("");
	g_string_printf (sTopInfo, "Top %d :", myConfig.iNbDisplayedProcesses);
	for (i = 0; i < myConfig.iNbDisplayedProcesses; i ++)
	{
		pProcess = myData.pTopList[i];
		if (pProcess == NULL)
			break;
		g_string_append_printf (sTopInfo, "\n  %s (%d) : %.1f%%", pProcess->cName, pProcess->iPid, 100 * pProcess->fCpuPercent);
	}
	
	cairo_dock_remove_dialog_if_any (myIcon);
	cairo_dock_show_temporary_dialog(sTopInfo->str, myIcon, myContainer, 0);
	g_string_free (sTopInfo, TRUE);
	
	/*{
		g_timer_destroy (pClock);
		cd_cpusage_clean_all_processes ();
		return FALSE;
	}*/
	cd_cpusage_clean_old_processes (fTime);
	return TRUE;
}
CD_APPLET_ON_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		if (cairo_dock_remove_dialog_if_any (myIcon))
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		
		GTimeVal time_val;
		g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
		cd_cpusage_get_process_times (time_val.tv_sec + time_val.tv_usec * 1e-6, 0.);
		
		GTimer *pClock = g_timer_new ();
		g_timeout_add_seconds (myConfig.iProcessCheckInterval, (GSourceFunc) _cd_cpusage_top_timer, pClock);
	}
	else
		cairo_dock_show_temporary_dialog(D_("Data acquisition has failed"), myIcon, myContainer, 4e3);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		/// afficher : utilisation de chaque coeur, nbre de processus en cours.
		if (cairo_dock_remove_dialog_if_any (myIcon))
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		
		gchar *cUpTime = NULL, *cActivityTime = NULL;
		cd_cpusage_get_uptime (&cUpTime, &cActivityTime);
		cairo_dock_show_temporary_dialog ("%s : %s\n%s : %d MHz (%d %s)\n%s : %s / %s : %s", myIcon, myContainer, 10e3, D_("Model Name"), myData.cModelName, D_("Frequency"), myData.iFrequency, myData.iNbCPU, D_("core(s)"), D_("Up time"), cUpTime, D_("Activity time"), cActivityTime);
		g_free (cUpTime);
		g_free (cActivityTime);
	}
	else
		cairo_dock_show_temporary_dialog(D_("Data acquisition has failed"), myIcon, myContainer, 4e3);
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
		CD_APPLET_ADD_SUB_MENU ("cpusage", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
