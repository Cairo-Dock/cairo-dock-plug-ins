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

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar *cThemePath;
	CairoDockLoadImageModifier iLoadingModifier;
	gint iWinkDelay;
	gint iWinkDuration;
	gboolean bFastCheck;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	// centre de la pupille, dans le referentiel de la surface cairo.
	gint iXeyes[2], iYeyes[2];
	gint iEyesWidth[2], iEyesHeight[2];
	gdouble fPrevXpupil[2], fPrevYpupil[2];
	gdouble fXpupil[2], fYpupil[2];
	// image du fond
	cairo_surface_t *pBgSurface;
	GLuint iBgTexture;
	gdouble iXbg, iYbg;
	gint iBgWidth, iBgHeight;
	// pupille
	cairo_surface_t *pPupilSurface[2];
	GLuint iPupilTexture[2];
	gint iPupilWidth[2], iPupilHeight[2];
	// paupiere
	cairo_surface_t *pEyelidSurface;
	GLuint iEyelidTexture;
	gdouble iXeyelid, iYeyelid;
	gint iEyelidWidth, iEyelidHeight;
	// masque
	cairo_surface_t *pToonSurface;
	GLuint iToonTexture;
	gint iToonWidth, iToonHeight;
	// clignement des yeux.
	gint iTimeCount;
	gboolean bWink;
	} ;


#endif
