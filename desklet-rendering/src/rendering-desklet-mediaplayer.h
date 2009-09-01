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


#ifndef __RENDERING_DESKLET_MEDIAPLAYER__
#define  __RENDERING_DESKLET_MEDIAPLAYER__

#include "cairo.h"

#define MY_APPLET_MEDIAPLAYER_DESKLET_RENDERER_NAME "Mediaplayer"


typedef struct {
	gchar *cArtist;
	gchar *cTitle;
	cairo_surface_t *pArtistSurface;
	cairo_surface_t *pTitleSurface;
	gboolean bControlButton;
	
	gdouble fDeskletWidth;
	
	gint fArtistWidth;
	gint fArtistHeight;
	gdouble fArtistXOffset;
	gdouble fArtistYOffset;
	
	gint fTitleWidth;
	gint fTitleHeight;
	gdouble fTitleXOffset;
	gdouble fTitleYOffset;
	
	gint iNbIcons;
	gint iIconsLimit;
	gdouble fBandWidth;
	gdouble fIconBandOffset;
	
	Icon *pClickedIcon;
} CDMediaplayerParameters;


CDMediaplayerParameters *rendering_configure_mediaplayer (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_free_mediaplayer_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_mediaplayer (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_load_mediaplayer_data (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_draw_mediaplayer_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_mediaplayer_desklet_renderer (void);

void rendering_update_text_for_mediaplayer(CairoDesklet *pDesklet, gpointer *pNewData);

#endif
