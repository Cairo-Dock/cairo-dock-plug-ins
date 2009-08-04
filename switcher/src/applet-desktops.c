/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Cchumi & Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <math.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-draw.h"
#include "applet-desktops.h"


void cd_switcher_get_current_desktop (void)
{
	myData.switcher.iCurrentDesktop = cairo_dock_get_current_desktop ();
	
	int iCurrentViewportX, iCurrentViewportY;
	cairo_dock_get_current_viewport (&iCurrentViewportX, &iCurrentViewportY);
	myData.switcher.iCurrentViewportX = iCurrentViewportX / g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
	myData.switcher.iCurrentViewportY = iCurrentViewportY / g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
	
	cd_debug ("%s () -> %d;%d;%d", __func__, myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	myData.switcher.iNbViewportTotal = g_iNbDesktops * g_iNbViewportX * g_iNbViewportY;
	
	cd_switcher_compute_desktop_coordinates (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY, &myData.switcher.iCurrentLine, &myData.switcher.iCurrentColumn);
}



static void _cd_switcher_get_best_agencement (int iNbViewports, int *iBestNbLines, int *iBestNbColumns)
{
	double fZoomX, fZoomY;
	int iNbLines, iNbDesktopByLine;
	
	if (myConfig.bPreserveScreenRatio)  // on va chercher a minimiser la deformation de l'image de fond d'ecran.
	{
		/*double fZoom, fUsedSurface, fMaxUsedSurface=0;
		for (iNbLines = 1; iNbLines <= iNbViewports; iNbLines ++)
		{
			if (iNbViewports % iNbLines != 0)
				continue;
			iNbDesktopByLine = iNbViewports / iNbLines;
			fZoomX = myIcon->fWidth / (iNbDesktopByLine * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]);
			fZoomY = myIcon->fHeight / (iNbLines * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]);
			fZoom = MIN (fZoomX, fZoomY);  // zoom qui conserve le ratio.
			fUsedSurface = (fZoom * iNbDesktopByLine * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]) * (fZoom * iNbLines * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]);
			g_print ("%d lignes => fUsedSurface: %.2f pix^2\n", iNbLines, fUsedSurface);
			
			if (fUsedSurface > fMaxUsedSurface)
			{
				fMaxUsedSurface= fUsedSurface;
				*iBestNbColumns = iNbDesktopByLine;
				*iBestNbLines = iNbLines;
			}
		}*/
		double fRatio, fMinRatio=9999;
		for (iNbLines = 1; iNbLines <= iNbViewports; iNbLines ++)
		{
			if (iNbViewports % iNbLines != 0)
				continue;
			iNbDesktopByLine = iNbViewports / iNbLines;
			fZoomX = myIcon->fWidth / (iNbDesktopByLine * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]);
			fZoomY = myIcon->fHeight / (iNbLines * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]);
			fRatio = (fZoomX > fZoomY ? fZoomX / fZoomY : fZoomY / fZoomX);  // ratio ramene dans [1, inf].
			cd_debug ("%d lignes => fRatio: %.2f", iNbLines, fRatio);
			if (fRatio < fMinRatio)
			{
				fMinRatio = fRatio;
				*iBestNbColumns = iNbDesktopByLine;
				*iBestNbLines = iNbLines;
			}
		}
	}
	else  // on va chercher a repartir au mieux les bureaux sur l'icone.
	{
		if (myIcon->fWidth >= myIcon->fHeight)
		{
			*iBestNbColumns = (int) ceil (sqrt (iNbViewports));
			*iBestNbLines = iNbViewports / (*iBestNbColumns);
		}
		else
		{
			*iBestNbLines = (int) ceil (sqrt (iNbViewports));
			*iBestNbColumns = iNbViewports / (*iBestNbLines);
		}
	}
}
void cd_switcher_compute_nb_lines_and_columns (void)
{
	if (g_iNbDesktops > 1)  // plusieurs bureaux simples (Metacity) ou etendus (Compiz avec 2 cubes).
	{
		if (g_iNbViewportX * g_iNbViewportY > 1)  // plusieurs bureaux etendus (Compiz avec N cubes).
		{
			myData.switcher.iNbLines = g_iNbDesktops;  // on respecte l'agencement de l'utilisateur (groupement par bureau).
			myData.switcher.iNbColumns = g_iNbViewportX * g_iNbViewportY;
		}
		else  // plusieurs bureaux simples (Metacity)
		{
			_cd_switcher_get_best_agencement (g_iNbDesktops, &myData.switcher.iNbLines, &myData.switcher.iNbColumns);
		}
	}
	else  // un seul bureau etendu.
	{
		if (g_iNbViewportY > 1)  // desktop wall.
		{
			myData.switcher.iNbLines = g_iNbViewportY;  // on respecte l'agencement de l'utilisateur.
			myData.switcher.iNbColumns = g_iNbViewportX;
		}
		else  // cube.
		{
			_cd_switcher_get_best_agencement (g_iNbViewportX, &myData.switcher.iNbLines, &myData.switcher.iNbColumns);
		}
	}
}


void cd_switcher_compute_desktop_coordinates (int iNumDesktop, int iNumViewportX, int iNumViewportY, int *iNumLine, int *iNumColumn)
{
	cd_debug ("%s (%d;%d)", __func__, iNumViewportX, iNumViewportY);
	if (g_iNbDesktops > 1)  // plusieurs bureaux simples (Metacity) ou etendus (Compiz avec 2 cubes).
	{
		if (g_iNbViewportX * g_iNbViewportY > 1)  // plusieurs bureaux etendus (Compiz avec N cubes).
		{
			*iNumLine = iNumDesktop;
			*iNumColumn = iNumViewportY * g_iNbViewportX + iNumViewportX;
		}
		else  // plusieurs bureaux simples (Metacity)
		{
			*iNumLine = iNumDesktop / myData.switcher.iNbColumns;
			*iNumColumn = iNumDesktop % myData.switcher.iNbColumns;
		}
	}
	else  // un seul bureau etendu.
	{
		if (g_iNbViewportY > 1)  // desktop wall.
		{
			*iNumLine = iNumViewportY;
			*iNumColumn = iNumViewportX;
		}
		else  // cube.
		{
			*iNumLine = iNumViewportX / myData.switcher.iNbColumns;
			*iNumColumn = iNumViewportX % myData.switcher.iNbColumns;
		}
	}
}


void cd_switcher_compute_desktop_from_coordinates (int iNumLine, int iNumColumn, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY)
{
	if (g_iNbDesktops > 1)  // plusieurs bureaux simples (Metacity) ou etendus (Compiz avec 2 cubes).
	{
		if (g_iNbViewportX * g_iNbViewportY > 1)  // plusieurs bureaux etendus (Compiz avec N cubes).
		{
			*iNumDesktop = iNumLine;
			*iNumViewportX = iNumColumn % g_iNbViewportX;
			*iNumViewportY = iNumColumn / g_iNbViewportX;
		}
		else  // plusieurs bureaux simples (Metacity)
		{
			*iNumDesktop = iNumLine * myData.switcher.iNbColumns +iNumColumn;
			*iNumViewportX = 0;
			*iNumViewportY = 0;
		}
	}
	else  // un seul bureau etendu.
	{
		*iNumDesktop = 0;
		if (g_iNbViewportY > 1)  // desktop wall.
		{
			*iNumViewportX = iNumColumn;
			*iNumViewportY = iNumLine;
		}
		else  // cube.
		{
			*iNumViewportX = iNumLine * myData.switcher.iNbColumns +iNumColumn;
			*iNumViewportY = 0;
		}
	}
}


int cd_switcher_compute_index (int iNumDesktop, int iNumViewportX, int iNumViewportY)
{
	return iNumDesktop * g_iNbViewportX * g_iNbViewportY + iNumViewportX * g_iNbViewportY + iNumViewportY;
}

void cd_switcher_compute_viewports_from_index (int iIndex, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY)
{
	if (g_iNbViewportX == 0 || g_iNbViewportY == 0)  // des fois (chgt de resolution sous Compiz), le rafraichissement se passe mal, on le force donc ici pour eviter une division par 0.
	{
		cd_switcher_refresh_desktop_values (myApplet);
	}
	g_return_if_fail (g_iNbViewportX > 0 && g_iNbViewportY > 0);
	
	*iNumDesktop = iIndex / (g_iNbViewportX * g_iNbViewportY);
	int index2 = iIndex % (g_iNbViewportX * g_iNbViewportY);
	*iNumViewportX = index2 / g_iNbViewportY;
	*iNumViewportY = index2 % g_iNbViewportY;
	g_print (" -> %d;%d;%d\n", *iNumDesktop, *iNumViewportX, *iNumViewportY);
}


static void cd_switcher_change_nb_desktops (int iDeltaNbDesktops)
{
	if (g_iNbDesktops >= g_iNbViewportX * g_iNbViewportY)
	{
		cairo_dock_set_nb_desktops (g_iNbDesktops + iDeltaNbDesktops);
	}
	else
	{
		if (g_iNbViewportX >= g_iNbViewportY)
			cairo_dock_set_nb_viewports (g_iNbViewportX + iDeltaNbDesktops, g_iNbViewportY);
		else
			cairo_dock_set_nb_viewports (g_iNbViewportX, g_iNbViewportY + iDeltaNbDesktops);
	}
}

void cd_switcher_add_a_desktop (void)
{
	cd_switcher_change_nb_desktops (+1);
}

void cd_switcher_remove_last_desktop (void)
{
	cd_switcher_change_nb_desktops (-1);
}


void cd_switcher_update_from_screen_geometry (void)
{
	//\___________________ On calcule la geometrie de l'icone en mode compact.
	cd_switcher_compute_nb_lines_and_columns ();
	
	//\___________________ On recupere le bureau courant et sa position sur la grille.
	cd_switcher_get_current_desktop ();
	
	//\___________________ On charge le bon nombre d'icones dans le sous-dock ou le desklet.
	cd_switcher_load_icons ();
	
	//\___________________ On dessine l'icone principale.
	cd_switcher_draw_main_icon ();
}

gboolean cd_switcher_refresh_desktop_values (CairoDockModuleInstance *myApplet)
{
	g_iNbDesktops = cairo_dock_get_nb_desktops ();
	cairo_dock_get_nb_viewports (&g_iNbViewportX, &g_iNbViewportY);
	cd_switcher_update_from_screen_geometry ();
	myData.iSidAutoRefresh = 0;
	return FALSE;
}
