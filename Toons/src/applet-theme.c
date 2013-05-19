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

#include "applet-struct.h"
#include "applet-theme.h"

#define _print_error(erreur) \
	if (erreur != NULL) {\
		cd_warning (erreur->message);\
		g_error_free (erreur);\
		erreur = NULL; }

static const gchar *group[2] = {"Left eye", "Right eye"};

gboolean cd_xeyes_load_theme (GldiModuleInstance *myApplet)
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
		
		iPupilWidth[i] = g_key_file_get_integer (pKeyFile, cGroupName, "pupil width", &erreur);  // needed, because we can only load an image at a given size, not a given scale.
		_print_error(erreur);
		iPupilHeight[i] = g_key_file_get_integer (pKeyFile, cGroupName, "pupil height", &erreur);
		_print_error(erreur);
	}
	
	iXeyelid = g_key_file_get_integer (pKeyFile, "Eyelid", "x", &erreur);
	_print_error(erreur);
	iYeyelid = g_key_file_get_integer (pKeyFile, "Eyelid", "y", &erreur);
	_print_error(erreur);
	iEyelidWidth = g_key_file_get_integer (pKeyFile, "Eyelid", "width", &erreur);  // needed, because we can only load an image at a given size, not a given scale.
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
	
	g_string_printf (sPath, "%s/%s", myConfig.cThemePath, cToonImage);
	myData.pToonImage = cairo_dock_create_image_buffer (sPath->str,
		iWidth, iHeight,
		myConfig.iLoadingModifier);
	g_return_val_if_fail (myData.pToonImage != NULL, FALSE);
	
	double fZoomX = myData.pToonImage->fZoomX, fZoomY = myData.pToonImage->fZoomY;
	double dx = .5*(iWidth - myData.pToonImage->iWidth);  // offset du au 'keep ratio'.
	double dy = .5*(iHeight - myData.pToonImage->iHeight);
	
	g_string_printf (sPath, "%s/%s", myConfig.cThemePath, cPupilImage);
	for (i = 0; i < 2; i ++)
	{
		if (bEyeVisible[i])
		{
			myData.pPupilImage[i] = cairo_dock_create_image_buffer (sPath->str,
				iPupilWidth[i] * fZoomX, iPupilHeight[i] * fZoomY,
				myConfig.iLoadingModifier);
			
			myData.iXeyes[i] = iXeyes[i] * fZoomX + dx;
			myData.iYeyes[i] = iYeyes[i] * fZoomY + dy;
			
			myData.iEyesWidth[i] = iEyesWidth[i] * fZoomX;
			myData.iEyesHeight[i] = iEyesHeight[i] * fZoomY;
		}
	}
	
	g_string_printf (sPath, "%s/%s", myConfig.cThemePath, cEyelidImage);
	myData.pEyelidImage = cairo_dock_create_image_buffer (sPath->str,
		iEyelidWidth * fZoomX, iEyelidHeight * fZoomY,
		myConfig.iLoadingModifier);
	myData.iXeyelid = iXeyelid * fZoomX + dx;
	myData.iYeyelid = iYeyelid * fZoomY + dy;
	//g_print ("eyelid : %dx%d ; (%d;%d)\n", myData.iEyelidWidth, myData.iEyelidHeight, myData.iXeyelid, myData.iYeyelid);
	
	if (cBgImage != NULL && *cBgImage != '\0')
	{
		g_string_printf (sPath, "%s/%s", myConfig.cThemePath, cBgImage);
		myData.pBgImage = cairo_dock_create_image_buffer (sPath->str,
			iBgWidth * fZoomX, iBgHeight * fZoomY,
			myConfig.iLoadingModifier);
		myData.iXbg = iXbg * fZoomX + dx;
		myData.iYbg = iYbg * fZoomY + dy;
	}
	
	g_free (cBgImage);
	g_free (cPupilImage);
	g_free (cEyelidImage);
	g_free (cToonImage);
	g_string_free (sPath, TRUE);
	g_key_file_free (pKeyFile);
	
	return TRUE;
}


void cd_xeyes_unload_theme (GldiModuleInstance *myApplet)
{
	int i;
	for (i = 0; i < 2; i ++)
	{
		cairo_dock_free_image_buffer (myData.pPupilImage[i]);
	}
	
	cairo_dock_free_image_buffer (myData.pBgImage);
	cairo_dock_free_image_buffer (myData.pEyelidImage);
	cairo_dock_free_image_buffer (myData.pToonImage);
}
