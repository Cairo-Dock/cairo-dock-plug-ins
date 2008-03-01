
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-mixer.h"
#include "applet-notifications.h"
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
	
	myConfig.cShowAdvancedMixerCommand = CD_CONFIG_GET_STRING ("Configuration", "show mixer");
	
	myConfig.cShortcut = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "shortkey", "<Ctrl>F3");
	
	
	myConfig.iVolumeDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "display volume");
	
	myConfig.iVolumeEffect = CD_CONFIG_GET_INTEGER ("Configuration", "effect");
	
	myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cBrokenIcon = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
	myConfig.cMuteIcon = CD_CONFIG_GET_STRING ("Configuration", "mute icon");
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.card_id);
	myConfig.card_id = NULL;
	g_free (myConfig.cShowAdvancedMixerCommand);
	myConfig.cShowAdvancedMixerCommand = NULL;
	if (myConfig.cShortcut)
		cd_keybinder_unbind(myConfig.cShortcut, (CDBindkeyHandler) mixer_on_keybinding_pull);
	g_free (myConfig.cShortcut);
	myConfig.cShortcut = NULL;
	g_free (myConfig.cDefaultIcon);
	myConfig.cDefaultIcon = NULL;
	g_free (myConfig.cBrokenIcon);
	myConfig.cBrokenIcon = NULL;
	g_free (myConfig.cMuteIcon);
	myConfig.cMuteIcon = NULL;
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	if (myData.pScale != NULL)
	{
		gtk_widget_destroy (myData.pScale);
		myData.pScale = NULL;
	}
	mixer_stop ();
	cairo_surface_destroy (myData.pSurface);
	myData.pSurface = NULL;
	cairo_surface_destroy (myData.pBrokenSurface);
	myData.pBrokenSurface = NULL;
	cairo_surface_destroy (myData.pMuteSurface);
	myData.pMuteSurface = NULL;
	cairo_dock_dialog_unreference (myData.pDialog);
	myData.pDialog = NULL;
	g_free (myData.cErrorMessage);
	myData.cErrorMessage = NULL;
	g_free (myData.mixer_card_name);
	myData.mixer_card_name = NULL;
	g_free (myData.mixer_device_name);
	myData.mixer_device_name= NULL;
	memset (&myData, 0, sizeof (AppletData));
}


void mixer_write_elements_list (gchar *cConfFilePath, GKeyFile *pKeyFile)
{
	g_print ("%s ()\n", __func__);
	gchar *cElements = mixer_get_elements_list ();
	
	cairo_dock_update_conf_file_with_list (pKeyFile, cConfFilePath, cElements, "Configuration", "mixer element", NULL);
	
	g_free (cElements);
}
