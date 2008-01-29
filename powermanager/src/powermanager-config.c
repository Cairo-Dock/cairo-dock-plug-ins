#include <cairo-dock.h>

#include "powermanager-config.h"


extern cairo_surface_t *my_pSurfaceBattery04;
extern cairo_surface_t *my_pSurfaceBattery14;
extern cairo_surface_t *my_pSurfaceBattery24;
extern cairo_surface_t *my_pSurfaceBattery34;
extern cairo_surface_t *my_pSurfaceBattery44;
extern cairo_surface_t *my_pSurfaceCharge04;
extern cairo_surface_t *my_pSurfaceCharge14;
extern cairo_surface_t *my_pSurfaceCharge24;
extern cairo_surface_t *my_pSurfaceCharge34;
extern cairo_surface_t *my_pSurfaceCharge44;
extern cairo_surface_t *my_pSurfaceSector;
extern cairo_surface_t *my_pSurfaceBroken;

CD_APPLET_CONFIG_BEGIN ("PowerManager", NULL)
	
CD_APPLET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_surface_destroy (my_pSurfaceBattery44);
	my_pSurfaceBattery44 = NULL;
	cairo_surface_destroy (my_pSurfaceBattery34);
	my_pSurfaceBattery34 = NULL;
	cairo_surface_destroy (my_pSurfaceBattery24);
	my_pSurfaceBattery24 = NULL;
	cairo_surface_destroy (my_pSurfaceBattery14);
	my_pSurfaceBattery14 = NULL;
	cairo_surface_destroy (my_pSurfaceBattery04);
	my_pSurfaceBattery04 = NULL;
	cairo_surface_destroy (my_pSurfaceCharge44);
	my_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (my_pSurfaceCharge34);
	my_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (my_pSurfaceCharge24);
	my_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (my_pSurfaceCharge14);
	my_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (my_pSurfaceCharge04);
	my_pSurfaceCharge34 = NULL;
	cairo_surface_destroy (my_pSurfaceSector);
	my_pSurfaceSector = NULL;
	cairo_surface_destroy (my_pSurfaceBroken);
	my_pSurfaceBroken = NULL;
CD_APPLET_RESET_DATA_END
