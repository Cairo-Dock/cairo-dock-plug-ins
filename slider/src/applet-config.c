/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cDirectory 	= CD_CONFIG_GET_STRING("Configuration", "directory");
	myConfig.dSlideTime 	= 1000 * CD_CONFIG_GET_DOUBLE ("Configuration", "slide time");
	myConfig.bSubDirs 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "sub directories");
	myConfig.bNoStrench 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "no strench");
	myConfig.pAnimation 	= CD_CONFIG_GET_INTEGER ("Configuration", "change animation");
	myConfig.bRandom 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "random");
	CD_CONFIG_GET_COLOR ("Configuration", "background color", myConfig.pBackgroundColor);
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before yo get the applet's config, and when your applet is stopped.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free(myConfig.cDirectory);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped.
CD_APPLET_RESET_DATA_BEGIN
	myData.pElement = NULL;
	if (myData.pList != NULL) {
		g_list_foreach (myData.pList, (GFunc) g_free, NULL);
  	g_list_free (myData.pList);
  	myData.pList = NULL;
  }
	
CD_APPLET_RESET_DATA_END
