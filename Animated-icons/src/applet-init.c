/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdlib.h"

#include "chrome-tex.h"

#include "applet-config.h"
#include "applet-blink.h"
#include "applet-bounce.h"
#include "applet-busy.h"
#include "applet-pulse.h"
#include "applet-rotation.h"
#include "applet-spot.h"
#include "applet-wave.h"
#include "applet-wobbly.h"
#include "applet-mesh-factory.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("Animated icons"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_THEME,
	N_("This plug-in provides many different animations for your icons."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE;	
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (!CD_APPLET_RESERVE_DATA_SLOT())
		return;
	
	gldi_object_register_notification (&myContainerObjectMgr,
		NOTIFICATION_ENTER_ICON,
		(GldiNotificationFunc) cd_animations_on_enter,
		GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myContainerObjectMgr,
		NOTIFICATION_CLICK_ICON,
		(GldiNotificationFunc) cd_animations_on_click,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_REQUEST_ICON_ANIMATION,
		(GldiNotificationFunc) cd_animations_on_request,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_UPDATE_ICON,
		(GldiNotificationFunc) cd_animations_update_icon,
		GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_RENDER_ICON,
		(GldiNotificationFunc) cd_animations_render_icon,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_RENDER_ICON,
		(GldiNotificationFunc) cd_animations_post_render_icon,
		GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_STOP_ICON,
		(GldiNotificationFunc) cd_animations_free_data,
		GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_UNFOLD_SUBDOCK,
		(GldiNotificationFunc) cd_animations_unfold_subdock,
		GLDI_RUN_AFTER, NULL);
	
	// register each animation, in their rendering order.
	cd_animations_register_bounce ();  // alter context
	cd_animations_register_spot ();  // alter context
	cd_animations_register_blink ();  // alter context
	cd_animations_register_rotation ();  // draw icon
	cd_animations_register_wave ();  // draw icon
	cd_animations_register_wobbly ();  // draw icon
	cd_animations_register_pulse ();  // draw above icon
	cd_animations_register_busy ();  // draw above icon
CD_APPLET_INIT_END

static void _free_data_on_icon (Icon *pIcon, CairoDock *pDock, gpointer data)
{
	cd_animations_free_data (NULL, pIcon);
}
//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_ENTER_ICON,
		(GldiNotificationFunc) cd_animations_on_enter, NULL);
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_CLICK_ICON,
		(GldiNotificationFunc) cd_animations_on_click, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_REQUEST_ICON_ANIMATION,
		(GldiNotificationFunc) cd_animations_on_request, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_UPDATE_ICON,
		(GldiNotificationFunc) cd_animations_update_icon, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_RENDER_ICON,
		(GldiNotificationFunc) cd_animations_render_icon, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_RENDER_ICON,
		(GldiNotificationFunc) cd_animations_post_render_icon, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_STOP_ICON,
		(GldiNotificationFunc) cd_animations_free_data, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_UNFOLD_SUBDOCK,
		(GldiNotificationFunc) cd_animations_unfold_subdock, NULL);
	
	CDAnimation *pAnimation;
	int i;
	for (i = 0; i < CD_ANIMATIONS_NB_EFFECTS; i ++)
	{
		pAnimation = &myData.pAnimations[i];
		cairo_dock_unregister_animation (pAnimation->cName);
	}
	
	gldi_icons_foreach ((CairoDockForeachIconFunc) _free_data_on_icon, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
static void _update_busy_image_on_icon (Icon *pIcon, CairoDock *pDock, gpointer data)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData != NULL && pData->pBusyImage)
	{
		memcpy (pData->pBusyImage, myData.pBusyImage, sizeof (CairoDockImageBuffer));
	}
}
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
			if (myData.iHaloTexture != 0)
			{
				glDeleteTextures (1, &myData.iHaloTexture);
				myData.iHaloTexture = 0;
			}
			if (myData.iRaysTexture != 0)
			{
				glDeleteTextures (1, &myData.iRaysTexture);
				myData.iRaysTexture = 0;
			}
		}
		if (myData.iSpotFrontTexture != 0)
		{
			glDeleteTextures (1, &myData.iSpotFrontTexture);
			myData.iSpotFrontTexture = 0;
		}
		if (myData.iSpotTexture != 0)
		{
			glDeleteTextures (1, &myData.iSpotTexture);
			myData.iSpotTexture = 0;
		}
	}
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		// recreate myData.pBusyImage, and update icons having the 'busy' animation running.
		if (myData.pBusyImage != NULL)
		{
			cairo_dock_free_image_buffer (myData.pBusyImage);
			myData.pBusyImage = cairo_dock_create_image_buffer (myConfig.cBusyImage ? myConfig.cBusyImage : MY_APPLET_SHARE_DATA_DIR"/busy.svg",
				0, 0,
				CAIRO_DOCK_ANIMATED_IMAGE);
			
			gldi_icons_foreach ((CairoDockForeachIconFunc) _update_busy_image_on_icon, NULL);  // since the image is loaded as a shared ressources for all icons, we have to update them if they were using it.
		}
	}
CD_APPLET_RELOAD_END
