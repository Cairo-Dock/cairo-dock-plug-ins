#include <string.h>

#include "tomboy-struct.h"
#include "tomboy-dbus.h"
#include "tomboy-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cIconDefault 		= CD_CONFIG_GET_STRING ("Icon", "default icon");
	myConfig.cIconClose		= CD_CONFIG_GET_STRING ("Icon", "close icon");
	myConfig.cIconBroken 		= CD_CONFIG_GET_STRING ("Icon", "broken icon");
	myConfig.bNoDeletedSignal 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "no deleted signal");
	myConfig.cRenderer 		= CD_CONFIG_GET_STRING ("Configuration", "renderer");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	
	g_free (myConfig.cIconDefault);
	g_free (myConfig.cIconClose);
	g_free (myConfig.cIconBroken);
	g_free (myConfig.cRenderer);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_surface_destroy (myData.pSurfaceDefault);
	cairo_surface_destroy (myData.pSurfaceClose);
	cairo_surface_destroy (myData.pSurfaceBroken);
	
	free_all_notes ();
	g_hash_table_destroy (myData.hNoteTable);
	
	if (myIcon->pSubDock != NULL)
		cairo_dock_destroy_dock (myIcon->pSubDock,myIcon->acName, NULL, NULL);
CD_APPLET_RESET_DATA_END
