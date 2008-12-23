/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


#define _get_particle_system_config(cGroupName, parameters) \
	parameters.iDuration = CD_CONFIG_GET_INTEGER (cGroupName, "duration");\
	parameters.bContinue = CD_CONFIG_GET_BOOLEAN (cGroupName, "continue");\
	CD_CONFIG_GET_COLOR_RVB (cGroupName, "color1", parameters.pColor1);\
	CD_CONFIG_GET_COLOR_RVB (cGroupName, "color2", parameters.pColor2);\
	parameters.bMystical = CD_CONFIG_GET_BOOLEAN (cGroupName, "mystical");\
	parameters.iNbParticles = CD_CONFIG_GET_INTEGER (cGroupName, "nb part");\
	parameters.iParticleSize = CD_CONFIG_GET_INTEGER (cGroupName, "part size");\
	parameters.fParticleSpeed = CD_CONFIG_GET_DOUBLE (cGroupName, "part speed")


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bBackGround = CD_CONFIG_GET_BOOLEAN ("Global", "background");
	myConfig.bRotateEffects = CD_CONFIG_GET_BOOLEAN ("Global", "rotate");
	int i,j;
	for (i = 0; i < CD_ICON_EFFECT_NB_EFFECTS; i ++)
	{
		myConfig.iEffectsUsed[i] = -1;
	}
	for (j = 0; j < CAIRO_DOCK_NB_TYPES; j += 2)
	{
		for (i = 0; i < CD_ICON_EFFECT_NB_EFFECTS; i ++)
		{
			myConfig.iEffectsOnClick[j][i] = -1;
		}
	}
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "effects", CD_ICON_EFFECT_NB_EFFECTS, (int *)myConfig.iEffectsUsed);
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "click launchers", CD_ICON_EFFECT_NB_EFFECTS,
		(int *)myConfig.iEffectsOnClick[CAIRO_DOCK_LAUNCHER]);
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "click applis", CD_ICON_EFFECT_NB_EFFECTS,
		(int *)myConfig.iEffectsOnClick[CAIRO_DOCK_APPLI]);
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "click applets", CD_ICON_EFFECT_NB_EFFECTS,
		(int *)myConfig.iEffectsOnClick[CAIRO_DOCK_APPLET]);
	
	myConfig.iFireDuration = CD_CONFIG_GET_INTEGER ("Fire", "duration");
	myConfig.bContinueFire = CD_CONFIG_GET_BOOLEAN ("Fire", "continue");
	CD_CONFIG_GET_COLOR_RVB ("Fire", "color1", myConfig.pFireColor1);
	CD_CONFIG_GET_COLOR_RVB ("Fire", "color2", myConfig.pFireColor2);
	myConfig.bMysticalFire = CD_CONFIG_GET_BOOLEAN ("Fire", "mystical");
	myConfig.iNbFireParticles = CD_CONFIG_GET_INTEGER ("Fire", "nb part");
	myConfig.iFireParticleSize = CD_CONFIG_GET_INTEGER ("Fire", "part size");
	myConfig.fFireParticleSpeed = CD_CONFIG_GET_DOUBLE ("Fire", "part speed");
	
	myConfig.iRainDuration = CD_CONFIG_GET_INTEGER ("Rain", "duration");
	myConfig.bContinueRain = CD_CONFIG_GET_BOOLEAN ("Rain", "continue");
	CD_CONFIG_GET_COLOR_RVB ("Rain", "color1", myConfig.pRainColor1);
	CD_CONFIG_GET_COLOR_RVB ("Rain", "color2", myConfig.pRainColor2);
	myConfig.iNbRainParticles = CD_CONFIG_GET_INTEGER ("Rain", "nb part");
	myConfig.iRainParticleSize = CD_CONFIG_GET_INTEGER ("Rain", "part size") / 2;  // cette texture est pleine alors que les 2 precedentes sont floutees, donc pour conserver un ordre de grandeur identique on divise par 2.
	myConfig.fRainParticleSpeed = CD_CONFIG_GET_DOUBLE ("Rain", "part speed");
	
	myConfig.iRainDuration = CD_CONFIG_GET_INTEGER ("Rain", "duration");
	myConfig.bContinueRain = CD_CONFIG_GET_BOOLEAN ("Rain", "continue");
	CD_CONFIG_GET_COLOR_RVB ("Rain", "color1", myConfig.pRainColor1);
	CD_CONFIG_GET_COLOR_RVB ("Rain", "color2", myConfig.pRainColor2);
	myConfig.iNbRainParticles = CD_CONFIG_GET_INTEGER ("Rain", "nb part");
	myConfig.iRainParticleSize = CD_CONFIG_GET_INTEGER ("Rain", "part size") / 2;  // cette texture est pleine alors que les 2 precedentes sont floutees, donc pour conserver un ordre de grandeur identique on divise par 2.
	myConfig.fRainParticleSpeed = CD_CONFIG_GET_DOUBLE ("Rain", "part speed");
	
	myConfig.iSnowDuration = CD_CONFIG_GET_INTEGER ("Snow", "duration");
	myConfig.bContinueSnow = CD_CONFIG_GET_BOOLEAN ("Snow", "continue");
	CD_CONFIG_GET_COLOR_RVB ("Snow", "color1", myConfig.pSnowColor1);
	CD_CONFIG_GET_COLOR_RVB ("Snow", "color2", myConfig.pSnowColor2);
	myConfig.iNbSnowParticles = CD_CONFIG_GET_INTEGER ("Snow", "nb part");
	myConfig.iSnowParticleSize = CD_CONFIG_GET_INTEGER ("Snow", "part size") / 2;  // meme remarque.
	myConfig.fSnowParticleSpeed = CD_CONFIG_GET_DOUBLE ("Snow", "part speed");
	
	myConfig.iStormDuration = CD_CONFIG_GET_INTEGER ("Storm", "duration");
	myConfig.bContinueStorm = CD_CONFIG_GET_BOOLEAN ("Storm", "continue");
	CD_CONFIG_GET_COLOR_RVB ("Storm", "color1", myConfig.pStormColor1);
	CD_CONFIG_GET_COLOR_RVB ("Storm", "color2", myConfig.pStormColor2);
	myConfig.iNbStormParticles = CD_CONFIG_GET_INTEGER ("Storm", "nb part");
	myConfig.iStormParticleSize = CD_CONFIG_GET_INTEGER ("Storm", "part size");  // meme remarque.
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.iFireTexture != 0)
		glDeleteTextures (1, &myData.iFireTexture);
	if (myData.iRainTexture != 0)
		glDeleteTextures (1, &myData.iRainTexture);
	if (myData.iSnowTexture != 0)
		glDeleteTextures (1, &myData.iSnowTexture);
	if (myData.iStarTexture != 0)
		glDeleteTextures (1, &myData.iStarTexture);
	
CD_APPLET_RESET_DATA_END
