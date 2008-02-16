/**********************************************************************************

This file is a part of the cairo-dock clock applet,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <string.h>
#include "stdlib.h"

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-init.h"

extern AppletConfig myConfig;
extern AppletData myData;


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


CD_APPLET_DEFINITION ("clock", 1, 5, 0)


static void _load_theme (void)
{
	cd_message ("%s (%s)\n", __func__, myConfig.cThemePath);
	//\_______________ On charge le theme choisi (on n'a pas besoin de connaitre les dimmensions de l'icone).
	if (myConfig.cThemePath != NULL)
	{
		GString *sElementPath = g_string_new ("");
		int i;
		for (i = 0; i < CLOCK_ELEMENTS; i ++)
		{
			g_string_printf (sElementPath, "%s/%s", myConfig.cThemePath, s_cFileNames[i]);

			myData.pSvgHandles[i] = rsvg_handle_new_from_file (sElementPath->str, NULL);
			//g_print (" + %s\n", cElementPath);
		}
		g_string_free (sElementPath, TRUE);
		rsvg_handle_get_dimensions (myData.pSvgHandles[CLOCK_DROP_SHADOW], &myData.DimensionData);
	}
	else
	{
		myData.DimensionData.width = 48;  // valeur par defaut si aucun theme.
		myData.DimensionData.height = 48;
	}
}
static void _load_back_and_fore_ground (void)
{
	cd_debug ("\n");
	double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);

	//\_______________ On construit les surfaces d'arriere-plan et d'avant-plan une bonne fois pour toutes.
	myData.pBackgroundSurface = update_surface (NULL,
		myDrawContext,
		myIcon->fWidth * fMaxScale,
		myIcon->fHeight * fMaxScale,
		KIND_BACKGROUND);
	myData.pForegroundSurface = update_surface (NULL,
		myDrawContext,
		myIcon->fWidth * fMaxScale,
		myIcon->fHeight * fMaxScale,
		KIND_FOREGROUND);
}

CD_APPLET_INIT_BEGIN (erreur)
	//\_______________ On charge nos surfaces.
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, myDesklet->iWidth - 2 * g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - 2 * g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius;
		myIcon->fDrawY = g_iDockRadius;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = cd_clock_draw_in_desklet;
	}
	_load_theme ();
	_load_back_and_fore_ground ();
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	
	//\_______________ On lance le timer.
	cd_clock_update_with_time (myIcon);
	myData.iSidUpdateClock = g_timeout_add ((myConfig.bShowSeconds ? 1e3: 60e3), (GSourceFunc) cd_clock_update_with_time, (gpointer) myIcon);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT

	//\_______________ On stoppe le timer.
	g_source_remove (myData.iSidUpdateClock);
	myData.iSidUpdateClock = 0;

	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	cd_debug ("%s\n", CD_APPLET_MY_CONF_FILE);
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, myDesklet->iWidth - 2 * g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - 2 * g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius;
		myIcon->fDrawY = g_iDockRadius;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = cd_clock_draw_in_desklet;
	}
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On stoppe le timer.
		g_source_remove (myData.iSidUpdateClock);
		myData.iSidUpdateClock = 0;

		//\_______________ On charge notre theme.
		_load_theme ();
		//\_______________ On charge les surfaces d'avant et arriere-plan.
		_load_back_and_fore_ground ();

		//\_______________ On relance le timer.
		cd_clock_update_with_time (myIcon);
		myData.iSidUpdateClock = g_timeout_add ((myConfig.bShowSeconds ? 1e3: 60e3), (GSourceFunc) cd_clock_update_with_time, (gpointer) myIcon);
	}
	else
	{
		//\_______________ On charge les surfaces d'avant et arriere-plan, les rsvg_handle ne dependent pas de g_fAmplitude.
		cairo_surface_destroy (myData.pForegroundSurface);
		cairo_surface_destroy (myData.pBackgroundSurface);
		_load_back_and_fore_ground ();

		cd_clock_update_with_time (myIcon);
	}
CD_APPLET_RELOAD_END
