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
	myConfig.iPreferedSite = CD_CONFIG_GET_INTEGER ("Configuration", "prefered site");
	myConfig.cIconAnimation = CD_CONFIG_GET_STRING ("Configuration", "animation");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);  // stoppe le thread.
	
	g_free (myData.cCurrentFilePath);  // on libere la memoire partagee apres.
	g_strfreev (myData.cResultUrls);
	
	cd_dnd2share_clear_history ();
	
	g_free (myData.cLastURL);
	g_free (myData.cWorkingDirPath);
CD_APPLET_RESET_DATA_END
