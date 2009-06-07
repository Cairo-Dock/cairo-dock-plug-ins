/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-slider.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cDirectory 		= CD_CONFIG_GET_STRING("Configuration", "directory");
	myConfig.iSlideTime 		= CD_CONFIG_GET_INTEGER ("Configuration", "slide time");
	myConfig.bSubDirs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "sub directories");
	myConfig.bRandom 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "random");
	myConfig.bImageName		= CD_CONFIG_GET_BOOLEAN ("Configuration", "image name");
	
	myConfig.bNoStretch 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "no stretch");
	myConfig.bFillIcon 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "fill icon");
	myConfig.iAnimation 		= CD_CONFIG_GET_INTEGER ("Configuration", "change animation");
	myConfig.iNbAnimationStep 	= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb step", 20);
	myConfig.iClickOption 		= CD_CONFIG_GET_INTEGER ("Configuration", "click");
	
	myConfig.bUseThread 		= CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "use_thread", TRUE);
	CD_CONFIG_GET_COLOR ("Configuration", "background color", myConfig.pBackgroundColor);
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before yo get the applet's config, and when your applet is stopped.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free(myConfig.cDirectory);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped.
CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureDirectory);
	cairo_dock_free_measure_timer (myData.pMeasureImage);
	cd_slider_free_images_list (myData.pList);
	
	if (myData.pPrevCairoSurface != NULL && myData.pPrevCairoSurface != myData.pCairoSurface)
		cairo_surface_destroy (myData.pPrevCairoSurface);
	if (myData.pCairoSurface != NULL)
		cairo_surface_destroy (myData.pCairoSurface);
	
	if (myData.iPrevTexture != 0 && myData.iPrevTexture != myData.iTexture)
		_cairo_dock_delete_texture (myData.iPrevTexture);
	if (myData.iTexture != 0)
		_cairo_dock_delete_texture (myData.iTexture);
	
CD_APPLET_RESET_DATA_END
