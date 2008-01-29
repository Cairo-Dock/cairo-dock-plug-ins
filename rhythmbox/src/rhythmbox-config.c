#include <cairo-dock.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-config.h"

extern cairo_surface_t *rhythmbox_pSurface;
extern cairo_surface_t *rhythmbox_pPlaySurface;
extern cairo_surface_t *rhythmbox_pPauseSurface;
extern cairo_surface_t *rhythmbox_pStopSurface;
extern cairo_surface_t *rhythmbox_pCover;
extern cairo_surface_t *rhythmbox_pBrokenSurface;

CairoDockAnimationType conf_changeAnimation;
gboolean conf_enableDialogs;
double conf_timeDialogs;
gboolean conf_enableCover;
MyAppletQuickInfoType conf_quickInfoType;

CD_APPLET_CONFIG_BEGIN ("Rhythmbox", NULL)
	conf_enableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	conf_enableCover 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_cover");
	conf_timeDialogs 		= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 3000);
	conf_changeAnimation 	= CD_CONFIG_GET_ANIMATION_WITH_DEFAULT ("Configuration", "change_animation", CAIRO_DOCK_ROTATE);
	conf_quickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
CD_APPLET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_surface_destroy (rhythmbox_pSurface);
	rhythmbox_pSurface = NULL;
	cairo_surface_destroy (rhythmbox_pPlaySurface);
	rhythmbox_pPlaySurface = NULL;
	cairo_surface_destroy (rhythmbox_pPauseSurface);
	rhythmbox_pPauseSurface = NULL;
	cairo_surface_destroy (rhythmbox_pCover);
	rhythmbox_pCover = NULL;
	cairo_surface_destroy (rhythmbox_pBrokenSurface);
	rhythmbox_pBrokenSurface = NULL;
CD_APPLET_RESET_DATA_END
