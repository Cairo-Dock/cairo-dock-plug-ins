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

typedef struct {
	gint iNumDesktop;
	gint iNumViewportX;
	gint iNumViewportY;
	gint iOneViewportWidth;
	gint iOneViewportHeight;
	cairo_t *pCairoContext;
	} CDSwitcherDesktop;
 
static void _cd_switcher_draw_windows_on_viewport (Icon *pIcon, CDSwitcherDesktop *data)
{
	if (pIcon == NULL || pIcon->fInsertRemoveFactor > 0)
		return ;
	GldiWindowActor *actor = pIcon->pAppli;
	if (actor->bIsHidden && ! myConfig.bDisplayHiddenWindows)
		return ;
	int iNumDesktop = data->iNumDesktop;
	int iNumViewportX = data->iNumViewportX;
	int iNumViewportY = data->iNumViewportY;
	// skip if not on this desktop/viewport
	if (!gldi_window_is_on_desktop (actor, iNumDesktop, iNumViewportX, iNumViewportY))
		return;
	int iOneViewportWidth = data->iOneViewportWidth;
	int iOneViewportHeight = data->iOneViewportHeight;
	cairo_t *pCairoContext = data->pCairoContext;
	
	// On calcule les coordonnees en repere absolu.
	int x = actor->windowGeometry.x;  // par rapport au viewport courant.
	x += myData.switcher.iCurrentViewportX * g_desktopGeometry.Xscreen.width;  // repere absolu
	if (x < 0)
		x += g_desktopGeometry.iNbViewportX * g_desktopGeometry.Xscreen.width;
	int y = actor->windowGeometry.y;
	y += myData.switcher.iCurrentViewportY * g_desktopGeometry.Xscreen.height;
	if (y < 0)
		y += g_desktopGeometry.iNbViewportY * g_desktopGeometry.Xscreen.height;
	int w = actor->windowGeometry.width, h = actor->windowGeometry.height;
	
	// on dessine ses traits.
	cairo_save (pCairoContext);

	GldiWindowActor *pActiveWindow = gldi_windows_get_active ();
	
	if (myConfig.bFillAllWindows && actor != pActiveWindow)
		cairo_set_source_rgba (pCairoContext, myConfig.RGBWFillColors[0], myConfig.RGBWFillColors[1], myConfig.RGBWFillColors[2], myConfig.RGBWFillColors[3]);
	else
	{
		if (myConfig.bUseDefaultColors)
			gldi_style_colors_set_line_color (myDrawContext);
		else
			cairo_set_source_rgba (pCairoContext, myConfig.RGBWLineColors[0], myConfig.RGBWLineColors[1], myConfig.RGBWLineColors[2], myConfig.RGBWLineColors[3]);
	}
	cairo_rectangle (pCairoContext,
		(1.*x/g_desktopGeometry.Xscreen.width - iNumViewportX)*iOneViewportWidth,
		(1.*y/g_desktopGeometry.Xscreen.height - iNumViewportY)*iOneViewportHeight,
		1.*w/g_desktopGeometry.Xscreen.width*iOneViewportWidth,
		1.*h/g_desktopGeometry.Xscreen.height*iOneViewportHeight);

	if (myConfig.bFillAllWindows || actor == pActiveWindow)
	{
		//g_print (" %s est la fenetre active\n", pIcon->cName);
		cairo_fill (pCairoContext);
	}
	else
	{
		cairo_stroke (pCairoContext);
	}
	
	if (myConfig.bDrawIcons)
	{
		const CairoDockImageBuffer *pImage = gldi_appli_icon_get_image_buffer (pIcon);
		if (pImage && pImage->pSurface)
		{
			double fZoomX = (double) w/g_desktopGeometry.Xscreen.width*iOneViewportWidth / pImage->iWidth;
			double fZoomY = (double) h/g_desktopGeometry.Xscreen.height*iOneViewportHeight / pImage->iHeight;
			double fZoom = MIN (fZoomX, fZoomY);  // on garde le ratio.
			
			cairo_translate (pCairoContext,
				(1.*x/g_desktopGeometry.Xscreen.width - iNumViewportX)*iOneViewportWidth + (fZoomX - fZoom) * pImage->iWidth/2,
				(1.*y/g_desktopGeometry.Xscreen.height - iNumViewportY)*iOneViewportHeight + (fZoomY - fZoom) * pImage->iHeight/2);
			cairo_scale (pCairoContext,
				fZoom,
				fZoom);
			cairo_set_source_surface (pCairoContext,
				pImage->pSurface,
				0.,
				0.);
			cairo_paint (pCairoContext);
		}
	}
	
	cairo_restore (pCairoContext);
}


static int _compare_icons_stack_order (Icon *icon1, Icon *icon2)
{
	if (icon1 == NULL)  // les icones nulles vont a la fin.
		return 1;
	if (icon2 == NULL)
		return -1;
	if (icon1->pAppli->iStackOrder < icon2->pAppli->iStackOrder)  // ordre petit => dessus => dessinee en dernier.
		return -1;
	else
		return 1;
}
void cd_switcher_draw_main_icon_compact_mode (void)
{
	if (myData.switcher.iNbColumns == 0 || myData.switcher.iNbLines == 0)  // may happen in desklet mode with a cube desktop, when the desklet is still 0x0.
		return;
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO ();
	///g_return_if_fail (myDrawContext != NULL);
	//g_print ("%s (%d;%d)\n", __func__, myData.switcher.iCurrentLine, myData.switcher.iCurrentColumn);
	// On efface l'icone.
	///cairo_dock_erase_cairo_context (myDrawContext);
	
	// definition des parametres de dessin.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	myData.switcher.fOneViewportHeight = (double) (iHeight - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (double) (iWidth - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	double dx=0, dy=0;
	double w = iWidth, h = iHeight;
	if (myConfig.bPreserveScreenRatio)
	{
		double r = (double) g_desktopGeometry.Xscreen.width / g_desktopGeometry.Xscreen.height;
		double r_ = myData.switcher.fOneViewportWidth / myData.switcher.fOneViewportHeight;
		if (r_ > r)  // on etire trop en largeur.
		{
			myData.switcher.fOneViewportWidth /= r_ / r;
			w = myData.switcher.fOneViewportWidth * myData.switcher.iNbColumns + 2 * myConfig.iLineSize + (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize;
			dx = (double) (iWidth - w) / 2;
		}
		else
		{
			myData.switcher.fOneViewportHeight /= r / r_;
			h = myData.switcher.fOneViewportHeight * myData.switcher.iNbLines + 2 * myConfig.iLineSize + (myData.switcher.iNbLines - 1) * myConfig.iInLineSize;
			dy = (iHeight - h) / 2;
		}
	}
	myData.switcher.fOffsetX = dx;
	myData.switcher.fOffsetY = dy;
	
	cairo_save (myDrawContext);
	cairo_translate (myDrawContext, dx, dy);
	
	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER)
	{
		pSurface = myData.pDesktopBgMapSurface;
	}
	if (pSurface == NULL)
	{
		pSurface = myData.pDefaultMapSurface;
	}
	fZoomX = (double) myData.switcher.fOneViewportWidth / myData.iSurfaceWidth;  // both surfaces are loaded at the same size.
	fZoomY= (double) myData.switcher.fOneViewportHeight / myData.iSurfaceHeight;
	
	// cadre exterieur.
	cairo_set_line_width (myDrawContext,myConfig.iLineSize);
	if (myConfig.bUseDefaultColors)
		gldi_style_colors_set_line_color (myDrawContext);
	else
		cairo_set_source_rgba(myDrawContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
	cairo_rectangle(myDrawContext,
		.5*myConfig.iLineSize,
		.5*myConfig.iLineSize,
		w - myConfig.iLineSize,
		h - myConfig.iLineSize);

	cairo_stroke (myDrawContext);
	
	// lignes interieures.
	cairo_set_line_width (myDrawContext,myConfig.iInLineSize);
	if (myConfig.bUseDefaultColors)
		gldi_style_colors_set_line_color (myDrawContext);
	else
		cairo_set_source_rgba (myDrawContext,myConfig.RGBInLineColors[0],myConfig.RGBInLineColors[1],myConfig.RGBInLineColors[2],myConfig.RGBInLineColors[3]);
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
	int k = 0, N = g_desktopGeometry.iNbDesktops * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY;
	for (j = 0; j < myData.switcher.iNbLines && k < N; j ++)
	{
		for (i = 0; i < myData.switcher.iNbColumns && k < N; i ++)
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
			
			if ((myConfig.iDrawCurrentDesktopMode == SWICTHER_FILL_INVERTED && (i != myData.switcher.iCurrentColumn || j != myData.switcher.iCurrentLine))
			|| (myConfig.iDrawCurrentDesktopMode == SWICTHER_FILL && (i == myData.switcher.iCurrentColumn && j == myData.switcher.iCurrentLine)))
			{
				cairo_save (myDrawContext);
				
				if (myConfig.bUseDefaultColors)
					gldi_style_colors_set_selected_bg_color (myDrawContext);
				else
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
				CDSwitcherDesktop data = {iNumDesktop, iNumViewportX, iNumViewportY, (int) myData.switcher.fOneViewportWidth, (int) myData.switcher.fOneViewportHeight, myDrawContext};
				g_list_foreach (pWindowList, (GFunc) _cd_switcher_draw_windows_on_viewport, &data);
				
				cairo_restore (myDrawContext);
			}
			
			iNumViewportX ++;
			if (iNumViewportX == g_desktopGeometry.iNbViewportX)
			{
				iNumViewportX = 0;
				iNumViewportY ++;
				if (iNumViewportY == g_desktopGeometry.iNbViewportY)
				{
					iNumViewportY = 0;
					iNumDesktop ++;
				}
			}
			k ++;
		}
	}
	
	// dessin de l'indicateur sur le bureau courant (on le fait maintenant car dans le cas ou la ligne interieure est plus petite que la ligne de l'indicateur, les surfaces suivantes recouvreraient en partie la ligne.
	if (myConfig.iDrawCurrentDesktopMode == SWICTHER_DRAW_FRAME)
	{
		i = myData.switcher.iCurrentColumn;
		j = myData.switcher.iCurrentLine;
		xi = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize);
		yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize);
		
		cairo_set_line_width (myDrawContext,myConfig.iLineSize);
		if (myConfig.bUseDefaultColors)
			gldi_style_colors_set_selected_bg_color (myDrawContext);
		else
			cairo_set_source_rgba (myDrawContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
		cairo_rectangle(myDrawContext,
			xi - .5*myConfig.iLineSize,
			yj - .5*myConfig.iLineSize,
			myData.switcher.fOneViewportWidth + myConfig.iLineSize,
			myData.switcher.fOneViewportHeight + myConfig.iLineSize);
		
		if (myConfig.iDrawCurrentDesktopMode == SWICTHER_FILL)
			cairo_fill (myDrawContext);  // maybe we need to fill it with an alpha pattern in case we use the global style color ?...
		else
		{
			cairo_set_line_width (myDrawContext, MIN (4, 2*myConfig.iLineSize));
			cairo_stroke(myDrawContext);
		}
	}
	
	cairo_restore (myDrawContext);
	g_list_free (pWindowList);  // le contenu appartient a la hash table, mais pas la liste.
	
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
}


void cd_switcher_draw_main_icon_expanded_mode (void)
{
	// apply the desktop bg or the user image on the main icon, in dock mode
	int iWidth, iHeight;
	
	if (myDock)
	{
		CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
		myData.switcher.fOneViewportHeight = (iHeight - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
		myData.switcher.fOneViewportWidth = (iWidth - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.

		cairo_surface_t *pSurface = NULL;
		double fZoomX, fZoomY;
		if (myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER)
		{
			cairo_dock_erase_cairo_context (myDrawContext);

			pSurface = myData.pDesktopBgMapSurface;
			fZoomX = 1. * iWidth / myData.iSurfaceWidth;
			fZoomY= 1. * iHeight / myData.iSurfaceHeight;
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
		}
		else
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
	}
	
	if (myConfig.bDrawWindows)
	{
		GList *pWindowList = cairo_dock_get_current_applis_list ();
		pWindowList = g_list_sort (pWindowList, (GCompareFunc) _compare_icons_stack_order);
		
		CDSwitcherDesktop data;
		int iNumDesktop=0, iNumViewportX=0, iNumViewportY=0;
		cairo_t *pCairoContext;
		Icon *pIcon;
		GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
		GList *ic;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
			
			pCairoContext = cairo_create (pIcon->image.pSurface);
			cairo_set_line_width (pCairoContext, 1.);
			if (myConfig.bUseDefaultColors)
				gldi_style_colors_set_line_color (myDrawContext);
			else
				cairo_set_source_rgba (pCairoContext, myConfig.RGBWLineColors[0], myConfig.RGBWLineColors[1], myConfig.RGBWLineColors[2], myConfig.RGBWLineColors[3]);
			
			data.iNumDesktop = iNumDesktop;
			data.iNumViewportX = iNumViewportX;
			data.iNumViewportY = iNumViewportY;
			data.iOneViewportWidth = iWidth;
			data.iOneViewportHeight = iHeight;
			data.pCairoContext = pCairoContext;
			g_list_foreach (pWindowList, (GFunc) _cd_switcher_draw_windows_on_viewport, &data);
			
			iNumViewportX ++;
			if (iNumViewportX == g_desktopGeometry.iNbViewportX)
			{
				iNumViewportY ++;
				if (iNumViewportY == g_desktopGeometry.iNbViewportY)
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
	CD_APPLET_ENTER;
	//g_print ("%s (%.2Fx%.2f)\n", __func__, myData.switcher.fOneViewportWidth, myData.switcher.fOneViewportHeight);
	double x, y, w, h;
	glTranslatef (-pDesklet->container.iWidth/2, -pDesklet->container.iHeight/2, 0.);
	
	w = myData.switcher.fOneViewportWidth/2;
	h = myData.switcher.fOneViewportHeight/2;
	int i, j;
	int k = 0, N = g_desktopGeometry.iNbDesktops * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY;
	
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
	CD_APPLET_LEAVE ();
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
	CD_APPLET_ENTER;
	gldi_window_show (pIcon->pAppli);
	CD_APPLET_LEAVE ();
}

static void _show_desktop (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	int iIndex = GPOINTER_TO_INT (data);
	int iNumDesktop, iNumViewportX, iNumViewportY;
	cd_switcher_compute_desktop_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	
	if (iNumViewportX != myData.switcher.iCurrentViewportX
	|| iNumViewportY != myData.switcher.iCurrentViewportY
	|| iNumDesktop != myData.switcher.iCurrentDesktop)
		gldi_desktop_set_current (iNumDesktop, iNumViewportX, iNumViewportY);
	CD_APPLET_LEAVE ();
}
static void _cd_switcher_add_window_on_viewport (Icon *pIcon, int iNumDesktop, int iNumViewportX, int iNumViewportY, GtkWidget *pMenu)
{
	//g_print (" + %s\n", pIcon->cName);
	
	// on cree une copie de la surface de l'icone a la taille du menu.
	GdkPixbuf *pixbuf = cairo_dock_icon_buffer_to_pixbuf (pIcon);
	if (pixbuf == NULL)  // icon not loaded (because not in a dock, because inhibited)
	{
		const gchar *cIcon = cairo_dock_get_class_icon (pIcon->cClass);
		gint iDesiredIconSize = cairo_dock_search_icon_size (GTK_ICON_SIZE_LARGE_TOOLBAR); // 24px
		gchar *cIconPath = cairo_dock_search_icon_s_path (cIcon, iDesiredIconSize);
		if (cIconPath)
		{
			pixbuf = gdk_pixbuf_new_from_file_at_size (cIconPath,
				iDesiredIconSize,
				iDesiredIconSize,
				NULL);
		}
	}
	
	// on ajoute une entree au menu avec le pixbuf.
	gchar *cLabel = cairo_dock_cut_string (pIcon->cName, 50);
	GtkWidget *pMenuItem = gldi_menu_add_item (pMenu, cLabel, "", G_CALLBACK (_show_window), pIcon);
	g_free (cLabel);
	
	if (pixbuf)
	{
		GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
		gldi_menu_item_set_image (pMenuItem, image);
		g_object_unref (pixbuf);
	}
}
void cd_switcher_build_windows_list (GtkWidget *pMenu)
{
	GList *pWindowList = NULL;
	pWindowList = cairo_dock_get_current_applis_list ();
	pWindowList = g_list_sort (pWindowList, (GCompareFunc) _compare_icons_stack_order);
	
	// chaque bureau/viewport.
	int iNumDesktop=0, iNumViewportX=0, iNumViewportY=0;
	int k = 0, N = g_desktopGeometry.iNbDesktops * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY;
	int iIndex = cd_switcher_compute_index_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
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
			
			if (k < myData.iNbNames)
			{
				if (k == iIndex)
					g_string_printf (sDesktopName, "<b>%s (%s)</b>", myData.cDesktopNames[k], D_("Current"));
				else
					g_string_printf (sDesktopName, "<b>%s</b>", myData.cDesktopNames[k]);
			}
			else
			{
				if (k == iIndex)
					g_string_printf (sDesktopName, "<b>%s %d (%s)</b>", D_("Desktop"), k+1, D_("Current"));
				else
					g_string_printf (sDesktopName, "<b>%s %d</b>", D_("Desktop"), k+1);
			}
			pMenuItem = gldi_menu_add_item (pMenu, sDesktopName->str, NULL, G_CALLBACK (_show_desktop), GINT_TO_POINTER (k));
			GtkWidget *pLabel = gtk_bin_get_child (GTK_BIN(pMenuItem));
			gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
			gtk_misc_set_alignment (GTK_MISC (pLabel), .5, .5);
			
			pMenuItem = gtk_separator_menu_item_new ();
			gtk_menu_shell_append(GTK_MENU_SHELL (pMenu), pMenuItem);
			g_object_set (pMenuItem, "height-request", 3, NULL);
			
			// on ajoute les fenetres du viewport au menu.
			cd_debug ("Windows' listing (%d;%d;%d) ...", iNumDesktop, iNumViewportX, iNumViewportY);
			cd_switcher_foreach_window_on_viewport (iNumDesktop,
				iNumViewportX,
				iNumViewportY,
				(CDSwitcherActionOnViewportFunc) _cd_switcher_add_window_on_viewport,
				pMenu);
			
			// on passe au viewport suivant.
			iNumViewportX ++;
			if (iNumViewportX == g_desktopGeometry.iNbViewportX)
			{
				iNumViewportX = 0;
				iNumViewportY ++;
				if (iNumViewportY == g_desktopGeometry.iNbViewportY)
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
	
	gldi_window_move_to_desktop (pIcon->pAppli,
		iDestNumDesktop,
		(iDestNumViewportX - myData.switcher.iCurrentViewportX) * g_desktopGeometry.Xscreen.width,
		(iDestNumViewportY - myData.switcher.iCurrentViewportY) * g_desktopGeometry.Xscreen.height);
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
