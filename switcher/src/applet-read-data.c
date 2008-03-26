/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>
#include <glib.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-read-data.h"
#include "applet-load-icons.h"



extern AppletConfig myConfig;
extern AppletData myData;


void cd_switcher_get_current_desktop (int *ScreenCurrentSize,int *ScreenCurrentNum)
{
	GError *erreur = NULL;
int iCurrentViewPortX, iCurrentViewPortY;
cairo_dock_get_current_viewport (&iCurrentViewPortX, &iCurrentViewPortY);
int iNbViewportX, iNbViewportY;
cd_message ("test %s",&iCurrentViewPortX);
cairo_dock_get_nb_viewports (&iNbViewportX, &iNbViewportY);
int desknum= cairo_dock_get_current_desktop();

int desktopnum = cairo_dock_get_nb_desktops();
int ScreenWidthSize=g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
int ScreenHeightSize=g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
int ScreenCurrentSizeX=iCurrentViewPortX;
int test = ScreenCurrentSizeX/ScreenWidthSize;


*ScreenCurrentNum=test;

*ScreenCurrentSize=ScreenWidthSize;

}



