/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-draw.h"


gboolean my_bRotateIconsOnEllipse = TRUE;


// Dessin de l'icone principale avec division suivant le nombre de bureau configurer dans compiz .Tous les dessins sont divisés en deux parties, si c'est le bureau courant on preserve le rectangle pour afficher l'indicateur.Je pense que cette fonction est à épurer même si l'ai beaucoup eclairci je la trouve encore fouilli.

void cd_switcher_draw_main_icon_compact_mode (void)
{
	cd_debug ("%s (%d;%d)", __func__, myData.switcher.iCurrentLine, myData.switcher.iCurrentColumn);
	// On efface l'icone.
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	cairo_save (myDrawContext);
	double fRatio = (myDock ? myDock->fRatio : 1.);
	
	// definition des parametres de dessin.
	double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	myData.switcher.fOneViewportHeight = (myIcon->fHeight/fRatio * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (myIcon->fWidth/fRatio * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	
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
		fZoomX = (double) myData.switcher.fOneViewportWidth / (myIcon->fWidth/fRatio * fMaxScale);
		fZoomY = (double) myData.switcher.fOneViewportHeight / (myIcon->fHeight/fRatio * fMaxScale);
	}
	
	// cadre exterieur.
	cairo_set_line_width (myDrawContext,myConfig.iLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
	cairo_rectangle(myDrawContext,
		.5*myConfig.iLineSize,
		.5*myConfig.iLineSize,
		myIcon->fWidth/fRatio * fMaxScale - myConfig.iLineSize,
		myIcon->fHeight/fRatio * fMaxScale - myConfig.iLineSize);

	cairo_stroke (myDrawContext);
	
	// lignes interieures.
	cairo_set_line_width (myDrawContext,myConfig.iInLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBInLineColors[0],myConfig.RGBInLineColors[1],myConfig.RGBInLineColors[2],myConfig.RGBInLineColors[3]);
	double xi, yj;
	int i, j;
	for (i = 1; i <myData.switcher.iNbColumns; i ++)  // lignes verticales.
	{
		xi = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		cairo_move_to (myDrawContext, xi, myConfig.iLineSize);
		cairo_rel_line_to (myDrawContext, 0, myIcon->fHeight/fRatio * fMaxScale - 2*myConfig.iLineSize);
		cairo_stroke (myDrawContext);
	}
	for (j = 1; j < myData.switcher.iNbLines; j ++)  // lignes horizontales.
	{
		yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		cairo_move_to (myDrawContext, myConfig.iLineSize, yj);
		cairo_rel_line_to (myDrawContext, myIcon->fWidth/fRatio * fMaxScale - 2*myConfig.iLineSize, 0);
		cairo_stroke (myDrawContext);
	}
	
	// chaque bureau/viewport.
	for (i = 0; i < myData.switcher.iNbColumns; i ++)
	{
		for (j = 0; j < myData.switcher.iNbLines; j ++)
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
			
			
			if (myConfig.iDrawCurrentDesktopMode == SWICTHER_FILL_INVERTED && (i != myData.switcher.iCurrentColumn || j != myData.switcher.iCurrentLine))
			{
				cairo_restore (myDrawContext);  // avant la translation et le zoom.
				cairo_save (myDrawContext);
				
				cairo_set_source_rgba (myDrawContext, myConfig.RGBIndColors[0], myConfig.RGBIndColors[1], myConfig.RGBIndColors[2], myConfig.RGBIndColors[3]);
				cairo_rectangle(myDrawContext,
					xi - .5*myConfig.iLineSize,
					yj - .5*myConfig.iLineSize,
					myData.switcher.fOneViewportWidth + myConfig.iLineSize,
					myData.switcher.fOneViewportHeight + myConfig.iLineSize);
				cairo_fill (myDrawContext);
			}
			
			cairo_restore (myDrawContext);
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
	
	if (myConfig.bDrawWindows)
	{
		cd_switcher_draw_windows_on_each_viewports (xi, yj,myData.switcher.fOneViewportWidth + myConfig.iInLineSize,myData.switcher.fOneViewportHeight + myConfig.iInLineSize);
	}
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		cairo_dock_update_icon_texture (myIcon);
}

void cd_switcher_draw_windows_on_each_viewports(double Xposition, double Yposition, double Xsize, double Ysize)
{
	cairo_save (myDrawContext);
	cairo_set_line_width (myDrawContext,myConfig.iWLineSize);

	GList *iWinList = cairo_dock_get_current_applis_list ();

	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	Window windowactive = cairo_dock_get_current_active_window ();

	Icon *icon;
	GList *ic;
	for (ic = iWinList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (! CAIRO_DOCK_IS_APPLI (icon) || icon->bIsHidden)  // ce peut etre une icon NULL si c'est une fenetre qu'on ignore.
			continue ;

		
		//printf("icon->acName : %s \n",icon->acName);

		cairo_set_source_rgba(myDrawContext,myConfig.RGBWLineColors[0],myConfig.RGBWLineColors[1],myConfig.RGBWLineColors[2],myConfig.RGBWLineColors[3]);

		double Xgeo, Ygeo;
		int i, j;
				cairo_save (myDrawContext);
	
		Xgeo = Xposition;
		Ygeo = Yposition;
	
		double 	XWgeo= ((double)icon->windowGeometry.x/((double)g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]))* (myData.switcher.fOneViewportWidth + myConfig.iInLineSize);
		double 	YWgeo = ((double)icon->windowGeometry.y/((double)g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]))* (myData.switcher.fOneViewportHeight + myConfig.iInLineSize);
		
		

		double 	x0       =   XWgeo+Xgeo,
			y0	=   YWgeo+Ygeo,
			rect_width  = ((double)icon->windowGeometry.width/(double)g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL])*Xsize,
			rect_height = ((double)icon->windowGeometry.height/(double)g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL])*Ysize,
			radius = 8.0,   
			windowtitle = 3.0;
		
		double x1,y1;
		x1=x0+rect_width;
		y1=y0+rect_height;
		
		/*printf("x0 : %f \n",x0);
		printf("y0 : %f \n",y0);
		printf("rect_width : %f \n",rect_width);
		printf("rect_height : %f \n",rect_height);
		printf("radius : %f \n",radius);
		printf("Xposition : %f \n",Xposition);
		printf("Yposition : %f \n",Yposition);
		printf("X1 : %f \n",x1);
		printf("Y1 : %f \n",y1);
		printf("Xgeo : %f \n",Xgeo);
		printf("Ygeo : %f \n",Ygeo);	
		printf("XWgeo : %f \n",XWgeo);
		printf("YWgeo : %f \n",YWgeo);*/

		// # Dessin du contour des fenetres.
		
		cairo_move_to  (myDrawContext, x0, y0 + radius);
		cairo_curve_to (myDrawContext, x0 ,y0, x0, y0, (x0 + x1)/2, y0);
		cairo_curve_to (myDrawContext, x1, y0, x1, y0, x1, y0 + radius);
		cairo_line_to (myDrawContext, x1 , y1);
		cairo_line_to (myDrawContext, x0 , y1);
		cairo_close_path                    (myDrawContext);
	
		// # Dessin des la barre de titre.
	
		cairo_move_to  (myDrawContext, x0, y0 + (radius-windowtitle));
		cairo_line_to (myDrawContext, x1 , y0 + (radius-windowtitle));

		if (windowactive!=icon->Xid)
		{
			// # Si fenetre non active on laisse telle quelle en ligne.
			cairo_stroke (myDrawContext);
		}
		else
		{
			// # Si fenetre active on la remplit.
			cairo_fill (myDrawContext);
		}
	}
	
	g_list_free (iWinList);  // le contenu appartient a la hash table, mais pas la liste.
	cairo_restore (myDrawContext);
}


void cd_switcher_draw_main_icon_expanded_mode (void)
{
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba(myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	// definition des parametres de dessin.
	double fRatio = (myDock ? myDock->fRatio : 1.);
	double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	myData.switcher.fOneViewportHeight = (myIcon->fHeight/fRatio * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (myIcon->fWidth/fRatio * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.

	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.bMapWallpaper)
	{
		pSurface = cairo_dock_get_desktop_bg_surface ();
		fZoomX = (double) myIcon->fHeight/fRatio * fMaxScale/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
		fZoomY= (double) myIcon->fHeight/fRatio * fMaxScale/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];	
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
	else if (myIcon->acFileName == NULL)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
	}

	/*if (pSurface == NULL)
	{
		pSurface = myData.pDefaultMapSurface;
		//fZoomX = (double) myData.switcher.fOneViewportWidth / (myIcon->fWidth * fMaxScale);
		//fZoomY = (double) myData.switcher.fOneViewportHeight / (myIcon->fHeight * fMaxScale);
	}*/
		
	if (myConfig.bDrawWindows)
	{
		double XWgeo= (myIcon->fWidth/fRatio* fMaxScale/myData.switcher.fOneViewportWidth)* fMaxScale;
		double YWgeo = (myIcon->fHeight/fRatio* fMaxScale/myData.switcher.fOneViewportHeight)* fMaxScale;
		cd_debug ("XWgeo : %f",XWgeo);
		cd_debug ("YWgeo : %f",YWgeo);
		fZoomX = myIcon->fWidth/fRatio * fMaxScale;
		fZoomY = myIcon->fHeight/fRatio * fMaxScale;
		cairo_save(myDrawContext);
		cd_switcher_draw_windows_on_each_viewports(XWgeo,YWgeo,fZoomX,fZoomY);
		cairo_restore (myDrawContext);
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
	
	if ((myDesklet && ! myConfig.bCompactView) || (myDock && myDock->bUseReflect))
		cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer);
	CD_APPLET_REDRAW_MY_ICON;
}
