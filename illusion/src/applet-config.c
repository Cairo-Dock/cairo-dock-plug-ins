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
#include <math.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iDisappearanceEffect = CD_CONFIG_GET_INTEGER ("Global", "disappearance");
	myConfig.iAppearanceEffect = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Global", "appearance", CD_ILLUSION_BLACK_HOLE);
	
	myConfig.iEvaporateDuration = MAX (100, CD_CONFIG_GET_INTEGER ("Evaporate", "duration"));
	CD_CONFIG_GET_COLOR_RVB ("Evaporate", "color1", myConfig.pEvaporateColor1);
	CD_CONFIG_GET_COLOR_RVB ("Evaporate", "color2", myConfig.pEvaporateColor2);
	myConfig.bMysticalEvaporate = CD_CONFIG_GET_BOOLEAN ("Evaporate", "mystical");
	myConfig.iNbEvaporateParticles = CD_CONFIG_GET_INTEGER ("Evaporate", "nb part");
	myConfig.iEvaporateParticleSize = CD_CONFIG_GET_INTEGER ("Evaporate", "part size");
	myConfig.fEvaporateParticleSpeed = CD_CONFIG_GET_DOUBLE ("Evaporate", "part speed");
	myConfig.bEvaporateFromBottom = CD_CONFIG_GET_BOOLEAN ("Evaporate", "from bottom");
	
	myConfig.iFadeOutDuration = MAX (100, CD_CONFIG_GET_INTEGER ("Fade out", "duration"));
	
	myConfig.iExplodeDuration = MAX (100, CD_CONFIG_GET_INTEGER ("Explode", "duration"));
	int iExplodeNbPieces = CD_CONFIG_GET_INTEGER ("Explode", "nb pieces");
	myConfig.iExplodeNbPiecesX = sqrt (iExplodeNbPieces);
	myConfig.iExplodeNbPiecesY = iExplodeNbPieces / myConfig.iExplodeNbPiecesX;
	myConfig.fExplosionRadius = CD_CONFIG_GET_DOUBLE ("Explode", "radius");
	myConfig.bExplodeCube = CD_CONFIG_GET_BOOLEAN ("Explode", "cubes");
	
	myConfig.iBreakDuration = MAX (100, CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Break", "duration", 600));
	int iBreakNbPieces = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Break", "nb pieces", 7);
	myConfig.iBreakNbBorderPoints = MAX (1, (iBreakNbPieces - 3) / 2);
	
	myConfig.iBlackHoleDuration = MAX (100, CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Black Hole", "duration", 2000));  // ms
	myConfig.fBlackHoleRotationSpeed = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Black Hole", "omega", 1.5);  // tr/s
	myConfig.iAttraction = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Black Hole", "attraction", 4);
	
	myConfig.iLightningDuration = MAX (100, CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Lightning", "duration", 3000));  // ms
	myConfig.iLightningNbSources = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Lightning", "nb sources", 3);
	myConfig.iLightningNbCtrlPts = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Lightning", "nb ctrl", 10);
	CD_CONFIG_GET_COLOR_RVB ("Lightning", "color", myConfig.fLightningColor);
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.iEvaporateTexture != 0)
		glDeleteTextures (1, &myData.iEvaporateTexture);
	
CD_APPLET_RESET_DATA_END
