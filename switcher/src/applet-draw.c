/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Cchumi & Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
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
	//g_print (" %s : (%d;%d) %dx%d\n", pIcon->acName, x, y, w, h);
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
		//g_print (" %s est la fenetre active\n", pIcon->acName);
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
	//double fRatio = (myDock ? myDock->fRatio : 1.);
	//double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	myData.switcher.fOneViewportHeight = (iHeight - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (iWidth - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	
	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.bMapWallpaper)
	{
		pSurface = cairo_dock_get_desktop_bg_surface ();
		fZoomX = (double) myData.switcher.fOneViewportWidth / g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
		fZoomY= (double) myData.switcher.fOneViewportHeight / g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
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
		iWidth - myConfig.iLineSize,
		iHeight - myConfig.iLineSize);

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
		cairo_rel_line_to (myDrawContext, 0, iHeight - 2*myConfig.iLineSize);
		cairo_stroke (myDrawContext);
	}
	for (j = 1; j < myData.switcher.iNbLines; j ++)  // lignes horizontales.
	{
		yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		cairo_move_to (myDrawContext, myConfig.iLineSize, yj);
		cairo_rel_line_to (myDrawContext, iWidth - 2*myConfig.iLineSize, 0);
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
				iNumViewportY ++;
				if (iNumViewportY == g_iNbViewportY)
					iNumDesktop ++;
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
	//double fRatio = (myDock ? myDock->fRatio : 1.);
	//double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	myData.switcher.fOneViewportHeight = (iHeight - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (iWidth - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.

	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.bMapWallpaper)
	{
		cairo_dock_erase_cairo_context (myDrawContext);
		
		pSurface = cairo_dock_get_desktop_bg_surface ();
		fZoomX = 1. * iWidth / g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
		fZoomY= 1. * iHeight / g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
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
		//fRatio = (myIcon->pSubDock != NULL ? myIcon->pSubDock->fRatio : 1);
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
	glTranslatef (-pDesklet->iWidth/2, -pDesklet->iHeight/2, 0.);
	
	w = myData.switcher.fOneViewportWidth/2;
	h = myData.switcher.fOneViewportHeight/2;
	int i, j;
	int k = 0, N = g_iNbDesktops * g_iNbViewportX * g_iNbViewportY;
	
	for (j = 0; j < myData.switcher.iNbLines; j ++)  // lignes horizontales.
	{
		y = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		y = pDesklet->iHeight - (y + h);	
		
		for (i = 0; i < myData.switcher.iNbColumns; i ++)  // lignes verticales.
		{
			x = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
			x += w;
			
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
		x += w;
		y = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		y += h;
		*iCoordX = x;
		*iCoordY = y;
	}
}
