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


#ifndef __CD_APPLET_XML__
#define  __CD_APPLET_XML__

#include <cairo-dock.h>

typedef struct {
	gchar *cFont;
	gint iFontSizeWidth;
	gint iFontSizeHeight;
	double fTextColor[4];
	gchar *cText;
	gchar *cCommand;
	gchar *cInternal;	
	gchar *cResult;
	gchar *cAlignWidth;
	gchar *cAlignHeight;
	gboolean bRefresh;
	gint iRefresh;  // Refresh souhaité
	gint iTimer;  // S'incrémente jusqu'au refresh
	gboolean bEndOfLine;
	gboolean bNextNewLine;
	gboolean bBar;
	gboolean bLimitedBar;
	gint iHeight;
	gint iWidth;
	gint iImgSize;
	gchar *cImgPath;
	cairo_surface_t *pImgSurface;
	gboolean bImgDraw;
	gint iOverrideH;
	gint iOverrideW;
	gint iSpaceBetweenLines;
	// Disk usage
	gchar *cMountPoint;
	// Graph:
	double graphHistory[300];
	
} TextZone;



void cd_doncky_free_item (TextZone *pTextZone);


void cd_doncky_free_item_list (CairoDockModuleInstance *myApplet);

gboolean cd_doncky_readxml (CairoDockModuleInstance *myApplet);

#endif
