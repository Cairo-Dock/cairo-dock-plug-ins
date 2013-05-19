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

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("Folders"),
	2, 2, 0,
	CAIRO_DOCK_CATEGORY_APPLET_FILES,
	N_("This applet imports folders inside the Dock\n"
	"You can have as many instances of this applet as you want, each one with a different folder.\n"
	"To add a folder in your dock:\n"
	" - activate the applet, open its configuration panel, and select a folder\n"
	" - or just drop a folder into the dock\n"
	"Middle-click on the main icon opens the folder.\n"),
	"Fabounet")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
	gldi_object_register_notification (&myContainersMgr,
		NOTIFICATION_DROP_DATA,
		(GldiNotificationFunc) cd_folders_on_drop_data,
		GLDI_RUN_FIRST, NULL);
CD_APPLET_DEFINE_END


static inline void _set_comparaison_func (GldiModuleInstance *myApplet)
{
	switch (myConfig.iSortType)
	{
		case 0:  // name
		default:
			myData.comp = (GCompareFunc) cairo_dock_compare_icons_name;
		break;
		case 1:  // date
		case 2:  // size
			myData.comp = NULL;
		break;
		case 3:  // type
			myData.comp = (GCompareFunc) cairo_dock_compare_icons_extension;
		break;
	}
}

static inline void _set_icon_label (GldiModuleInstance *myApplet)
{
	if (myDock && myConfig.cDefaultTitle == NULL && myConfig.cDirPath != NULL)
	{
		gchar *cPath = g_filename_from_uri (myConfig.cDirPath, NULL, NULL);
		if (cPath)
		{
			gchar *str = strrchr (cPath, '/');
			if (str)
				CD_APPLET_SET_NAME_FOR_MY_ICON (str+1);
			g_free (cPath);
		}
	}
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN

	if (myConfig.cDirPath == NULL)
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cImageFile);
		gldi_dialog_show_temporary_with_icon (D_("Open the configuration of the applet to choose a folder to import."),
			myIcon, myContainer,
			8000.,
			myConfig.iSubdockViewType == 0 ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
	else
	{
		//\_______________ On lance la tache recuperation des fichiers.
		_set_comparaison_func (myApplet);
		if (myConfig.bShowFiles)
		{
			cd_folders_start (myApplet);
		}

		//\_______________ set the icon rendering
		if (myDock)  // dock mode: set the image or the sub-dock renderer
		{
			cairo_dock_set_subdock_content_renderer (myIcon, myConfig.iSubdockViewType);
			if (myConfig.iSubdockViewType == 0)
				CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cImageFile);
		}
		else  // desklet mode: set the image if we don't show the files.
		{
			if (! myConfig.bShowFiles)
			{
				CD_APPLET_SET_DESKLET_RENDERER ("Simple");
				CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cImageFile);
			}
		}
		
		//\_______________ set the label
		_set_icon_label (myApplet);
	}
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	gldi_object_remove_notification (&myContainersMgr,
		NOTIFICATION_CLICK_ICON,
		(GldiNotificationFunc) CD_APPLET_ON_CLICK_FUNC,
		myApplet);
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On detruit les icones des fichiers.
		cd_folders_free_all_data (myApplet);

		if (myConfig.cDirPath == NULL)
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cImageFile);
			gldi_dialog_show_temporary_with_icon (D_("Open the configuration of the applet to choose a folder to import."),
				myIcon, myContainer,
				8000.,
				myConfig.iSubdockViewType == 0 ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
		else
		{
			//\_______________ On charge les icones dans un sous-dock.
			_set_comparaison_func (myApplet);
			if (myConfig.bShowFiles)
			{
				cd_folders_start (myApplet);
			}
			else if (myDock && myIcon->pSubDock)  // dans ce cas on veut un comportement de type lanceur, donc on ne veut pas d'un sous-dock vide.
			{
				gldi_object_unref (GLDI_OBJECT(myIcon->pSubDock));
				myIcon->pSubDock = NULL;
			}
			
			//\_______________ set the icon rendering
			if (myDock)  // dock mode: set the image or the sub-dock renderer
			{
				cairo_dock_set_subdock_content_renderer (myIcon, myConfig.iSubdockViewType);
				if (myConfig.iSubdockViewType == 0)
					CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cImageFile);
			}
			else  // desklet mode: set the image if we don't show the files.
			{
				if (! myConfig.bShowFiles)
				{
					CD_APPLET_SET_DESKLET_RENDERER ("Simple");
					CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cImageFile);
				}
			}
			
			//\_______________ set the label
			_set_icon_label (myApplet);
		}
	}
CD_APPLET_RELOAD_END
