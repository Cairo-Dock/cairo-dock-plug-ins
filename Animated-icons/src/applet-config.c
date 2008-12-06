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
	gboolean bUse;
	
	bUse = CD_CONFIG_GET_BOOLEAN ("Rotation", "use");
	if (bUse)
	{
		myConfig.iRotationDuration = CD_CONFIG_GET_INTEGER ("Rotation", "duration");
		myConfig.bContinueRotation = CD_CONFIG_GET_BOOLEAN ("Rotation", "continue");
		myConfig.iMeshType = CD_CONFIG_GET_INTEGER ("Rotation", "mesh");
		gdouble pMeshColor[4];
		CD_CONFIG_GET_COLOR ("Rotation", "color", pMeshColor);
		int i;
		for (i=0; i<4; i++)
			myConfig.pMeshColor[i] = pMeshColor[i];
	}
	
	bUse = CD_CONFIG_GET_BOOLEAN ("Spot", "use");
	if (bUse)
	{
		myConfig.iSpotDuration = CD_CONFIG_GET_INTEGER ("Spot", "duration");
		myConfig.bContinueSpot = CD_CONFIG_GET_BOOLEAN ("Spot", "continue");
		int i;
		gdouble pColor[4];
		CD_CONFIG_GET_COLOR_RVB ("Spot", "spot color", pColor);
		for (i=0; i<3; i++)
			myConfig.pSpotColor[i] = pColor[i];
		CD_CONFIG_GET_COLOR ("Spot", "halo color", pColor);
		for (i=0; i<4; i++)
			myConfig.pHaloColor[i] = pColor[i];
		
		
		CD_CONFIG_GET_COLOR_RVB ("Spot", "color1", myConfig.pRaysColor1);
		//for (i=0; i<3; i++)
		//	myConfig.pRaysColor1[i] = pColor[i];
		CD_CONFIG_GET_COLOR_RVB ("Spot", "color2", myConfig.pRaysColor2);
		//for (i=0; i<3; i++)
		//	myConfig.pRaysColor2[i] = pColor[i];
		myConfig.bMysticalRays = CD_CONFIG_GET_BOOLEAN ("Spot", "mystical");
		myConfig.iNbRaysParticles = CD_CONFIG_GET_INTEGER ("Spot", "nb part");
		myConfig.iRaysParticleSize = CD_CONFIG_GET_INTEGER ("Spot", "part size");
		myConfig.fRaysParticleSpeed = CD_CONFIG_GET_DOUBLE ("Spot", "part speed");
	}
	
	bUse = CD_CONFIG_GET_BOOLEAN ("Wobbly", "use");
	if (bUse)
	{
		myConfig.iInitialStrecth = CD_CONFIG_GET_INTEGER ("Wobbly", "stretch");
		myConfig.fSpringConstant = CD_CONFIG_GET_DOUBLE ("Wobbly", "spring cst");
		myConfig.fFriction = CD_CONFIG_GET_DOUBLE ("Wobbly", "friction");
		myConfig.iNbGridNodes = CD_CONFIG_GET_INTEGER ("Wobbly", "grid nodes");
	}
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.iChromeTexture != 0)
		glDeleteTextures (1, &myData.iChromeTexture);
	
	if (myData.iCallList[CD_SQUARE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_SQUARE_MESH], 1);
	if (myData.iCallList[CD_CUBE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_CUBE_MESH], 1);
	if (myData.iCallList[CD_CAPSULE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_CAPSULE_MESH], 1);
	
CD_APPLET_RESET_DATA_END
