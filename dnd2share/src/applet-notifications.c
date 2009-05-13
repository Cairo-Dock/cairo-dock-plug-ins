/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-dnd2share.h"

static void cd_dnd2share_delete_picture_in_menu (GtkMenuItem *menu_item, gpointer *data)
{	
	cd_dnd2share_delete_picture ();
}

static void cd_dnd2share_delete_all_pictures_in_menu (GtkMenuItem *menu_item, gpointer *data)
{	
	cd_dnd2share_delete_all_pictures ();
}

static void cd_dnd2share_copy_url_0_into_clipboard_in_menu (GtkMenuItem *menu_item, gpointer *data)
{
	myData.iUrlTypeToCopy = 0;
	cd_dnd2share_copy_url_into_clipboard (myData.iUrlTypeToCopy);
} 

static void cd_dnd2share_copy_url_1_into_clipboard_in_menu (GtkMenuItem *menu_item, gpointer *data)
{
	myData.iUrlTypeToCopy = 1;
	cd_dnd2share_copy_url_into_clipboard (myData.iUrlTypeToCopy);
} 

static void cd_dnd2share_copy_url_2_into_clipboard_in_menu (GtkMenuItem *menu_item, gpointer *data)
{
	myData.iUrlTypeToCopy = 2;
	cd_dnd2share_copy_url_into_clipboard (myData.iUrlTypeToCopy);
} 

static void cd_dnd2share_copy_url_3_into_clipboard_in_menu (GtkMenuItem *menu_item, gpointer *data)
{
	myData.iUrlTypeToCopy = 3;
	cd_dnd2share_copy_url_into_clipboard (myData.iUrlTypeToCopy);
} 

static void cd_dnd2share_copy_url_4_into_clipboard_in_menu (GtkMenuItem *menu_item, gpointer *data)
{
	myData.iUrlTypeToCopy = 4;
	cd_dnd2share_copy_url_into_clipboard (myData.iUrlTypeToCopy);
} 




//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN

	cairo_dock_remove_dialog_if_any (myIcon);
	
	myData.cCurrentLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
	myData.cCurrentConfigFile = g_strdup_printf ("%s/%i.conf", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
	
	if ( g_file_test (myData.cCurrentLogFile, G_FILE_TEST_EXISTS) || g_file_test (myData.cCurrentConfigFile, G_FILE_TEST_EXISTS))
	{
		if (g_file_test (myData.cCurrentLogFile, G_FILE_TEST_EXISTS))
		{
			// On génère d'abord le fichier de conf:
			cd_dnd2share_extract_urls_from_log ();
		}	
			
		// Puis  dans tous les cas on extrait les urls :
		cd_dnd2share_get_urls_from_stored_file ();
		// Et on affiche l'info-bulle :
		cairo_dock_show_temporary_dialog ("The prefered URL of picture %i\nhas been stored into the clipboard\nJust use 'CTRL+v' in a text fill to use it",
			myIcon,
			myContainer,
			myConfig.dTimeDialogs,
			myData.iCurrentPictureNumber);
		
		cd_dnd2share_copy_url_into_clipboard (myConfig.iUrlPicturesType);
	}
	else
	{
		if (myData.iNumberOfStoredPic == 0)
		{
			cairo_dock_show_temporary_dialog ("No stored files\nJust drag'n drop a file on the icon to upload it",
				myIcon,
				myContainer,
				myConfig.dTimeDialogs);
		}
		else
		{
			cairo_dock_show_temporary_dialog ("Waiting for the log file.\nPlease wait...",
				myIcon,
				myContainer,
				myConfig.dTimeDialogs);
		}
	}
		
	/// A VOIR L'UTILITE :
	cd_dnd2share_check_number_of_stored_pictures ();
CD_APPLET_ON_CLICK_END



CD_APPLET_ON_DROP_DATA_BEGIN

	if( strncmp(CD_APPLET_RECEIVED_DATA, "file://", 7) == 0)
	{
		cd_debug ("DND2SHARE : ''%s'' --> nouvelle image !", CD_APPLET_RECEIVED_DATA);
		// Les formats supportés par Uppix.net sont : GIF, JPEG, PNG, Flash (SWF or SWC), BMP, PSD, TIFF, JP2, JPX,
		// JB2, JPC, WBMP, and XBM.
		// ... mais l'applet ne prendra en charge que les plus utilisés :
		
		gboolean isPicture = g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpg") 
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"JPG")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpeg")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"JPEG")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"GIF")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"gif")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"PNG")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"png")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"BMP")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"bmp")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"TIFF")
			|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"tiff");
		
		if(isPicture)
		{
			cd_debug ("DND2SHARE : Le format est compatible");
			
			gchar *cDroppedPicturePath = (*CD_APPLET_RECEIVED_DATA == '/' ? g_strdup (CD_APPLET_RECEIVED_DATA) : g_filename_from_uri (CD_APPLET_RECEIVED_DATA, NULL, NULL));
			cd_dnd2share_new_picture (cDroppedPicturePath);
			g_free (cDroppedPicturePath);
					
		}
		else
		{
			
			cd_debug ("DND2SHARE : Le format n'est pas supporté ... ou ce n'est pas une image !");
			
			// On affiche une info-bulle :
			if (myConfig.bEnableDialogs)
			{
				cairo_dock_remove_dialog_if_any (myIcon);
				cairo_dock_show_temporary_dialog ("%s\n%s",
					myIcon,
					myContainer,
					myConfig.dTimeDialogs,
					D_("Sorry, the picture format is not allowed"),
					D_("... or this is not a picture"));
			}
		}
		
	}
CD_APPLET_ON_DROP_DATA_END



CD_APPLET_ON_SCROLL_BEGIN
		cd_dnd2share_check_number_of_stored_pictures (); // avant toute chose, on recompte nos images enregistrées
		if (myData.iNumberOfStoredPic == 0)
		{
			CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
			CD_APPLET_REDRAW_MY_ICON;
			
			if (myConfig.bEnableDialogs)
			{
				cairo_dock_remove_dialog_if_any (myIcon);
				cairo_dock_show_temporary_dialog ("%s",
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				D_("No picture"));
			}
		}
		else
		{	
				
			if (CD_APPLET_SCROLL_DOWN)
			{
				if (myData.iCurrentPictureNumber == 1)
					myData.iCurrentPictureNumber = myData.iNumberOfStoredPic;
				else
					myData.iCurrentPictureNumber--;
				
				// On affiche une info-bulle :
				if (myConfig.bEnableDialogs)
				{
					cairo_dock_remove_dialog_if_any (myIcon);
					cairo_dock_show_temporary_dialog ("Picture %i:\nPress 'Left mouse button' to\ncopy the prefered url\ninto the clipboard",
						myIcon,
						myContainer,
						myConfig.dTimeDialogs,
						myData.iCurrentPictureNumber);
				}
				
				myData.cCurrentPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
				myData.cCurrentLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
				myData.cCurrentConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, myData.iCurrentPictureNumber);
	
				CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCurrentPicturePath);
				CD_APPLET_REDRAW_MY_ICON;
			}
			else if (CD_APPLET_SCROLL_UP)
			{
				if (myData.iCurrentPictureNumber ==  myData.iNumberOfStoredPic)
					myData.iCurrentPictureNumber = 1;
				else
					myData.iCurrentPictureNumber++;
					
				// On affiche une info-bulle :
				if (myConfig.bEnableDialogs)
				{
					cairo_dock_remove_dialog_if_any (myIcon);
					cairo_dock_show_temporary_dialog ("Picture %i:\nPress 'Left mouse button' to\ncopy the prefered url\ninto the clipboard",
						myIcon,
						myContainer,
						myConfig.dTimeDialogs,
						myData.iCurrentPictureNumber);
				}
				
				myData.cCurrentPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
				myData.cCurrentLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
				myData.cCurrentConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		
				CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCurrentPicturePath);
				CD_APPLET_REDRAW_MY_ICON;
			}
			else
				return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		}
CD_APPLET_ON_SCROLL_END



CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.iNumberOfStoredPic > 0)
	{
		cd_dnd2share_delete_picture ();
	}
CD_APPLET_ON_MIDDLE_CLICK_END



//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pModuleSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();

	cd_dnd2share_check_number_of_stored_pictures ();
	if (myData.iNumberOfStoredPic > 0)
	{		
		CD_APPLET_ADD_IN_MENU (D_("Delete this picture"), cd_dnd2share_delete_picture_in_menu, pModuleSubMenu);
		
		CD_APPLET_ADD_IN_MENU (D_("Delete ALL pictures"), cd_dnd2share_delete_all_pictures_in_menu, pModuleSubMenu);
		
		
		
		CD_APPLET_ADD_IN_MENU (D_("BBcode 150px (forums)"), cd_dnd2share_copy_url_0_into_clipboard_in_menu, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_IN_MENU (D_("BBcode 600px (forums)"), cd_dnd2share_copy_url_1_into_clipboard_in_menu, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_IN_MENU (D_("BBcode FullPic (forums)"), cd_dnd2share_copy_url_2_into_clipboard_in_menu, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_IN_MENU (D_("Display Image (html page)"), cd_dnd2share_copy_url_3_into_clipboard_in_menu, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_IN_MENU (D_("Direct Link (picture)"), cd_dnd2share_copy_url_4_into_clipboard_in_menu, CD_APPLET_MY_MENU);
	}
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pModuleSubMenu);
CD_APPLET_ON_BUILD_MENU_END
