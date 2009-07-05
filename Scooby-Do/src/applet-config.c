/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-session.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cShortkeyNav = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	myConfig.cShortkeySearch = CD_CONFIG_GET_STRING ("Configuration", "shortkey search");
	myConfig.iAppearanceDuration = CD_CONFIG_GET_INTEGER ("Configuration", "appear duration");
	myConfig.iCloseDuration = CD_CONFIG_GET_INTEGER ("Configuration", "stop duration");
	myConfig.cIconAnimation = CD_CONFIG_GET_STRING ("Configuration", "animation");
	CD_CONFIG_GET_COLOR ("Configuration", "frame color", myConfig.pFrameColor);
	
	myConfig.fFontSizeRatio = CD_CONFIG_GET_DOUBLE ("Configuration", "font size");
	myConfig.bTextOnTop = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "text on top", TRUE);
	myConfig.labelDescription.cFont = CD_CONFIG_GET_STRING ("Configuration", "text font");
	int iWeight = CD_CONFIG_GET_INTEGER ("Configuration", "text weight");
	myConfig.labelDescription.iWeight = cairo_dock_get_pango_weight_from_1_9 (iWeight);
	myConfig.labelDescription.bOutlined = CD_CONFIG_GET_BOOLEAN ("Configuration", "text outlined");
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "text color", myConfig.labelDescription.fColorStart);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "text color", myConfig.labelDescription.fColorStop);
	myConfig.labelDescription.iStyle = PANGO_STYLE_NORMAL;
	myConfig.labelDescription.iMargin = 2;
	CD_CONFIG_GET_COLOR ("Configuration", "bg color", myConfig.labelDescription.fBackgroundColor);
	if (myConfig.iAppearanceDuration == 0 || ! g_bUseOpenGL)  // si pas d'animation 3D, on ne met pas de fond aux lettres.
	{
		//myConfig.labelDescription.fBackgroundColor[3] = 0;
	}
	
	myConfig.iNbResultMax = CD_CONFIG_GET_INTEGER ("Configuration", "nb results max");
	myConfig.infoDescription.cFont = "Sans";
	myConfig.infoDescription.iSize = 14;
	myConfig.infoDescription.iWeight = cairo_dock_get_pango_weight_from_1_9 (5);
	myConfig.infoDescription.bOutlined = FALSE;
	myConfig.infoDescription.fColorStart[0] = 1;
	myConfig.infoDescription.fColorStart[1] = 0;
	myConfig.infoDescription.fColorStart[2] = 0;
	myConfig.infoDescription.fColorStop[0] = 1;
	myConfig.infoDescription.fColorStop[1] = 0;
	myConfig.infoDescription.fColorStop[2] = 0;
	myConfig.infoDescription.iStyle = PANGO_STYLE_NORMAL;
	myConfig.infoDescription.fBackgroundColor[3] = 1;
	myConfig.infoDescription.fBackgroundColor[3] = 0;
	myConfig.infoDescription.fBackgroundColor[3] = 0;
	myConfig.infoDescription.fBackgroundColor[3] = 0.33;
	myConfig.infoDescription.iMargin = 8;
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	if (myConfig.cShortkeyNav)
		cd_keybinder_unbind (myConfig.cShortkeyNav, (CDBindkeyHandler) cd_do_on_shortkey_nav);
	g_free (myConfig.cShortkeyNav);
	if (myConfig.cShortkeySearch)
		cd_keybinder_unbind (myConfig.cShortkeySearch, (CDBindkeyHandler) cd_do_on_shortkey_search);
	g_free (myConfig.cShortkeySearch);
	g_free (myConfig.cIconAnimation);
	g_free (myConfig.labelDescription.cFont);
	g_strfreev (myConfig.pDirList);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cd_do_close_session ();
	cd_do_exit_session ();
	
	/*if (myData.dir_hash)
		g_hash_table_destroy (myData.dir_hash);
	
	GList *l;
	for (l = myData.possible_executables; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.possible_executables);
	
	for (l = myData.completion_items; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.completion_items);
	
	if (myData.completion)
		g_completion_free (myData.completion);*/
	
	if (myData.pPromptSurface != NULL)
		cairo_surface_destroy (myData.pPromptSurface);
	if (myData.iPromptTexture != 0)
		_cairo_dock_delete_texture (myData.iPromptTexture);
	if (myData.pArrowSurface != NULL)
		cairo_surface_destroy (myData.pArrowSurface);
	if (myData.iArrowTexture != 0)
		_cairo_dock_delete_texture (myData.iArrowTexture);
CD_APPLET_RESET_DATA_END
