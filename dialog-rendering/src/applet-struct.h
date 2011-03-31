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


#ifndef __DIALOG_RENDERING_STRUCT__
#define  __DIALOG_RENDERING_STRUCT__

#include <cairo-dock.h>

struct _AppletConfig {
	gint iComicsRadius;
	gint iComicsLineWidth;
	gdouble fComicsLineColor[4];
	
	gint iModernRadius;
	gint iModernLineWidth;
	gdouble fModernLineColor[4];
	gint iModernLineSpacing;
	
	gint iPlaneRadius;
	gint iPlaneLineWidth;
	gdouble fPlaneLineColor[4];
	gdouble fPlaneColor[4];
	
	gint iTooltipRadius;
	gint iTooltipLineWidth;
	gdouble fTooltipLineColor[4];
	
	gint iCurlyRadius;
	gint iCurlyLineWidth;
	gdouble fCurlyLineColor[4];
	gdouble fCurlyCurvature;
	gboolean bCulrySideToo;
} ;

struct _AppletData {
	gint no_data_yet;
} ;

#endif
