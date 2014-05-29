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

/* Minimum extension version required */
#define MINMAJOR 2
#define MINMINOR 0

/* Maximum and Minimum gamma values */
#define GAMMA_MIN 0.2
#define GAMMA_MAX 2.0

#include <cairo-dock.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>

struct _AppletConfig {
	gint iScrollVariation;
	gdouble fInitialGamma;
	gchar *cDefaultTitle;
	gchar *cShortkey;
	gchar *cShortkey2;
	} ;

struct _AppletData {
	gboolean bVideoExtensionOK;
	CairoDialog *pDialog;
	GtkWidget *pWidget;
	GtkWidget *pGlobalScale;
	GtkWidget *pRedScale;
	GtkWidget *pGreenScale;
	GtkWidget *pBlueScale;
	guint iGloalScaleSignalID;
	guint iRedScaleSignalID;
	guint iGreenScaleSignalID;
	guint iBlueScaleSignalID;
	XF86VidModeGamma Xgamma;
	XF86VidModeGamma XoldGamma;
	guint iSidScrollAction;
	gint iScrollCount;
	GldiShortkey *pKeyBinding;
	GldiShortkey *pKeyBinding2;
	} ;


#endif
