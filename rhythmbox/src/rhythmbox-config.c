#include <cairo-dock.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-config.h"

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
