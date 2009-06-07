/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>

#include "applet-struct.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "Classic");
	
	myConfig.iDelayBetweenChanges = MAX (2, CD_CONFIG_GET_INTEGER ("Configuration", "change delay"));
	
	myConfig.fAlpha = CD_CONFIG_GET_DOUBLE ("Configuration", "alpha");
	
	myConfig.bFree = CD_CONFIG_GET_BOOLEAN ("Configuration", "free");
	
	myConfig.iGroundOffset = CD_CONFIG_GET_INTEGER ("Configuration", "ground");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cThemePath);
CD_APPLET_RESET_CONFIG_END


static void _penguin_reset_one_animation (PenguinAnimation *pAnimation)
{
	if (pAnimation->pSurfaces != NULL)
	{
		int i, j;
		for (i = 0; i < pAnimation->iNbDirections; i ++)
		{
			for (j = 0; j < pAnimation->iNbFrames; j ++)
			{
				cairo_surface_destroy (pAnimation->pSurfaces[i][j]);
			}
			g_free (pAnimation->pSurfaces[i]);
			pAnimation->pSurfaces[i] = NULL;
		}
		g_free (pAnimation->pSurfaces);
		pAnimation->pSurfaces = NULL;
	}
	if (pAnimation->iTexture != 0)
	{
		_cairo_dock_delete_texture (pAnimation->iTexture);
		pAnimation->iTexture = 0;
	}
}

CD_APPLET_RESET_DATA_BEGIN
	int i;
	for (i = 0; i < myData.iNbAnimations; i++)
	{
		_penguin_reset_one_animation (&myData.pAnimations[i]);
	}
	
	g_free (myData.pAnimations);
	g_free (myData.pBeginningAnimations);
	g_free (myData.pEndingAnimations);
	g_free (myData.pGoUpAnimations);
	g_free (myData.pMovmentAnimations);
	g_free (myData.pRestAnimations);
CD_APPLET_RESET_DATA_END
