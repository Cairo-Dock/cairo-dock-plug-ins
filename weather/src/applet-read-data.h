
#ifndef __APPLET_READ_DATA__
#define  __APPLET_READ_DATA__

#include <cairo-dock.h>


gchar *cd_weather_get_location_data (gchar *cLocation);

void cd_weather_acquisition (CairoDockModuleInstance *myApplet);


GList *cd_weather_parse_location_data  (gchar *cDataFilePath, GError **erreur);

void cd_weather_parse_data (CairoDockModuleInstance *myApplet, gchar *cDataFilePath, gboolean bParseHeader, GError **erreur);

void cd_weather_read_data (CairoDockModuleInstance *myApplet);


#endif
