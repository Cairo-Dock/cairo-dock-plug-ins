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
#include "applet-draw.h"
#include "applet-nvidia.h"

CD_APPLET_DEFINITION ("nVidia", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY)


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	//Initialisation de la jauge
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.pGauge = init_cd_Gauge(myDrawContext, myConfig.cGThemePath, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
	
	myData.iPreviousGPUTemp = -1;  // force le dessin.
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		cd_nvidia_acquisition,
		cd_nvidia_read_data,
		cd_nvidia_update_from_data);
	cairo_dock_launch_measure_delayed (myData.pMeasureTimer, 1000);
	
	myData.pConfigMeasureTimer = cairo_dock_new_measure_timer (0,
		cd_nvidia_config_acquisition,
		cd_nvidia_config_read_data,
		cd_nvidia_config_update_from_data);
	cairo_dock_launch_measure (myData.pConfigMeasureTimer);
	
	myData.bAlerted = FALSE;
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	myData.bAlerted = FALSE;
	
	free_cd_Gauge(myData.pGauge);
	myData.pGauge = NULL;
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.pGauge = init_cd_Gauge(myDrawContext, myConfig.cGThemePath, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
	
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		cairo_dock_stop_measure_timer (myData.pMeasureTimer);  // on stoppe avant car  on ne veut pas attendre la prochaine iteration.
		cairo_dock_change_measure_frequency (myData.pMeasureTimer, myConfig.iCheckInterval);
		myData.iPreviousGPUTemp = -1;  // on force le redessin.
		cairo_dock_launch_measure (myData.pMeasureTimer);  // mesure immediate.
	}
	else {
		myData.iPreviousGPUTemp = -1;  // force le redessin.
		
		if (myData.bAcquisitionOK) 
			cd_nvidia_draw_icon ();
		else
			cd_nvidia_draw_no_data ();
	}
CD_APPLET_RELOAD_END
