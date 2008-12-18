/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "fire-tex.h"

#include "applet-config.h"
#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-init.h"


CD_APPLET_PRE_INIT_BEGIN("icon effects", 2, 0, 0, CAIRO_DOCK_CATEGORY_PLUG_IN)
	if (! g_bUseOpenGL)
		return FALSE;
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
CD_APPLET_PRE_INIT_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! cairo_dock_reserve_data_slot (myApplet))
		return;
	
	cairo_dock_register_notification (CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) cd_icon_effect_start, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_icon_effect_update_icon, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_PRE_RENDER_ICON, (CairoDockNotificationFunc) cd_icon_effect_pre_render_icon, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_icon_effect_render_icon, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_icon_effect_free_data, CAIRO_DOCK_RUN_AFTER, NULL);
CD_APPLET_INIT_END


static void _free_data_on_icon (Icon *pIcon, CairoDock *pDock, gpointer data)
{
	cd_icon_effect_free_data (NULL, pIcon);
}
//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func (CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) cd_icon_effect_start, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_icon_effect_update_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_PRE_RENDER_ICON, (CairoDockNotificationFunc) cd_icon_effect_pre_render_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_icon_effect_render_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_icon_effect_free_data, NULL);
	
	cairo_dock_foreach_icons ((CairoDockForeachIconFunc) _free_data_on_icon, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.iFireTexture != 0 && (myConfig.iFireDuration == 0 && myConfig.iStormDuration == 0))
		{
			glDeleteTextures (1, &myData.iFireTexture);
			myData.iFireTexture = 0;
		}
		if (myData.iStarTexture != 0 && myConfig.iStarDuration == 0)
		{
			glDeleteTextures (1, &myData.iStarTexture);
			myData.iStarTexture = 0;
		}
		if (myData.iSnowTexture != 0 && myConfig.iSnowDuration == 0)
		{
			glDeleteTextures (1, &myData.iSnowTexture);
			myData.iSnowTexture = 0;
		}
		if (myData.iRainTexture != 0 && myConfig.iRainDuration == 0)
		{
			glDeleteTextures (1, &myData.iRainTexture);
			myData.iRainTexture = 0;
		}
	}
CD_APPLET_RELOAD_END
