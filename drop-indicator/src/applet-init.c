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


CD_APPLET_PRE_INIT_BEGIN("drop indicator", 2, 0, 0, CAIRO_DOCK_CATEGORY_PLUG_IN)
	if (! g_bUseOpenGL)
		return FALSE;
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
CD_APPLET_PRE_INIT_END

static void _load_drop_indicator (void)
{
	cairo_t* pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	double fMaxScale = cairo_dock_get_max_scale (g_pMainDock);
	myData.iInitialWidth = g_tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] * fMaxScale;
	myData.iInitialHeight = g_tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] * fMaxScale / 2;
	
	gchar *cImagePath;
	if (myConfig.cDropIndicatorImageName != NULL)
	{
		cImagePath = cairo_dock_generate_file_path (myConfig.cDropIndicatorImageName);
	}
	else
	{
		cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_DEFAULT_DROP_INDICATOR_NAME);
	}
	cd_drop_indicator_load_drop_indicator (cImagePath,
		pCairoContext,
		myData.iInitialWidth,
		myData.iInitialHeight);
	g_free (cImagePath);
	cairo_destroy (pCairoContext);
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! cairo_dock_reserve_data_slot (myApplet))
		return;
	
	_load_drop_indicator ();
	
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_render, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) cd_drop_indicator_mouse_moved, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_update_dock, CAIRO_DOCK_RUN_AFTER, NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_render, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) cd_drop_indicator_mouse_moved, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_drop_indicator_update_dock, NULL);
	
	/// foreach dock : free data.
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.iDropIndicatorTexture != 0)
			glDeleteTextures (1, &myData.iDropIndicatorTexture);
		myData.iDropIndicatorTexture = 0;
		
		if (myData.pDropIndicatorSurface != NULL)
			cairo_surface_destroy (myData.pDropIndicatorSurface);
		myData.pDropIndicatorSurface = NULL;
		_load_drop_indicator ();
	}
	
CD_APPLET_RELOAD_END
