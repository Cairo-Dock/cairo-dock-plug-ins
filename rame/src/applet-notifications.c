#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-rame.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the rame applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


static void _cd_rame_get_top_list (CairoDockModuleInstance *myApplet)
{
	cd_rame_get_process_memory ();
}

static gboolean _cd_rame_update_top_list (CairoDockModuleInstance *myApplet)
{
	CDProcess *pProcess;
	int i;
	
	for (i = 0; i < myConfig.iNbDisplayedProcesses; i ++)
	{
		if (myData.pTopList[i] == NULL || myData.pPreviousTopList[i] == NULL || myData.pTopList[i]->iPid != myData.pPreviousTopList[i]->iPid || myData.pTopList[i]->iMemAmount != myData.pPreviousTopList[i]->iMemAmount)
			break ;
	}
	if (i == myConfig.iNbDisplayedProcesses)  // aucun changement.
	{
		return TRUE;
	}
	
	GString *sTopInfo = g_string_new ("");
	for (i = 0; i < myConfig.iNbDisplayedProcesses; i ++)
	{
		pProcess = myData.pTopList[i];
		if (pProcess == NULL)
			break;
		g_string_append_printf (sTopInfo, "  %s (%d) : %.1f%s\n", pProcess->cName, pProcess->iPid, (double) pProcess->iMemAmount / (myConfig.bTopInPercent && myData.ramTotal ? 10.24 * myData.ramTotal : 1024 * 1024), (myConfig.bTopInPercent && myData.ramTotal ? "%" : D_("Mb")));
	}
	sTopInfo->str[sTopInfo->len-1] = '\0';
	
	cairo_dock_render_dialog_with_new_data (myData.pTopDialog, (CairoDialogRendererDataPtr) sTopInfo->str);
	g_string_free (sTopInfo, TRUE);
	return TRUE;
}

CD_APPLET_ON_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		if (myData.pTopDialog != NULL)
		{
			cairo_dock_stop_measure_timer (myData.pTopMeasureTimer);
			cairo_dock_dialog_unreference (myData.pTopDialog);
			myData.pTopDialog = NULL;
			cairo_surface_destroy (myData.pTopSurface);
			myData.pTopSurface = NULL;
			cd_rame_clean_all_processes ();
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		}
		
		gchar *cTitle = g_strdup_printf ("  [ Top %d ] :", myConfig.iNbDisplayedProcesses);
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
		
		if (myData.pTopMeasureTimer == NULL)
			myData.pTopMeasureTimer = cairo_dock_new_measure_timer (5,
				NULL,
				(CairoDockReadTimerFunc) _cd_rame_get_top_list,
				(CairoDockUpdateTimerFunc) _cd_rame_update_top_list,
				myApplet);
		cairo_dock_launch_measure (myData.pTopMeasureTimer);
	}
	else
		cairo_dock_show_temporary_dialog(D_("Data acquisition has failed"), myIcon, myContainer, 3e3);
CD_APPLET_ON_CLICK_END


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
		CD_APPLET_ADD_SUB_MENU ("ram-meter", pSubMenu, CD_APPLET_MY_MENU);
		CD_APPLET_ADD_IN_MENU (D_("Monitor System"), _show_monitor_system, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END
