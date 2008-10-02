/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>

#include "applet-struct.h"
#include "applet-desktops.h"
#include "applet-load-icons.h"
#include "applet-draw.h"
CD_APPLET_INCLUDE_MY_VARS


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
			pIcon->acName = g_strdup_printf ("%s (%d)", D_("Current"), i+1);
			pIcon->bHasIndicator = TRUE;
			pIcon->fAlpha = .7;
		}
		else
		{
			pIcon->acName = g_strdup_printf ("%s %d", D_("Desktop"), i+1);
			pIcon->bHasIndicator = FALSE;
			pIcon->fAlpha = 1.;
		}
		pIcon->cQuickInfo = g_strdup_printf ("%d",i+1);
		pIcon->fOrder = i;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->acCommand = g_strdup ("none");
		pIcon->cParentDockName = g_strdup (myIcon->acName);
		pIcon->acFileName = (myConfig.bMapWallpaper ? NULL : (myConfig.cDefaultIcon != NULL ? g_strdup (myConfig.cDefaultIcon) : g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "default.svg")));
		
		pIconList = g_list_append (pIconList, pIcon);
	}
	
	return pIconList;
}



void cd_switcher_load_icons (void)
{
	if (myConfig.bCompactView)
	{
		if (myIcon->pSubDock != NULL)
		{
			CD_APPLET_DESTROY_MY_SUBDOCK;
		}
		if (myDesklet)
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		cd_switcher_load_default_map_surface ();
		
		cd_message ("SWITCHER : chargement de l'icone Switcher sans sous-dock");
	}
	else
	{
		//\_______________________ On cree la liste des icones de prevision.
		GList *pIconList = _load_icons ();
		
		//\_______________________ On efface l'ancienne liste.
		if (myIcon->pSubDock != NULL)
		{
			g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
		}
		
		//\_______________________ On charge la nouvelle liste.
		if (myDock)
		{
			if (myIcon->pSubDock == NULL)
			{
				if (pIconList != NULL)
				{
					CD_APPLET_CREATE_MY_SUBDOCK (pIconList, myConfig.cRenderer);
				}
			}
			else  // on a deja notre sous-dock, on remplace juste ses icones.
			{
				if (pIconList == NULL)  // inutile de le garder.
				{
					CD_APPLET_DESTROY_MY_SUBDOCK;
					return ;
				}
				else
				{
					CD_APPLET_LOAD_ICONS_IN_MY_SUBDOCK (pIconList);
				}
			}
		}
		else
		{
			if (myIcon->pSubDock != NULL)
			{
				CD_APPLET_DESTROY_MY_SUBDOCK;
			}
			if (myDesklet->icons != NULL)  // si on recharge la liste a cause d'un changement de configuration du bureau.
			{
				g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
				g_list_free (myDesklet->icons);
			}
			myDesklet->icons = pIconList;
			
			gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", pConfig);
		}
		
		//\_______________________ On peint les icones.
		cd_switcher_paint_icons ();
	}
}

void cd_switcher_paint_icons (void)
{
	if (myConfig.bCompactView)
		return ;
	
	//\_______________________ On applique la surface.
	CairoContainer *pContainer = (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer);
	GList *pIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	
	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	Icon *pFirstIcon = pIconList->data;
	
	if (myConfig.bMapWallpaper)
	{
		cd_switcher_draw_main_icon();
		pSurface = cairo_dock_get_desktop_bg_surface ();
		double fMaxScale = cairo_dock_get_max_scale (pContainer);
		fZoomX = (double) pFirstIcon->fWidth * fMaxScale / g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
		fZoomY = (double) pFirstIcon->fHeight * fMaxScale / g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];

	}
	if (pSurface == NULL)
	{
		cd_switcher_load_default_map_surface ();
		pSurface = myData.pDefaultMapSurface;
		fZoomX = pFirstIcon->fWidth / myIcon->fWidth;
		fZoomY = pFirstIcon->fHeight / myIcon->fHeight;
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
