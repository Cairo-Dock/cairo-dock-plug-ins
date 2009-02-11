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
#include "applet-notifications.h"
#include "applet-init.h"


static char s_cFileNames[CLOCK_ELEMENTS][30] = {
	"clock-drop-shadow.svg",
	"clock-face.svg",
	"clock-marks.svg",
	"clock-hour-hand-shadow.svg",
	"clock-minute-hand-shadow.svg",
	"clock-second-hand-shadow.svg",
	"clock-hour-hand.svg",
	"clock-minute-hand.svg",
	"clock-second-hand.svg",
	"clock-face-shadow.svg",
	"clock-glass.svg",
	"clock-frame.svg" };

CD_APPLET_PRE_INIT_BEGIN ("clock", 2, 0, 0, CAIRO_DOCK_CATEGORY_ACCESSORY)
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	pInterface->load_custom_widget = cd_clock_load_custom_widget;
	pInterface->save_custom_widget = cd_clock_save_custom_widget;
CD_APPLET_PRE_INIT_END

static void _load_theme (CairoDockModuleInstance *myApplet)
{
	cd_message ("%s (%s)", __func__, myConfig.cThemePath);
	//\_______________ On charge le theme choisi (on n'a pas besoin de connaitre les dimmensions de l'icone).
	if (myConfig.cThemePath != NULL)
	{
		GString *sElementPath = g_string_new ("");
		int i;
		for (i = 0; i < CLOCK_ELEMENTS; i ++)
		{
			g_string_printf (sElementPath, "%s/%s", myConfig.cThemePath, s_cFileNames[i]);
			myData.pSvgHandles[i] = rsvg_handle_new_from_file (sElementPath->str, NULL);
		}
		g_string_free (sElementPath, TRUE);
		rsvg_handle_get_dimensions (myData.pSvgHandles[CLOCK_DROP_SHADOW], &myData.DimensionData);
		rsvg_handle_get_dimensions (myData.pSvgHandles[CLOCK_HOUR_HAND], &myData.needleDimension);
	}
	else
	{
		myData.DimensionData.width = 48;  // valeurs par defaut si aucun theme.
		myData.DimensionData.height = 48;
		myData.needleDimension.width = 48;
		myData.needleDimension.height = 48;
	}
}
static void _load_back_and_fore_ground (CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	double fMaxScale = (myDock != NULL ? (1 + g_fAmplitude) / myDock->fRatio : 1);

	//\_______________ On construit les surfaces d'arriere-plan et d'avant-plan une bonne fois pour toutes.
	myData.pBackgroundSurface = update_surface (myApplet,
		NULL,
		myDrawContext,
		myIcon->fWidth * fMaxScale,
		myIcon->fHeight * fMaxScale,
		KIND_BACKGROUND);
	myData.pForegroundSurface = update_surface (myApplet,
		NULL,
		myDrawContext,
		myIcon->fWidth * fMaxScale,
		myIcon->fHeight * fMaxScale,
		KIND_FOREGROUND);
	if (g_bUseOpenGL)
	{
		if (myData.pBackgroundSurface)
			myData.iBgTexture = cairo_dock_create_texture_from_surface (myData.pBackgroundSurface);
		if (myData.pForegroundSurface)
			myData.iFgTexture = cairo_dock_create_texture_from_surface (myData.pForegroundSurface);
		cairo_surface_t *pNeedleSurface = 
	}
}

CD_APPLET_INIT_BEGIN
	//\_______________ On charge nos surfaces.
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (myConfig.cLocation != NULL)
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cLocation+1);
	
	_load_theme (myApplet);
	_load_back_and_fore_ground (myApplet);
	
	cd_clock_configure_digital (myApplet);
	
	myData.cSystemLocation = g_strdup (g_getenv ("TZ"));
	myData.iLastCheckedMinute = -1;
	myData.iLastCheckedDay = -1;
	myData.iLastCheckedMonth = -1;
	myData.iLastCheckedYear = -1;
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	//\_______________ On lance le timer.
	cd_clock_update_with_time (myApplet);
	myData.iSidUpdateClock = g_timeout_add_seconds ((myConfig.bShowSeconds ? 1: 60), (GSourceFunc) cd_clock_update_with_time, (gpointer) myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;

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
	
	cd_clock_configure_digital (myApplet);
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On stoppe le timer.
		g_source_remove (myData.iSidUpdateClock);
		myData.iSidUpdateClock = 0;

		//\_______________ On charge notre theme.
		_load_theme (myApplet);
		//\_______________ On charge les surfaces d'avant et arriere-plan.
		_load_back_and_fore_ground (myApplet);
		
		if (myConfig.cLocation != NULL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cLocation+1);
		
		//\_______________ On relance le timer.
		cd_clock_update_with_time (myApplet);
		myData.iSidUpdateClock = g_timeout_add_seconds ((myConfig.bShowSeconds ? 1: 60), (GSourceFunc) cd_clock_update_with_time, (gpointer) myApplet);
	}
	else
	{
		//\_______________ On charge les surfaces d'avant et arriere-plan, les rsvg_handle ne dependent pas de g_fAmplitude.
		cairo_surface_destroy (myData.pForegroundSurface);
		cairo_surface_destroy (myData.pBackgroundSurface);
		_load_back_and_fore_ground (myApplet);

		cd_clock_update_with_time (myApplet);
	}
CD_APPLET_RELOAD_END
