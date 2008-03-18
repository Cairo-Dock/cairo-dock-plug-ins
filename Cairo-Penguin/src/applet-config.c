/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "Classic");
	
	myConfig.iDelayBetweenChanges = MAX (2, CD_CONFIG_GET_INTEGER ("Configuration", "change delay"));
	
	myConfig.fAlpha = CD_CONFIG_GET_DOUBLE ("Configuration", "alpha");
	
	myConfig.bFree = CD_CONFIG_GET_BOOLEAN ("Configuration", "free");
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.cThemePath);
	myConfig.cThemePath = NULL;
	memset (&myConfig, 0, sizeof (AppletConfig));
}



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
}

void reset_data (void)
{
	int i;
	for (i = 0; i < myData.iNbAnimations; i++)
	{
		_penguin_reset_one_animation (&myData.pAnimations[i]);
	}
	
	g_free (myData.pAnimations);
	myData.pAnimations = NULL;
	g_free (myData.pBeginningAnimations);
	myData.pBeginningAnimations = NULL;
	g_free (myData.pEndingAnimations);
	myData.pEndingAnimations = NULL;
	g_free (myData.pGoUpAnimations);
	myData.pGoUpAnimations = NULL;
	g_free (myData.pMovmentAnimations);
	myData.pMovmentAnimations = NULL;
	g_free (myData.pRestAnimations);
	myData.pRestAnimations = NULL;
	
	memset (&myData, 0, sizeof (AppletData));
}
