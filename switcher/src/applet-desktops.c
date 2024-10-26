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

static void cd_switcher_compute_coordinates_from_index (int iIndex, int *iNumLine, int *iNumColumn);
static int cd_switcher_compute_index_from_coordinates (int iNumLine, int iNumColumn);


void cd_switcher_get_current_desktop (void)
{
	gldi_desktop_get_current (&myData.switcher.iCurrentDesktop, &myData.switcher.iCurrentViewportX, &myData.switcher.iCurrentViewportY);
	
	myData.switcher.iNbViewportTotal = g_desktopGeometry.iNbDesktops * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY;
	if (myData.switcher.iNbViewportTotal == 0) // obviously, having 0 desktop cannot be true, so we force to 1 to avoid any "division by 0" later.
		myData.switcher.iNbViewportTotal = 1;
	
	cd_switcher_compute_coordinates_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY, &myData.switcher.iCurrentLine, &myData.switcher.iCurrentColumn);
	
	cd_debug ("desktop: %d;%d;%d, %dx%d", g_desktopGeometry.iNbDesktops, g_desktopGeometry.iNbViewportX, g_desktopGeometry.iNbViewportY, myData.switcher.iCurrentLine, myData.switcher.iCurrentColumn);
}


static void _cd_switcher_get_best_agencement (int iNbViewports, int *iBestNbLines, int *iBestNbColumns)
{
	*iBestNbLines = 0;
	*iBestNbColumns = 0;
	g_return_if_fail (iNbViewports != 0);
	//g_print ("%s (%d)\n", __func__, iNbViewports);
	
	int w, h;
	if (myConfig.bCompactView)
	{
		CD_APPLET_GET_MY_ICON_EXTENT (&w, &h);
		if (w == 0 || h == 0)  // may happen in desklet mode on startup, until the desklet's window reaches its definite size.
			return;
	}
	else  // in expanded mode, we don't use the grid, except for computing an index, so any size will be ok.
	{
		w = h = 48;
	}
	//cd_debug ("%d; %dx%d", iNbViewports, w, h);
	
	double fZoomX, fZoomY;
	int iNbLines, iNbDesktopByLine;
	double fZoom, fZoomMax=0.;
	for (iNbLines = 1; iNbLines <= iNbViewports; iNbLines ++)
	{
		iNbDesktopByLine = ceil ((double)iNbViewports / iNbLines);
		
		fZoomX = (double)w / (iNbDesktopByLine * g_desktopGeometry.Xscreen.width);
		fZoomY = (double)h / (iNbLines * g_desktopGeometry.Xscreen.height);
		fZoom = MIN (fZoomX, fZoomY);  // on preserve le ratio
		//cd_debug ("%d lignes => iNbDesktopByLine: %d, zooms: %.3f,%.3f", iNbLines, iNbDesktopByLine, fZoomX, fZoomY);
		if (fZoom > fZoomMax)
		{
			fZoomMax = fZoom;
			*iBestNbColumns = iNbDesktopByLine;
			*iBestNbLines = iNbLines;
		}
		else if (fabs (fZoom - fZoomMax) < 1e-4)  // same zoom, the solution which minimizes the number of squares is better.
		{
			if (iNbLines * iNbDesktopByLine < *iBestNbColumns * *iBestNbLines)
			{
				*iBestNbColumns = iNbDesktopByLine;
				*iBestNbLines = iNbLines;	
			}
		}
	}
}
void cd_switcher_compute_nb_lines_and_columns (void)
{
	if (myConfig.iDesktopsLayout == SWICTHER_LAYOUT_AUTO)
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
	}
	else  // force the layout on N lines.
	{
		int w, h;
		CD_APPLET_GET_MY_ICON_EXTENT (&w, &h);
		if (w >= h)  // icon is wide.
		{
			myData.switcher.iNbLines = myConfig.iDesktopsLayout;  // single-line, 2-lines, etc. Note: currently the config window only gives us access to the "single-line" choice, we might add more.
			myData.switcher.iNbColumns = ceil ((double) g_desktopGeometry.iNbDesktops * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY / myData.switcher.iNbLines);
		}
		else  // icon is high
		{
			myData.switcher.iNbColumns = myConfig.iDesktopsLayout;
			myData.switcher.iNbLines = ceil ((double) g_desktopGeometry.iNbDesktops * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY / myData.switcher.iNbColumns);
		}
	}
	myData.iPrevIndexHovered = -1;  // cela invalide le dernier bureau survole.
}


void cd_switcher_compute_coordinates_from_desktop (int iNumDesktop, int iNumViewportX, int iNumViewportY, int *iNumLine, int *iNumColumn)
{
	if (myData.switcher.iNbColumns == 0)  // may happen in desklet mode with a cube desktop, when the desklet is still 0x0.
	{
		*iNumLine = 0;
		*iNumColumn = 0;
		return;
	}
	int index = cd_switcher_compute_index_from_desktop (iNumDesktop, iNumViewportX, iNumViewportY);
	cd_switcher_compute_coordinates_from_index (index, iNumLine, iNumColumn);
	cd_debug ("(%d;%d;%d) -> %d -> (%d;%d)", iNumDesktop, iNumViewportX, iNumViewportY, index, *iNumLine, *iNumColumn);
	/**if (g_desktopGeometry.iNbDesktops > 1)  // plusieurs bureaux simples (Metacity) ou etendus (Compiz avec 2 cubes).
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
	}*/
}


void cd_switcher_compute_desktop_from_coordinates (int iNumLine, int iNumColumn, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY)
{
	int index = cd_switcher_compute_index_from_coordinates (iNumLine, iNumColumn);
	cd_switcher_compute_desktop_from_index (index, iNumDesktop, iNumViewportX, iNumViewportY);
	cd_debug ("(%d;%d) -> %d -> (%d;%d;%d)", iNumLine, iNumColumn, index, *iNumDesktop, *iNumViewportX, *iNumViewportY);
	/**if (g_desktopGeometry.iNbDesktops > 1)  // plusieurs bureaux simples (Metacity) ou etendus (Compiz avec 2 cubes).
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
	}*/
}


int cd_switcher_compute_index_from_desktop (int iNumDesktop, int iNumViewportX, int iNumViewportY)
{
	return iNumDesktop * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY + iNumViewportX + iNumViewportY * g_desktopGeometry.iNbViewportX;
}

int cd_switcher_compute_index_from_coordinates (int iNumLine, int iNumColumn)
{
	return iNumLine * myData.switcher.iNbColumns + iNumColumn;
}

void cd_switcher_compute_desktop_from_index (int iIndex, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY)
{
	if (g_desktopGeometry.iNbViewportX == 0 || g_desktopGeometry.iNbViewportY == 0)  // des fois (chgt de resolution sous Compiz), le rafraichissement se passe mal, on le force donc ici pour eviter une division par 0.
	{
		cd_switcher_refresh_desktop_values (myApplet);
	}
	g_return_if_fail (g_desktopGeometry.iNbViewportX > 0 && g_desktopGeometry.iNbViewportY > 0);
	
	*iNumDesktop = iIndex / (g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY);
	int index2 = iIndex % (g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY);
	*iNumViewportX = index2 % g_desktopGeometry.iNbViewportX;
	*iNumViewportY = index2 / g_desktopGeometry.iNbViewportX;
	cd_debug ("%d -> (%d, %d, %d) ; nX=%d ; nY=%d", iIndex, *iNumDesktop, *iNumViewportX, *iNumViewportY, g_desktopGeometry.iNbViewportX, g_desktopGeometry.iNbViewportY);
}

void cd_switcher_compute_coordinates_from_index (int iIndex, int *iNumLine, int *iNumColumn)
{
	if (g_desktopGeometry.iNbViewportX == 0 || g_desktopGeometry.iNbViewportY == 0)  // des fois (chgt de resolution sous Compiz), le rafraichissement se passe mal, on le force donc ici pour eviter une division par 0.
	{
		cd_switcher_refresh_desktop_values (myApplet);
	}
	g_return_if_fail (g_desktopGeometry.iNbViewportX > 0 && g_desktopGeometry.iNbViewportY > 0);
	
	*iNumLine = iIndex / myData.switcher.iNbColumns;
	*iNumColumn = iIndex % myData.switcher.iNbColumns;
}


static gboolean _update_idle (gpointer data)
{
	//\___________________ On calcule la geometrie de l'icone en mode compact.
	cd_switcher_compute_nb_lines_and_columns ();
	
	//\___________________ On recupere le bureau courant et sa position sur la grille.
	cd_switcher_get_current_desktop ();
	
	//\___________________ On charge le bon nombre d'icones dans le sous-dock ou le desklet.
	cd_switcher_load_icons ();
	
	//\___________________ On dessine l'icone principale.
	cd_switcher_draw_main_icon ();
	
	//\___________________ draw the current desktop number.
	if (myConfig.bDisplayNumDesk)
	{
		int iIndex = cd_switcher_compute_index_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1);
	}
	
	myData.iSidUpdateIdle = 0;
	return FALSE;
}
void cd_switcher_trigger_update_from_screen_geometry (gboolean bNow)
{
	if (myData.iSidUpdateIdle == 0)
	{
		if (bNow)
			myData.iSidUpdateIdle = g_idle_add (_update_idle, NULL);
		else
			myData.iSidUpdateIdle = g_timeout_add_seconds (1, _update_idle, NULL);
	}
}

static gboolean _update_wallpaper_idle (gpointer data)
{
	cd_debug ("");
	// load new surface
	cd_switcher_load_desktop_bg_map_surface ();
	// draw the new icon
	cd_switcher_draw_main_icon ();

	myData.iSidUpdateIdle = 0;
	return FALSE;
}

void cd_switcher_trigger_update_from_wallpaper (void)
{
	/* with a delay to avoid many reloads: if there is a fading effect when
	 * changing wallpaper, the dock will consume a lot of resources because it
	 * will update the surface for each step of the fading effect
	 */
	if (myData.iSidUpdateIdle == 0)
		myData.iSidUpdateIdle = g_timeout_add (1250, _update_wallpaper_idle, NULL);
}


void cd_switcher_refresh_desktop_values (GldiModuleInstance *myApplet)
{
	gldi_desktop_refresh ();
	cd_switcher_trigger_update_from_screen_geometry (TRUE);
}



static void _cd_switcher_action_on_one_window_from_viewport (Icon *pIcon, gpointer *data)
{
	int iNumDesktop = GPOINTER_TO_INT (data[0]);
	int iNumViewportX = GPOINTER_TO_INT (data[1]);
	int iNumViewportY = GPOINTER_TO_INT (data[2]);
	CDSwitcherActionOnViewportFunc pFunction = data[3];
	gpointer pUserData = data[4];
	
	if (! gldi_window_is_on_desktop (pIcon->pAppli, iNumDesktop, iNumViewportX, iNumViewportY))
		return ;
	
	pFunction (pIcon, iNumDesktop, iNumViewportX, iNumViewportY, pUserData);
}
void cd_switcher_foreach_window_on_viewport (int iNumDesktop, int iNumViewportX, int iNumViewportY, CDSwitcherActionOnViewportFunc pFunction, gpointer pUserData)
{
	gpointer data[5] = {GINT_TO_POINTER (iNumDesktop), GINT_TO_POINTER (iNumViewportX), GINT_TO_POINTER (iNumViewportY), pFunction, pUserData};
	cairo_dock_foreach_appli_icon ((GldiIconFunc)_cd_switcher_action_on_one_window_from_viewport, data);
}
