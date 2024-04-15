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
#include <unistd.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-notifications.h"

static void _on_answer_clear_history (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		cd_dnd2share_clear_working_directory ();
		cd_dnd2share_clear_history ();
	}
	CD_APPLET_LEAVE ();
}

static void _clear_history (GtkMenuItem *menu_item, gpointer data)
{
	gldi_dialog_show_with_question (D_("Clear the list of the recently uploaded files?"),
		myIcon, myContainer,
		"same icon",
		(CairoDockActionOnAnswerFunc) _on_answer_clear_history, NULL, (GFreeFunc)NULL);
}

static void _show_local_file (GtkMenuItem *menu_item, CDUploadedItem *pItem)
{
	CD_APPLET_ENTER;
	if (pItem->iFileType == CD_TYPE_TEXT)
	{
		cd_dnd2share_copy_url_to_clipboard (pItem->cLocalPath);
		if (myConfig.bEnableDialogs)
		{
			gldi_dialogs_remove_on_icon (myIcon);
			gldi_dialog_show_temporary_with_icon (D_("The text has been pasted in the clipboard.\nYou can retrieve it with CTRL+v."),
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
				gldi_dialogs_remove_on_icon (myIcon);
				gldi_dialog_show_temporary_with_icon (D_("Sorry, couldn't find the original file nor a preview of it."),
					myIcon,
					myContainer,
					myConfig.dTimeDialogs,
					MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			}
			g_free (cPreviewPath);
		}
	}
	CD_APPLET_LEAVE ();
}
static void _remove_from_history (GtkMenuItem *menu_item, CDUploadedItem *pItem)
{
	CD_APPLET_ENTER;
	cd_dnd2share_remove_one_item (pItem);
	CD_APPLET_LEAVE ();
}

static void _copy_url_into_clipboard (GtkMenuItem *menu_item, const gchar *cURL)
{
	CD_APPLET_ENTER;
	cd_dnd2share_copy_url_to_clipboard (cURL);
	if (myConfig.bEnableDialogs)
	{
		gldi_dialogs_remove_on_icon (myIcon);
		gldi_dialog_show_temporary_with_icon (D_("The URL has been stored in the clipboard.\nJust use 'CTRL+v' to paste it anywhere."),
			myIcon,
			myContainer,
			myConfig.dTimeDialogs,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
	CD_APPLET_LEAVE ();
}

static void _store_last_url (gboolean bIntoClipboard)
{
	if (myData.cLastURL == NULL)
	{
		gldi_dialogs_remove_on_icon (myIcon);
		gldi_dialog_show_temporary_with_icon (myConfig.iNbItems != 0 ?
			D_("No uploaded file available\n.Just drag'n drop a file on the icon to upload it") :
			D_("No uploaded file available.\nConsider activating the history if you want the applet to remember previous uploads."),
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
			gldi_dialogs_remove_on_icon (myIcon);
			gldi_dialog_show_temporary_with_icon (bIntoClipboard ? 
					D_("The current URL has been stored in the clipboard.\nJust use 'CTRL+v' to paste it anywhere.") :
					D_("The current URL has been stored into the selection.\nJust middle-click to paste it anywhere."),
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
	}
}

static void _on_drop_data (const gchar *cMyData)
{
	CDFileType iFileType = CD_UNKNOWN_TYPE;
	gchar *cFilePath = NULL;
	if (strncmp(cMyData, "file://", 7) == 0)
	{
		// Uppix.net supports : GIF, JPEG, PNG, Flash (SWF or SWC), BMP, PSD, TIFF, JP2, JPX,
		// JB2, JPC, WBMP, and XBM.
		// ... but the applet will support the most common
		
		cFilePath = g_filename_from_uri (cMyData, NULL, NULL);  // switch to UTF-8.
		g_return_if_fail (cFilePath != NULL);
		
		if (strchr(cFilePath, ',') != NULL) // if there is a comma, curl doesn't like it
		{
			myData.cTmpFilePath = g_strdup ("/tmp/dnd2share-file_with_comma.XXXXXX");
			int fds = mkstemp (myData.cTmpFilePath);
			if (fds == -1)
			{
				g_free (myData.cTmpFilePath);
				myData.cTmpFilePath = NULL;
				return;
			}
			close(fds);
			
			gchar *cCommandCopyFileWithComma = g_strdup_printf ("cp '%s' '%s'", cFilePath, myData.cTmpFilePath); // copy the current file to tmp.
			int r = system (cCommandCopyFileWithComma);
			if (r < 0)
				cd_warning ("Not able to launch this command: %s", cCommandCopyFileWithComma);
			g_free (cCommandCopyFileWithComma);
			g_free (cFilePath);
			cFilePath = g_strdup (myData.cTmpFilePath);  // we use a tmp file which will be remove at the end of the update
		}
		
		guint64 iSize;
		time_t iLastModificationTime;
		gchar *cMimeType = NULL;
		int iUID, iGID, iPermissionsMask;
		if (cairo_dock_fm_get_file_properties (cMyData, &iSize, &iLastModificationTime, &cMimeType, &iUID, &iGID, &iPermissionsMask))
		{
			if (cMimeType != NULL)
			{
				cd_debug ("cMimeType : %s (%s)", cMimeType, cMyData);
				if (strncmp (cMimeType, "image", 5) == 0)
					iFileType = CD_TYPE_IMAGE;
				else if (strncmp (cMimeType, "video", 5) == 0)
					iFileType = CD_TYPE_VIDEO;
				else if (strncmp (cMimeType, "text", 4) == 0)
					iFileType = CD_TYPE_TEXT;
			}
			g_free (cMimeType);
		}
		
		if (iFileType == CD_UNKNOWN_TYPE)
		{
			if (g_str_has_suffix(cMyData,"jpg") 
				|| g_str_has_suffix(cMyData,"JPG")
				|| g_str_has_suffix(cMyData,"png")
				|| g_str_has_suffix(cMyData,"PNG")
				|| g_str_has_suffix(cMyData,"jpeg")
				|| g_str_has_suffix(cMyData,"JPEG")
				|| g_str_has_suffix(cMyData,"gif")
				|| g_str_has_suffix(cMyData,"GIF")
				|| g_str_has_suffix(cMyData,"bmp")
				|| g_str_has_suffix(cMyData,"BMP")
				|| g_str_has_suffix(cMyData,"TIFF")
				|| g_str_has_suffix(cMyData,"tiff"))
				iFileType = CD_TYPE_IMAGE;
			else if (g_str_has_suffix(cMyData,"avi") 
				|| g_str_has_suffix(cMyData,"AVI")
				|| g_str_has_suffix(cMyData,"ogg")
				|| g_str_has_suffix(cMyData,"OGG")
				|| g_str_has_suffix(cMyData,"ogv")
				|| g_str_has_suffix(cMyData,"OGV")
				|| g_str_has_suffix(cMyData,"mp4")
				|| g_str_has_suffix(cMyData,"MP4")
				|| g_str_has_suffix(cMyData,"mov")
				|| g_str_has_suffix(cMyData,"MOV"))
				iFileType = CD_TYPE_VIDEO;
		}
	}
	else  // text.
	{
		// cd_debug ("TEXT\n");
		iFileType = CD_TYPE_TEXT;
	}
	
	if (iFileType == CD_UNKNOWN_TYPE)
	{
		iFileType = CD_TYPE_FILE;
		cd_debug ("we'll consider this as an archive.");
	}
	cd_dnd2share_launch_upload (cFilePath ? cFilePath : cMyData, iFileType);
	g_free (cFilePath);
}

static void _get_image (GtkClipboard *clipboard, GdkPixbuf *pixbuf, gpointer data)
{
	g_return_if_fail (pixbuf != NULL);
	if (myData.cTmpFilePath != NULL)
	{
		cd_warning ("Please wait the current upload is finished before starting a new one.");
		return ;
	}
	myData.cTmpFilePath = g_strdup ("/tmp/dnd2share-tmp-file.XXXXXX");
	int fds = mkstemp (myData.cTmpFilePath);
	if (fds == -1)
	{
		g_free (myData.cTmpFilePath);
		myData.cTmpFilePath = NULL;
		return ;
	}
	close(fds);
	
	CD_APPLET_ENTER;
	gboolean bSaved = gdk_pixbuf_save (pixbuf,
		myData.cTmpFilePath,
		"png",
		NULL,
		NULL);
	CD_APPLET_LEAVE_IF_FAIL (bSaved);
	//g_return_if_fail (bSaved);
	
	cd_dnd2share_launch_upload (myData.cTmpFilePath, CD_TYPE_IMAGE);
	CD_APPLET_LEAVE ();
}
static void _get_text (GtkClipboard *clipboard, const gchar *cText, gpointer data)
{
	g_return_if_fail (cText != NULL);
	CD_APPLET_ENTER;
	gchar *cFilePath = NULL;
	const gchar *cDropData = NULL;
	if ( *cText == '/' && g_file_test (cText, G_FILE_TEST_EXISTS) )
		cFilePath = g_strdup_printf ("file://%s", cText);
	else
		cDropData = cText;
	
	_on_drop_data (cFilePath ? cFilePath : cDropData);
	CD_APPLET_LEAVE ();
}
static void _send_clipboard (GtkMenuItem *menu_item, gpointer *data)
{
	CD_APPLET_ENTER;
	GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gboolean bDataAvailable = gtk_clipboard_wait_is_image_available (pClipBoard);
	g_return_if_fail (myIcon != NULL);  // protection, this function is blocked but the mainloop continue to run
	if (bDataAvailable)
	{
		gtk_clipboard_request_image (pClipBoard, (GtkClipboardImageReceivedFunc) _get_image, data);
		CD_APPLET_LEAVE ();
		//return;
	}
	bDataAvailable = gtk_clipboard_wait_is_text_available (pClipBoard);
	g_return_if_fail (myIcon != NULL);
	if (bDataAvailable)
	{
		gtk_clipboard_request_text (pClipBoard, (GtkClipboardTextReceivedFunc) _get_text, data);
		CD_APPLET_LEAVE ();
		//return;
	}
	CD_APPLET_LEAVE ();
}

// checkbox => Use only a file web hosting
static void _set_use_only_file_type (GtkCheckMenuItem *pMenuItem, gpointer *data)
{
	myConfig.bUseOnlyFileType = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem));
	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
		G_TYPE_BOOLEAN, "Configuration", "only file type", myConfig.bUseOnlyFileType,
		G_TYPE_INVALID); // updated the config file => maybe not needed if we want a temporary status
}


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	_store_last_url (TRUE);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_debug  ("DND2SHARE : drop de '%s'", CD_APPLET_RECEIVED_DATA);
	_on_drop_data (CD_APPLET_RECEIVED_DATA);
CD_APPLET_ON_DROP_DATA_END


CD_APPLET_ON_SCROLL_BEGIN
	if (myData.pUpoadedItems == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	CDUploadedItem *pItem;
	if (CD_APPLET_SCROLL_DOWN)
	{
		myData.iCurrentItemNum ++;  // next item.
		pItem = g_list_nth_data (myData.pUpoadedItems, myData.iCurrentItemNum);
		if (pItem == NULL)
		{
			pItem = myData.pUpoadedItems->data;
			myData.iCurrentItemNum = 0;
		}
	}
	else if (CD_APPLET_SCROLL_UP)
	{
		myData.iCurrentItemNum --;  // prev item.
		pItem = g_list_nth_data (myData.pUpoadedItems, myData.iCurrentItemNum);
		if (pItem == NULL)
		{
			pItem = g_list_last (myData.pUpoadedItems)->data;
			cd_debug ("dernier item\n");
			myData.iCurrentItemNum = g_list_length (myData.pUpoadedItems) - 1;
		}
	}
	else
		return GLDI_NOTIFICATION_LET_PASS;
	g_free (myData.cLastURL);
	myData.cLastURL = NULL;
	
	g_return_val_if_fail (pItem != NULL, GLDI_NOTIFICATION_LET_PASS);  // parano
	
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
		gldi_dialogs_remove_on_icon (myIcon);
		gldi_dialog_show_temporary_with_icon_printf ("%s '%s' (n°%d):\n%s",
			myIcon,
			myContainer,
			myConfig.dTimeDialogs,
			"same icon",
			(pItem->iFileType == CD_TYPE_TEXT ? D_("Text") : D_("File")),
			pItem->cFileName,
			myData.iCurrentItemNum,
			D_("Click on the icon to copy the URL into the clipboard."));
	}
CD_APPLET_ON_SCROLL_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_store_last_url (FALSE);
CD_APPLET_ON_MIDDLE_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Send the clipboard's content"), GLDI_ICON_NAME_PASTE, _send_clipboard, CD_APPLET_MY_MENU);
	
	GtkWidget *mi;
	GtkWidget *pHistoryMenu = gldi_menu_add_sub_menu_full (CD_APPLET_MY_MENU, D_("History"), GLDI_ICON_NAME_SORT_DESCENDING, &mi);
	
	if (myData.pUpoadedItems != NULL)
	{
		CDSiteBackend *pBackend;
		CDUploadedItem *pItem;
		GtkWidget *pItemSubMenu;
		gchar *cName = NULL, *cURI = NULL;
		gboolean bIsDirectory;
		int iVolumeID;
		double fOrder;
		gint iDesiredIconSize = cairo_dock_search_icon_size (GTK_ICON_SIZE_LARGE_TOOLBAR); // 24px by default
		int i;
		GList *it;
		for (it = myData.pUpoadedItems; it != NULL; it = it->next)
		{
			pItem = it->data;
			
			// we're looking for a thumbnail (for the menu)
			gchar *cPreview = NULL;
			if (pItem->iFileType == CD_TYPE_IMAGE)
			{
				cPreview = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, pItem->cItemName);
				if (! g_file_test (cPreview, G_FILE_TEST_EXISTS))
				{
					g_free (cPreview);
					cPreview = cairo_dock_search_icon_s_path ("image-x-generic", iDesiredIconSize);
				}
			}
			else if (pItem->iFileType == CD_TYPE_TEXT)
			{
				cPreview = cairo_dock_search_icon_s_path ("text-x-generic", iDesiredIconSize);
			}
			else if (pItem->iFileType == CD_TYPE_VIDEO)
			{
				cPreview = cairo_dock_search_icon_s_path ("video-x-generic", iDesiredIconSize);
			}
			if (cPreview == NULL)
			{
				gchar *cIconName = NULL;
				cairo_dock_fm_get_file_info (pItem->cLocalPath, &cName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, 0);
				cPreview = cairo_dock_search_icon_s_path (cIconName, iDesiredIconSize);
				g_free (cIconName);
				g_free (cName);
				cName = NULL;
				g_free (cURI);
				cURI = NULL;
			}
			
			// we create a sub menu for this file
			pItemSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (pItem->cFileName, pHistoryMenu, cPreview);
			g_free (cPreview);
			
			// we add links
			pBackend = &myData.backends[pItem->iFileType][pItem->iSiteID];
			for (i = 0; i < pBackend->iNbUrls; i ++)
			{
				//g_print ("%d) %s : ", i, pBackend->cUrlLabels[i]);
				//g_print (" + %s\n", pItem->cDistantUrls[i]);
				if (pItem->cDistantUrls[i] != NULL)  // can be null (e.g. tiny url).
					CD_APPLET_ADD_IN_MENU_WITH_DATA (dgettext (MY_APPLET_GETTEXT_DOMAIN, pBackend->cUrlLabels[i]), _copy_url_into_clipboard, pItemSubMenu, pItem->cDistantUrls[i]);
			}
			if (pItem->iFileType != CD_TYPE_TEXT)
				CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Open file"), _show_local_file, pItemSubMenu, pItem);
			else
				CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Get text"), _show_local_file, pItemSubMenu, pItem);
			
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Remove from history"), GLDI_ICON_NAME_REMOVE, _remove_from_history, pItemSubMenu, pItem);
		}
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Clear History"), GLDI_ICON_NAME_CLEAR, _clear_history, pHistoryMenu);
	}
	else
		gtk_widget_set_sensitive (GTK_WIDGET (mi), FALSE);

	pMenuItem = gtk_check_menu_item_new_with_label (D_("Use only a files hosting site"));
	gtk_menu_shell_append (GTK_MENU_SHELL (CD_APPLET_MY_MENU), pMenuItem);
	if (myConfig.bUseOnlyFileType)
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (pMenuItem), TRUE);
	g_signal_connect (G_OBJECT (pMenuItem), "toggled", G_CALLBACK (_set_use_only_file_type), NULL);
CD_APPLET_ON_BUILD_MENU_END
