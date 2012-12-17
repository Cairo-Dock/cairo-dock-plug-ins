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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-notifications.h"

#define _XEYES_SENSITIVITY .5


CD_APPLET_ON_UPDATE_ICON_BEGIN
	//\_________________ On chope les coordonnees du curseur par rapport a notre container.
 	gldi_container_update_mouse_position (myContainer);
	
	//\_________________ On calcule les nouvelles coordonnees.
	gboolean bNeedsUpdate = FALSE;
	double fScale = myIcon->fScale / cairo_dock_get_icon_max_scale (myIcon) * myContainer->fRatio;
	int dx, dy;
	double tana, cosa, sina;
	int i;
	for (i = 0; i < 2; i ++)
	{
		if (myContainer->bIsHorizontal)
		{
			dx = myContainer->iMouseX - (myIcon->fDrawX + myData.iXeyes[i] * fScale);
			dy = myContainer->iMouseY - (myIcon->fDrawY + myData.iYeyes[i] * fScale);
		}
		else
		{
			dx = myContainer->iMouseY - (myIcon->fDrawY + myData.iXeyes[i] * fScale);
			dy = myContainer->iMouseX - (myIcon->fDrawX + myData.iYeyes[i] * fScale);
		}
		
		if (dx != 0)
		{
			tana = 1.*dy / dx;
			cosa = 1. / sqrt (1 + tana * tana);
			if (dx < 0)
				cosa = - cosa;
			sina = cosa * tana;
		}
		else
		{
			cosa = 0.;
			sina = (dy > 0 ? 1. : -1.);
		}
		
		if (fabs (dx) > fabs (.5*myData.iEyesWidth[i] * cosa))
			myData.fXpupil[i] = myData.iXeyes[i] + .5*myData.iEyesWidth[i] * cosa;
		else
			myData.fXpupil[i] = myData.iXeyes[i] + dx;
		if (fabs (dy) > fabs (.5*myData.iEyesHeight[i] * sina))
			myData.fYpupil[i] = myData.iYeyes[i] + .5*myData.iEyesHeight[i] * sina;
		else
			myData.fYpupil[i] = myData.iYeyes[i] + dy;
		
		if (fabs (myData.fXpupil[i] - myData.fPrevXpupil[i]) > _XEYES_SENSITIVITY || fabs (myData.fYpupil[i] - myData.fPrevYpupil[i]) > _XEYES_SENSITIVITY)
		{
			memcpy (&myData.fPrevXpupil[i], &myData.fXpupil[i], 2 * sizeof (double));
			bNeedsUpdate = TRUE;
		}
	}
	
	//\_________________ On gere le clignement des yeux.
	int iDetlaT = (myConfig.bFastCheck ? cairo_dock_get_animation_delta_t (myContainer) : cairo_dock_get_slow_animation_delta_t (myContainer));
	myData.iTimeCount += iDetlaT;
	if (myData.bWink)  // le clignement dure 150ms
	{
		if (myData.iTimeCount >= myConfig.iWinkDuration)
		{
			myData.iTimeCount = 0;
			myData.bWink = FALSE;
			bNeedsUpdate = TRUE;
		}
	}
	else
	{
		if (myData.iTimeCount >= 1000)  // toutes les secondes on regarde si on cligne.
		{
			myData.iTimeCount = 0;
			myData.bWink = (g_random_double () < 1. / myConfig.iWinkDelay);
			bNeedsUpdate |= myData.bWink;
		}
	}
	
	//\_________________ On redessine si neccessaire.
	if (! bNeedsUpdate)
		CD_APPLET_SKIP_UPDATE_ICON;
	
	// taille du dessin.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	// render to surface/texture.
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		cd_xeyes_render_to_texture (myApplet, iWidth, iHeight);
	else
		cd_xeyes_render_to_surface (myApplet, iWidth, iHeight);
CD_APPLET_ON_UPDATE_ICON_END
