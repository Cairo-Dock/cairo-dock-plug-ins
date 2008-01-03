/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include "stdlib.h"

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-init.h"


gboolean my_bShowDate;
gboolean my_bShowSeconds;
gboolean my_b24Mode;
gboolean my_bOldStyle;
double my_fTextColor[4];

int my_iSidUpdateClock = 0;
//GHashTable *my_pThemeTable = NULL;
gchar *my_cThemePath = NULL;

cairo_surface_t* my_pBackgroundSurface = NULL;
cairo_surface_t* my_pForegroundSurface = NULL;
RsvgDimensionData my_DimensionData;
RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];

GPtrArray *my_pAlarms = NULL;

char my_cFileNames[CLOCK_ELEMENTS][30] = {
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


CD_APPLET_DEFINITION ("clock", 1, 4, 7)


CD_APPLET_INIT_BEGIN (erreur)
	//\_______________ On met a jour la liste des themes disponibles.
	/*if (my_pThemeTable != NULL)
		cairo_dock_update_conf_file_with_hash_table (CD_APPLET_MY_CONF_FILE, my_pThemeTable, "MODULE", "theme", NULL, (GHFunc) cairo_dock_write_one_theme_name, TRUE, FALSE);*/
	
	//\_______________ On charge le theme choisi (on n'a pas besoin de connaitre les dimmensions de l'icone).
	/*if (cThemeName != NULL && my_pThemeTable != NULL)
	{
		gchar *cThemePath = g_hash_table_lookup (my_pThemeTable, cThemeName);
		if (cThemePath == NULL)
			cThemePath = g_hash_table_lookup (my_pThemeTable, "default");
		g_return_if_fail (cThemePath != NULL);*/
	if (my_cThemePath != NULL)
	{
		GString *sElementPath = g_string_new ("");
		int i;
		for (i = 0; i < CLOCK_ELEMENTS; i ++)
		{
			g_string_printf (sElementPath, "%s/%s", my_cThemePath, my_cFileNames[i]);
			
			my_pSvgHandles[i] = rsvg_handle_new_from_file (sElementPath->str, NULL);
			//g_print (" + %s\n", cElementPath);
		}
		g_string_free (sElementPath, TRUE);
		rsvg_handle_get_dimensions (my_pSvgHandles[CLOCK_DROP_SHADOW], &my_DimensionData);
	}
	else
	{
		my_DimensionData.width = 48;  // valeur par defaut si aucun theme.
		my_DimensionData.height = 48;
	}
	//g_free (cThemeName);
	
	//\_______________ On construit les surfaces d'arriere-plan et d'avant-plan une bonne fois pour toutes.
	my_pBackgroundSurface = update_surface (NULL,
		myDrawContext,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude),
		KIND_BACKGROUND);
	my_pForegroundSurface = update_surface (NULL,
		myDrawContext,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude),
		KIND_FOREGROUND);
	
	//\_______________ On enregistre nos notifications.
	cairo_dock_register_first_notifications (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_CLICK,
		CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU,
		-1);
	
	//\_______________ On lance le timer.
	cd_clock_update_with_time (myIcon);
	my_iSidUpdateClock = g_timeout_add ((my_bShowSeconds ? 1e3: 60e3), (GSourceFunc) cd_clock_update_with_time, (gpointer) myIcon);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	cairo_dock_remove_notification_funcs (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_CLICK,
		CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU,
		-1);
	
	//\_______________ On stoppe le timer.
	g_source_remove (my_iSidUpdateClock);
	my_iSidUpdateClock = 0;
	
	//\_______________ On libere toutes nos ressources.
	int i;
	for (i = 0; i < CLOCK_ELEMENTS; i ++)
	{
		rsvg_handle_free (my_pSvgHandles[i]);
		my_pSvgHandles[i] = NULL;
	}
	
	cairo_surface_destroy (my_pForegroundSurface);
	my_pForegroundSurface = NULL;
	cairo_surface_destroy (my_pBackgroundSurface);
	my_pBackgroundSurface = NULL;
	
	/*g_hash_table_destroy (my_pThemeTable);
	my_pThemeTable = NULL;*/
	g_free (my_cThemePath);
	my_cThemePath = NULL;
	
	CDClockAlarm *pAlarm;
	for (i = 0; i < my_pAlarms->len; i ++)
	{
		pAlarm = g_ptr_array_index (my_pAlarms, i);
		cd_clock_free_alarm (pAlarm);
	}
	g_ptr_array_free (my_pAlarms, TRUE);
	my_pAlarms = NULL;
CD_APPLET_STOP_END
