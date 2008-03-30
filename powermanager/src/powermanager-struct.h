#ifndef __POWERMANAGER_STRUCT__
#define  __POWERMANAGER_STRUCT__

#include <cairo-dock.h>


typedef enum {
	POWER_MANAGER_NOTHING = 0,
	POWER_MANAGER_CHARGE,
	POWER_MANAGER_TIME,
	POWER_MANAGER_NB_QUICK_INFO_TYPE
  } MyAppletQuickInfoType;

typedef enum {
  POWER_MANAGER_EFFECT_NONE = 0,
  POWER_MANAGER_EFFECT_ZOOM,
  POWER_MANAGER_EFFECT_TRANSPARENCY,
  POWER_MANAGER_EFFECT_BAR,
  } MyAppletEffect;

typedef struct {
	gchar *defaultTitle;
	MyAppletQuickInfoType quickInfoType;
	gint iCheckInterval;
	
	gboolean highBatteryWitness;
	gboolean lowBatteryWitness;
	gboolean lowBatteryValue;
	gchar *cThemePath;
	
	MyAppletEffect iEffect;
  } AppletConfig;


typedef struct {
	cairo_surface_t *pSurfaceBattery04;
	cairo_surface_t *pSurfaceBattery14;
	cairo_surface_t *pSurfaceBattery24;
	cairo_surface_t *pSurfaceBattery34;
	cairo_surface_t *pSurfaceBattery44;
	cairo_surface_t *pSurfaceCharge04;
	cairo_surface_t *pSurfaceCharge14;
	cairo_surface_t *pSurfaceCharge24;
	cairo_surface_t *pSurfaceCharge34;
	cairo_surface_t *pSurfaceCharge44;
	cairo_surface_t *pSurfaceSector;
	cairo_surface_t *pSurfaceBroken;
	gboolean dbus_enable;
	gboolean battery_present;
	gboolean on_battery;
	gint battery_time;
	gint battery_charge;
	gboolean previously_on_battery;
	gint previous_battery_time;
	gint previous_battery_charge;
	gint checkLoop;
	Gauge *pGauge;
	} AppletData;


#endif
