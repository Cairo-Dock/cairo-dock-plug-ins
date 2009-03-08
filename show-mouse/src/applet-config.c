/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iParticleLifeTime = CD_CONFIG_GET_INTEGER ("Configuration", "life");
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "color1", myConfig.pColor1);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "color2", myConfig.pColor2);
	myConfig.iNbParticles = CD_CONFIG_GET_INTEGER ("Configuration", "nb part");
	myConfig.iParticleSize = CD_CONFIG_GET_INTEGER ("Configuration", "part size");
	myConfig.bMysticalFire = CD_CONFIG_GET_BOOLEAN ("Configuration", "mystical");
	
	myConfig.iNbSources = CD_CONFIG_GET_INTEGER ("Configuration", "nb sources");
	myConfig.fRotationSpeed = CD_CONFIG_GET_DOUBLE ("Configuration", "rotation speed");
	myConfig.fScattering = CD_CONFIG_GET_DOUBLE ("Configuration", "scattering");
	myConfig.iContainerType  = CD_CONFIG_GET_INTEGER ("Configuration", "container") + 1;
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.iTexture != 0)
		glDeleteTextures (1, &myData.iTexture);
CD_APPLET_RESET_DATA_END
