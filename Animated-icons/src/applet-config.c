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
	int i,j;
	for (i = 0; i < CD_ANIMATIONS_NB_EFFECTS; i ++)
	{
		myConfig.iEffectsOnMouseOver[i] = -1;
	}
	for (j = 0; j < CAIRO_DOCK_NB_TYPES; j += 2)
	{
		for (i = 0; i < CD_ANIMATIONS_NB_EFFECTS; i ++)
		{
			myConfig.iEffectsOnClick[j][i] = -1;
		}
	}
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "hover effects", CD_ANIMATIONS_NB_EFFECTS, (int *)myConfig.iEffectsOnMouseOver);
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "click launchers", CD_ANIMATIONS_NB_EFFECTS,
		(int *)myConfig.iEffectsOnClick[CAIRO_DOCK_LAUNCHER]);
	myConfig.iNbRoundsOnClick[CAIRO_DOCK_LAUNCHER] = CD_CONFIG_GET_INTEGER ("Global", "nb rounds launchers");
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "click applis", CD_ANIMATIONS_NB_EFFECTS,
		(int *)myConfig.iEffectsOnClick[CAIRO_DOCK_APPLI]);
	myConfig.iNbRoundsOnClick[CAIRO_DOCK_APPLI] = CD_CONFIG_GET_INTEGER ("Global", "nb rounds applis");
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "click applets", CD_ANIMATIONS_NB_EFFECTS,
		(int *)myConfig.iEffectsOnClick[CAIRO_DOCK_APPLET]);
	myConfig.iNbRoundsOnClick[CAIRO_DOCK_APPLET] = CD_CONFIG_GET_INTEGER ("Global", "nb rounds applets");
	
	
	myConfig.iRotationDuration = CD_CONFIG_GET_INTEGER ("Rotation", "duration");
	myConfig.bContinueRotation = CD_CONFIG_GET_BOOLEAN ("Rotation", "continue");
	myConfig.iMeshType = CD_CONFIG_GET_INTEGER ("Rotation", "mesh");
	gdouble pMeshColor[4];
	CD_CONFIG_GET_COLOR ("Rotation", "color", pMeshColor);
	for (i=0; i<4; i++)
		myConfig.pMeshColor[i] = pMeshColor[i];
	
	//myConfig.bContinueWobbly = CD_CONFIG_GET_BOOLEAN ("Wobbly", "continue");
	myConfig.iInitialStrecth = CD_CONFIG_GET_INTEGER ("Wobbly", "stretch");
	myConfig.fSpringConstant = CD_CONFIG_GET_DOUBLE ("Wobbly", "spring cst");
	myConfig.fFriction = CD_CONFIG_GET_DOUBLE ("Wobbly", "friction");
	myConfig.iNbGridNodes = CD_CONFIG_GET_INTEGER ("Wobbly", "grid nodes");
	
	myConfig.iSpotDuration = CD_CONFIG_GET_INTEGER ("Spot", "duration");
	myConfig.bContinueSpot = CD_CONFIG_GET_BOOLEAN ("Spot", "continue");
	myConfig.cSpotImage = CD_CONFIG_GET_STRING ("Spot", "spot image");
	myConfig.cSpotFrontImage = CD_CONFIG_GET_STRING ("Spot", "spot imagefront ");
	gdouble pColor[4];
	CD_CONFIG_GET_COLOR_RVB ("Spot", "spot color", pColor);
	for (i=0; i<3; i++)
		myConfig.pSpotColor[i] = pColor[i];
	CD_CONFIG_GET_COLOR ("Spot", "halo color", pColor);
	for (i=0; i<4; i++)
		myConfig.pHaloColor[i] = pColor[i];
	CD_CONFIG_GET_COLOR_RVB ("Spot", "color1", pColor);
	for (i=0; i<3; i++)
		myConfig.pRaysColor1[i] = pColor[i];
	CD_CONFIG_GET_COLOR_RVB ("Spot", "color2", pColor);
	for (i=0; i<3; i++)
		myConfig.pRaysColor2[i] = pColor[i];
	myConfig.bMysticalRays = CD_CONFIG_GET_BOOLEAN ("Spot", "mystical");
	myConfig.iNbRaysParticles = CD_CONFIG_GET_INTEGER ("Spot", "nb part");
	myConfig.iRaysParticleSize = CD_CONFIG_GET_INTEGER ("Spot", "part size");
	myConfig.fRaysParticleSpeed = CD_CONFIG_GET_DOUBLE ("Spot", "part speed");
	
	myConfig.iWaveDuration = CD_CONFIG_GET_INTEGER ("Wave", "duration");
	myConfig.bContinueWave = CD_CONFIG_GET_BOOLEAN ("Wave", "continue");
	myConfig.fWaveWidth = CD_CONFIG_GET_DOUBLE ("Wave", "width");
	myConfig.fWaveAmplitude = CD_CONFIG_GET_DOUBLE ("Wave", "amplitude");
	
	myConfig.iPulseDuration = CD_CONFIG_GET_INTEGER ("Pulse", "duration");
	myConfig.bContinuePulse = CD_CONFIG_GET_BOOLEAN ("Pulse", "continue");
	myConfig.fPulseZoom = CD_CONFIG_GET_DOUBLE ("Pulse", "zoom");
	myConfig.bPulseSameShape = CD_CONFIG_GET_BOOLEAN ("Pulse", "same shape");
	
	myConfig.iBounceDuration = CD_CONFIG_GET_INTEGER ("Bounce", "duration");
	myConfig.bContinueBounce = CD_CONFIG_GET_BOOLEAN ("Bounce", "continue");
	myConfig.fBounceResize = CD_CONFIG_GET_DOUBLE ("Bounce", "resize");
	myConfig.fBounceFlatten = CD_CONFIG_GET_DOUBLE ("Bounce", "flatten");
	
	myConfig.iBlinkDuration = CD_CONFIG_GET_INTEGER ("Blink", "duration");
	myConfig.bContinueBlink = CD_CONFIG_GET_BOOLEAN ("Blink", "continue");
	
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cSpotImage);
	g_free (myConfig.cSpotFrontImage);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.iChromeTexture != 0)
		glDeleteTextures (1, &myData.iChromeTexture);
	if (myData.iSpotTexture != 0)
		glDeleteTextures (1, &myData.iSpotTexture);
	if (myData.iHaloTexture != 0)
		glDeleteTextures (1, &myData.iHaloTexture);
	if (myData.iSpotFrontTexture != 0)
		glDeleteTextures (1, &myData.iSpotFrontTexture);
	if (myData.iRaysTexture != 0)
		glDeleteTextures (1, &myData.iRaysTexture);
	
	if (myData.iCallList[CD_SQUARE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_SQUARE_MESH], 1);
	if (myData.iCallList[CD_CUBE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_CUBE_MESH], 1);
	if (myData.iCallList[CD_CAPSULE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_CAPSULE_MESH], 1);
CD_APPLET_RESET_DATA_END
