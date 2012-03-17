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

#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-trashes-manager.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("dustbin"),
	2, 2, 0,
	CAIRO_DOCK_CATEGORY_APPLET_FILES,
	N_("This applet manages the dustbin\n"
	"You can threw files and unmount disks by drag-and-dropping them on the icon.\n"
	"Middle-click on the icon will empty the dustbin."),
	"Fabounet (Fabrice Rey)")

static void _get_theme (void)
{
	// get the user images first, as they overwrite the theme.
	if (myConfig.cEmptyUserImage != NULL)
	{
		gchar *cPath = cairo_dock_search_icon_s_path (myConfig.cEmptyUserImage, CAIRO_DOCK_DEFAULT_ICON_SIZE);
		if (! g_file_test (cPath, G_FILE_TEST_EXISTS))
		{
			g_free (myConfig.cEmptyUserImage);
			myConfig.cEmptyUserImage = NULL;
		}
		g_free (cPath);
	}
	if (myConfig.cFullUserImage != NULL)
	{
		gchar *cPath = cairo_dock_search_icon_s_path (myConfig.cFullUserImage, CAIRO_DOCK_DEFAULT_ICON_SIZE);
		if (! g_file_test (cPath, G_FILE_TEST_EXISTS))
		{
			g_free (myConfig.cFullUserImage);
			myConfig.cFullUserImage = NULL;
		}
		g_free (cPath);
	}
	// if a theme is defined, and user images are not defined, use the theme.
	if (myConfig.cThemePath != NULL)
	{
		if (myConfig.cEmptyUserImage == NULL)
		{
			myConfig.cEmptyUserImage = g_strdup_printf ("%s/%s", myConfig.cThemePath, "trashcan_empty.svg");
			if (! g_file_test (myConfig.cEmptyUserImage, G_FILE_TEST_EXISTS))
			{
				g_free (myConfig.cEmptyUserImage);
				myConfig.cEmptyUserImage = g_strdup_printf ("%s/%s", myConfig.cThemePath, "trashcan_empty.png");
				if (! g_file_test (myConfig.cEmptyUserImage, G_FILE_TEST_EXISTS))  // no svg nor png, use the default theme.
				{
					g_free (myConfig.cEmptyUserImage);
					myConfig.cEmptyUserImage = g_strdup (MY_APPLET_SHARE_DATA_DIR"/themes/default/trashcan_empty.svg");
					cd_warning ("using the default theme for Dustbin, as neither the user image (%s) nor the theme (%s) are valid", myConfig.cEmptyUserImage, myConfig.cThemePath);
				}
			}
		}
		if (myConfig.cFullUserImage == NULL)
		{
			myConfig.cFullUserImage = g_strdup_printf ("%s/%s", myConfig.cThemePath, "trashcan_full.svg");
			if (! g_file_test (myConfig.cFullUserImage, G_FILE_TEST_EXISTS))
			{
				g_free (myConfig.cFullUserImage);
				myConfig.cFullUserImage = g_strdup_printf ("%s/%s", myConfig.cThemePath, "trashcan_full.png");
				if (! g_file_test (myConfig.cFullUserImage, G_FILE_TEST_EXISTS))
				{
					g_free (myConfig.cFullUserImage);
					myConfig.cFullUserImage = g_strdup (MY_APPLET_SHARE_DATA_DIR"/themes/default/trashcan_full.svg");
					cd_warning ("using the default theme for Dustbin, as neither the user image (%s) nor the theme (%s) are valid", myConfig.cFullUserImage, myConfig.cThemePath);
				}
			}
		}
	}
}

CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	//\_______________ On recupere les images du theme choisi.
	_get_theme ();
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	
	//\_______________ On demarre la surveillance de la poubelle.
	cd_dustbin_start (myApplet);
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cEmptyUserImage);
	
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	
	//\_______________ On stoppe la surveillance.
	cd_dustbin_stop (myApplet);
	
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		//\_______________ On stoppe la surveillance.
		cd_dustbin_stop (myApplet);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		
		//\_______________ On la redemarre.
		_get_theme ();
		cd_dustbin_start (myApplet);
		
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.bDisplayFullIcon ? myConfig.cFullUserImage: myConfig.cEmptyUserImage);
	}
CD_APPLET_RELOAD_END
