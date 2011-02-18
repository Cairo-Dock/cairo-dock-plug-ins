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


#ifndef __APPLET_XGAMMA__
#define  __APPLET_XGAMMA__

#include <stdio.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include <ctype.h>
#include <stdlib.h>


void xgamma_add_gamma (XF86VidModeGamma *pGamma, gboolean bAdd);

double xgamma_get_gamma (XF86VidModeGamma *pGamma);

void xgamma_set_gamma (XF86VidModeGamma *pGamma);


void xgamma_create_scales_widget (double fGamma, XF86VidModeGamma *pGamma);


CairoDialog *xgamma_build_dialog (void);
void xgamma_build_and_show_widget (void);


CairoDialog *xgamma_build_dialog_simple (void);


#endif
