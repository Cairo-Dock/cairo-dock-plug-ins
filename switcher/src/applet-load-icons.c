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


static void _load_desktop_icon (Icon *pIcon)
{
	int iWidth = cairo_dock_icon_get_allocated_width (pIcon);
	int iHeight = cairo_dock_icon_get_allocated_height (pIcon);
	
	cairo_surface_t *pSurface = cairo_dock_duplicate_surface (myData.pDesktopBgMapSurface,
		myData.iSurfaceWidth, myData.iSurfaceHeight,
		iWidth, iHeight);
	cairo_dock_load_image_buffer_from_surface (&pIcon->image, pSurface, iWidth, iHeight);
}

static GList * _load_icons (void)
{
	GList *pIconList = NULL;
	int iIndex = cd_switcher_compute_index_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	Icon *pIcon;
	int i;
	for (i = 0; i < myData.switcher.iNbViewportTotal; i ++)
	{
		pIcon = cairo_dock_create_dummy_launcher (NULL,
			(myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER ? NULL : (myConfig.cDefaultIcon != NULL ? g_strdup (myConfig.cDefaultIcon) : g_strdup (MY_APPLET_SHARE_DATA_DIR"/default.svg"))),
			NULL,
			g_strdup_printf ("%d",i+1),
			i);
		if (i == iIndex)
		{
			pIcon->cName = g_strdup_printf ("%s (%d)", D_("Current"), i+1);
			pIcon->bHasIndicator = TRUE;
			pIcon->fAlpha = .7;
		}
		else
		{
			pIcon->cName = (i < myData.iNbNames ? g_strdup (myData.cDesktopNames[i]) : g_strdup_printf ("%s %d", D_("Desktop"), i+1));
			pIcon->bHasIndicator = FALSE;
			pIcon->fAlpha = 1.;
		}
		pIcon->cParentDockName = g_strdup (myIcon->cName);
		
		if (myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER)
		{
			pIcon->iface.load_image = _load_desktop_icon;
		}
		
		pIconList = g_list_append (pIconList, pIcon);
	}
	
	return pIconList;
}



void cd_switcher_load_icons (void)
{
	CD_APPLET_DELETE_MY_ICONS_LIST;
	cairo_surface_destroy (myData.pDesktopBgMapSurface);
	myData.pDesktopBgMapSurface = NULL;
	cairo_surface_destroy (myData.pDefaultMapSurface);
	myData.pDefaultMapSurface = NULL;
	
	if (myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER)
	{
		cd_switcher_load_desktop_bg_map_surface ();
	}
	if (myData.pDesktopBgMapSurface == NULL)
	{
		cd_switcher_load_default_map_surface ();
	}
	
	if (myConfig.bCompactView)
	{
		if (myIcon->pSubDock != NULL)  // si on est passe de expanded a compact, le sous-dock vide reste.
		{
			cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->cName);
			myIcon->pSubDock = NULL;
		}
		if (myDesklet)
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			myDesklet->render_bounding_box = cd_switcher_draw_desktops_bounding_box;  // pour le picking du bureau clique.
		}
	}
	else
	{
		//\_______________________ On cree la liste des icones de prevision.
		GList *pIconList = _load_icons ();
		
		//\_______________________ On charge la nouvelle liste.
		CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, "Slide", NULL);
		
		//\_______________________ On peint les icones.
		///cd_switcher_trigger_paint_icons ();
	}
}

/**void cd_switcher_paint_icons (void)
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
	cairo_dock_get_icon_extent (pFirstIcon, &iWidth, &iHeight);
	int _iWidth, _iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&_iWidth, &_iHeight);
	
	if (myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER)
	{
		cd_switcher_draw_main_icon();
		pSurface = myData.pDesktopBgMapSurface;
		fZoomX = (double)iWidth / _iWidth;
		fZoomY = (double)iHeight / _iHeight;
	
	}
	if (pSurface == NULL)
	{
		pSurface = myData.pDefaultMapSurface;
		fZoomX = (double)iWidth / _iWidth;
		fZoomY = (double)iHeight / _iHeight;
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

static gboolean _paint_icons (gpointer data)
{
	CD_APPLET_ENTER;
	if (! myConfig.bCompactView)
		cd_switcher_paint_icons ();
	myData.iSidPainIcons = 0;
	CD_APPLET_LEAVE (FALSE);
}
void cd_switcher_trigger_paint_icons (void)
{
	if (myData.iSidPainIcons != 0)  // on la lance en idle, car les icones sont chargees en idle.
		g_source_remove (myData.iSidPainIcons);
	myData.iSidPainIcons = g_idle_add ((GSourceFunc)_paint_icons, NULL);
}*/


void cd_switcher_load_desktop_bg_map_surface (void)
{
	// get the background surface
	CairoDockDesktopBackground *db = cairo_dock_get_desktop_background (FALSE);  // FALSE <=> sans texture.
	cairo_surface_t *pBgSurface = cairo_dock_get_desktop_bg_surface (db);
	
	if (myData.pDesktopBgMapSurface != NULL)
		cairo_surface_destroy (myData.pDesktopBgMapSurface);
	if (pBgSurface == NULL)
	{
		cd_warning ("couldn't get the wallpaper");
		myData.pDesktopBgMapSurface = NULL;
		cairo_dock_destroy_desktop_background (db);
		return ;
	}
	
	// load it at a decent size.
	if (myDock)
	{
		CD_APPLET_GET_MY_ICON_EXTENT (&myData.iSurfaceWidth, &myData.iSurfaceHeight);
	}
	else
	{
		myData.iSurfaceWidth = MAX (1, myContainer->iWidth / myData.switcher.iNbViewportTotal);
		myData.iSurfaceHeight = MAX (1, myContainer->iHeight / myData.switcher.iNbViewportTotal);
	}
	//g_print ("%s (%dx%d)\n", __func__, myData.iSurfaceWidth, myData.iSurfaceHeight);
	
	myData.pDesktopBgMapSurface = cairo_dock_duplicate_surface (pBgSurface,
		g_desktopGeometry.Xscreen.width, g_desktopGeometry.Xscreen.height,
		myData.iSurfaceWidth, myData.iSurfaceHeight);
	
	cairo_dock_destroy_desktop_background (db);
}

void cd_switcher_load_default_map_surface (void)
{
	if (myData.pDefaultMapSurface != NULL)
		cairo_surface_destroy (myData.pDefaultMapSurface);
	if (myDock)
	{
		CD_APPLET_GET_MY_ICON_EXTENT (&myData.iSurfaceWidth, &myData.iSurfaceHeight);
	}
	else
	{
		myData.iSurfaceWidth = MAX (1, myContainer->iWidth / myData.switcher.iNbViewportTotal);
		myData.iSurfaceHeight = MAX (1, myContainer->iHeight / myData.switcher.iNbViewportTotal);
	}
	cd_debug ("%s (%dx%d)", __func__, myData.iSurfaceWidth, myData.iSurfaceHeight);
	if (myConfig.iIconDrawing == SWICTHER_MAP_COLOUR)
	{
		// create a surface and paint the Bg color
		myData.pDefaultMapSurface = cairo_dock_create_blank_surface (
				myData.iSurfaceWidth, myData.iSurfaceHeight);
		cairo_t *pImageContext = cairo_create (myData.pDefaultMapSurface);
		cairo_set_source_rgba (pImageContext,
			myConfig.RGBBgColors[0],
			myConfig.RGBBgColors[1],
			myConfig.RGBBgColors[2],
			myConfig.RGBBgColors[3]);
		cairo_paint (pImageContext);
		cairo_destroy (pImageContext);
	}
	else
		myData.pDefaultMapSurface = cairo_dock_create_surface_from_image_simple (myConfig.cDefaultIcon,
			myData.iSurfaceWidth,
			myData.iSurfaceHeight);
}
