
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-mixer.h"
#include "applet-notifications.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.card_id = CD_CONFIG_GET_STRING ("Configuration", "card id");
	if (myConfig.card_id == NULL)
		myConfig.card_id = g_strdup ("default");
	
	myConfig.cMixerElementName = CD_CONFIG_GET_STRING ("Configuration", "mixer element");
	
	myConfig.cShowAdvancedMixerCommand = CD_CONFIG_GET_STRING ("Configuration", "show mixer");
	
	myConfig.cShortcut = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	
	myConfig.iScrollVariation = CD_CONFIG_GET_INTEGER ("Configuration", "scroll variation");
	
	myConfig.bHideScaleOnLeave = CD_CONFIG_GET_BOOLEAN ("Configuration", "hide on leave");
	
	
	myConfig.iVolumeDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "display volume");
	
	myConfig.iVolumeEffect = CD_CONFIG_GET_INTEGER ("Configuration", "effect");
	
	myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cBrokenIcon = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
	myConfig.cMuteIcon = CD_CONFIG_GET_STRING ("Configuration", "mute icon");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.card_id);
	g_free (myConfig.cShowAdvancedMixerCommand);
	if (myConfig.cShortcut)
		cd_keybinder_unbind(myConfig.cShortcut, (CDBindkeyHandler) mixer_on_keybinding_pull);
	g_free (myConfig.cShortcut);
	g_free (myConfig.cDefaultIcon);
	g_free (myConfig.cBrokenIcon);
	g_free (myConfig.cMuteIcon);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myData.pScale != NULL)
	{
		gtk_widget_destroy (myData.pScale);
		myData.pScale = NULL;
	}
	mixer_stop ();
	cairo_surface_destroy (myData.pSurface);
	cairo_surface_destroy (myData.pMuteSurface);
	cairo_dock_dialog_unreference (myData.pDialog);
	g_free (myData.cErrorMessage);
	g_free (myData.mixer_card_name);
	g_free (myData.mixer_device_name);
CD_APPLET_RESET_DATA_END


void mixer_write_elements_list (gchar *cConfFilePath, GKeyFile *pKeyFile)
{
	gchar *cElements = mixer_get_elements_list ();
	
	cairo_dock_update_conf_file_with_list (pKeyFile, cConfFilePath, cElements, "Configuration", "mixer element", NULL);
	
	g_free (cElements);
}
