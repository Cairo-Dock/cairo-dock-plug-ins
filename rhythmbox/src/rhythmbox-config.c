#include <cairo-dock.h>

#include "rhythmbox-config.h"

CairoDockAnimationType conf_changeAnimation;
gboolean conf_enableDialogs;
double conf_timeDialogs;
gboolean conf_enableCover;
gchar *conf_quickInfoType;

CD_APPLET_CONFIG_BEGIN ("Rhythmbox", NULL)
	conf_enableDialogs 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	conf_enableCover 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_cover");
	conf_timeDialogs 		= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 3000);
	conf_changeAnimation 	= CD_CONFIG_GET_ANIMATION_WITH_DEFAULT ("Configuration", "change_animation", CAIRO_DOCK_ROTATE);
	conf_quickInfoType 		= CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "quick_info_type", "elapsed");
CD_APPLET_CONFIG_END
