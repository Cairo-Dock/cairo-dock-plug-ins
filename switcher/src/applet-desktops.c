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

#include <math.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-draw.h"
#include "applet-desktops.h"


void cd_switcher_get_current_desktop (void)
{
	cairo_dock_get_current_desktop_and_viewport (&myData.switcher.iCurrentDesktop, &myData.switcher.iCurrentViewportX, &myData.switcher.iCurrentViewportY);
	//g_print ("%s () -> %d;%d;%d\n", __func__, myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	myData.switcher.iNbViewportTotal = g_desktopGeometry.iNbDesktops * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY;
	
	cd_switcher_compute_desktop_coordinates (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY, &myData.switcher.iCurrentLine, &myData.switcher.iCurrentColumn);
}



static void _cd_switcher_get_best_agencement (int iNbViewports, int *iBestNbLines, int *iBestNbColumns)
{
	g_return_if_fail (iNbViewports != 0);
	//g_print ("%s (%d)\n", __func__, iNbViewports);
	double fZoomX, fZoomY;
	int iNbLines, iNbDesktopByLine;
	int Nx, Ny;
	
	if (myConfig.bPreserveScreenRatio)  // on va chercher a minimiser la deformation de l'image de fond d'ecran.
	{
		double fRatio, fMinRatio=9999;
		for (iNbLines = 1; iNbLines <= iNbViewports; iNbLines ++)
		{
			if (iNbViewports % iNbLines != 0)
				continue;
			iNbDesktopByLine = iNbViewports / iNbLines;
			fZoomX = myIcon->fWidth / (iNbDesktopByLine * g_desktopGeometry.iXScreenWidth[CAIRO_DOCK_HORIZONTAL]);
			fZoomY = myIcon->fHeight / (iNbLines * g_desktopGeometry.iXScreenHeight[CAIRO_DOCK_HORIZONTAL]);
			fRatio = (fZoomX > fZoomY ? fZoomX / fZoomY : fZoomY / fZoomX);  // ratio ramene dans [1, inf].
			//cd_debug ("%d lignes => fRatio: %.2f", iNbLines, fRatio);
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
			*iBestNbLines = (int) ceil ((double)iNbViewports / (*iBestNbColumns));
		}
		else
		{
			*iBestNbLines = (int) ceil (sqrt (iNbViewports));
			*iBestNbColumns = (int) ceil ((double)iNbViewports / (*iBestNbLines));
		}
	}
	
}
void cd_switcher_compute_nb_lines_and_columns (void)
{
	if (g_desktopGeometry.iNbDesktops > 1)  // plusieurs bureaux simples (Metacity) ou etendus (Compiz avec 2 cubes).
	{
		if (g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY > 1)  // plusieurs bureaux etendus (Compiz avec N cubes).
		{
			myData.switcher.iNbLines = g_desktopGeometry.iNbDesktops;  // on respecte l'agencement de l'utilisateur (groupement par bureau).
			myData.switcher.iNbColumns = g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY;
		}
		else  // plusieurs bureaux simples (Metacity)
		{
			_cd_switcher_get_best_agencement (g_desktopGeometry.iNbDesktops, &myData.switcher.iNbLines, &myData.switcher.iNbColumns);
		}
	}
	else  // un seul bureau etendu.
	{
		if (g_desktopGeometry.iNbViewportY > 1)  // desktop wall.
		{
			myData.switcher.iNbLines = g_desktopGeometry.iNbViewportY;  // on respecte l'agencement de l'utilisateur.
			myData.switcher.iNbColumns = g_desktopGeometry.iNbViewportX;
		}
		else  // cube.
		{
			_cd_switcher_get_best_agencement (g_desktopGeometry.iNbViewportX, &myData.switcher.iNbLines, &myData.switcher.iNbColumns);
		}
	}
	myData.iPrevIndexHovered = -1;  // cela invalide le dernier bureau survole.
}


void cd_switcher_compute_desktop_coordinates (int iNumDesktop, int iNumViewportX, int iNumViewportY, int *iNumLine, int *iNumColumn)
{
	g_return_if_fail (myData.switcher.iNbColumns != 0);
	//cd_debug ("%s (%d;%d)", __func__, iNumViewportX, iNumViewportY);
	if (g_desktopGeometry.iNbDesktops > 1)  // plusieurs bureaux simples (Metacity) ou etendus (Compiz avec 2 cubes).
	{
		if (g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY > 1)  // plusieurs bureaux etendus (Compiz avec N cubes).
		{
			*iNumLine = iNumDesktop;
			*iNumColumn = iNumViewportY * g_desktopGeometry.iNbViewportX + iNumViewportX;
		}
		else  // plusieurs bureaux simples (Metacity)
		{
			*iNumLine = iNumDesktop / myData.switcher.iNbColumns;
			*iNumColumn = iNumDesktop % myData.switcher.iNbColumns;
		}
	}
	else  // un seul bureau etendu.
	{
		if (g_desktopGeometry.iNbViewportY > 1)  // desktop wall.
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
	if (g_desktopGeometry.iNbDesktops > 1)  // plusieurs bureaux simples (Metacity) ou etendus (Compiz avec 2 cubes).
	{
		if (g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY > 1)  // plusieurs bureaux etendus (Compiz avec N cubes).
		{
			*iNumDesktop = iNumLine;
			*iNumViewportX = iNumColumn % g_desktopGeometry.iNbViewportX;
			*iNumViewportY = iNumColumn / g_desktopGeometry.iNbViewportX;
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
		if (g_desktopGeometry.iNbViewportY > 1)  // desktop wall.
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
	return iNumDesktop * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY + iNumViewportX * g_desktopGeometry.iNbViewportY + iNumViewportY;
}

void cd_switcher_compute_viewports_from_index (int iIndex, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY)
{
	if (g_desktopGeometry.iNbViewportX == 0 || g_desktopGeometry.iNbViewportY == 0)  // des fois (chgt de resolution sous Compiz), le rafraichissement se passe mal, on le force donc ici pour eviter une division par 0.
	{
		cd_switcher_refresh_desktop_values (myApplet);
	}
	g_return_if_fail (g_desktopGeometry.iNbViewportX > 0 && g_desktopGeometry.iNbViewportY > 0);
	
	*iNumDesktop = iIndex / (g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY);
	int index2 = iIndex % (g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY);
	*iNumViewportX = index2 / g_desktopGeometry.iNbViewportY;
	*iNumViewportY = index2 % g_desktopGeometry.iNbViewportY;
	//g_print (" -> %d;%d;%d\n", *iNumDesktop, *iNumViewportX, *iNumViewportY);
}


static void cd_switcher_change_nb_desktops (int iDeltaNbDesktops)
{
	if (g_desktopGeometry.iNbDesktops >= g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY)
	{
		cairo_dock_set_nb_desktops (g_desktopGeometry.iNbDesktops + iDeltaNbDesktops);
	}
	else
	{
		if (g_desktopGeometry.iNbViewportX >= g_desktopGeometry.iNbViewportY)
			cairo_dock_set_nb_viewports (g_desktopGeometry.iNbViewportX + iDeltaNbDesktops, g_desktopGeometry.iNbViewportY);
		else
			cairo_dock_set_nb_viewports (g_desktopGeometry.iNbViewportX, g_desktopGeometry.iNbViewportY + iDeltaNbDesktops);
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
	g_desktopGeometry.iNbDesktops = cairo_dock_get_nb_desktops ();
	cairo_dock_get_nb_viewports (&g_desktopGeometry.iNbViewportX, &g_desktopGeometry.iNbViewportY);
	cd_switcher_update_from_screen_geometry ();
	myData.iSidAutoRefresh = 0;
	return FALSE;
}



static void _cd_switcher_action_on_one_window_from_viewport (Icon *pIcon, CairoContainer *pContainer, gpointer *data)
{
	int iNumDesktop = GPOINTER_TO_INT (data[0]);
	int iNumViewportX = GPOINTER_TO_INT (data[1]);
	int iNumViewportY = GPOINTER_TO_INT (data[2]);
	CDSwitcherActionOnViewportFunc pFunction = data[3];
	gpointer pUserData = data[4];
	
	if (! cairo_dock_appli_is_on_desktop (pIcon, iNumDesktop, iNumViewportX, iNumViewportY))
		return ;
	
	pFunction (pIcon, iNumDesktop, iNumViewportX, iNumViewportY, pUserData);
}
void cd_switcher_foreach_window_on_viewport (int iNumDesktop, int iNumViewportX, int iNumViewportY, CDSwitcherActionOnViewportFunc pFunction, gpointer pUserData)
{
	gpointer data[5] = {GINT_TO_POINTER (iNumDesktop), GINT_TO_POINTER (iNumViewportX), GINT_TO_POINTER (iNumViewportY), pFunction, pUserData};
	cairo_dock_foreach_applis ((CairoDockForeachIconFunc) _cd_switcher_action_on_one_window_from_viewport, FALSE, data);
}
