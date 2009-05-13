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
	myConfig.dTimeDialogs = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 5000);
	myConfig.bEnableHistory = CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_history");
	myConfig.bEnableHistoryLimit = CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_history_limit");
	myConfig.iNbItems = CD_CONFIG_GET_INTEGER ("Configuration", "nb_items");
	myConfig.iUrlPicturesType = CD_CONFIG_GET_INTEGER ("Configuration", "url_pictures_type");
	
	if (!myConfig.bEnableHistory)
	{
		myConfig.bEnableHistoryLimit = TRUE;
		myConfig.iNbItems = 1;
	}
		
	myData.cWorkingDirPath = g_strdup_printf ("%s/.config/cairo-dock/dnd2share",
					g_getenv ("HOME"));
	if (g_file_test (myData.cWorkingDirPath, G_FILE_TEST_EXISTS))
	{
		cd_debug ("DND2SHARE : Le dossier '%s' existe déjà", myData.cWorkingDirPath);
	}
	else  
	{
		cd_debug ("DND2SHARE : le dossier '%s' n'existe pas encore -> On le crée", myData.cWorkingDirPath);
		gchar *cCommandMkdir = g_strdup_printf ("mkdir %s", myData.cWorkingDirPath);
		g_spawn_command_line_async (cCommandMkdir, NULL);
		g_free (cCommandMkdir);
	}
	
	cd_dnd2share_check_number_of_stored_pictures ();
		
	if (myData.iNumberOfStoredPic != 0)
	{
		myData.iCurrentPictureNumber = myData.iNumberOfStoredPic;
		cd_debug("DND2SHARE : Je vais récupérer les info dans %i.conf", myData.iCurrentPictureNumber);
		cd_dnd2share_get_urls_from_stored_file ();
	}
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	
	
CD_APPLET_RESET_DATA_END
