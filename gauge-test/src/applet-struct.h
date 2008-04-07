
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *cThemePath;
	} AppletConfig;

typedef struct {
	double gaugeValue;
	Gauge *pGauge;
	} AppletData;


#endif
