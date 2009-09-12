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
#include "applet-desktops.h"
#include "applet-draw.h"
#include "applet-load-icons.h"


static GList * _load_icons (void)
{
	GList *pIconList = NULL;
	int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	Icon *pIcon;
	int i;
	for (i = 0; i < myData.switcher.iNbViewportTotal; i ++)
	{
		pIcon = g_new0 (Icon, 1);
		
		if (i == iIndex)
		{
			pIcon->cName = g_strdup_printf ("%s (%d)", D_("Current"), i+1);
			pIcon->bHasIndicator = TRUE;
			pIcon->fAlpha = .7;
		}
		else
		{
			pIcon->cName = g_strdup_printf ("%s %d", D_("Desktop"), i+1);
			pIcon->bHasIndicator = FALSE;
			pIcon->fAlpha = 1.;
		}
		pIcon->cQuickInfo = g_strdup_printf ("%d",i+1);
		pIcon->fOrder = i;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->cCommand = g_strdup ("none");
		pIcon->cParentDockName = g_strdup (myIcon->cName);
		pIcon->cFileName = (myConfig.bMapWallpaper ? NULL : (myConfig.cDefaultIcon != NULL ? g_strdup (myConfig.cDefaultIcon) : g_strdup (MY_APPLET_SHARE_DATA_DIR"/default.svg")));
		
		pIconList = g_list_append (pIconList, pIcon);
	}
	
	return pIconList;
}



void cd_switcher_load_icons (void)
{
	CD_APPLET_DELETE_MY_ICONS_LIST;
	if (myConfig.bCompactView)
	{
		if (myIcon->pSubDock != NULL)  // si on est passe de expanded a compact, le sous-dock vide reste.
		{
			CD_APPLET_DESTROY_MY_SUBDOCK;
		}
		if (myDesklet)
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			myDesklet->render_bounding_box = cd_switcher_draw_desktops_bounding_box;  // pour le picking du bureau clique.
		}
		cd_switcher_load_default_map_surface ();
		
		cd_message ("SWITCHER : chargement de l'icone Switcher sans sous-dock");
	}
	else
	{
		//\_______________________ On cree la liste des icones de prevision.
		GList *pIconList = _load_icons ();
		
		//\_______________________ On charge la nouvelle liste.
		gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
		CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, "Caroussel", pConfig);
		
		//\_______________________ On peint les icones.
		cd_switcher_paint_icons ();
	}
}

void cd_switcher_paint_icons (void)
{
	//\_______________________ On applique la surface.
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	GList *pIconList = CD_APPLET_MY_ICONS_LIST;
	if (pIconList == NULL)
		return ;
	
	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	Icon *pFirstIcon = pIconList->data;
	
	int iWidth, iHeight;
	cairo_dock_get_icon_extent (pFirstIcon, pContainer, &iWidth, &iHeight);
	
	if (myConfig.bMapWallpaper)
	{
		cd_switcher_draw_main_icon();
		pSurface = cairo_dock_get_desktop_bg_surface ();
		double fMaxScale = cairo_dock_get_max_scale (pContainer);
		fZoomX = 1. * iWidth / g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
		fZoomY = 1. * iHeight / g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];

	}
	if (pSurface == NULL)
	{
		int _iWidth, _iHeight;
		CD_APPLET_GET_MY_ICON_EXTENT (&_iWidth, &_iHeight);
		cd_switcher_load_default_map_surface ();
		pSurface = myData.pDefaultMapSurface;
		fZoomX = 1. * iWidth / _iWidth;
		fZoomY = 1. * iHeight / _iHeight;
	}
	
	cairo_t *pIconContext;
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		//g_print (" (%.2f; %.2f) %.2fx%.2f\n", icon->fDrawX, icon->fDrawY, icon->fWidth, icon->fHeight);
		pIconContext = cairo_create (icon->pIconBuffer);
		cairo_scale (pIconContext,
			fZoomX,
			fZoomY);

		cairo_dock_set_icon_surface_with_reflect (pIconContext, pSurface, icon, pContainer);
		cairo_destroy (pIconContext);
	}
}


void cd_switcher_load_default_map_surface (void)
{
	g_return_if_fail (myDrawContext != NULL);
	if (myData.pDefaultMapSurface != NULL)
		cairo_surface_destroy (myData.pDefaultMapSurface);
	cd_debug ("%s (%.2fx%.2f)", __func__, myIcon->fWidth, myIcon->fHeight);
	myData.pDefaultMapSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (myConfig.cDefaultIcon);
}
