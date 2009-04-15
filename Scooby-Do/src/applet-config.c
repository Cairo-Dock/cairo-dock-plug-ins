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
	myConfig.cShortkey = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	myConfig.iAnimationDuration = CD_CONFIG_GET_INTEGER ("Configuration", "anim duration");
	myConfig.iAppearanceDuration = CD_CONFIG_GET_INTEGER ("Configuration", "appear duration");
	myConfig.iCloseDuration = CD_CONFIG_GET_INTEGER ("Configuration", "stop duration");
	myConfig.cIconAnimation = CD_CONFIG_GET_STRING ("Configuration", "animation");
	CD_CONFIG_GET_COLOR ("Configuration", "bg color", myConfig.pBackgroundColor);
	gsize length=0;
	myConfig.pDirList = CD_CONFIG_GET_STRING_LIST ("Configuration", "dirs", &length);
	
	myConfig.fFontSizeRatio = CD_CONFIG_GET_DOUBLE ("Configuration", "font size");
	myConfig.labelDescription.cFont = CD_CONFIG_GET_STRING ("Configuration", "text font");
	int iWeight = CD_CONFIG_GET_INTEGER ("Configuration", "text weight");
	myConfig.labelDescription.iWeight = cairo_dock_get_pango_weight_from_1_9 (iWeight);
	myConfig.labelDescription.bOutlined = CD_CONFIG_GET_BOOLEAN ("Configuration", "text outlined");
	CD_CONFIG_GET_COLOR ("Configuration", "text color", myConfig.labelDescription.fColorStart);
	CD_CONFIG_GET_COLOR ("Configuration", "text color", myConfig.labelDescription.fColorStop);
	myConfig.labelDescription.iStyle = PANGO_STYLE_NORMAL;
	myConfig.labelDescription.fBackgroundColor[3] = (myConfig.iAnimationDuration != 0 && CD_APPLET_MY_CONTAINER_IS_OPENGL ? 1. : 0);  // si pas d'animation, on ne met pas de fond aux lettres.
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_print ("myConfig.cShortkey  %s\n", myConfig.cShortkey);
	if (myConfig.cShortkey)
		cd_keybinder_unbind (myConfig.cShortkey, (CDBindkeyHandler) cd_do_on_shortkey);
	g_free (myConfig.cShortkey);
	g_free (myConfig.cIconAnimation);
	g_free (myConfig.labelDescription.cFont);
	g_strfreev (myConfig.pDirList);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cd_do_close_session ();
	cd_do_exit_session ();
	
	if (myData.dir_hash)
		g_hash_table_destroy (myData.dir_hash);
	
	GList *l;
	for (l = myData.possible_executables; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.possible_executables);
	
	for (l = myData.completion_items; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.completion_items);
	
	if (myData.completion)
		g_completion_free (myData.completion);
CD_APPLET_RESET_DATA_END
