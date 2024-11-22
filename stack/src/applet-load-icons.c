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

#include <string.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-stack.h"


static gboolean _isin (gchar **cString, gchar *cCompar) {
	if (cString == NULL)
		return FALSE; //Nothing to search in
	
	cd_debug ("%s (%s)", __func__, cCompar);
	int i=0;
	gchar *tmp;
	while (cString[i] != NULL) {
		tmp = g_strstr_len (cCompar, -1, cString[i]);
		if (tmp != NULL)
			return TRUE; //We found what we want
		i++;
	}
	
	return FALSE; //We didn't found anything
}
static void _load_html_icon (Icon *pIcon)
{
	int iWidth = cairo_dock_icon_get_allocated_width (pIcon);
	int iHeight = cairo_dock_icon_get_allocated_height (pIcon);
	if (pIcon->cFileName)  // icone possedant une image, on affiche celle-ci.
	{
		gchar *cIconPath = cairo_dock_search_icon_s_path (pIcon->cFileName, MAX (iWidth, iHeight));
		if (cIconPath != NULL && *cIconPath != '\0')
		{
			cairo_surface_t *pSurface = cairo_dock_create_surface_from_image_simple (cIconPath,
				iWidth,
				iHeight);
			cairo_dock_load_image_buffer_from_surface (&pIcon->image, pSurface, iWidth, iHeight);
			
			if (pIcon->image.pSurface != NULL && pIcon->cBaseURI != NULL)
			{
				cairo_dock_print_overlay_on_icon_from_image (pIcon, pIcon->cBaseURI, CAIRO_OVERLAY_LOWER_RIGHT);
			}
		}
		g_free (cIconPath);
	}
}
Icon *cd_stack_build_one_icon (GldiModuleInstance *myApplet, GKeyFile *pKeyFile)
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
	if (cairo_dock_string_is_address (cContent))
	{
		if (strncmp (cContent, "http://", 7) == 0 || strncmp (cContent, "https://", 8) == 0)  // URL web
		{
			pIcon = cairo_dock_create_dummy_launcher (NULL,
				g_strdup (myConfig.cUrlIcon),
				cContent,
				NULL,
				0);
			pIcon->iface.load_image = _load_html_icon;
			if (myConfig.bSeparateTypes)
				pIcon->iGroup = 6;
		}
		else  // URI file
		{
			gchar *cCanonicName=NULL, *cRealURI=NULL, *cIconName=NULL;
			gboolean bIsDirectory;
			int iVolumeID;
			double fOrder;
			cairo_dock_fm_get_file_info (cContent, &cCanonicName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_NAME);
			cd_debug ("un fichier -> %s , %s", cCanonicName, cIconName);
			g_free (cRealURI);
			
			if (myConfig.bFilter && cIconName != NULL && _isin(myConfig.cMimeTypes, cIconName))
			{
				g_free (cIconName);
				g_free (cCanonicName);
				g_free (cContent);
				return NULL;
			}
			
			pIcon = cairo_dock_create_dummy_launcher (NULL,
				cIconName,
				cContent,
				NULL,
				0);
			g_free (cCanonicName);
			if (myConfig.bSeparateTypes)
				pIcon->iGroup = 8;
		}
		pIcon->iVolumeID = 1;  // let's use this as a flag for the URI items.
	}
	else  // text
	{
		pIcon = cairo_dock_create_dummy_launcher (NULL,
			g_strdup (myConfig.cTextIcon),
			cContent,
			NULL,
			0);
		if (myConfig.bSeparateTypes)
			pIcon->iGroup = 10;
	}
	
	pIcon->cName = g_key_file_get_string (pKeyFile, "Desktop Entry", "Name", &erreur);
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
	
	pIcon->cBaseURI = g_key_file_get_string (pKeyFile, "Desktop Entry", "Favicon", NULL);  // we use this parameter to store the favicon path; it's quite dirty, but easier than allocating our own data.
	
	pIcon->pAppletOwner = myApplet;
	
	return pIcon;
}

Icon *cd_stack_build_one_icon_from_file (GldiModuleInstance *myApplet, gchar *cDesktopFilePath)
{
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cDesktopFilePath);
	g_return_val_if_fail (pKeyFile != NULL, NULL);
	
	Icon *pIcon = cd_stack_build_one_icon (myApplet, pKeyFile);
	
	g_key_file_free (pKeyFile);
	return pIcon;
}

GList *cd_stack_build_icons_list (GldiModuleInstance *myApplet, gchar *cStackDir)
{
	// on parcourt tous les fichiers du repertoire.
	GDir *dir = g_dir_open (cStackDir, 0, NULL);
	g_return_val_if_fail (dir != NULL, NULL);
	
	GList *pIconsList = NULL;
	Icon *pIcon;
	GString *sDesktopFilePath = g_string_new ("");
	const gchar *cFileName;
	while ((cFileName = g_dir_read_name (dir)) != NULL)
	{
		g_string_printf (sDesktopFilePath, "%s/%s", cStackDir, cFileName);
		
		pIcon = cd_stack_build_one_icon_from_file (myApplet, sDesktopFilePath->str);
		if (pIcon != NULL)
		{
			pIcon->cDesktopFileName = g_strdup (cFileName);
			
			pIconsList = g_list_prepend (pIconsList, pIcon);
		}
	}
	
	g_string_free (sDesktopFilePath, TRUE);
	g_dir_close (dir);
	
	// on classe la liste.
	if (myConfig.iSortType == CD_STACK_SORT_BY_NAME)
	{
		pIconsList = g_list_sort (pIconsList, (GCompareFunc) cairo_dock_compare_icons_name);
		int i = 0;
		GList *ic;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			pIcon->fOrder = i ++;
		}
	}
	else if (myConfig.iSortType == CD_STACK_SORT_BY_TYPE)
	{
		pIconsList = g_list_sort (pIconsList, (GCompareFunc) cairo_dock_compare_icons_extension);  /// a defaut d'avoir les types mime...
		int i = 0;
		GList *ic;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			pIcon->fOrder = i ++;
		}
	}
	else
	{
		pIconsList = g_list_sort (pIconsList, (GCompareFunc) cairo_dock_compare_icons_order);
	}
	
	return pIconsList;
}

void cd_stack_build_icons (GldiModuleInstance *myApplet)
{
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On liste les icones.
	GList *pIconList = cd_stack_build_icons_list (myApplet, myConfig.cStackDir);
	
	//\_______________________ On charge la nouvelle liste.
	if (pIconList != NULL)
	{
		const gchar *cDeskletRendererName = NULL;
		switch (myConfig.iDeskletRendererType)
		{
			case CD_DESKLET_SLIDE :
			default :
				cDeskletRendererName = "Viewport";
			break ;
			
			case CD_DESKLET_TREE :
				cDeskletRendererName = "Tree";
			break ;
		}
		CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, cDeskletRendererName, NULL);
	}
	else if (myDock)  // sinon on ne veut pas du sous-dock vide.
	{
		gldi_object_unref (GLDI_OBJECT(myIcon->pSubDock));
		myIcon->pSubDock = NULL;
	}
}
