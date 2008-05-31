#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-rame.h"

CD_APPLET_DEFINITION ("rame", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY);

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	//Initialisation de la jauge
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.pGauge = init_cd_Gauge (myDrawContext, myConfig.cThemePath, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
	make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, 0.);
	
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		NULL,
		cd_rame_read_data,
		cd_rame_update_from_data);
	myData.bAcquisitionOK = TRUE;
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (myData.pGauge != NULL) { // reset jauges.
		free_cd_Gauge(myData.pGauge);
		myData.pGauge = NULL;
	}
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.pGauge = init_cd_Gauge (myDrawContext, myConfig.cThemePath, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
	
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_ICON)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL)
		}
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_LABEL)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle)
		}
		
		myData.fPrevRamPercent = 0;  // on force le redessin.
		cairo_dock_relaunch_measure_immediately (myData.pMeasureTimer, myConfig.iCheckInterval);
		
		if (cairo_dock_measure_is_active (myData.pTopMeasureTimer))
		{
			cd_rame_clean_all_processes ();
			cairo_dock_stop_measure_timer (myData.pTopMeasureTimer);
			g_free (myData.pTopList);
			myData.pTopList = NULL;
			g_free (myData.pPreviousTopList);
			myData.pPreviousTopList = NULL;
			cairo_dock_launch_measure (myData.pTopMeasureTimer);
		}
	}
	else {  // on redessine juste l'icone.
		CairoDockLabelDescription *pOldLabelDescription = myConfig.pTopTextDescription;  // on recupere le nouveau style des etiquettes en cas de changement de la config du dock.
		myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&g_dialogTextDescription);
		memcpy (myConfig.pTopTextDescription->fColorStart, pOldLabelDescription->fColorStart, 3*sizeof (double));
		memcpy (myConfig.pTopTextDescription->fColorStop, pOldLabelDescription->fColorStop, 3*sizeof (double));
		myConfig.pTopTextDescription->bVerticalPattern = TRUE;
		cairo_dock_free_label_description (pOldLabelDescription);
		
		myData.fPrevRamPercent = 0;  // on force le redessin.
		cd_rame_update_from_data ();
	}
CD_APPLET_RELOAD_END
