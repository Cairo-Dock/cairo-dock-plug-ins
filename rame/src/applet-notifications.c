#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-rame.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the rame applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


static void _cd_rame_get_top_list (void)
{
	cd_rame_get_process_memory ();
}

static void _cd_rame_update_top_list (void)
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
		return ;
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

static gboolean _cd_rame_draw_top_list_on_dialog (GtkWidget *pWidget,
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
			G_CALLBACK (_cd_rame_draw_top_list_on_dialog),
			myData.pTopDialog);
		
		if (myData.pTopMeasureTimer == NULL)
			myData.pTopMeasureTimer = cairo_dock_new_measure_timer (5,
				NULL,
				_cd_rame_get_top_list,
				_cd_rame_update_top_list);
		cairo_dock_launch_measure (myData.pTopMeasureTimer);
	}
	else
		cairo_dock_show_temporary_dialog(D_("Data acquisition has failed"), myIcon, myContainer, 3e3);
CD_APPLET_ON_CLICK_END


static void _rame_recheck_ (GtkMenuItem *menu_item, gpointer *data) {
	cairo_dock_stop_measure_timer (myData.pMeasureTimer);
	cairo_dock_launch_measure (myData.pMeasureTimer);
}

CD_APPLET_ON_BUILD_MENU_BEGIN
		CD_APPLET_ADD_SUB_MENU ("rame", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
