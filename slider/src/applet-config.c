/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cDirectory 		= CD_CONFIG_GET_STRING("Configuration", "directory");
	myConfig.iSlideTime 		= 1000 * CD_CONFIG_GET_INTEGER ("Configuration", "slide time");
	myConfig.bSubDirs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "sub directories");
	myConfig.bRandom 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "random");
	
	myConfig.bNoStretch 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "no stretch");
	myConfig.bFillIcon 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "fill icon");
	myConfig.iAnimation 		= CD_CONFIG_GET_INTEGER ("Configuration", "change animation");
	myConfig.iClickOption 		= CD_CONFIG_GET_INTEGER ("Configuration", "click");
	
	myConfig.fFrameAlpha		= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "frame alpha", 1.);
	myConfig.cFrameImage 	= CD_CONFIG_GET_FILE_PATH ("Configuration", "frame", "frame.svg");
	myConfig.iFrameOffset		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "frame offset", 5);
	myConfig.fReflectAlpha		= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "reflect alpha", 1.);
	myConfig.cReflectImage 	= CD_CONFIG_GET_FILE_PATH ("Configuration", "reflect", "reflect.svg");
	
	CD_CONFIG_GET_COLOR ("Configuration", "background color", myConfig.pBackgroundColor);
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before yo get the applet's config, and when your applet is stopped.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free(myConfig.cDirectory);
	g_free(myConfig.cFrameImage);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped.
CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	cd_slider_free_images_list (myData.pList);
	
	cairo_surface_destroy (myData.pCairoSurface);
	cairo_surface_destroy (myData.pPrevCairoSurface);
	//cairo_surface_destroy (myData.pCairoFrameSurface);
	//cairo_surface_destroy (myData.pCairoReflectSurface);
	
CD_APPLET_RESET_DATA_END
