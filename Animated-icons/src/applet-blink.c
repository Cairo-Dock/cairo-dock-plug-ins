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
#include "applet-notifications.h"
#include "applet-bounce.h"


static void init (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	//g_print ("INIT blink\n");
	pData->iBlinkCount = myConfig.iBlinkDuration / dt - 1;
}

static gboolean update (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bRepeat)
{
	//g_print ("UPDATE blink\n");
	int c = pData->iBlinkCount;
	int n = (int) floor (myConfig.iBlinkDuration / dt) / 2;  // nbre d'iteration pour une inversion d'alpha.
	if ( (c/n) & 1)
		pData->fBlinkAlpha = 1. * (c%n) / n;
	else
		pData->fBlinkAlpha = 1. * (n - 1 - (c%n)) / n;
	pData->fBlinkAlpha *= pData->fBlinkAlpha;  // pour accentuer.
	if (pData->fBlinkAlpha < .01)
		pData->fBlinkAlpha = .01;
	
	pData->iBlinkCount --;
	
	cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (pDock));
	
	gboolean bContinue = (pData->iBlinkCount > 0);
	if (! bContinue && bRepeat)
		init (pIcon, pDock, pData, dt, bUseOpenGL);
	
	//g_print (" -> %d\n", bContinue);
	return bContinue;
}

static void render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	//g_print ("RENDER blink\n");
	pIcon->fAlpha *= pData->fBlinkAlpha;
}

static void post_render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	//g_print ("POST RENDER blink\n");
	pIcon->fAlpha /= pData->fBlinkAlpha;
}


void cd_animations_register_blink (void)
{
	CDAnimation *pAnimation = &myData.pAnimations[CD_ANIMATIONS_BLINK];
	pAnimation->cName = "blink";
	pAnimation->cDisplayedName = D_("Blink");
	pAnimation->id = CD_ANIMATIONS_BLINK;
	pAnimation->bDrawIcon = FALSE;
	pAnimation->bDrawReflect = FALSE;
	pAnimation->init = init;
	pAnimation->update = update;
	pAnimation->render = render;
	pAnimation->post_render = post_render;
	cd_animations_register_animation (pAnimation);
}
