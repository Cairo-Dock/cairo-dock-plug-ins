/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS



static gchar * _penguin_get_animation_properties (GKeyFile *pKeyFile, gchar *cAnimationName, PenguinAnimation *pAnimation)
{
	if (! g_key_file_has_group (pKeyFile, cAnimationName))
		return NULL;
	cd_debug ("%s (%s)", __func__, cAnimationName);
	
	gchar *cFileName = g_key_file_get_string (pKeyFile, cAnimationName, "file", NULL);
	if (cFileName != NULL && *cFileName == '\0')
	{
		g_free (cFileName);
		cFileName = NULL;
	}
	
	GError *erreur = NULL;
	pAnimation->iNbDirections = g_key_file_get_integer (pKeyFile, cAnimationName, "nb directions", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iNbDirections = myData.defaultAnimation.iNbDirections;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iNbFrames = g_key_file_get_integer (pKeyFile, cAnimationName, "nb frames", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iNbFrames = myData.defaultAnimation.iNbFrames;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iSpeed = g_key_file_get_integer (pKeyFile, cAnimationName, "speed", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iSpeed = myData.defaultAnimation.iSpeed;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iAcceleration = g_key_file_get_integer (pKeyFile, cAnimationName, "acceleration", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iAcceleration = myData.defaultAnimation.iAcceleration;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iTerminalVelocity = g_key_file_get_integer (pKeyFile, cAnimationName, "terminal velocity", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iTerminalVelocity = myData.defaultAnimation.iTerminalVelocity;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->bEnding = g_key_file_get_boolean (pKeyFile, cAnimationName, "ending", &erreur);
	if (erreur != NULL)
	{
		pAnimation->bEnding = myData.defaultAnimation.bEnding;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iDirection = g_key_file_get_integer (pKeyFile, cAnimationName, "direction", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iDirection = myData.defaultAnimation.iDirection;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	return cFileName;
}


void penguin_load_theme (gchar *cThemePath)
{
	g_return_if_fail (cThemePath != NULL);
	cd_debug ("%s (%s)", __func__, cThemePath);
	
	//\___________________ On ouvre le fichier de conf.
	gchar *cConfFilePath = g_strconcat (cThemePath, "/theme.conf", NULL);
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	myData.fFrameDelay = (double) g_key_file_get_integer (pKeyFile, "Theme", "delay", &erreur) * 1e-3;
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		myData.fFrameDelay = .1;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	_penguin_get_animation_properties (pKeyFile, "Default", &myData.defaultAnimation);
	
	gsize length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	
	g_free (myData.pAnimations);
	myData.iNbAnimations = 0;
	myData.pAnimations = g_new0 (PenguinAnimation, length - 1);
	
	g_free (myData.pBeginningAnimations);
	myData.iNbBeginningAnimations = 0;
	myData.pBeginningAnimations = g_new0 (int, length - 1);
	
	g_free (myData.pEndingAnimations);
	myData.iNbEndingAnimations = 0;
	myData.pEndingAnimations = g_new0 (int, length - 1);
	
	g_free (myData.pGoUpAnimations);
	myData.iNbGoUpAnimations = 0;
	myData.pGoUpAnimations = g_new0 (int, length - 1);
	
	g_free (myData.pMovmentAnimations);
	myData.iNbMovmentAnimations = 0;
	myData.pMovmentAnimations = g_new0 (int, length - 1);
	
	g_free (myData.pRestAnimations);
	myData.iNbRestAnimations = 0;
	myData.pRestAnimations = g_new0 (int, length - 1);
	
	PenguinAnimation *pAnimation;
	gchar *cFileName, *cGroupName;
	int i, iNumAnimation = 0;
	for (i = 0; pGroupList[i] != NULL; i++)
	{
		cGroupName = pGroupList[i];
		if (strcmp (cGroupName, "Theme") != 0 && strcmp (cGroupName, "Default") != 0)
		{
			cd_debug ("%d)", iNumAnimation);
			pAnimation = &myData.pAnimations[iNumAnimation];
			
			cFileName = _penguin_get_animation_properties (pKeyFile, cGroupName, pAnimation);
			if (cFileName != NULL)
			{
				pAnimation->cFilePath = g_strconcat (cThemePath, "/", cFileName, NULL);
				g_free (cFileName);
			}
			if (pAnimation->bEnding)
			{
				myData.pEndingAnimations[myData.iNbEndingAnimations++] = iNumAnimation;
				cd_debug (" %s : ending", pAnimation->cFilePath);
			}
			else if (pAnimation->iDirection == PENGUIN_DOWN)  // descente.
			{
				myData.pBeginningAnimations[myData.iNbBeginningAnimations++] = iNumAnimation;
				cd_debug (" %s : beginning", pAnimation->cFilePath);
			}
			else if (pAnimation->iDirection == PENGUIN_UP)
			{
				myData.pGoUpAnimations[myData.iNbGoUpAnimations++] = iNumAnimation;
				cd_debug (" %s : go up", pAnimation->cFilePath);
			}
			else if (pAnimation->iSpeed == 0 && pAnimation->iAcceleration == 0 && pAnimation->iNbFrames == 1)
			{
				myData.pRestAnimations[myData.iNbRestAnimations++] = iNumAnimation;
				cd_debug (" %s : rest", pAnimation->cFilePath);
			}
			else
			{
				myData.pMovmentAnimations[myData.iNbMovmentAnimations++] = iNumAnimation;
				cd_debug (" %s : moving", pAnimation->cFilePath);
			}
			
			iNumAnimation ++;
		}
	}
	
	g_strfreev (pGroupList);
	g_free (cConfFilePath);
	g_key_file_free (pKeyFile);
}


void penguin_load_animation_buffer (PenguinAnimation *pAnimation, cairo_t *pSourceContext)
{
	cd_debug ("%s (%s)", __func__, pAnimation->cFilePath);
	if (pAnimation->cFilePath == NULL)
		return;
	
	double fImageWidth=0, fImageHeight=0;
	cairo_surface_t *pBigSurface = cairo_dock_load_image (
		pSourceContext,
		pAnimation->cFilePath,
		&fImageWidth,
		&fImageHeight,
		0.,
		myConfig.fAlpha,
		FALSE);
	pAnimation->iFrameWidth = (int) fImageWidth / pAnimation->iNbFrames, pAnimation->iFrameHeight = (int) fImageHeight / pAnimation->iNbDirections;
	if (pBigSurface != NULL)
	{
		cd_debug ("  surface chargee (%dx%d)", pAnimation->iFrameWidth, pAnimation->iFrameHeight);
		pAnimation->pSurfaces = g_new (cairo_surface_t **, pAnimation->iNbDirections);
		int i, j;
		for (i = 0; i < pAnimation->iNbDirections; i ++)
		{
			pAnimation->pSurfaces[i] = g_new (cairo_surface_t *, pAnimation->iNbFrames);
			for (j = 0; j < pAnimation->iNbFrames; j ++)
			{
				//cd_debug ("    dir %d, frame %d)", i, j);
				pAnimation->pSurfaces[i][j] = cairo_surface_create_similar (cairo_get_target (pSourceContext),
					CAIRO_CONTENT_COLOR_ALPHA,
					pAnimation->iFrameWidth,
					pAnimation->iFrameHeight);
				cairo_t *pCairoContext = cairo_create (pAnimation->pSurfaces[i][j]);
				
				cairo_set_source_surface (pCairoContext,
					pBigSurface,
					- j * pAnimation->iFrameWidth,
					- i * pAnimation->iFrameHeight);
				cairo_paint (pCairoContext);
				
				cairo_destroy (pCairoContext);
			}
		}
	}
}
