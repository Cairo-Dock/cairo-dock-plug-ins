/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-theme.h"

#define _print_error(erreur) \
	if (erreur != NULL) {\
		cd_warning (erreur->message);\
		g_error_free (erreur);\
		erreur = NULL; }

static gchar *group[2] = {"Left eye", "Right eye"};

gboolean cd_xeyes_load_theme (CairoDockModuleInstance *myApplet)
{
	GString *sPath = g_string_new ("");
	g_string_printf (sPath, "%s/theme.conf", myConfig.cThemePath);
	
	GKeyFile *pKeyFile = cairo_dock_open_key_file (sPath->str);
	g_return_val_if_fail (pKeyFile != NULL, FALSE);
	GError *erreur = NULL;
	
	gboolean bEyeVisible[2] = {FALSE, FALSE};
	gint iPupilWidth[2], iPupilHeight[2];
	gint iEyesWidth[2], iEyesHeight[2];
	gint iXeyes[2], iYeyes[2];
	gint iXeyelid, iYeyelid;
	gint iEyelidWidth, iEyelidHeight;
	gint iXbg, iYbg;
	gint iToonWidth, iToonHeight;
	gint iBgWidth, iBgHeight;
	gchar *cBgImage, *cPupilImage, *cEyelidImage, *cToonImage;
	
	cBgImage = g_key_file_get_string (pKeyFile, "Files", "bg image", &erreur);
	_print_error(erreur);
	cPupilImage = g_key_file_get_string (pKeyFile, "Files", "pupil image", &erreur);
	_print_error(erreur);
	cEyelidImage = g_key_file_get_string (pKeyFile, "Files", "eyelid image", &erreur);
	_print_error(erreur);
	cToonImage = g_key_file_get_string (pKeyFile, "Files", "toon image", &erreur);
	_print_error(erreur);
	
	const gchar *cGroupName;
	int i;
	for (i = 0; i < 2; i ++)
	{
		cGroupName = group[i];
		if (! g_key_file_has_group (pKeyFile, cGroupName))
			continue;
		
		bEyeVisible[i] = TRUE;
		
		iXeyes[i] = g_key_file_get_integer (pKeyFile, cGroupName, "x center", &erreur);
		_print_error(erreur);
		iYeyes[i] = g_key_file_get_integer (pKeyFile, cGroupName, "y center", &erreur);
		_print_error(erreur);
		
		iEyesWidth[i] = g_key_file_get_integer (pKeyFile, cGroupName, "eye width", &erreur);
		_print_error(erreur);
		iEyesHeight[i] = g_key_file_get_integer (pKeyFile, cGroupName, "eye height", &erreur);
		_print_error(erreur);
		
		iPupilWidth[i] = g_key_file_get_integer (pKeyFile, cGroupName, "pupil width", &erreur);
		_print_error(erreur);
		iPupilHeight[i] = g_key_file_get_integer (pKeyFile, cGroupName, "pupil height", &erreur);
		_print_error(erreur);
	}
	
	iXeyelid = g_key_file_get_integer (pKeyFile, "Eyelid", "x", &erreur);
	_print_error(erreur);
	iYeyelid = g_key_file_get_integer (pKeyFile, "Eyelid", "y", &erreur);
	_print_error(erreur);
	iEyelidWidth = g_key_file_get_integer (pKeyFile, "Eyelid", "width", &erreur);
	_print_error(erreur);
	iEyelidHeight = g_key_file_get_integer (pKeyFile, "Eyelid", "height", &erreur);
	_print_error(erreur);
	
	iXbg = g_key_file_get_integer (pKeyFile, "Background", "x", &erreur);
	_print_error(erreur);
	iYbg = g_key_file_get_integer (pKeyFile, "Background", "y", &erreur);
	_print_error(erreur);
	iBgWidth = g_key_file_get_integer (pKeyFile, "Background", "width", &erreur);
	_print_error(erreur);
	iBgHeight = g_key_file_get_integer (pKeyFile, "Background", "height", &erreur);
	_print_error(erreur);
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	double fImageWidth, fImageHeight;
	double fZoomX = 1., fZoomY = 1.;
	g_string_printf (sPath, "%s/%s", myConfig.cThemePath, cToonImage);
	myData.pToonSurface = cairo_dock_create_surface_from_image (sPath->str,
		myDrawContext,
		1.,
		iWidth, iHeight,
		myConfig.iLoadingModifier,
		&fImageWidth, &fImageHeight,
		&fZoomX, &fZoomY);
	myData.iToonWidth = fImageWidth;
	myData.iToonHeight = fImageHeight;
	
	double dx = .5*(iWidth - myData.iToonWidth);  // offset du au 'keep ratio'.
	double dy = .5*(iHeight - myData.iToonHeight);
	
	g_string_printf (sPath, "%s/%s", myConfig.cThemePath, cPupilImage);
	for (i = 0; i < 2; i ++)
	{
		if (bEyeVisible[i])
		{
			myData.pPupilSurface[i] = cairo_dock_create_surface_from_image (sPath->str,
				myDrawContext,
				1.,
				iPupilWidth[i] * fZoomX, iPupilHeight[i] * fZoomY,
				myConfig.iLoadingModifier,
				&fImageWidth, &fImageHeight,
				NULL, NULL);
			myData.iPupilWidth[i] = fImageWidth;
			myData.iPupilHeight[i] = fImageHeight;
			
			myData.iXeyes[i] = iXeyes[i] * fZoomX + dx;
			myData.iYeyes[i] = iYeyes[i] * fZoomY + dy;
			
			myData.iEyesWidth[i] = iEyesWidth[i] * fZoomX;
			myData.iEyesHeight[i] = iEyesHeight[i] * fZoomY;
		}
	}
	
	g_string_printf (sPath, "%s/%s", myConfig.cThemePath, cEyelidImage);
	myData.pEyelidSurface = cairo_dock_create_surface_from_image (sPath->str,
		myDrawContext,
		1.,
		iEyelidWidth * fZoomX, iEyelidHeight * fZoomY,
		myConfig.iLoadingModifier,
		&fImageWidth, &fImageHeight,
		NULL, NULL);
	myData.iEyelidWidth = fImageWidth;
	myData.iEyelidHeight = fImageHeight;
	myData.iXeyelid = iXeyelid * fZoomX + dx;
	myData.iYeyelid = iYeyelid * fZoomY + dy;
	//g_print ("eyelid : %dx%d ; (%d;%d)\n", myData.iEyelidWidth, myData.iEyelidHeight, myData.iXeyelid, myData.iYeyelid);
	
	if (cBgImage != NULL && *cBgImage != '\0')
	{
		g_string_printf (sPath, "%s/%s", myConfig.cThemePath, cBgImage);
		myData.pBgSurface = cairo_dock_create_surface_from_image (sPath->str,
			myDrawContext,
			1.,
			iBgWidth * fZoomX, iBgHeight * fZoomY,
			myConfig.iLoadingModifier,
			&fImageWidth, &fImageHeight,
			NULL, NULL);
		myData.iBgWidth = fImageWidth;
		myData.iBgHeight = fImageHeight;
		myData.iXbg = iXbg * fZoomX + dx;
		myData.iYbg = iYbg * fZoomY + dy;
	}
	
	if (g_bUseOpenGL)
	{
		myData.iBgTexture = cairo_dock_create_texture_from_surface (myData.pBgSurface);
		if (myData.pPupilSurface[0])
			myData.iPupilTexture[0] = cairo_dock_create_texture_from_surface (myData.pPupilSurface[0]);
		if (myData.pPupilSurface[1])
			myData.iPupilTexture[1] = cairo_dock_create_texture_from_surface (myData.pPupilSurface[1]);
		myData.iEyelidTexture = cairo_dock_create_texture_from_surface (myData.pEyelidSurface);
		if (myData.pToonSurface)
			myData.iToonTexture = cairo_dock_create_texture_from_surface (myData.pToonSurface);
	}
	
	g_free (cBgImage);
	g_free (cPupilImage);
	g_free (cEyelidImage);
	g_free (cToonImage);
	g_string_free (sPath, TRUE);
	g_key_file_free (pKeyFile);
	
	return TRUE;
}


void cd_xeyes_unload_theme (CairoDockModuleInstance *myApplet)
{
	int i;
	for (i = 0; i < 2; i ++)
	{
		if (myData.pPupilSurface[i])
		{
			cairo_surface_destroy (myData.pPupilSurface[i]);
			myData.pPupilSurface[i] = NULL;
		}
		if (myData.iPupilTexture[i])
		{
			_cairo_dock_delete_texture (myData.iPupilTexture[i]);
			myData.iPupilTexture[i] = 0;
		}
	}
	
	if (myData.pBgSurface)
	{
		cairo_surface_destroy (myData.pBgSurface);
		myData.pBgSurface = NULL;
	}
	if (myData.iBgTexture)
	{
		_cairo_dock_delete_texture (myData.iBgTexture);
		myData.iBgTexture = 0;
	}
	
	if (myData.pEyelidSurface)
	{
		cairo_surface_destroy (myData.pEyelidSurface);
		myData.pEyelidSurface = NULL;
	}
	if (myData.iEyelidTexture)
	{
		_cairo_dock_delete_texture (myData.iEyelidTexture);
		myData.iEyelidTexture = 0;
	}
	
	if (myData.pToonSurface)
	{
		cairo_surface_destroy (myData.pToonSurface);
		myData.pToonSurface = NULL;
	}
	if (myData.iToonTexture)
	{
		_cairo_dock_delete_texture (myData.iToonTexture);
		myData.iToonTexture = 0;
	}
}
