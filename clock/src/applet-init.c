/**********************************************************************************

This file is a part of the cairo-dock clock applet,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <string.h>
#include "stdlib.h"

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-digital.h" //Digital html like renderer
#include "applet-config.h"
#include "applet-theme.h"
#include "applet-notifications.h"
#include "applet-init.h"


CD_APPLET_PRE_INIT_BEGIN (N_("clock"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet displays time and date in your dock.\n"
	"2 view are available : numeric and analogic, based on Cairo-Clock.\n"
	"It is compatible with the Cairo-Clock's themes, and you can detach itself to be a perfect clone of Cairo-Clock.\n"
	"It supports alarms, and a basic calendar, and allows you to setup time and date.\n"
	"Left-click to show/hide the calendar, Middle-click to stop an alarm."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	pInterface->load_custom_widget = cd_clock_load_custom_widget;
	pInterface->save_custom_widget = cd_clock_save_custom_widget;
CD_APPLET_PRE_INIT_END


CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (myConfig.cLocation != NULL)
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cLocation+1);
	
	//\_______________ On charge notre theme.
	cd_clock_load_theme (myApplet);
	cd_clock_load_back_and_fore_ground (myApplet);
	g_print ("CD_APPLET_MY_CONTAINER_IS_OPENGL : %d\n", CD_APPLET_MY_CONTAINER_IS_OPENGL);
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		cd_clock_load_textures (myApplet);
	
	///cd_clock_configure_digital (myApplet);  // mis en commentaire jusqu'a ce que ca soit fini.
	
	myData.cSystemLocation = g_strdup (g_getenv ("TZ"));
	myData.iLastCheckedMinute = -1;
	myData.iLastCheckedDay = -1;
	myData.iLastCheckedMonth = -1;
	myData.iLastCheckedYear = -1;
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOldStyle && myConfig.bShowSeconds && myConfig.iSmoothAnimationDuration != 0)
	{
		CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
		cairo_dock_launch_animation (myContainer);
	}
	
	//\_______________ On lance le timer.
	if (! myConfig.bShowSeconds)  // pour ne pas attendre 1 mn avant d'avoir le dessin.
		cd_clock_update_with_time (myApplet);
	myData.iSidUpdateClock = g_timeout_add_seconds ((myConfig.bShowSeconds ? 1: 60), (GSourceFunc) cd_clock_update_with_time, (gpointer) myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
	
	//\_______________ On stoppe le timer.
	g_source_remove (myData.iSidUpdateClock);
	myData.iSidUpdateClock = 0;

	cd_clock_free_timezone_list ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	///cd_clock_configure_digital (myApplet);  // mis en commentaire jusqu'a ce que ca soit fini.
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On stoppe le timer.
		g_source_remove (myData.iSidUpdateClock);
		myData.iSidUpdateClock = 0;
		CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
		//\_______________ On efface tout
		cd_clock_clear_theme (myApplet, TRUE);
		//\_______________ On charge notre theme.
		cd_clock_load_theme (myApplet);
		//\_______________ On charge les surfaces d'avant et arriere-plan.
		cd_clock_load_back_and_fore_ground (myApplet);
		//\_______________ On charge les textures.
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		{
			cd_clock_load_textures (myApplet);
			cairo_dock_launch_animation (myContainer);
		}
		
		if (myConfig.cLocation != NULL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cLocation+1);
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOldStyle && myConfig.bShowSeconds && myConfig.iSmoothAnimationDuration != 0)
		{
			CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
			cairo_dock_launch_animation (myContainer);
		}
		
		//\_______________ On relance le timer.
		myData.iLastCheckedMinute = -1;
		myData.iLastCheckedDay = -1;
		myData.iLastCheckedMonth = -1;
		myData.iLastCheckedYear = -1;
		cd_clock_update_with_time (myApplet);
		myData.iSidUpdateClock = g_timeout_add_seconds ((myConfig.bShowSeconds ? 1: 60), (GSourceFunc) cd_clock_update_with_time, (gpointer) myApplet);
	}
	else
	{
		//\_______________ On charge les surfaces d'avant et arriere-plan et les textures, les rsvg_handle ne dependent pas de la taille.
		cd_clock_clear_theme (myApplet, FALSE);
		cd_clock_load_back_and_fore_ground (myApplet);
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
			cd_clock_load_textures (myApplet);

		cd_clock_update_with_time (myApplet);
	}
CD_APPLET_RELOAD_END
