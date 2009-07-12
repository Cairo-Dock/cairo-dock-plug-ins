#include <string.h>

#include "tomboy-struct.h"
#include "tomboy-dbus.h"
#include "tomboy-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cIconDefault 		= CD_CONFIG_GET_STRING ("Icon", "default icon");
	myConfig.cIconClose		= CD_CONFIG_GET_STRING ("Icon", "close icon");
	myConfig.cIconBroken 		= CD_CONFIG_GET_STRING ("Icon", "broken icon");
	myConfig.bNoDeletedSignal 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "no deleted signal");
	myConfig.cRenderer 		= CD_CONFIG_GET_STRING ("Configuration", "renderer");
	myConfig.bDrawContent 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "draw content");
	myConfig.cDateFormat 		= CD_CONFIG_GET_STRING ("Configuration", "date format");
	if (myConfig.cDateFormat == NULL)
		myConfig.cDateFormat = g_strdup ("%d/%m/%y");
	myConfig.bAutoNaming = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "auto-naming", TRUE);
	myConfig.bAskBeforeDelete = CD_CONFIG_GET_BOOLEAN ("Configuration", "ask delete");
	double couleur[3] = {1., 0., 0.5};
	CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT ("Configuration", "text color", myConfig.fTextColor, couleur);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	
	g_free (myConfig.cIconDefault);
	g_free (myConfig.cIconClose);
	g_free (myConfig.cIconBroken);
	g_free (myConfig.cRenderer);
	g_free (myConfig.cDateFormat);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pTask);
	cairo_surface_destroy (myData.pSurfaceDefault);
	cairo_surface_destroy (myData.pSurfaceNote);
	
	free_all_notes ();
	g_hash_table_destroy (myData.hNoteTable);
	
	CD_APPLET_DESTROY_MY_SUBDOCK;
CD_APPLET_RESET_DATA_END
