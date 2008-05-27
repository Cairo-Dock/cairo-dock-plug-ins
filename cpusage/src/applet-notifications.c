#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the cpusage applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


static void _cd_cpusage_get_top_list (void)
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

static void _cd_cpusage_update_top_list (void)
{
	/*if (myData.pTopDialog->iRefCount == 1)
	{
		cairo_dock_stop_measure_timer (myData.pTopMeasureTimer);
		cairo_dock_dialog_unreference (myData.pTopDialog);
		myData.pTopDialog = NULL;
		g_timer_destroy (myData.pTopClock);
		myData.pTopClock = NULL;
		cairo_surface_destroy (myData.pTopSurface);
		myData.pTopSurface = NULL;
		return ;
	}*/
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
	sTopInfo->str[sTopInfo->len-1] = '\0';
	
	int iTextWidth, iTextHeight;
	double fTextXOffset, fTextYOffset;
	cairo_surface_destroy (myData.pTopSurface);
	myData.pTopSurface = cairo_dock_create_surface_from_text (sTopInfo->str,
		myDrawContext,
		myConfig.pTopTextDescription,
		1.,
		&iTextWidth, &iTextHeight, &fTextXOffset, &fTextYOffset);
	g_string_free (sTopInfo, TRUE);
	
	if (iTextWidth > myData.pTopDialog->iInteractiveWidth || iTextHeight > myData.pTopDialog->iInteractiveHeight)
		gtk_widget_set_size_request (myData.pTopDialog->pInteractiveWidget, iTextWidth, iTextHeight);
	
	//g_print (" -> (%d;%d) (%dx%d)\n", area.x, area.y, area.width, area.height);
#ifdef HAVE_GLITZ
	if (myData.pTopDialog->pDrawFormat && myData.pTopDialog->pDrawFormat->doublebuffer)
		gtk_widget_queue_draw (myData.pTopDialog->pWidget);
	else
#endif
		gtk_widget_queue_draw (myData.pTopDialog->pInteractiveWidget);
}

static gboolean _cd_cpusage_draw_top_list_on_dialog (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	gpointer data)
{
	cairo_t *pCairoContext = gdk_cairo_create (pWidget->window);
	g_return_val_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS, FALSE);
	
	cairo_set_source_surface (pCairoContext, myData.pTopSurface, pExpose->area.x, pExpose->area.y);
	cairo_paint (pCairoContext);
	
	cairo_destroy (pCairoContext);
}

CD_APPLET_ON_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		if (myData.pTopDialog != NULL)
		{
			cairo_dock_stop_measure_timer (myData.pTopMeasureTimer);
			cairo_dock_dialog_unreference (myData.pTopDialog);
			//cairo_dock_dialog_unreference (myData.pTopDialog);
			myData.pTopDialog = NULL;
			g_timer_destroy (myData.pTopClock);
			myData.pTopClock = NULL;
			cairo_surface_destroy (myData.pTopSurface);
			myData.pTopSurface = NULL;
			cd_cpusage_clean_all_processes ();
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
			0,  // 5000 * myConfig.iProcessCheckInterval
			cIconPath,
			GTK_BUTTONS_NONE,
			pInteractiveWidget,
			NULL,
			NULL,
			NULL);
		//cairo_dock_dialog_reference (myData.pTopDialog);
		g_free (cTitle);
		g_free (cIconPath);
		g_return_val_if_fail (myData.pTopDialog != NULL, CAIRO_DOCK_INTERCEPT_NOTIFICATION);
		g_signal_connect_after (G_OBJECT (pInteractiveWidget),
			"expose-event",
			G_CALLBACK (_cd_cpusage_draw_top_list_on_dialog),
			myData.pTopDialog);
		
		myData.pTopClock = g_timer_new ();
		if (myData.pTopMeasureTimer == NULL)
			myData.pTopMeasureTimer = cairo_dock_new_measure_timer (myConfig.iProcessCheckInterval,
				NULL,
				_cd_cpusage_get_top_list,
				_cd_cpusage_update_top_list);
		cairo_dock_launch_measure (myData.pTopMeasureTimer);
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


CD_APPLET_ON_BUILD_MENU_BEGIN
		CD_APPLET_ADD_SUB_MENU ("cpusage", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
