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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define SWITCHER_DEFAULT_NAME "Switcher"


typedef enum {
	SWICTHER_DRAW_FRAME,
	SWICTHER_FILL,
	SWICTHER_FILL_INVERTED,
	SWICTHER_NB_MODES
	} SwitcherDrawCurrentDesktopMode;

typedef enum {
	SWICTHER_WINDOWS_LIST=0,
	SWICTHER_SHOW_DESKTOP,
	SWICTHER_EXPOSE,
	SWICTHER_NB_ACTIONS
	} SwitcherAction;

struct _AppletConfig {
	gboolean bCompactView;
	gboolean bMapWallpaper;
	gboolean bDrawWindows;
	gboolean bDisplayNumDesk;
	gchar *cDefaultIcon;
	gboolean bDesklet3D;
	gchar *cThemePath;
	gchar *cRenderer;
	gdouble RGBInLineColors[4];
	gdouble RGBLineColors[4];
	gdouble RGBWLineColors[4];
	gdouble RGBIndColors[4];
	gint iInLineSize;
	gint iLineSize;
	gint iWLineSize;
	gboolean bPreserveScreenRatio;
	SwitcherDrawCurrentDesktopMode iDrawCurrentDesktopMode;
	gboolean bDisplayHiddenWindows;
	gchar **cDesktopNames;
	gint iNbNames;
	SwitcherAction iActionOnMiddleClick;
	} ;

typedef struct
{
	gint iCurrentDesktop, iCurrentViewportX, iCurrentViewportY;
	gint iNbViewportTotal;
	gint iNbLines, iNbColumns;
	gint iCurrentLine, iCurrentColumn;
	double fOneViewportWidth;
	double fOneViewportHeight;
	double fOffsetX, fOffsetY;
} SwitcherApplet;

typedef void (*CDSwitcherActionOnViewportFunc) (Icon *pIcon, int iNumDesktop, int iNumViewportX, int iNumViewportY, gpointer pUserData);

struct _AppletData {
	SwitcherApplet switcher;
	cairo_surface_t *pDefaultMapSurface;
	cairo_surface_t *pDesktopBgMapSurface;
	gint iSidRedrawMainIconIdle;
	gint iSidAutoRefresh;
	gint iPrevIndexHovered;
	gdouble fDesktopNameAlpha;
	guint iSidPainIcons;
} ;
#endif
