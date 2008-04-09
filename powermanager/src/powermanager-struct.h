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
	
	gboolean batteryWitness;
	gboolean highBatteryWitness;
	gboolean lowBatteryWitness;
	CairoDockAnimationType batteryWitnessAnimation;
	gint lowBatteryValue;
	gchar *cThemePath;
	
	gboolean bUseGauge;
	gchar *cUserBatteryIconName;
	gchar *cUserChargeIconName;
	MyAppletEffect iEffect;
  } AppletConfig;


typedef struct {
	cairo_surface_t *pSurfaceBattery;
	cairo_surface_t *pSurfaceCharge;
	gboolean dbus_enable;
	gboolean battery_present;
	gboolean on_battery, previously_on_battery;
	gint battery_time, previous_battery_time;
	gint battery_charge, previous_battery_charge;
	gboolean alerted;
	gint checkLoop;
	Gauge *pGauge;
	} AppletData;


#endif
