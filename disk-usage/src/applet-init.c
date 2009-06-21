/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-hdd.h"

CD_APPLET_DEFINITION (N_("disk-usage"),
	1, 6, 3,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet shows you the Available/Used space of a storage system.\n"
	"You can have more details with a middle-click."),
	"Jackass (Benjamin SANS)")

static gboolean _unthreaded_task (CairoDockModuleInstance *myApplet)
{
	cd_hdd_read_data (myApplet);
	cd_hdd_update_from_data (myApplet);
	return TRUE;
}

CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		_unthreaded_task (myApplet);
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.pGauge = cairo_dock_load_gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
	if (myConfig.cWatermarkImagePath != NULL)
		cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
	CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
	
	
	//Initialisation du timer de mesure.
	myData.pClock = g_timer_new ();
	myData.pTask = cairo_dock_new_task (myConfig.iCheckInterval,
		NULL,
		(CairoDockUpdateSyncFunc) _unthreaded_task, 
		myApplet);
	myData.bAcquisitionOK = TRUE;
	cairo_dock_launch_task (myData.pTask);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		_unthreaded_task (myApplet);
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		cairo_dock_free_gauge (myData.pGauge);
		myData.pGauge = cairo_dock_load_gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
		if (myConfig.cWatermarkImagePath != NULL)
			cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
		
		CairoDockLabelDescription *pOldLabelDescription = myConfig.pTopTextDescription;
		myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&myDialogs.dialogTextDescription);
		memcpy (myConfig.pTopTextDescription->fColorStart, pOldLabelDescription->fColorStart, 3*sizeof (double));
		memcpy (myConfig.pTopTextDescription->fColorStop, pOldLabelDescription->fColorStop, 3*sizeof (double));
		myConfig.pTopTextDescription->bVerticalPattern = TRUE;
		cairo_dock_free_label_description (pOldLabelDescription);
		
		cairo_dock_relaunch_task_immediately (myData.pTask, myConfig.iCheckInterval);
		
		//if (!cairo_dock_fm_add_monitor_full(myConfig.cDevice, FALSE, NULL, (CairoDockFMMonitorCallback) cairo_dock_fm_action_on_file_event, myIcon))
		//	cd_warning ("Disk-usage : can't monitor drives");
	} else {
		cairo_dock_reload_gauge (myDrawContext, myData.pGauge, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
		_unthreaded_task (myApplet);
	}

CD_APPLET_RELOAD_END
