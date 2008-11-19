/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "chrome-tex.h"

#include "applet-config.h"
#include "applet-icon-renderer.h"
#include "applet-mesh-factory.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_PRE_INIT_BEGIN("Animated icons", 2, 0, 0, CAIRO_DOCK_CATEGORY_PLUG_IN)
	if (! g_bUseOpenGL)
		return FALSE;
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
CD_APPLET_PRE_INIT_END


static GLuint _load_mesh (CDAnimationsMeshType iMeshType)
{
	GLuint iCallList = 0;
	switch (iMeshType)
	{
		case CD_SQUARE_MESH :
			iCallList = cairo_dock_load_square_calllist ();
		break ;
		
		case CD_CUBE_MESH :
			iCallList = cairo_dock_load_cube_calllist ();
		break ;
		
		case CD_CAPSULE_MESH :
			iCallList = cairo_dock_load_capsule_calllist ();
		break ;
	}
	return iCallList;
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! cairo_dock_reserve_data_slot (myApplet))
		return;
	
	myData.iCallList[CD_SQUARE_MESH] = cairo_dock_load_square_calllist ();
	
	if (myConfig.iRotationDuration != 0)
	{
		myData.iChromeTexture = cd_animation_load_chrome_texture ();
		if (myData.iCallList[myConfig.iMeshType] == 0)
			myData.iCallList[myConfig.iMeshType] = _load_mesh (myConfig.iMeshType);
	}
	
	cairo_dock_register_notification (CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) cd_animations_start, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_animations_update_icon , CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_animations_render_icon, CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_animations_post_render_icon, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_animations_free_data, CAIRO_DOCK_RUN_AFTER, NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func (CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) cd_animations_start, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_animations_update_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_animations_render_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_ICON, (CairoDockNotificationFunc) cd_animations_post_render_icon, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_animations_free_data, NULL);
	
	/// foreach icons : free data.
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myConfig.iRotationDuration != 0 && myData.iChromeTexture != 0)
		{
			glDeleteTextures (1, &myData.iChromeTexture);
			myData.iChromeTexture = 0;
		}
		
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
			myData.iCallList[myConfig.iMeshType] = _load_mesh (myConfig.iMeshType);
	}
	else
	{
		
	}
CD_APPLET_RELOAD_END
