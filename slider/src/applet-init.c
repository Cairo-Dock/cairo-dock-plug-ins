/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-silder.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("slider", 1, 5, 5, CAIRO_DOCK_CATEGORY_ACCESSORY)


static void _slider_set_desklet_renderer (void)
{
	const gchar *cConfigName = NULL;
	switch (myConfig.iDecoration)
	{
		case SLIDER_PERSONNAL :
		break ;
		case SLIDER_FRAME_REFLECTS :
			cConfigName = "frame&reflects";
		break ;
		case SLIDER_SCOTCH :
				cConfigName = "scotch";
		break ;
		case SLIDER_FRAME_SCOTCH :
				cConfigName = "frame with scotch";
		break ;
		default :
			return ;
	}
	if (cConfigName != NULL)
	{
		CairoDeskletRendererConfig *pConfig = cairo_dock_get_desklet_renderer_predefined_config ("Simple", cConfigName);
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
	}
	else if (myConfig.cFrameImage != NULL || myConfig.cReflectImage != NULL)
	{
		gpointer pManualConfig[9] = {myConfig.cFrameImage, myConfig.cReflectImage, GINT_TO_POINTER (CAIRO_DOCK_FILL_SPACE), &myConfig.fFrameAlpha, &myConfig.fReflectAlpha, GINT_TO_POINTER (myConfig.iLeftOffset), GINT_TO_POINTER (myConfig.iTopOffset), GINT_TO_POINTER (myConfig.iRightOffset), GINT_TO_POINTER (myConfig.iBottomOffset)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pManualConfig);
	}
	else
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet) {
		_slider_set_desklet_renderer ();
	}
	
	double fRatio = (myDock ? myDock->fRatio : 1.);
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.fSurfaceWidth = myIcon->fWidth / fRatio * fMaxScale;
	myData.fSurfaceHeight = myIcon->fHeight / fRatio * fMaxScale;
	
	myData.pMeasureDirectory = cairo_dock_new_measure_timer (0,
		NULL,
		cd_slider_read_directory,
		cd_slider_launch_slides);  // 0 <=> one shot measure.
	cairo_dock_launch_measure (myData.pMeasureDirectory);
	
	myData.pMeasureImage = cairo_dock_new_measure_timer (0,
		NULL,
		cd_slider_read_image,
		cd_slider_update_slide);  // 0 <=> one shot measure.
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT
	
	if (myData.iTimerID != 0)
		g_source_remove(myData.iTimerID);
	if (myData.iAnimTimerID != 0)
		g_source_remove(myData.iAnimTimerID);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	
	//Stop all process!
	myData.bPause = TRUE;
	if (myData.iTimerID != 0) {
		g_source_remove(myData.iTimerID);
		myData.iTimerID = 0;
	}
	if (myData.iAnimTimerID != 0) {
		g_source_remove(myData.iAnimTimerID);
		myData.iAnimTimerID = 0;
	}
	
	
	cairo_surface_destroy (myData.pCairoSurface);
	myData.pCairoSurface = NULL;
	cairo_surface_destroy (myData.pPrevCairoSurface);
	myData.pPrevCairoSurface = NULL;
	
	if (myDesklet) {
		_slider_set_desklet_renderer ();
	}
	
	double fRatio = (myDock ? myDock->fRatio : 1.);  // meme si le container n'a pas change, car un desklet se redimensionne, et l'icone avec.
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.fSurfaceWidth = myIcon->fWidth / fRatio * fMaxScale;
	myData.fSurfaceHeight = myIcon->fHeight / fRatio * fMaxScale;
	
	myData.bPause = FALSE; //On coupe la pause pour repartir de plus belle
	
	//\_______________ Reload all changed data.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cd_slider_free_images_list (myData.pList);
		myData.pList = NULL;
		cairo_dock_stop_measure_timer (myData.pMeasureImage);
		cairo_dock_stop_measure_timer (myData.pMeasureDirectory);
		cairo_dock_launch_measure (myData.pMeasureDirectory);
	}
	else {
		//Nothing to do ^^
		cd_slider_draw_images(); //restart sliding
	}
CD_APPLET_RELOAD_END
