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

/*static void _cd_slider_load_frame (void) {
	cd_debug ("%s", __func__);
	
	if (myData.pCairoFrameSurface != NULL) {
		cd_warning ("Can't load frame surface, one already loaded");
		return;
	}
	
	gchar *cImagePath;
	if (myConfig.cFrameImage != NULL) {
		cImagePath = cairo_dock_generate_file_path (myConfig.cFrameImage);
	}
	else {
		cImagePath = g_strdup_printf ("%s/frame.svg", MY_APPLET_SHARE_DATA_DIR);
	}
	cd_debug ("Background frame: %s", cImagePath);

	double fIW, fIH;
	myData.pCairoFrameSurface = cairo_dock_create_surface_from_image (cImagePath,
		myDrawContext,
		cairo_dock_get_max_scale (myContainer),
		myIcon->fWidth, myIcon->fHeight,
		&fIW, &fIH,
		FALSE);
		
	if (myData.pCairoReflectSurface != NULL) {
		cd_warning ("Can't load reflect surface, one already loaded");
		return;
	}
	
	if (myConfig.cReflectImage != NULL) {
		cImagePath = cairo_dock_generate_file_path (myConfig.cReflectImage);
	}
	else {
		cImagePath = g_strdup_printf ("%s/reflect.svg", MY_APPLET_SHARE_DATA_DIR);
	}
	cd_debug ("Reflection frame: %s", cImagePath);

	myData.pCairoReflectSurface = cairo_dock_create_surface_from_image (cImagePath,
		myDrawContext,
		cairo_dock_get_max_scale (myContainer),
		myIcon->fWidth, myIcon->fHeight,
		&fIW, &fIH,
		TRUE);
	
	g_free(cImagePath);
}*/

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet) {
		if (myConfig.fFrameAlpha != 0 || myConfig.fReflectAlpha != 0)
		{
			gpointer pConfig[6] = {myConfig.cFrameImage, myConfig.cReflectImage, GINT_TO_POINTER (CAIRO_DOCK_FILL_SPACE), &myConfig.fFrameAlpha, &myConfig.fReflectAlpha, GINT_TO_POINTER (myConfig.iFrameOffset)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
		}
		else
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
	}
	
	double fRatio = (myDock ? myDock->fRatio : 1.);
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.fSurfaceWidth = myIcon->fWidth / fRatio * fMaxScale;
	myData.fSurfaceHeight = myIcon->fHeight / fRatio * fMaxScale;
	
	//_cd_slider_load_frame(); //load background frame image
	//cd_slider_get_files_from_dir();  /// suggestion : le threader car ca prend du temps de parcourir le disque.
	
	myData.pMeasureTimer = cairo_dock_new_measure_timer (0,
		NULL,
		cd_slider_read_directory,
		cd_slider_launch_slides);
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	
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
		if (myConfig.fFrameAlpha != 0 || myConfig.fReflectAlpha != 0)
		{
			gpointer pConfig[6] = {myConfig.cFrameImage, myConfig.cReflectImage, GINT_TO_POINTER (CAIRO_DOCK_FILL_SPACE), &myConfig.fFrameAlpha, &myConfig.fReflectAlpha, GINT_TO_POINTER (myConfig.iFrameOffset)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
		}
		else
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
	}
	
	double fRatio = (myDock ? myDock->fRatio : 1.);  // meme si le container n'a pas change, car un desklet se redimensionne, et l'icone avec.
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.fSurfaceWidth = myIcon->fWidth / fRatio * fMaxScale;
	myData.fSurfaceHeight = myIcon->fHeight / fRatio * fMaxScale;
	
	/*cairo_surface_destroy (myData.pCairoFrameSurface);
	myData.pCairoFrameSurface = NULL;
	cairo_surface_destroy (myData.pCairoReflectSurface);
	myData.pCairoReflectSurface = NULL;
	_cd_slider_load_frame(); //load background frame image*/
	
	//\_______________ Reload all changed data.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cd_slider_free_images_list (myData.pList);
		myData.pList = NULL;
		//cd_slider_get_files_from_dir(); //reload image list
		cairo_dock_launch_measure (myData.pMeasureTimer);
	}
	else {
		//Nothing to do ^^
	}
	
	myData.bPause = FALSE;
	cd_slider_draw_images(); //restart sliding
CD_APPLET_RELOAD_END
