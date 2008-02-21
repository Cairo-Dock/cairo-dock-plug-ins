
#ifndef __APPLET_READ_DATA__
#define  __APPLET_READ_DATA__

#include <cairo-dock.h>


void cd_weather_get_data (gchar **cCurrentConditionsFilePath, gchar **cForecastFilePath);


void cd_weather_parse_data (gchar *cDataFilePath, gboolean bParseHeader, GError **erreur);


#endif
