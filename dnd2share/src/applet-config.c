/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-dnd2share.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bEnableDialogs = CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.dTimeDialogs = 1000. * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "dialogs_duration", 5);
	myConfig.iNbItems = CD_CONFIG_GET_INTEGER ("Configuration", "nb_items");
	myConfig.bkeepCopy = CD_CONFIG_GET_BOOLEAN ("Configuration", "keep copy");
	myConfig.bDisplayLastImage = myConfig.bkeepCopy && CD_CONFIG_GET_BOOLEAN ("Configuration", "display last image");
	myConfig.cIconAnimation = CD_CONFIG_GET_STRING ("Configuration", "animation");
	myConfig.iPreferedSite[CD_TYPE_TEXT] = CD_CONFIG_GET_INTEGER ("Configuration", "text site");
	myConfig.iPreferedSite[CD_TYPE_IMAGE] = CD_CONFIG_GET_INTEGER ("Configuration", "image site");
	myConfig.iPreferedSite[CD_TYPE_VIDEO] = CD_CONFIG_GET_INTEGER ("Configuration", "video site");
	myConfig.iPreferedSite[CD_TYPE_FILE] = CD_CONFIG_GET_INTEGER ("Configuration", "file site");
	
	myConfig.cCustomScripts[CD_TYPE_TEXT] = CD_CONFIG_GET_STRING ("Configuration", "text script");
	myConfig.cCustomScripts[CD_TYPE_IMAGE] = CD_CONFIG_GET_STRING ("Configuration", "image script");
	myConfig.cCustomScripts[CD_TYPE_VIDEO] = CD_CONFIG_GET_STRING ("Configuration", "video script");
	myConfig.cCustomScripts[CD_TYPE_FILE] = CD_CONFIG_GET_STRING ("Configuration", "file script");
	int i;
	for (i = 0; i < CD_NB_FILE_TYPES; i ++)  // on empeche de choisir le backend custom si aucun script n'est fourni.
	{
		if (myConfig.cCustomScripts[i] == NULL && myConfig.iPreferedSite[i] == 0)
			myConfig.iPreferedSite[i] = 1;
	}
	myConfig.cDropboxDir = CD_CONFIG_GET_STRING ("Configuration", "dropbox dir");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cIconAnimation);
	int i;
	for (i = 0; i < CD_NB_FILE_TYPES; i ++)
		g_free (myConfig.cCustomScripts[i]);
	g_free (myConfig.cDropboxDir);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pTask);  // stoppe la tache.
	
	g_free (myData.cCurrentFilePath);  // on libere la memoire partagee apres.
	g_strfreev (myData.cResultUrls);
	
	cd_dnd2share_clear_history ();
	
	g_free (myData.cLastURL);
	g_free (myData.cWorkingDirPath);
CD_APPLET_RESET_DATA_END
