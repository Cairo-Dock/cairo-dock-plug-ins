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

#include <string.h>
#include <glib/gstdio.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-draw.h"

gboolean my_bRotateIconsOnEllipse = TRUE;


static void _cd_switcher_draw_windows_on_viewport (Icon *pIcon, gint *data)
{
	if (pIcon == NULL || pIcon->fPersonnalScale > 0)
		return ;
	if (pIcon->bIsHidden && ! myConfig.bDisplayHiddenWindows)
		return ;
	int iNumDesktop = data[0];
	int iNumViewportX = data[1];
	int iNumViewportY = data[2];
	int iOneViewportWidth = data[3];
	int iOneViewportHeight = data[4];
	cairo_t *pCairoContext = GINT_TO_POINTER (data[5]);
	
	// On calcule les coordonnees en repere absolu.
	int x = pIcon->windowGeometry.x;  // par rapport au viewport courant.
	x += myData.switcher.iCurrentViewportX * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];  // repere absolu
	if (x < 0)
		x += g_iNbViewportX * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
	int y = pIcon->windowGeometry.y;
	y += myData.switcher.iCurrentViewportY * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
	if (y < 0)
		y += g_iNbViewportY * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
	int w = pIcon->windowGeometry.width, h = pIcon->windowGeometry.height;
	
	// test d'intersection avec le viewport donne.
	//g_print (" %s : (%d;%d) %dx%d\n", pIcon->cName, x, y, w, h);
	if ((pIcon->iNumDesktop != -1 && pIcon->iNumDesktop != iNumDesktop) ||
		x + w <= iNumViewportX * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] ||
		x >= (iNumViewportX + 1) * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] ||
		y + h <= iNumViewportY * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] ||
		y >= (iNumViewportY + 1) * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL])
		return ;
	//g_print (" > on la dessine (%x)\n", pIcon->pIconBuffer);
	
	// on dessine ses traits.
	cairo_save (pCairoContext);
	
	cairo_set_source_rgba (pCairoContext, myConfig.RGBWLineColors[0], myConfig.RGBWLineColors[1], myConfig.RGBWLineColors[2], myConfig.RGBWLineColors[3]);
	cairo_rectangle (pCairoContext,
		(1.*x/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] - iNumViewportX)*iOneViewportWidth,
		(1.*y/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] - iNumViewportY)*iOneViewportHeight,
		1.*w/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]*iOneViewportWidth,
		1.*h/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]*iOneViewportHeight);
	if (pIcon->Xid == cairo_dock_get_current_active_window ())
	{
		//g_print (" %s est la fenetre active\n", pIcon->cName);
		cairo_fill (pCairoContext);
	}
	else
	{
		cairo_stroke (pCairoContext);
	}
	
	if (pIcon->pIconBuffer != NULL)
	{
		CairoDock *pParentDock = NULL;
		pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
		if (pParentDock == NULL)
			pParentDock = g_pMainDock;
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pParentDock), &iWidth, &iHeight);
		double fZoomX = (double) w/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]*iOneViewportWidth / iWidth;
		double fZoomY = (double) h/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]*iOneViewportHeight / iHeight;
		double fZoom = MIN (fZoomX, fZoomY);  // on garde le ratio.
		
		cairo_translate (pCairoContext,
			(1.*x/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] - iNumViewportX)*iOneViewportWidth + (fZoomX - fZoom) * iWidth/2,
			(1.*y/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] - iNumViewportY)*iOneViewportHeight + (fZoomY - fZoom) * iHeight/2);
		cairo_scale (pCairoContext,
			fZoom,
			fZoom);
		cairo_set_source_surface (pCairoContext,
			pIcon->pIconBuffer,
			0.,
			0.);
		cairo_paint (pCairoContext);
	}
	
	cairo_restore (pCairoContext);
}


static int _compare_icons_stack_order (Icon *icon1, Icon *icon2)
{
	if (icon1 == NULL)  // les icones nulles vont a la fin.
		return 1;
	if (icon2 == NULL)
		return -1;
	if (icon1->iStackOrder < icon2->iStackOrder)  // ordre petit => dessus => dessinee en dernier.
		return -1;
	else
		return 1;
}
void cd_switcher_draw_main_icon_compact_mode (void)
{
	//cd_debug ("%s (%d;%d)", __func__, myData.switcher.iCurrentLine, myData.switcher.iCurrentColumn);
	// On efface l'icone.
	cairo_dock_erase_cairo_context (myDrawContext);
	
	// definition des parametres de dessin.
	//double fRatio = (myDock ? myDock->container.fRatio : 1.);
	//double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	myData.switcher.fOneViewportHeight = (double) (iHeight - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (double) (iWidth - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	double dx=0, dy=0;
	double w = iWidth, h = iHeight;
	if (myConfig.bPreserveScreenRatio)
	{
		double r = (double) g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] / g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
		double r_ = myData.switcher.fOneViewportWidth / myData.switcher.fOneViewportHeight;
		if (r_ > r)  // on etire trop en largeur.
		{
			myData.switcher.fOneViewportWidth /= r_ / r;
			dx = (double) iWidth/2 * (1 - r / r_);
			w /= r_ / r;
		}
		else
		{
			myData.switcher.fOneViewportHeight /= r / r_;
			dy = (double) iHeight/2 * (1 - r_ / r);
			h /= r / r_;
		}
	}
	myData.switcher.fOffsetX = dx;
	myData.switcher.fOffsetY = dy;
	
	cairo_save (myDrawContext);
	cairo_translate (myDrawContext, dx, dy);
	
	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.bMapWallpaper)
	{
		pSurface = myData.pDesktopBgMapSurface;
		fZoomX = (double) myData.switcher.fOneViewportWidth / iWidth;
		fZoomY= (double) myData.switcher.fOneViewportHeight / iHeight;
	}
	if (pSurface == NULL)
	{
		pSurface = myData.pDefaultMapSurface;
		fZoomX = (double) myData.switcher.fOneViewportWidth / iWidth;
		fZoomY = (double) myData.switcher.fOneViewportHeight / iHeight;
	}
	
	// cadre exterieur.
	cairo_set_line_width (myDrawContext,myConfig.iLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
	cairo_rectangle(myDrawContext,
		.5*myConfig.iLineSize,
		.5*myConfig.iLineSize,
		w - myConfig.iLineSize,
		h - myConfig.iLineSize);

	cairo_stroke (myDrawContext);
	
	// lignes interieures.
	cairo_set_line_width (myDrawContext,myConfig.iInLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBInLineColors[0],myConfig.RGBInLineColors[1],myConfig.RGBInLineColors[2],myConfig.RGBInLineColors[3]);
	double xi, yj;
	int i, j;
	for (i = 1; i < myData.switcher.iNbColumns; i ++)  // lignes verticales.
	{
		xi = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		cairo_move_to (myDrawContext, xi, myConfig.iLineSize);
		cairo_rel_line_to (myDrawContext, 0, h - 2*myConfig.iLineSize);
		cairo_stroke (myDrawContext);
	}
	for (j = 1; j < myData.switcher.iNbLines; j ++)  // lignes horizontales.
	{
		yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		cairo_move_to (myDrawContext, myConfig.iLineSize, yj);
		cairo_rel_line_to (myDrawContext, w - 2*myConfig.iLineSize, 0);
		cairo_stroke (myDrawContext);
	}
	
	GList *pWindowList = NULL;
	if (myConfig.bDrawWindows)
	{
		pWindowList = cairo_dock_get_current_applis_list ();
		pWindowList = g_list_sort (pWindowList, (GCompareFunc) _compare_icons_stack_order);
	}
	
	// chaque bureau/viewport.
	int iNumDesktop=0, iNumViewportX=0, iNumViewportY=0;
	int k = 0, N = g_iNbDesktops * g_iNbViewportX * g_iNbViewportY;
	for (j = 0; j < myData.switcher.iNbLines; j ++)
	{
		for (i = 0; i < myData.switcher.iNbColumns; i ++)
		{
			cairo_save (myDrawContext);

			xi = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize);
			yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize);

			cairo_translate (myDrawContext,
				xi,
				yj);

			cairo_scale (myDrawContext,
				fZoomX,
				fZoomY);
			cairo_set_source_surface (myDrawContext,
				pSurface,
				0.,
				0.);
			cairo_paint(myDrawContext);
			
			cairo_restore (myDrawContext);
			
			if (myConfig.iDrawCurrentDesktopMode == SWICTHER_FILL_INVERTED && (i != myData.switcher.iCurrentColumn || j != myData.switcher.iCurrentLine))
			{
				cairo_save (myDrawContext);
				
				cairo_set_source_rgba (myDrawContext, myConfig.RGBIndColors[0], myConfig.RGBIndColors[1], myConfig.RGBIndColors[2], myConfig.RGBIndColors[3]);
				cairo_rectangle(myDrawContext,
					xi - .5*myConfig.iLineSize,
					yj - .5*myConfig.iLineSize,
					myData.switcher.fOneViewportWidth + myConfig.iLineSize,
					myData.switcher.fOneViewportHeight + myConfig.iLineSize);
				cairo_fill (myDrawContext);
				
				cairo_restore (myDrawContext);
			}
			
			if (myConfig.bDrawWindows)
			{
				cairo_save (myDrawContext);
				
				cairo_translate (myDrawContext,
					xi,
					yj);
				cairo_set_line_width (myDrawContext, 1.);
				cairo_rectangle (myDrawContext,
					0.,
					0.,
					myData.switcher.fOneViewportWidth,
					myData.switcher.fOneViewportHeight);
				cairo_clip (myDrawContext);
				
				//g_print (" dessin des fenetres du bureau (%d;%d;%d) ...\n", iNumDesktop, iNumViewportX, iNumViewportY);
				gint data[6] = {iNumDesktop, iNumViewportX, iNumViewportY, (int) myData.switcher.fOneViewportWidth, (int) myData.switcher.fOneViewportHeight, GPOINTER_TO_INT (myDrawContext)};
				g_list_foreach (pWindowList, (GFunc) _cd_switcher_draw_windows_on_viewport, data);
				
				cairo_restore (myDrawContext);
			}
			
			iNumViewportX ++;
			if (iNumViewportX == g_iNbViewportX)
			{
				iNumViewportX = 0;
				iNumViewportY ++;
				if (iNumViewportY == g_iNbViewportY)
				{
					iNumViewportY = 0;
					iNumDesktop ++;
				}
			}
			k ++;
			if (k == N)
				break ;
		}
	}
	
	// dessin de l'indicateur sur le bureau courant (on le fait maintenant car dans le cas ou la ligne interieure est plus petite que la ligne de l'indicateur, les surfaces suivantes recouvreraient en partie la ligne.
	if (myConfig.iDrawCurrentDesktopMode != SWICTHER_FILL_INVERTED)
	{
		i = myData.switcher.iCurrentColumn;
		j = myData.switcher.iCurrentLine;
		xi = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize);
		yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize);
		
		cairo_set_line_width (myDrawContext,myConfig.iLineSize);
		cairo_set_source_rgba (myDrawContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
		cairo_rectangle(myDrawContext,
			xi - .5*myConfig.iLineSize,
			yj - .5*myConfig.iLineSize,
			myData.switcher.fOneViewportWidth + myConfig.iLineSize,
			myData.switcher.fOneViewportHeight + myConfig.iLineSize);
		
		if (myConfig.iDrawCurrentDesktopMode == SWICTHER_FILL)
			cairo_fill (myDrawContext);
		else
			cairo_stroke(myDrawContext);
	}
	
	cairo_restore (myDrawContext);
	g_list_free (pWindowList);  // le contenu appartient a la hash table, mais pas la liste.
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		cairo_dock_update_icon_texture (myIcon);
	else
		CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
}


void cd_switcher_draw_main_icon_expanded_mode (void)
{
	// definition des parametres de dessin.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	//double fRatio = (myDock ? myDock->container.fRatio : 1.);
	//double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	myData.switcher.fOneViewportHeight = (iHeight - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (iWidth - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.

	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.bMapWallpaper)
	{
		cairo_dock_erase_cairo_context (myDrawContext);
		
		pSurface = myData.pDesktopBgMapSurface;
		fZoomX = 1. * iWidth / iWidth;
		fZoomY= 1. * iHeight / iHeight;
		cairo_translate (myDrawContext,
			0.,
			0.);

		cairo_save (myDrawContext);
		cairo_scale (myDrawContext,
			fZoomX ,
			fZoomY );
		cairo_set_source_surface (myDrawContext,
			pSurface,
			0.,
			0.);
		cairo_paint(myDrawContext);
		cairo_restore (myDrawContext);
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
			cairo_dock_update_icon_texture (myIcon);
		else
			CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
	}
	else
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
	}

	if (myConfig.bDrawWindows)
	{
		GList *pWindowList = cairo_dock_get_current_applis_list ();
		pWindowList = g_list_sort (pWindowList, (GCompareFunc) _compare_icons_stack_order);
		
		//fMaxScale = (myIcon->pSubDock != NULL ? cairo_dock_get_max_scale (myIcon->pSubDock) : 1);
		//fRatio = (myIcon->pSubDock != NULL ? myIcon->pSubDock->container.fRatio : 1);
		gint data[6];
		int iNumDesktop=0, iNumViewportX=0, iNumViewportY=0;
		cairo_t *pCairoContext;
		Icon *pIcon;
		CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
		GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
		GList *ic;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			cairo_dock_get_icon_extent (pIcon, pContainer, &iWidth, &iHeight);
			
			data[0] = iNumDesktop;
			data[1] = iNumViewportX;
			data[2] = iNumViewportY;
			data[3] = iWidth;
			data[4] = iHeight;
			pCairoContext = cairo_create (pIcon->pIconBuffer);
			data[5] = GPOINTER_TO_INT (pCairoContext);
			cairo_set_line_width (pCairoContext, 1.);
			cairo_set_source_rgba (pCairoContext, myConfig.RGBWLineColors[0], myConfig.RGBWLineColors[1], myConfig.RGBWLineColors[2], myConfig.RGBWLineColors[3]);
		
			g_list_foreach (pWindowList, (GFunc) _cd_switcher_draw_windows_on_viewport, data);
			
			iNumViewportX ++;
			if (iNumViewportX == g_iNbViewportX)
			{
				iNumViewportY ++;
				if (iNumViewportY == g_iNbViewportY)
					iNumDesktop ++;
			}
			cairo_destroy (pCairoContext);
		}
		g_list_free (pWindowList);  // le contenu appartient a la hash table, mais pas la liste.
	}
}

/*Fonction de base pour toutes les autres*/
void cd_switcher_draw_main_icon (void)
{
	cd_message ("%s (%d)", __func__, myConfig.bCompactView);
	if (myConfig.bCompactView)
	{
		cd_switcher_draw_main_icon_compact_mode ();
	}
	else
	{
		cd_switcher_draw_main_icon_expanded_mode ();
	}
	
	CD_APPLET_REDRAW_MY_ICON;
}


void cd_switcher_draw_desktops_bounding_box (CairoDesklet *pDesklet)
{
	//g_print ("%s ()\n", __func__);
	double x, y, w, h;
	glTranslatef (-pDesklet->container.iWidth/2, -pDesklet->container.iHeight/2, 0.);
	
	w = myData.switcher.fOneViewportWidth/2;
	h = myData.switcher.fOneViewportHeight/2;
	int i, j;
	int k = 0, N = g_iNbDesktops * g_iNbViewportX * g_iNbViewportY;
	
	for (j = 0; j < myData.switcher.iNbLines; j ++)  // lignes horizontales.
	{
		y = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		y = pDesklet->container.iHeight - (y + h + myData.switcher.fOffsetY);	
		
		for (i = 0; i < myData.switcher.iNbColumns; i ++)  // lignes verticales.
		{
			x = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
			x += w + myData.switcher.fOffsetX;
			
			glLoadName(i * myData.switcher.iNbLines + j + 1);  // +1 pour ne pas avoir 0.
			
			glBegin(GL_QUADS);
			glVertex3f(x-w, y+h, 0.);
			glVertex3f(x+w, y+h, 0.);
			glVertex3f(x+w, y-h, 0.);
			glVertex3f(x-w, y-h, 0.);
			glEnd();
			
			k ++;
			if (k == N)
				break;
		}
	}
}

void cd_switcher_extract_viewport_coords_from_picked_object (CairoDesklet *pDesklet, int *iCoordX, int *iCoordY)
{
	//g_print ("%s (%d)\n", __func__, pDesklet->iPickedObject);
	if (pDesklet->iPickedObject != 0)
	{
		pDesklet->iPickedObject --;  // cf le +1
		int i, j;
		i = pDesklet->iPickedObject / myData.switcher.iNbLines;
		j = pDesklet->iPickedObject % myData.switcher.iNbLines;
		//g_print ("bureau (%d;%d)\n", i, j);
		
		double x, y, w, h;
		w = myData.switcher.fOneViewportWidth/2;
		h = myData.switcher.fOneViewportHeight/2;
		x = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		x += w + myData.switcher.fOffsetX;
		y = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		y += h + myData.switcher.fOffsetY;
		*iCoordX = x;
		*iCoordY = y;
	}
}


static void _show_window (GtkMenuItem *menu_item, Icon *pIcon)
{
	cairo_dock_show_xwindow (pIcon->Xid);
}

static void _show_desktop (GtkMenuItem *menu_item, gpointer data)
{
	int iIndex = GPOINTER_TO_INT (data);
	int iNumDesktop, iNumViewportX, iNumViewportY;
	cd_switcher_compute_viewports_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	if (iNumDesktop != myData.switcher.iCurrentDesktop)
		cairo_dock_set_current_desktop (iNumDesktop);
	if (iNumViewportX != myData.switcher.iCurrentViewportX || iNumViewportY != myData.switcher.iCurrentViewportY)
		cairo_dock_set_current_viewport (iNumViewportX, iNumViewportY);
}
static void _cd_switcher_list_window_on_viewport (Icon *pIcon, int iNumDesktop, int iNumViewportX, int iNumViewportY, GtkWidget *pMenu)
{
	g_print (" + %s\n", pIcon->cName);
	// on recupere la taille de l'icone.
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	if (pParentDock == NULL)
		pParentDock = g_pMainDock;
	int iWidth, iHeight;
	cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pParentDock), &iWidth, &iHeight);
	
	// on cree une copie de la surface de l'icone a la taille du menu.
	int w = 24, h = w;
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
		w,
		h);
	cairo_t *pCairoContext = cairo_create (surface);
	cairo_scale (pCairoContext, (double)w/iWidth, (double)h/iHeight);
	cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_destroy (pCairoContext);
	guchar *d, *data = cairo_image_surface_get_data (surface);
	int r = cairo_image_surface_get_stride (surface);
	
	// on la convertit en un pixbuf.
	GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
		TRUE,
		8,
		w,
		h);
	guchar *p, *pixels = gdk_pixbuf_get_pixels (pixbuf);
	int iNbChannels = gdk_pixbuf_get_n_channels (pixbuf);
	int iRowstride = gdk_pixbuf_get_rowstride (pixbuf);
	
	int x, y;
	int red, green, blue;
	float fAlphaFactor;
	for (y = 0; y < h; y ++)
	{
		for (x = 0; x < w; x ++)
		{
			p = pixels + y * iRowstride + x * iNbChannels;
			d = data + y * r + x * 4;
			
			fAlphaFactor = (float) d[3] / 255;
			if (fAlphaFactor != 0)
			{
				red = d[0] / fAlphaFactor;
				green = d[1] / fAlphaFactor;
				blue = d[2] / fAlphaFactor;
			}
			else
			{
				red = blue = green = 0;
			}
			p[0] = blue;
			p[1] = green;
			p[2] = red;
			p[3] = d[3];
		}
	}
	
	cairo_surface_destroy (surface);
	
	// on ajoute une entree au menu avec le pixbuf.
	GtkWidget *pMenuItem = gtk_image_menu_item_new_with_label (pIcon->cName);
	GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
	g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (_show_window), pIcon);
	
	g_object_unref (pixbuf);
}
void cd_switcher_build_windows_list (GtkWidget *pMenu)
{
	GList *pWindowList = NULL;
	pWindowList = cairo_dock_get_current_applis_list ();
	pWindowList = g_list_sort (pWindowList, (GCompareFunc) _compare_icons_stack_order);
	
	// chaque bureau/viewport.
	int iNumDesktop=0, iNumViewportX=0, iNumViewportY=0;
	int k = 0, N = g_iNbDesktops * g_iNbViewportX * g_iNbViewportY;
	int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	GString *sDesktopName = g_string_new ("");
	int i, j;
	for (j = 0; j < myData.switcher.iNbLines; j ++)
	{
		for (i = 0; i < myData.switcher.iNbColumns; i ++)
		{
			// on ajoute le nom du bureau/viewport dans le menu.
			GtkWidget *pMenuItem = gtk_separator_menu_item_new ();
			gtk_menu_shell_append(GTK_MENU_SHELL (pMenu), pMenuItem);
			g_object_set (pMenuItem, "height-request", 3, NULL);
			
			if (k < myConfig.iNbNames)
			{
				if (k == iIndex)
					g_string_printf (sDesktopName, "<b>%s (%s)</b>", myConfig.cDesktopNames[k], D_("current"));
				else
					g_string_printf (sDesktopName, "<b>%s</b>", myConfig.cDesktopNames[k]);
			}
			else
			{
				if (k == iIndex)
					g_string_printf (sDesktopName, "<b>%s %d (%s)</b>", D_("desktop"), k+1, D_("current"));
				else
					g_string_printf (sDesktopName, "<b>%s %d</b>", D_("desktop"), k+1);
			}  // les noms des bureaux : _NET_DESKTOP_NAMES, UTF8_STRING
			pMenuItem = gtk_menu_item_new ();
			GtkWidget *pLabel = gtk_label_new (sDesktopName->str);
			gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
			gtk_misc_set_alignment (GTK_MISC (pLabel), .5, .5);
			gtk_container_add (GTK_CONTAINER (pMenuItem), pLabel);
			gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
			g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (_show_desktop), GINT_TO_POINTER (k));
			
			pMenuItem = gtk_separator_menu_item_new ();
			gtk_menu_shell_append(GTK_MENU_SHELL (pMenu), pMenuItem);
			g_object_set (pMenuItem, "height-request", 3, NULL);
			
			// on ajoute les fenetres du viewport au menu.
			g_print (" listing des fenetres du bureau (%d;%d;%d) ...\n", iNumDesktop, iNumViewportX, iNumViewportY);
			cd_switcher_foreach_window_on_viewport (iNumDesktop,
				iNumViewportX,
				iNumViewportY,
				(CDSwitcherActionOnViewportFunc) _cd_switcher_list_window_on_viewport,
				pMenu);
			
			// on passe au viewport suivant.
			iNumViewportX ++;
			if (iNumViewportX == g_iNbViewportX)
			{
				iNumViewportX = 0;
				iNumViewportY ++;
				if (iNumViewportY == g_iNbViewportY)
				{
					iNumViewportY = 0;
					iNumDesktop ++;
				}
			}
			k ++;
			if (k == N)
				break ;
		}
	}
	g_string_free (sDesktopName, TRUE);
}



static void _cd_switcher_move_window_to_viewport (Icon *pIcon, int iNumDesktop, int iNumViewportX, int iNumViewportY, gint *data)
{
	int iDestNumDesktop = data[0];
	int iDestNumViewportX = data[1];
	int iDestNumViewportY = data[2];
	
	cairo_dock_move_xwindow_to_nth_desktop (pIcon->Xid,
		iDestNumDesktop,
		(iDestNumViewportX - myData.switcher.iCurrentViewportX) * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL],
		(iDestNumViewportY - myData.switcher.iCurrentViewportY) * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]);
}
void cd_switcher_move_current_desktop_to (int iNumDesktop, int iNumViewportX, int iNumViewportY)
{
	gint data[3] = {iNumDesktop, iNumViewportX, iNumViewportY};
	cd_switcher_foreach_window_on_viewport (myData.switcher.iCurrentDesktop,
		myData.switcher.iCurrentViewportX,
		myData.switcher.iCurrentViewportY,
		(CDSwitcherActionOnViewportFunc) _cd_switcher_move_window_to_viewport,
		data);
}
