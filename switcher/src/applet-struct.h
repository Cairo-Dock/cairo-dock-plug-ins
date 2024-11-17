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
	SWICTHER_MAP_WALLPAPER,
	SWICTHER_MAP_IMAGE,
	SWICTHER_MAP_COLOUR,
	SWICTHER_MAP_AUTO,
	SWICTHER_NB_DRAWING
	} SwitcherIconDrawing;

typedef enum {
	SWICTHER_WINDOWS_LIST=0,
	SWICTHER_SHOW_DESKTOP,
	SWICTHER_EXPOSE_DESKTOPS,
	SWICTHER_EXPOSE_WINDOWS,
	SWICTHER_NB_ACTIONS
	} SwitcherAction;

	typedef enum {
	SWICTHER_LAYOUT_AUTO,
	SWICTHER_LAYOUT_SINGLE_LINE,
	SWITCHER_LAYOUT_STRICT,
	SWICTHER_NB_LAYOUTS
	} SwitcherDesktopsLayout;

struct _AppletConfig {
	gboolean bCompactView;
	SwitcherDesktopsLayout iDesktopsLayout;
	SwitcherIconDrawing iIconDrawing;
	gboolean bDrawWindows;
	gboolean bDrawIcons;
	gboolean bFillAllWindows;
	gboolean bDisplayNumDesk;
	gchar *cDefaultIcon;
	gchar *cThemePath;
	gchar *cRenderer;
	gboolean bUseDefaultColors;
	gdouble RGBInLineColors[4];
	gdouble RGBLineColors[4];
	gdouble RGBWLineColors[4];
	gdouble RGBIndColors[4];
	gdouble RGBWFillColors[4];
	gdouble RGBBgColors[4];
	gint iInLineSize;
	gint iLineSize;
	gboolean bPreserveScreenRatio;
	SwitcherDrawCurrentDesktopMode iDrawCurrentDesktopMode;
	gboolean bDisplayHiddenWindows;
	gchar **cDesktopNames;
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
	gchar **cDesktopNames;
} SwitcherApplet;

typedef void (*CDSwitcherActionOnViewportFunc) (Icon *pIcon, int iNumDesktop, int iNumViewportX, int iNumViewportY, gpointer pUserData);

struct _AppletData {
	SwitcherApplet switcher;
	cairo_surface_t *pDefaultMapSurface;
	cairo_surface_t *pDesktopBgMapSurface;
	gint iSurfaceWidth;
	gint iSurfaceHeight;
	gint iSidRedrawMainIconIdle;
	gint iSidUpdateIdle;
	gint iPrevIndexHovered;
	gdouble fDesktopNameAlpha;
	guint iSidGetDesktopNames;
	gchar **cDesktopNames;
	gint iNbNames;
} ;
#endif
