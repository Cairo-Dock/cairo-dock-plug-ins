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

#include "applet-struct.h"
#include "applet-trashes-manager.h"
#include "applet-notifications.h"

static void _on_answer_delete_trash (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		cairo_dock_fm_empty_trash ();
	}
	CD_APPLET_LEAVE ();
}
static void _cd_dustbin_delete_trash (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	if (myConfig.bAskBeforeDelete)
	{
		gldi_dialog_show_with_question (D_("You're about to delete all files in all dustbins. Sure ?"),
			myIcon, myContainer,
			"same icon",
			(CairoDockActionOnAnswerFunc) _on_answer_delete_trash, NULL, (GFreeFunc)NULL);
	}
	else
	{
		cairo_dock_fm_empty_trash ();
	}
}

static void _cd_dustbin_show_trash (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	cairo_dock_fm_launch_uri ("trash:/"/**myData.cDustbinPath*/);  // on force l'utilisation de trash:/ ici, car on sait que tous les backends sauront l'ouvrir.
}


static void _free_info_dialog (GldiModuleInstance *myApplet)
{
	myData.pInfoDialog = NULL;
	if (myData.pInfoTask != NULL)
	{
		cairo_dock_discard_task (myData.pInfoTask);
		myData.pInfoTask = NULL;
	}
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	cd_debug ("free dustbin SM");
	g_free (pSharedMemory->cDustbinPath);
	g_free (pSharedMemory);
}
static void _measure_trash (CDSharedMemory *pSharedMemory)
{
	pSharedMemory->iMeasure = cairo_dock_fm_measure_diretory (pSharedMemory->cDustbinPath,
		(pSharedMemory->iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT ? 0 : 1),
		TRUE,
		pSharedMemory->bDiscard);
}
static gboolean _display_result (CDSharedMemory *pSharedMemory)
{
	if (myData.pInfoDialog != NULL)
	{
		int iSize=-1, iNbFiles=-1, iTrashes=-1;
		if (pSharedMemory->iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			iSize = myData.iMeasure;
			iNbFiles = pSharedMemory->iMeasure;
		}
		else
		{
			iSize = pSharedMemory->iMeasure;
			if (pSharedMemory->iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)
				iNbFiles = myData.iMeasure;
			else
			{
				gint iCancel = 0;
				iTrashes = cairo_dock_fm_measure_diretory (myData.cDustbinPath, 0, FALSE, &iCancel);  // ca c'est rapide.
			}
		}
		
		gldi_dialog_set_message_printf (myData.pInfoDialog, "%s :\n %d %s\n %.2f %s",
			D_("The trash contains"),
			iNbFiles > -1 ? iNbFiles : iTrashes,
			iNbFiles > -1 ? D_("files") : D_("elements"),
			(iSize > 1e6 ? (iSize >> 10) / 1024. : iSize / 1024.),
			(iSize > 1e6 ? D_("Mo") : D_("Ko")));
	}
	cairo_dock_discard_task (myData.pInfoTask);
	myData.pInfoTask = NULL;

	return FALSE;
}
static void _cd_dustbin_show_info (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	if (myData.pInfoDialog != NULL)
	{
		gldi_object_unref (GLDI_OBJECT(myData.pInfoDialog));
		myData.pInfoDialog = NULL;
	}
	g_return_if_fail (myData.pInfoTask == NULL);
	
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	attr.cImageFilePath = "same icon";
	attr.cText = g_strdup_printf ("%s ...\n\n", D_("Counting total size and files number..."));
	attr.pFreeDataFunc = (GFreeFunc)_free_info_dialog;
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;
	attr.pUserData = myApplet;
	myData.pInfoDialog = gldi_dialog_new (&attr);
	
	// launch the task and update the dialog when finished.
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->cDustbinPath = g_strdup (myData.cDustbinPath);
	pSharedMemory->iQuickInfoType = myConfig.iQuickInfoType;
	myData.pInfoTask = cairo_dock_new_task_full (0,
		(CairoDockGetDataAsyncFunc) _measure_trash,
		(CairoDockUpdateSyncFunc) _display_result,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	pSharedMemory->bDiscard = &myData.pInfoTask->bDiscard;
	
	cairo_dock_launch_task (myData.pInfoTask);
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Show Trash (click)"), GTK_STOCK_OPEN, _cd_dustbin_show_trash, CD_APPLET_MY_MENU, NULL);
	gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Empty Trash"), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (cLabel, GTK_STOCK_DELETE, _cd_dustbin_delete_trash, CD_APPLET_MY_MENU, NULL);
	g_free (cLabel);
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Display dustbins information"), GTK_STOCK_INFO, _cd_dustbin_show_info, CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END


static void _cd_dustbin_action_after_unmount (gboolean bMounting, gboolean bSuccess, const gchar *cName, gpointer data)
{
	g_return_if_fail (myIcon != NULL && ! bMounting);
	gchar *cMessage;
	if (bSuccess)
	{
		cMessage = g_strdup_printf (D_("%s successfully unmounted"), cName);
	}
	else
	{
		cMessage = g_strdup_printf (D_("Failed to unmount %s"), cName);
		
	}
	gldi_dialogs_remove_on_icon (myIcon);
	gldi_dialog_show_temporary (cMessage, myIcon, myContainer, 4000);
	g_free (cMessage);
}
CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message ("  '%s' --> a la poubelle !", CD_APPLET_RECEIVED_DATA);
	gchar *cName=NULL, *cURI=NULL, *cIconName=NULL;
	gboolean bIsDirectory;
	int iVolumeID = 0;
	double fOrder;
	if (cairo_dock_fm_get_file_info (CD_APPLET_RECEIVED_DATA,
		&cName,
		&cURI,
		&cIconName,
		&bIsDirectory,
		&iVolumeID,
		&fOrder,
		0))
	{
		if (iVolumeID > 0)
		{
			gldi_dialog_show_temporary_with_icon (D_("Unmouting this volume ..."), myIcon, myContainer, 15000., "same icon");  // le dialogue sera enleve lorsque le volume sera demonte.
			cairo_dock_fm_unmount_full (cURI, iVolumeID, (CairoDockFMMountCallback) _cd_dustbin_action_after_unmount, myApplet);
		}
		else
			cairo_dock_fm_delete_file (cURI, FALSE);
	}
	else
	{
		cd_warning ("can't get info for '%s'", CD_APPLET_RECEIVED_DATA);
	}
	g_free (cName);
	g_free (cURI);
	g_free (cIconName);
CD_APPLET_ON_DROP_DATA_END


CD_APPLET_ON_CLICK_BEGIN
	_cd_dustbin_show_trash (NULL, myApplet);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_cd_dustbin_delete_trash (NULL, myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END
