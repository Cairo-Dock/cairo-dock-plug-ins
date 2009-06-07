/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.fRotationSpeed = CD_CONFIG_GET_DOUBLE ("Configuration", "rotation speed");
	myConfig.iSpeed = CD_CONFIG_GET_INTEGER ("Configuration", "speed");
	myConfig.cDropIndicatorImageName = CD_CONFIG_GET_STRING ("Configuration", "drop indicator");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cDropIndicatorImageName);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
void cd_drop_indicator_free_buffers (void)
{
	if (myData.iDropIndicatorTexture != 0)
	{
		_cairo_dock_delete_texture (myData.iDropIndicatorTexture);
		myData.iDropIndicatorTexture = 0;
	}
	
	if (myData.pDropIndicatorSurface != NULL)
	{
		cairo_surface_destroy (myData.pDropIndicatorSurface);
		myData.pDropIndicatorSurface = NULL;
	}
}

void _reser_data_on_one_dock (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	CDDropIndicatorData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
		return ;
	g_free (pData);
}
CD_APPLET_RESET_DATA_BEGIN
	/// free our data on all docks ..
	cairo_dock_foreach_docks ((GHFunc) _reser_data_on_one_dock, NULL);
	
	cd_drop_indicator_free_buffers ();
	
	if (myData.iBilinearGradationTexture != 0)
	{
		_cairo_dock_delete_texture (myData.iBilinearGradationTexture);
		myData.iBilinearGradationTexture = 0;
	}
CD_APPLET_RESET_DATA_END
