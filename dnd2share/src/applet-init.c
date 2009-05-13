/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-dnd2share.h"


CD_APPLET_DEFINITION ("dnd2share",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet manages uploads\n"
	"You can send files to host web services simply by drag and droping them on the icon.\n"
	"The chosen type of url is automatically stored in clipboard to be directly copied in a forum.\n"
	"The applet stores your last uploads to retrieve them without any account.\n"),
	"Yann Dulieu (Nochka85)\n"
	"Based on a script made by pmd (http://pmdz.info).")

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	if (myIcon->acFileName == NULL)  // set a default icon if none is specified.
	{
		
		myData.cCurrentLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		myData.cCurrentPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		myData.cCurrentConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		
		if (!myConfig.bEnableHistory) // Si on ne souhaite pas d'historique -> On efface le contenu du myData.cWorkingDirPath
		{
			cd_debug ("DND2SHARE : Pas d'historique -> On efface le contenu de '%s'", myData.cWorkingDirPath);
			cd_dnd2share_delete_all_pictures ();
		}
		
		cd_dnd2share_check_number_of_stored_pictures ();
		
		if (myData.iNumberOfStoredPic == 0)
			CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
		else
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCurrentPicturePath);
		}
		
		
			
	}

	CD_APPLET_REDRAW_MY_ICON;
	
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
	/// To be continued ...
	
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myIcon->acFileName == NULL)
		{
			if (!myConfig.bEnableHistory) // Si on ne souhaite pas d'historique -> On efface le contenu du myData.cWorkingDirPath
			{
				cd_debug ("DND2SHARE : Pas d'historique -> On efface le contenu de '%s'", myData.cWorkingDirPath);
				cd_dnd2share_delete_all_pictures ();				
			}
			else
			{
				if (myConfig.bEnableHistoryLimit)
				{
					while (myData.iNumberOfStoredPic > myConfig.iNbItems)
					{
						// Je décale tout pour obtenir le nb d'images maxi demandé :
						gint iSourcePictureNumber = 2; 
						gchar *cSourcePicturePath;
						gchar *cSourceLogFile;
						gchar *cSourceConfigFile;
						
						gint iDestPictureNumber = 1; 
						gchar *cDestPicturePath;
						gchar *cDestLogFile;
						gchar *cDestConfigFile;
								
						gint i;
						
						for (i=0 ; i< myData.iNumberOfStoredPic; i++)
						{
							cd_debug ("DND2SHARE : Trop d'image -> J'en retire 1 !");
							cSourcePicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, iSourcePictureNumber);
							cSourceLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, iSourcePictureNumber);
							cSourceConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, iSourcePictureNumber);
							
							cDestPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, iDestPictureNumber);
							cDestLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, iDestPictureNumber);
							cDestConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, iDestPictureNumber);
							
							remove(cDestPicturePath);
							remove(cDestLogFile);
							remove(cDestConfigFile);
										
							rename(cSourcePicturePath, cDestPicturePath);
							rename(cSourceLogFile, cDestLogFile);
							rename(cSourceConfigFile, cDestConfigFile);
							
							iSourcePictureNumber = iSourcePictureNumber + 1;
							iDestPictureNumber = iDestPictureNumber + 1;
						}
						g_free (cSourcePicturePath);
						g_free (cSourceLogFile);
						g_free (cSourceConfigFile);
						g_free (cDestPicturePath);
						g_free (cDestLogFile);
						g_free (cDestConfigFile);
						
						cd_dnd2share_check_number_of_stored_pictures ();
					}
				
				myData.iCurrentPictureNumber = myData.iNumberOfStoredPic;
				myData.cCurrentLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
				myData.cCurrentPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
				myData.cCurrentConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		
				}
			}			
			
			cd_dnd2share_check_number_of_stored_pictures ();
			
			
			if (myData.iNumberOfStoredPic == 0)
				CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
			else
			{
				CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCurrentPicturePath);
			}
			
			CD_APPLET_REDRAW_MY_ICON;
		}
		
		/// To be continued ...
		
	}
CD_APPLET_RELOAD_END
