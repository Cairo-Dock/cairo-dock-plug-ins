
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-mixer.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN ("Sound", "")
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.card_id = CD_CONFIG_GET_STRING ("Configuration", "card id");
	if (myConfig.card_id == NULL)
		myConfig.card_id = g_strdup ("default");
	
	myConfig.cMixerElementName = CD_CONFIG_GET_STRING ("Configuration", "mixer element");
	if (myData.mixer_handle != NULL)  // sinon c'est qu'on n'a pas encore initialise la carte.
		mixer_write_elements_list (CD_APPLET_MY_CONF_FILE, CD_APPLET_MY_KEY_FILE);
	
	myConfig.cShowAdvancedMixerCommand = CD_CONFIG_GET_STRING ("Configuration", "show mixer");
	
	
	myConfig.iVolumeDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "display volume");
	
	myConfig.iVolumeEffect = CD_CONFIG_GET_INTEGER ("Configuration", "effect");
	
	myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.card_id);
	myConfig.card_id = NULL;
	g_free (myConfig.cShowAdvancedMixerCommand);
	myConfig.cShowAdvancedMixerCommand = NULL;
	g_free (myConfig.cDefaultLabel);
	myConfig.cDefaultLabel = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	mixer_stop ();
	cairo_surface_destroy (myData.pSurface);
	myData.pSurface = NULL;
	cairo_surface_destroy (myData.pBrokenSurface);
	myData.pBrokenSurface = NULL;
	cairo_surface_destroy (myData.pMuteSurface);
	myData.pMuteSurface = NULL;
	cairo_dock_dialog_unreference (myData.pDialog);
	myData.pDialog = NULL;
	
	memset (&myData, 0, sizeof (AppletData));
}


void mixer_write_elements_list (gchar *cConfFilePath, GKeyFile *pKeyFile)
{
	gchar *cElements = mixer_get_elements_list ();
	
	cairo_dock_update_conf_file_with_list (pKeyFile, cConfFilePath, cElements, "Configuration", "mixer element", NULL);
	
	g_free (cElements);
}
