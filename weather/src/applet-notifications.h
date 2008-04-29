
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>


CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

CD_APPLET_ON_MIDDLE_CLICK_H


CairoDialog *cd_weather_show_forecast_dialog (Icon *pIcon);

CairoDialog *cd_weather_show_current_conditions_dialog (void);

#endif
