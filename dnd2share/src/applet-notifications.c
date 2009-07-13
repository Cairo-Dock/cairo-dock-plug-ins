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

static void _clear_history (GtkMenuItem *menu_item, gpointer *data)
{	
	int iAnswer = cairo_dock_ask_question_and_wait (D_("Clear the list of the recently uploaded files ?"), myIcon, myContainer);
	if (iAnswer == GTK_RESPONSE_YES)
	{
		cd_dnd2share_clear_working_directory ();
		cd_dnd2share_clear_history ();
	}
}

static void _show_local_file (GtkMenuItem *menu_item, CDUploadedItem *pItem)
{
	if (pItem->iFileType == CD_TYPE_TEXT)
	{
		cd_dnd2share_copy_url_to_clipboard (pItem->cLocalPath);
		if (myConfig.bEnableDialogs)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (D_("The text has been pasted in the clipboard.\nYou can retrieve it with CTRL+v."),
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
	}
	else
	{
		if (g_file_test (pItem->cLocalPath, G_FILE_TEST_EXISTS))
			cairo_dock_fm_launch_uri (pItem->cLocalPath);
		else
		{
			gchar *cPreviewPath = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, pItem->cItemName);
			if (g_file_test (cPreviewPath, G_FILE_TEST_EXISTS))
			{
				cairo_dock_fm_launch_uri (cPreviewPath);
			}
			else
			{
				cd_warning ("couldn't find the orignial file nor a preview of it");
				cairo_dock_remove_dialog_if_any (myIcon);
				cairo_dock_show_temporary_dialog_with_icon (D_("Sorry, couldn't find the orignial file nor a preview of it."),
					myIcon,
					myContainer,
					myConfig.dTimeDialogs,
					MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			}
			g_free (cPreviewPath);
		}
	}
}
static void _remove_from_history (GtkMenuItem *menu_item, CDUploadedItem *pItem)
{
	cd_dnd2share_remove_one_item (pItem);
}

static void _copy_url_into_clipboard (GtkMenuItem *menu_item, const gchar *cURL)
{
	cd_dnd2share_copy_url_to_clipboard (cURL);
	if (myConfig.bEnableDialogs)
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (D_("The URL has been stored into the clipboard.\nJust use 'CTRL+v' to paste it anywhere."),
			myIcon,
			myContainer,
			myConfig.dTimeDialogs,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
} 

static void _store_last_url (gboolean bIntoClipboard)
{
	if (myData.cLastURL == NULL)
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (myConfig.iNbItems != 0 ?
			D_("No uploaded file available\n.Just drag'n drop a file on the icon to upload it") :
			D_("No uploaded file available\n.Consider activating the history if you want the applet remembers previous uploads."),
		myIcon,
		myContainer,
		myConfig.dTimeDialogs,
		MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
	else
	{
		if (bIntoClipboard)
			cd_dnd2share_copy_url_to_clipboard (myData.cLastURL);
		else
			cd_dnd2share_copy_url_to_primary (myData.cLastURL);
		
		if (myConfig.bEnableDialogs)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (bIntoClipboard ? 
					D_("The current URL has been stored into the clipboard.\nJust use 'CTRL+v' to paste it anywhere.") :
					D_("The current URL has been stored into the selection.\nJust middle-click to paste it anywhere."),
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	_store_last_url (TRUE);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	g_print  ("DND2SHARE : drop de '%s'\n", CD_APPLET_RECEIVED_DATA);
	CDFileType iFileType = CD_UNKNOWN_TYPE;
	
	if( strncmp(CD_APPLET_RECEIVED_DATA, "file://", 7) == 0)
	{
		// Les formats supportés par Uppix.net sont : GIF, JPEG, PNG, Flash (SWF or SWC), BMP, PSD, TIFF, JP2, JPX,
		// JB2, JPC, WBMP, and XBM.
		// ... mais l'applet ne prendra en charge que les plus utilisés :
		
		guint64 iSize;
		time_t iLastModificationTime;
		gchar *cMimeType = NULL;
		int iUID, iGID, iPermissionsMask;
		if (cairo_dock_fm_get_file_properties (CD_APPLET_RECEIVED_DATA, &iSize, &iLastModificationTime, &cMimeType, &iUID, &iGID, &iPermissionsMask))
		{
			if (cMimeType != NULL)
			{
				g_print ("cMimeType : %s (%s)\n", cMimeType, CD_APPLET_RECEIVED_DATA);
				if (strncmp (cMimeType, "image", 5) == 0)
					iFileType = CD_TYPE_IMAGE;
				else if (strncmp (cMimeType, "video", 5) == 0)
					iFileType = CD_TYPE_VIDEO;
			}
			g_free (cMimeType);
		}
		
		if (iFileType == CD_UNKNOWN_TYPE)
		{
			if (g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpg") 
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
				|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"tiff"))
				iFileType = CD_TYPE_IMAGE;
			else if (g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"avi") 
				|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"AVI")
				|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"mov")
				|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"ogg")
				|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"mp4"))
				iFileType = CD_TYPE_VIDEO;
		}
	}
	else  // c'est du texte.
	{
		g_print ("TEXT\n");
		iFileType = CD_TYPE_TEXT;
	}
	
	if (iFileType == CD_UNKNOWN_TYPE)
	{
		iFileType = CD_TYPE_FILE;
		g_print ("we'll consider this as an archive.");
	}
	cd_dnd2share_launch_upload (CD_APPLET_RECEIVED_DATA, iFileType);
CD_APPLET_ON_DROP_DATA_END


CD_APPLET_ON_SCROLL_BEGIN
	if (myData.pUpoadedItems == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CDUploadedItem *pItem;
	if (CD_APPLET_SCROLL_DOWN)
	{
		myData.iCurrentItemNum ++;  // item suivant.
		pItem = g_list_nth_data (myData.pUpoadedItems, myData.iCurrentItemNum);
		if (pItem == NULL)
		{
			pItem = myData.pUpoadedItems->data;
			myData.iCurrentItemNum = 0;
		}
	}
	else if (CD_APPLET_SCROLL_UP)
	{
		myData.iCurrentItemNum --;  // item precedent.
		pItem = g_list_nth_data (myData.pUpoadedItems, myData.iCurrentItemNum);
		if (pItem == NULL)
		{
			pItem = g_list_last (myData.pUpoadedItems)->data;
			myData.iCurrentItemNum = g_list_length (myData.pUpoadedItems) - 1;
		}
	}
	g_free (myData.cLastURL);
	myData.cLastURL = NULL;
	
	g_return_val_if_fail (pItem != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);  // parano
	
	myData.cLastURL = g_strdup (cd_dnd2share_get_prefered_url_from_item (pItem));
	if (myConfig.bDisplayLastImage)
	{
		gchar *cPreview = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, pItem->cItemName);
		if (g_file_test (cPreview, G_FILE_TEST_EXISTS))
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (cPreview);
			g_free (cPreview);
		}
		else
		{
			cPreview = pItem->cLocalPath;
			if (g_file_test (cPreview, G_FILE_TEST_EXISTS))
			{
				CD_APPLET_SET_IMAGE_ON_MY_ICON (cPreview);
			}
			else
			{
				CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			}
		}
		CD_APPLET_REDRAW_MY_ICON;
	}
	
	if (myConfig.bEnableDialogs)
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog (D_("%s '%s' (n°%d):\nPress 'Left mouse button' to copy the URL into the clipboard"),
			myIcon,
			myContainer,
			myConfig.dTimeDialogs,
			(pItem->iFileType == CD_TYPE_TEXT ? "Text" : "File"), pItem->cFileName, myData.iCurrentItemNum);
	}
CD_APPLET_ON_SCROLL_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_store_last_url (FALSE);
CD_APPLET_ON_MIDDLE_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pModuleSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	
	if (myData.pUpoadedItems != NULL)
		CD_APPLET_ADD_IN_MENU (D_("Clear History"), _clear_history, CD_APPLET_MY_MENU);
	
	CDSiteBackend *pBackend;
	CDUploadedItem *pItem;
	GtkWidget *pItemSubMenu;
	gchar *str;
	gchar *cName = NULL, *cURI = NULL, *cIconName = NULL;
	gboolean bIsDirectory;
	int iVolumeID;
	double fOrder;
	int i;
	GList *it;
	for (it = myData.pUpoadedItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		
		// on cherche une miniature a mettre dans le menu.
		gchar *cPreview = NULL;
		if (pItem->iFileType == CD_TYPE_IMAGE)
		{
			cPreview = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, pItem->cItemName);
			if (! g_file_test (cPreview, G_FILE_TEST_EXISTS))
			{
				g_free (cPreview);
				cPreview = cairo_dock_search_icon_s_path ("image-x-generic");;
			}
		}
		else if (pItem->iFileType == CD_TYPE_TEXT)
		{
			cPreview = cairo_dock_search_icon_s_path ("text-x-generic");
		}
		else if (pItem->iFileType == CD_TYPE_VIDEO)
		{
			cPreview = cairo_dock_search_icon_s_path ("video-x-generic");
		}
		if (cPreview == NULL)
		{
			cairo_dock_fm_get_file_info (pItem->cLocalPath, &cName, &cURI, &cPreview, &bIsDirectory, &iVolumeID, &fOrder, 0);
			g_free (cName);
			cName = NULL;
			g_free (cURI);
			cURI = NULL;
		}
		
		// on cree un sous-menu pour ce fichier.
		str = strchr (pItem->cFileName, '\n');
		if (str)
			*str = '\0';
		pItemSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (pItem->cFileName, pModuleSubMenu, cPreview);
		if (str)
			*str = '\n';
		g_free (cPreview);
		
		// on le peuple avec les liens.
		pBackend = &myData.backends[pItem->iFileType][pItem->iSiteID];
		for (i = 0; i < pBackend->iNbUrls; i ++)
		{
			g_print ("%d) %s : ", i, pBackend->cUrlLabels[i]);
			g_print (" + %s\n", pItem->cDistantUrls[i]);
			CD_APPLET_ADD_IN_MENU_WITH_DATA (pBackend->cUrlLabels[i], _copy_url_into_clipboard, pItemSubMenu, pItem->cDistantUrls[i]);
		}
		if (pItem->iFileType != CD_TYPE_TEXT)
			CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Open file"), _show_local_file, pItemSubMenu, pItem);
		else
			CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Get text"), _show_local_file, pItemSubMenu, pItem);
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Remove from history"), GTK_STOCK_REMOVE, _remove_from_history, pItemSubMenu, pItem);
	}
	
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END
