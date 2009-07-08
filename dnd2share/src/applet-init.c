/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <glib/gstdio.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-backend-pastebin.h"
#include "applet-backend-uppix.h"
#include "applet-backend-imagebin.h"
#include "applet-backend-imageshack.h"
#include "applet-backend-free.h"
#include "applet-backend-custom.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("dnd2share",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet lets you share files easily :\n"
	"You can send files to host web services by simply drag-and-dropping them on the icon.\n"
	"The resulting URL is automatically stored in the clipboard to be directly copied by CTRL+v.\n"
	"It can keep an history of your last uploads to retrieve them without any account.\n"
	"Based on a script made by pmd (http://pmdz.info). Needs 'curl' and 'wget' to upload data."),
	"Yann Dulieu (Nochka85) & Fabrice Rey (Fabounet)")

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	//\____________ on cree le repertoire de l'historique si necessaire.
	myData.cWorkingDirPath = g_strdup_printf ("%s/dnd2share", g_cCairoDockDataDir);
	if (! g_file_test (myData.cWorkingDirPath, G_FILE_TEST_EXISTS))
	{
		cd_debug ("DND2SHARE : le dossier '%s' n'existe pas encore -> On le crée", myData.cWorkingDirPath);
		if (g_mkdir (myData.cWorkingDirPath, 7*8*8+7*8+5) != 0)
		{
			cd_warning ("couldn't create directory '%s' !\nNo history will be available.", myData.cWorkingDirPath);
			myConfig.iNbItems == 0;
		}
	}
	
	//\____________ On nettoie le repertoire de l'historique si necessaire (config changee lorsque applet non active).
	cd_dnd2share_clean_working_directory ();
	
	//\____________ On enregistre les backends (attention a bien respecter l'ordre du fichier de conf !)
	// custom backends, ils prennent le numero 0.
	cd_dnd2share_register_custom_backends ();
	// text backends
	cd_dnd2share_register_pastebin_backend ();
	// image backends
	cd_dnd2share_register_uppix_backend ();
	cd_dnd2share_register_imagebin_backend ();
	cd_dnd2share_register_imageshack_backend ();
	// video backends
	// ...
	// file backends
	cd_dnd2share_register_free_backend ();
	
	int t;
	for (t = 0; t < CD_NB_FILE_TYPES; t ++)
		myData.pCurrentBackend[t] = &myData.backends[t][myConfig.iPreferedSite[t]];
	
	//\____________ On construit l'historique.
	if (myConfig.iNbItems != 0)
		cd_dnd2share_build_history ();
	
	//\____________ On remet la derniere URL en memoire pour le clic gauche.
	if (myData.pUpoadedItems != NULL)
	{
		CDUploadedItem *pItem = g_list_last (myData.pUpoadedItems)->data;
		cd_dnd2share_set_current_url_from_item (pItem);
	}
	
	//\____________ On affiche la derniere image uploadee.
	if (myConfig.bDisplayLastImage && myData.pUpoadedItems != NULL)
	{
		CDUploadedItem *pItem = myData.pUpoadedItems->data;
		gchar *cPreview = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, pItem->cItemName);
		if (g_file_test (cPreview, G_FILE_TEST_EXISTS))
			CD_APPLET_SET_IMAGE_ON_MY_ICON (cPreview);
		g_free (cPreview);
	}
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	
	//\____________ On s'abonne aux notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\____________ On nettoie le repertoire de travail si necessaire.
		cd_dnd2share_clean_working_directory ();
		
		//\____________ on reconstruit l'historique.
		cd_dnd2share_clear_history ();
		if (myConfig.iNbItems != 0)
			cd_dnd2share_build_history ();
		
		int t;
		for (t = 0; t < CD_NB_FILE_TYPES; t ++)
			myData.pCurrentBackend[t] = &myData.backends[t][myConfig.iPreferedSite[t]];
		
		//\____________ On affiche la derniere image uploadee.
		if (myConfig.bDisplayLastImage && myData.pUpoadedItems != NULL)
		{
			CDUploadedItem *pItem = g_list_nth_data (myData.pUpoadedItems, myData.iCurrentItemNum);
			if (pItem == NULL)
				pItem = myData.pUpoadedItems->data;
			gchar *cPreview = g_strdup_printf ("%s/%s", myData.cWorkingDirPath, pItem->cItemName);
			if (g_file_test (cPreview, G_FILE_TEST_EXISTS))
				CD_APPLET_SET_IMAGE_ON_MY_ICON (cPreview);
			g_free (cPreview);
		}
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	}
CD_APPLET_RELOAD_END
