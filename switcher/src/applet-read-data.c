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

//cd_message ("test %d", iCurrentViewPortX);
cairo_dock_get_nb_viewports (&myData.switcher.iNbViewportX, &myData.switcher.iNbViewportY);
int desknum= cairo_dock_get_current_desktop();

int desktopnum = cairo_dock_get_nb_desktops();
int ScreenWidthSize=g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
int ScreenHeightSize=g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
int ScreenCurrentSizeX=iCurrentViewPortX;
int test = ScreenCurrentSizeX/ScreenWidthSize;


*ScreenCurrentNum=test;

*ScreenCurrentSize=ScreenWidthSize;

}

void cd_switcher_grab_and_draw_icon(GdkPixbuf *icone)
{

GdkWindow * root;
	int i;

cairo_dock_get_nb_viewports (&myData.switcher.iNbViewportX, &myData.switcher.iNbViewportY);
//for (i=0;i<myData.switcher.iNbViewportX;i++)
//{

GdkScreen	*screen = gdk_screen_get_default();
root = gdk_screen_get_root_window( screen );
gint x = gdk_screen_get_width  (screen);
gint y = gdk_screen_get_height  (screen);

printf ("X%d \n",x);
printf ("Y%d \n",y);

myData.switcher.icon = gdk_pixbuf_get_from_drawable (NULL, root,
                                        NULL,
                                        0,0, 0, 0,
                                        x, y);
/*
icone = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 1680, 1050);

int xx = 0, yy = 0;


gdk_pixbuf_composite(myData.switcher.icon, icone, xx, yy, 
 1680/2, 1050/2, xx, yy, 0.5, 0.5, 
 GDK_INTERP_BILINEAR, 255);
 xx += 1680/2;
 gdk_pixbuf_composite(myData.switcher.icon, icone, xx, yy, 
 1680/2, 1050/2, xx, yy, 0.5, 0.5, 
GDK_INTERP_BILINEAR, 255);
xx=0, yy=1050/2;
gdk_pixbuf_composite(myData.switcher.icon, icone, xx, yy, 
 1680/2, 1050/2, xx, yy, 0.5, 0.5, 
 GDK_INTERP_BILINEAR, 255);
 xx += 1680/2;
 gdk_pixbuf_composite(myData.switcher.icon, icone, xx, yy, 
 1680/2, 1050/2, xx, yy, 0.5, 0.5, 
GDK_INTERP_BILINEAR, 255);*/

gchar *path = g_strdup_printf("%s/.cairo-dock/current_theme/plug-ins/switcher/default.png", g_getenv ("HOME"));

myData.switcher.iconedock =gdk_pixbuf_scale_simple(myData.switcher.icon,
    640, 480,
     GDK_INTERP_BILINEAR);

gdk_pixbuf_save(myData.switcher.iconedock, path, "png", NULL,
                      "compression", "9",
                     NULL);
}



