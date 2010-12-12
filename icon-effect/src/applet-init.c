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

#include "applet-config.h"
#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-fire.h"
#include "applet-star.h"
#include "applet-rain.h"
#include "applet-snow.h"
#include "applet-storm.h"
#include "applet-firework.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("icon effects"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_THEME,
	N_("This plugin adds many special effects to your icons."),
	"Fabounet (Fabrice Rey)")
	if (! g_bUseOpenGL)
		return FALSE;
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END

static inline void _set_effects_duration (void)
{
	myData.pEffects[CD_ICON_EFFECT_FIRE].iDuration = myConfig.iFireDuration;
	myData.pEffects[CD_ICON_EFFECT_FIRE].bRepeat = myConfig.bContinueFire;
	
	myData.pEffects[CD_ICON_EFFECT_STARS].iDuration = myConfig.iStarDuration;
	myData.pEffects[CD_ICON_EFFECT_STARS].bRepeat = myConfig.bContinueStar;
	
	myData.pEffects[CD_ICON_EFFECT_RAIN].iDuration = myConfig.iRainDuration;
	myData.pEffects[CD_ICON_EFFECT_RAIN].bRepeat = myConfig.bContinueRain;
	
	myData.pEffects[CD_ICON_EFFECT_SNOW].iDuration = myConfig.iSnowDuration;
	myData.pEffects[CD_ICON_EFFECT_SNOW].bRepeat = myConfig.bContinueSnow;
	
	myData.pEffects[CD_ICON_EFFECT_SAND].iDuration = myConfig.iStormDuration;
	myData.pEffects[CD_ICON_EFFECT_SAND].bRepeat = myConfig.bContinueStorm;
	
	myData.pEffects[CD_ICON_EFFECT_FIREWORK].iDuration = myConfig.iFireworkDuration;
	myData.pEffects[CD_ICON_EFFECT_FIREWORK].bRepeat = myConfig.bContinueFirework;
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (! g_bUseOpenGL || ! cairo_dock_reserve_data_slot (myApplet))
		return;
	
	cairo_dock_register_notification_on_object (&myContainersMgr,
		NOTIFICATION_ENTER_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_on_enter,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_object (&myContainersMgr,
		NOTIFICATION_CLICK_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_on_click,
		CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification_on_object (&myIconsMgr,
		NOTIFICATION_REQUEST_ICON_ANIMATION,
		(CairoDockNotificationFunc) cd_icon_effect_on_request,
		CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification_on_object (&myIconsMgr,
		NOTIFICATION_UPDATE_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_update_icon,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_object (&myIconsMgr,
		NOTIFICATION_PRE_RENDER_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_pre_render_icon,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_object (&myIconsMgr,
		NOTIFICATION_RENDER_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_render_icon,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_object (&myIconsMgr,
		NOTIFICATION_STOP_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_free_data,
		CAIRO_DOCK_RUN_AFTER, NULL);
	
	cd_icon_effect_register_fire (&myData.pEffects[CD_ICON_EFFECT_FIRE]);
	myData.iAnimationID[CD_ICON_EFFECT_FIRE] = cairo_dock_register_animation ("fire", D_("Fire"), TRUE);
	
	cd_icon_effect_register_stars (&myData.pEffects[CD_ICON_EFFECT_STARS]);
	myData.iAnimationID[CD_ICON_EFFECT_STARS] = cairo_dock_register_animation ("stars", D_("Stars"), TRUE);
	
	cd_icon_effect_register_rain (&myData.pEffects[CD_ICON_EFFECT_RAIN]);
	myData.iAnimationID[CD_ICON_EFFECT_RAIN] = cairo_dock_register_animation ("rain", D_("Rain"), TRUE);
	
	cd_icon_effect_register_snow (&myData.pEffects[CD_ICON_EFFECT_SNOW]);
	myData.iAnimationID[CD_ICON_EFFECT_SNOW] = cairo_dock_register_animation ("snow", D_("Snow"), TRUE);
	
	cd_icon_effect_register_storm (&myData.pEffects[CD_ICON_EFFECT_SAND]);
	myData.iAnimationID[CD_ICON_EFFECT_SAND] = cairo_dock_register_animation ("storm", D_("Storm"), TRUE);
	
	cd_icon_effect_register_firework (&myData.pEffects[CD_ICON_EFFECT_FIREWORK]);
	myData.iAnimationID[CD_ICON_EFFECT_FIREWORK] = cairo_dock_register_animation ("firework", D_("Firework"), TRUE);
	
	_set_effects_duration ();
CD_APPLET_INIT_END


static void _free_data_on_icon (Icon *pIcon, CairoDock *pDock, gpointer data)
{
	cd_icon_effect_free_data (NULL, pIcon);
}
//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_ENTER_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_on_enter, NULL);
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_CLICK_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_on_click, NULL);
	cairo_dock_remove_notification_func_on_object (&myIconsMgr,
		NOTIFICATION_REQUEST_ICON_ANIMATION,
		(CairoDockNotificationFunc) cd_icon_effect_on_request, NULL);
	cairo_dock_remove_notification_func_on_object (&myIconsMgr,
		NOTIFICATION_UPDATE_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_update_icon, NULL);
	cairo_dock_remove_notification_func_on_object (&myIconsMgr,
		NOTIFICATION_PRE_RENDER_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_pre_render_icon, NULL);
	cairo_dock_remove_notification_func_on_object (&myIconsMgr,
		NOTIFICATION_RENDER_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_render_icon, NULL);
	cairo_dock_remove_notification_func_on_object (&myIconsMgr,
		NOTIFICATION_STOP_ICON,
		(CairoDockNotificationFunc) cd_icon_effect_free_data, NULL);
	
	cairo_dock_unregister_animation ("fire");
	cairo_dock_unregister_animation ("stars");
	cairo_dock_unregister_animation ("rain");
	cairo_dock_unregister_animation ("snow");
	cairo_dock_unregister_animation ("storm");
	cairo_dock_unregister_animation ("firework");
	
	cairo_dock_foreach_icons ((CairoDockForeachIconFunc) _free_data_on_icon, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
static gboolean _effect_is_used_in_table (CDIconEffectsEnum iEffect, CDIconEffectsEnum *pEffectList)
{
	int i;
	for (i = 0; i < CD_ICON_EFFECT_NB_EFFECTS; i ++)
	{
		if (pEffectList[i] == iEffect)
			return TRUE;
		else if (myConfig.iEffectsUsed[i] >= CD_ICON_EFFECT_NB_EFFECTS)
			break ;
	}
	return FALSE;
}
static gboolean _effect_is_used (CDIconEffectsEnum iEffect)
{
	gboolean bUsed;
	bUsed = _effect_is_used_in_table (iEffect, myConfig.iEffectsUsed);
	if (bUsed)
		return TRUE;
	bUsed = _effect_is_used_in_table (iEffect, myConfig.iEffectsOnClick[CAIRO_DOCK_LAUNCHER]);
	if (bUsed)
		return TRUE;
	bUsed = _effect_is_used_in_table (iEffect, myConfig.iEffectsOnClick[CAIRO_DOCK_APPLI]);
	if (bUsed)
		return TRUE;
	bUsed = _effect_is_used_in_table (iEffect, myConfig.iEffectsOnClick[CAIRO_DOCK_APPLET]);
	return bUsed;
}
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.iFireTexture != 0 && ! _effect_is_used (CD_ICON_EFFECT_FIRE) && ! _effect_is_used (CD_ICON_EFFECT_SAND) && ! _effect_is_used (CD_ICON_EFFECT_FIREWORK))
		{
			glDeleteTextures (1, &myData.iFireTexture);
			myData.iFireTexture = 0;
		}
		if (myData.iStarTexture != 0 && ! _effect_is_used (CD_ICON_EFFECT_STARS))
		{
			glDeleteTextures (1, &myData.iStarTexture);
			myData.iStarTexture = 0;
		}
		if (myData.iSnowTexture != 0 && ! _effect_is_used (CD_ICON_EFFECT_SNOW))
		{
			glDeleteTextures (1, &myData.iSnowTexture);
			myData.iSnowTexture = 0;
		}
		if (myData.iRainTexture != 0 && ! _effect_is_used (CD_ICON_EFFECT_RAIN))
		{
			glDeleteTextures (1, &myData.iRainTexture);
			myData.iRainTexture = 0;
		}
		
		_set_effects_duration ();
	}
CD_APPLET_RELOAD_END
