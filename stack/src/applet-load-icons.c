/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <string.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-stack.h"

CD_APPLET_INCLUDE_MY_VARS


void cd_stack_destroy_icons (CairoDockModuleInstance *myApplet) {
	cd_debug ("");
	if (myDock && myIcon->pSubDock != NULL) {
		CD_APPLET_DESTROY_MY_SUBDOCK
	}
	else if (myDesklet && myDesklet->icons != NULL) {
		g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myDesklet->icons);
		myDesklet->icons = NULL;
	}
}


static gboolean _isin (gchar **cString, gchar *cCompar) {
	if (cString == NULL)
		return FALSE; //Nothing to search in
	
	cd_debug ("%s (%s)", __func__, cCompar);
	int i=0;
	gchar *tmp=NULL;
	while (cString[i] != NULL) {
		g_print ("   %s\n", cString[i]);
		tmp = g_strstr_len (cCompar, -1, cString[i]);
		if (tmp != NULL)
			return TRUE; //We found what we want
		i++;
	}
	
	return FALSE; //We didn't found anything
}
Icon *cd_stack_build_one_icon (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile)
{
	GError *erreur = NULL;
	gchar *cContent = g_key_file_get_string (pKeyFile, "Desktop Entry", "Content", &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Stack : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_return_val_if_fail (cContent != NULL, NULL);
	
	Icon *pIcon = NULL;
	if (cairo_dock_string_is_adress (cContent))
	{
		if (strncmp (cContent, "http://", 7) == 0)
		{
			pIcon = g_new0 (Icon, 1);
			pIcon->acCommand = cContent;
			pIcon->iVolumeID = 1;
			pIcon->acFileName = g_strdup (myConfig.cUrlIcon);
		}
		else
		{
			gchar *cCanonicName=NULL, *cRealURI=NULL, *cIconName=NULL;
			gboolean bIsDirectory;
			int iVolumeID;
			double fOrder;
			cairo_dock_fm_get_file_info (cContent, &cCanonicName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, g_iFileSortType);
			g_print ("un fichier -> %s , %s\n", cCanonicName, cIconName);
			g_free (cRealURI);
			
			if (myConfig.bFilter && cIconName != NULL && _isin(myConfig.cMimeTypes, cIconName))
			{
				g_free (cIconName);
				g_free (cCanonicName);
				g_free (cContent);
				return NULL;
			}
			
			pIcon = g_new0 (Icon, 1);
			pIcon->acCommand = cContent;
			pIcon->iVolumeID = 1;
			if (pIcon->acName == NULL)
				pIcon->acName = cCanonicName;
			else
				g_free (cCanonicName);
			
			pIcon->acFileName = cIconName;
		}
	}
	else
	{
		pIcon = g_new0 (Icon, 1);
		pIcon->acCommand = cContent;
		pIcon->acFileName = g_strdup (myConfig.cTextIcon);
	}
	
	pIcon->acName = g_key_file_get_string (pKeyFile, "Desktop Entry", "Name", &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Stack : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	if (myConfig.iSortType == CD_STACK_SORT_BY_DATE)
	{
		pIcon->fOrder = g_key_file_get_integer (pKeyFile, "Desktop Entry", "Date", &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Stack : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
	}
	else if (myConfig.iSortType == CD_STACK_SORT_MANUALLY)
	{
		pIcon->fOrder = g_key_file_get_double (pKeyFile, "Desktop Entry", "Order", &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Stack : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
	}
	
	return pIcon;
}

Icon *cd_stack_build_one_icon_from_file (CairoDockModuleInstance *myApplet, gchar *cDesktopFilePath)
{
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new();
	g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Stack : while trying to load %s : %s", cDesktopFilePath, erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	Icon *pIcon = cd_stack_build_one_icon (myApplet, pKeyFile);
	
	g_key_file_free (pKeyFile);
	return pIcon;
}
GList *cd_stack_insert_icon_in_list (CairoDockModuleInstance *myApplet, GList *pIconsList, Icon *pIcon)
{
	g_return_val_if_fail (pIcon != NULL, pIconsList);
	switch (myConfig.iSortType)
	{
		case CD_STACK_SORT_BY_DATE :
		case CD_STACK_SORT_MANUALLY :
			pIconsList = g_list_insert_sorted (pIconsList, pIcon, (GCompareFunc) cairo_dock_compare_icons_order);
		break;
		case CD_STACK_SORT_BY_NAME :
			pIconsList = g_list_insert_sorted (pIconsList, pIcon, (GCompareFunc) cairo_dock_compare_icons_name);
		break;
		case CD_STACK_SORT_BY_TYPE :
		default :
			pIconsList = g_list_insert_sorted (pIconsList, pIcon, (GCompareFunc) cairo_dock_compare_icons_extension);
		break;
	}
	return pIconsList;
}
GList *cd_stack_build_icons_list (CairoDockModuleInstance *myApplet, gchar *cStackDir)
{
	GDir *dir = g_dir_open (cStackDir, 0, NULL);
	g_return_val_if_fail (dir != NULL, NULL);
	
	GList *pIconsList = NULL;
	Icon *pIcon;
	GString *sDesktopFilePath = g_string_new ("");
	const gchar *cFileName;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		g_string_printf (sDesktopFilePath, "%s/%s", cStackDir, cFileName);
		
		pIcon = cd_stack_build_one_icon_from_file (myApplet, sDesktopFilePath->str);
		if (pIcon != NULL)
		{
			pIcon->acDesktopFileName = g_strdup (cFileName);
			
			pIconsList = cd_stack_insert_icon_in_list (myApplet, pIconsList, pIcon);
		}
	} while (1);
	
	g_string_free (sDesktopFilePath, TRUE);
	g_dir_close (dir);
	return pIconsList;
}

void cd_stack_build_icons (CairoDockModuleInstance *myApplet)
{
	cd_stack_destroy_icons (myApplet);
	if (myIcon->acName == NULL && myDock)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (CD_STACK_DEFAULT_NAME)
	}
	
	GList *pIconList = cd_stack_build_icons_list (myApplet, myConfig.cStackDir);
	
	if (myDock) {
		CD_APPLET_CREATE_MY_SUBDOCK (pIconList, myConfig.cRenderer)
	}
	else {
		myDesklet->icons = pIconList;
		CD_APPLET_SET_DESKLET_RENDERER ("Tree")
		///gtk_widget_queue_draw (myDesklet->pWidget);  // utile ?
	}
}
