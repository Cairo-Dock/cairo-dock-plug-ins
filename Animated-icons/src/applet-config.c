/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
	for (j = 0; j < CAIRO_DOCK_NB_GROUPS; j += 2)
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
	myConfig.bOpeningAnimation = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Global", "opening animation", TRUE);
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "click applis", CD_ANIMATIONS_NB_EFFECTS,
		(int *)myConfig.iEffectsOnClick[CAIRO_DOCK_APPLI]);
	myConfig.iNbRoundsOnClick[CAIRO_DOCK_APPLI] = CD_CONFIG_GET_INTEGER ("Global", "nb rounds applis");
	
	myConfig.iRotationDuration = CD_CONFIG_GET_INTEGER ("Rotation", "duration");
	myConfig.bContinue[CD_ANIMATIONS_ROTATE] = CD_CONFIG_GET_BOOLEAN ("Rotation", "continue");
	myConfig.iMeshType = CD_CONFIG_GET_INTEGER ("Rotation", "mesh");
	gdouble pMeshColor[4];
	CD_CONFIG_GET_COLOR ("Rotation", "color", pMeshColor);
	for (i=0; i<4; i++)
		myConfig.pMeshColor[i] = pMeshColor[i];
	
	myConfig.bContinue[CD_ANIMATIONS_WOBBLY] = FALSE;
	myConfig.iInitialStrecth = CD_CONFIG_GET_INTEGER ("Wobbly", "stretch");
	myConfig.fSpringConstant = CD_CONFIG_GET_DOUBLE ("Wobbly", "spring cst");
	myConfig.fFriction = CD_CONFIG_GET_DOUBLE ("Wobbly", "friction");
	myConfig.iNbGridNodes = CD_CONFIG_GET_INTEGER ("Wobbly", "grid nodes");
	
	myConfig.iSpotDuration = CD_CONFIG_GET_INTEGER ("Spot", "duration");
	myConfig.bContinue[CD_ANIMATIONS_SPOT] = CD_CONFIG_GET_BOOLEAN ("Spot", "continue");
	myConfig.cSpotImage = CD_CONFIG_GET_STRING ("Spot", "spot image");
	myConfig.cSpotFrontImage = CD_CONFIG_GET_STRING ("Spot", "spot front image");
	gdouble pColor[4];
	double col[4] = {1.,1.,1.,1.};
	CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT ("Spot", "spot-color", pColor, col);
	for (i=0; i<3; i++)
		myConfig.pSpotColor[i] = pColor[i];
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Spot", "halo-color", pColor, col);
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
	myConfig.bContinue[CD_ANIMATIONS_WAVE] = CD_CONFIG_GET_BOOLEAN ("Wave", "continue");
	myConfig.fWaveWidth = CD_CONFIG_GET_DOUBLE ("Wave", "width");
	myConfig.fWaveAmplitude = CD_CONFIG_GET_DOUBLE ("Wave", "amplitude");
	
	myConfig.iPulseDuration = CD_CONFIG_GET_INTEGER ("Pulse", "duration");
	myConfig.bContinue[CD_ANIMATIONS_PULSE] = CD_CONFIG_GET_BOOLEAN ("Pulse", "continue");
	myConfig.fPulseZoom = CD_CONFIG_GET_DOUBLE ("Pulse", "zoom");
	myConfig.bPulseSameShape = CD_CONFIG_GET_BOOLEAN ("Pulse", "same shape");
	
	myConfig.iBounceDuration = CD_CONFIG_GET_INTEGER ("Bounce", "duration");
	myConfig.bContinue[CD_ANIMATIONS_BOUNCE] = CD_CONFIG_GET_BOOLEAN ("Bounce", "continue");
	myConfig.fBounceResize = CD_CONFIG_GET_DOUBLE ("Bounce", "resize");
	myConfig.fBounceFlatten = CD_CONFIG_GET_DOUBLE ("Bounce", "flatten");
	
	myConfig.iBlinkDuration = CD_CONFIG_GET_INTEGER ("Blink", "duration");
	myConfig.bContinue[CD_ANIMATIONS_BLINK] = CD_CONFIG_GET_BOOLEAN ("Blink", "continue");
	
	myConfig.iBusyDuration = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Busy", "duration", 800);
	myConfig.bContinue[CD_ANIMATIONS_BUSY] = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Busy", "continue", TRUE);
	myConfig.cBusyImage = CD_CONFIG_GET_STRING ("Busy", "image");
	myConfig.fBusySize = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Busy", "size", .5);  // relatively to the size of the icon
	
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cSpotImage);
	g_free (myConfig.cSpotFrontImage);
	g_free (myConfig.cBusyImage);
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
