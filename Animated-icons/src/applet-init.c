/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "chrome-tex.h"

#include "applet-config.h"
#include "applet-rotation.h"
#include "applet-mesh-factory.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("Animated icons"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_PLUG_IN,
	N_("This plug-in provides many different animations for your icons."),
	"Fabounet (Fabrice Rey)")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! cairo_dock_reserve_data_slot (myApplet))
		return;
	
	cairo_dock_register_notification (CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) cd_animations_on_enter, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) cd_animations_on_click, CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_REQUEST_ICON_ANIMATION, (CairoDockNotificationFunc) cd_animations_on_request, CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_animations_update_icon , CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_animations_render_icon, CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_animations_post_render_icon, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_animations_free_data, CAIRO_DOCK_RUN_AFTER, NULL);
	
	myData.iAnimationID[CD_ANIMATIONS_BOUNCE] = cairo_dock_register_animation ("bounce");
	myData.iAnimationID[CD_ANIMATIONS_ROTATE] = cairo_dock_register_animation ("rotate");
	myData.iAnimationID[CD_ANIMATIONS_BLINK] = cairo_dock_register_animation ("blink");
	myData.iAnimationID[CD_ANIMATIONS_PULSE] = cairo_dock_register_animation ("pulse");
	myData.iAnimationID[CD_ANIMATIONS_WOBBLY] = cairo_dock_register_animation ("wobbly");
	myData.iAnimationID[CD_ANIMATIONS_WAVE] = cairo_dock_register_animation ("wave");
	myData.iAnimationID[CD_ANIMATIONS_SPOT] = cairo_dock_register_animation ("spot");
	
	if (! cairo_dock_is_loading ())
		cairo_dock_update_animations_list_for_gui ();
CD_APPLET_INIT_END

static void _free_data_on_icon (Icon *pIcon, CairoDock *pDock, gpointer data)
{
	cd_animations_free_data (NULL, pIcon);
}
//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func (CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) cd_animations_on_enter, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) cd_animations_on_click, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_REQUEST_ICON_ANIMATION, (CairoDockNotificationFunc) cd_animations_on_request, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_animations_update_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_animations_render_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_animations_post_render_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_animations_free_data, NULL);
	
	cairo_dock_unregister_animation ("bounce");
	cairo_dock_unregister_animation ("rotate");
	cairo_dock_unregister_animation ("blink");
	cairo_dock_unregister_animation ("pulse");
	cairo_dock_unregister_animation ("wobbly");
	cairo_dock_unregister_animation ("wave");
	cairo_dock_unregister_animation ("spot");
	cairo_dock_update_animations_list_for_gui ();
	
	cairo_dock_foreach_icons ((CairoDockForeachIconFunc) _free_data_on_icon, NULL);
CD_APPLET_STOP_END
   
   
//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED && g_bUseOpenGL)
	{
		if (myConfig.iRotationDuration == 0)
		{
			if (myData.iChromeTexture != 0)
			{
				glDeleteTextures (1, &myData.iChromeTexture);
				myData.iChromeTexture = 0;
			}
			if (myData.iCallList[CD_CUBE_MESH] != 0)
			{
				glDeleteLists (myData.iCallList[CD_CUBE_MESH], 1);
				myData.iCallList[CD_CUBE_MESH] = 0;
			}
			if (myData.iCallList[CD_CAPSULE_MESH] != 0)
			{
				glDeleteLists (myData.iCallList[CD_CAPSULE_MESH], 1);
				myData.iCallList[CD_CAPSULE_MESH] = 0;
			}
			if (myData.iCallList[CD_SQUARE_MESH] != 0)
			{
				glDeleteLists (myData.iCallList[CD_SQUARE_MESH], 1);
				myData.iCallList[CD_SQUARE_MESH] = 0;
			}
		}
		else
		{
			if (myConfig.iMeshType != CD_CUBE_MESH && myData.iCallList[CD_CUBE_MESH] != 0)
			{
				glDeleteLists (myData.iCallList[CD_CUBE_MESH], 1);
				myData.iCallList[CD_CUBE_MESH] = 0;
			}
			if (myConfig.iMeshType != CD_CAPSULE_MESH && myData.iCallList[CD_CAPSULE_MESH] != 0)
			{
				glDeleteLists (myData.iCallList[CD_CAPSULE_MESH], 1);
				myData.iCallList[CD_CAPSULE_MESH] = 0;
			}
			
			if (myData.iCallList[myConfig.iMeshType] == 0)
				myData.iCallList[myConfig.iMeshType] = cd_animations_load_mesh (myConfig.iMeshType);
		}
		
		if (myConfig.iSpotDuration == 0)
		{
			if (myData.iSpotTexture != 0)
			{
				glDeleteTextures (1, &myData.iSpotTexture);
				myData.iSpotTexture = 0;
			}
			if (myData.iHaloTexture != 0)
			{
				glDeleteTextures (1, &myData.iHaloTexture);
				myData.iHaloTexture = 0;
			}
			if (myData.iSpotFrontTexture != 0)
			{
				glDeleteTextures (1, &myData.iSpotFrontTexture);
				myData.iSpotFrontTexture = 0;
			}
			if (myData.iRaysTexture != 0)
			{
				glDeleteTextures (1, &myData.iRaysTexture);
				myData.iRaysTexture = 0;
			}
		}
	}
CD_APPLET_RELOAD_END
