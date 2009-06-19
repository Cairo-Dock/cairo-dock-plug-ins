#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"



static void _cd_cpusage_get_top_list (CairoDockModuleInstance *myApplet)
{
	g_timer_stop (myData.pTopClock);
	double fTimeElapsed = g_timer_elapsed (myData.pTopClock, NULL);
	g_timer_start (myData.pTopClock);
	GTimeVal time_val;
	g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
	double fTime = time_val.tv_sec + time_val.tv_usec * 1e-6;
	cd_cpusage_get_process_times (fTime, fTimeElapsed);
	
	cd_cpusage_clean_old_processes (fTime);
}

static gboolean _cd_cpusage_update_top_list (CairoDockModuleInstance *myApplet)
{
	CDProcess *pProcess;
	int i;
	GString *sTopInfo = g_string_new ("");
	for (i = 0; i < myConfig.iNbDisplayedProcesses; i ++)
	{
		pProcess = myData.pTopList[i];
		if (pProcess == NULL)
			break;
		g_string_append_printf (sTopInfo, "  %s (%d) : %.1f%%\n", pProcess->cName, pProcess->iPid, 100 * pProcess->fCpuPercent);
	}
	if (i == 0)
	{
		g_string_free (sTopInfo, TRUE);
		return TRUE;
	}
	sTopInfo->str[sTopInfo->len-1] = '\0';
	
	cairo_dock_render_dialog_with_new_data (myData.pTopDialog, (CairoDialogRendererDataPtr) sTopInfo->str);
	g_string_free (sTopInfo, TRUE);
	
	if (myData.iNbProcesses != g_hash_table_size (myData.pProcessTable))
	{
		myData.iNbProcesses = g_hash_table_size (myData.pProcessTable);
		gchar *cTitle = g_strdup_printf ("  [ Top %d / %d ] :", myConfig.iNbDisplayedProcesses, myData.iNbProcesses);
		cairo_dock_set_dialog_message (myData.pTopDialog, cTitle);
		g_free (cTitle);
	}
	return TRUE;
}

CD_APPLET_ON_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		if (myData.pTopDialog != NULL)
		{
			cairo_dock_stop_task (myData.pTopTask);
			cairo_dock_dialog_unreference (myData.pTopDialog);
			myData.pTopDialog = NULL;
			g_timer_destroy (myData.pTopClock);
			myData.pTopClock = NULL;
			cairo_surface_destroy (myData.pTopSurface);
			myData.pTopSurface = NULL;
			cd_cpusage_clean_all_processes ();
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		}
		
		gchar *cTitle = g_strdup_printf ("  [ Top %d ] :", myConfig.iNbDisplayedProcesses);  // g_hash_table_size (myData.pProcessTable)
		gchar *cIconPath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_ICON_FILE);
		GtkWidget *pInteractiveWidget = gtk_vbox_new (FALSE, 0);
		gtk_widget_set_size_request (pInteractiveWidget,
			myConfig.pTopTextDescription->iSize * 15,
			myConfig.pTopTextDescription->iSize * myConfig.iNbDisplayedProcesses);  // approximatif au depart.
		myData.pTopDialog = cairo_dock_show_dialog_full (cTitle,
			myIcon,
			myContainer,
			0,
			cIconPath,
			pInteractiveWidget,
			NULL,
			NULL,
			NULL);
		g_free (cTitle);
		g_free (cIconPath);
		g_return_val_if_fail (myData.pTopDialog != NULL, CAIRO_DOCK_INTERCEPT_NOTIFICATION);
		
		gpointer pConfig[2] = {myConfig.pTopTextDescription, "Loading ..."};
		cairo_dock_set_dialog_renderer_by_name (myData.pTopDialog, "Text", myDrawContext, (CairoDialogRendererConfigPtr) pConfig);
		
		myData.pTopClock = g_timer_new ();
		myData.iNbProcesses = 0;
		if (myData.pTopTask == NULL)
			myData.pTopTask = cairo_dock_new_task (myConfig.iProcessCheckInterval,
				(CairoDockGetDataAsyncFunc) _cd_cpusage_get_top_list,
				(CairoDockUpdateSyncFunc) _cd_cpusage_update_top_list,
				myApplet);
		cairo_dock_launch_task (myData.pTopTask);
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
		cd_cpusage_get_uptime (&cUpTime, &cActivityTime);
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
