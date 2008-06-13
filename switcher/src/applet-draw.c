/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>
#include <glib.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS

gboolean my_bRotateIconsOnEllipse = TRUE;


// Dessin de l'icone principale avec division suivant le nombre de bureau configurer dans compiz .Tous les dessins sont divisés en deux parties, si c'est le bureau courant on preserve le rectangle pour afficher l'indicateur.Je pense que cette fonction est à épurer même si l'ai beaucoup eclairci je la trouve encore fouilli.

void cd_switcher_draw_main_icon_compact_mode (void)
{
	// On efface l'icone.
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba(myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	cairo_save (myDrawContext);
	
	// definition des parametres de dessin.
	double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	myData.switcher.fOneViewportHeight = (myIcon->fHeight * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (myIcon->fWidth * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	
	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.bMapWallpaper)
	{
		pSurface = cairo_dock_get_desktop_bg_surface ();
		fZoomX = (double) myData.switcher.fOneViewportWidth / g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
		fZoomY= (double) myData.switcher.fOneViewportHeight / g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
	}
	if (pSurface == NULL)
	{
		pSurface = myData.pDefaultMapSurface;
		fZoomX = (double) myData.switcher.fOneViewportWidth / (myIcon->fWidth * fMaxScale);
		fZoomY = (double) myData.switcher.fOneViewportHeight / (myIcon->fHeight * fMaxScale);
	}
	// cadre exterieur.
	cairo_set_line_width (myDrawContext,myConfig.iLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
	cairo_rectangle(myDrawContext,
		.5*myConfig.iLineSize,
		.5*myConfig.iLineSize,
		myIcon->fWidth * fMaxScale - myConfig.iLineSize,
		myIcon->fHeight * fMaxScale - myConfig.iLineSize);
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
		cairo_rel_line_to (myDrawContext, 0, myIcon->fHeight * fMaxScale - 2*myConfig.iLineSize);
		cairo_stroke (myDrawContext);
	}
	for (j = 1; j < myData.switcher.iNbLines; j ++)  // lignes horizontales.
	{
		yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		cairo_move_to (myDrawContext, myConfig.iLineSize, yj);
		cairo_rel_line_to (myDrawContext, myIcon->fWidth * fMaxScale - 2*myConfig.iLineSize, 0);
		cairo_stroke (myDrawContext);
	}
	
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
			
			cairo_restore (myDrawContext);
		}
	}
	
	// dessin de l'indicateur sur le bureau courant (on le fait maintenant car dans le cas ou la ligne interieure est plus petite que la ligne de l'indicateur, les surfaces suivantes recouvreraient en partie la ligne.
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
	cairo_stroke(myDrawContext);
	
	cairo_restore (myDrawContext);
	/*int i, j;
	for (i = 0; i <g_iNbViewportX; i ++)
	{
		//définition de la taille et de la couleur des lignes par rapport aux variable de la conf.
		cairo_set_line_width (myDrawContext,myConfig.iInLineSize);
		cairo_set_source_rgba(myDrawContext,myConfig.RGBInLineColors[0],myConfig.RGBInLineColors[1],myConfig.RGBInLineColors[2],myConfig.RGBInLineColors[3]);
		cairo_save(myDrawContext);
		cd_message ("test i = %d-%d", i,myData.switcher.iNbDesktopByLine);
		if (i < myData.switcher.iNbDesktopByLine)
		{
			// Pour dessiner sur deux ligne je teste si i est inferieur au nombre de bureau divisé par 2
			if (i <= 0 )  // Si il est égale à 0 ( bureau 1)
			{
				cd_message (" bureau 1 %d -- %d ",myData.switcher.iNbDesktopByLine,i );
				 Pour tous les rectangles c'est identique :
				je dessine le rectangle puis je teste si c'est le bureau courant si oui alors on preserve le rectangle ce qui nous fait l'indicateur.
				Ensuite je redimenssione la surface de wallpaper aux dimension du rectangle je l'applique et je la peint.
				Enfin J'applique la couleur de l'indicateurs.
				C'est dans le calcul pour tracer le rectangle que j'ai mis beaucoup de temps.Surtout pour la deuxieme ligne.
		
				cairo_save(myDrawContext);
				cairo_rectangle(myDrawContext, 0, 0,myData.switcher.fOneViewportWidth , myData.switcher.fOneViewportHeight);
				
				 Ici on teste si le booleen de configuration pour inverser l'identification du bureau courant est valide et on inverse l'indication. Fonctionnalité soumise par Nochka
				if (myConfig.bInvertIndicator)
				{
					if (myData.switcher.ScreenCurrentNums == i)
					{
						cairo_stroke(myDrawContext);
					}
					else
					{
						cairo_stroke_preserve(myDrawContext);
					}
				}
				else
				{
					if (myData.switcher.ScreenCurrentNums == i)
					{
						cairo_stroke_preserve(myDrawContext);
					}
					else
					{
						cairo_stroke(myDrawContext);
					}
				}
				cairo_restore (myDrawContext);
				
				if (myConfig.bMapWallpaper)
				{
					cairo_save (myDrawContext);
					cairo_scale (myDrawContext, 
						(double) myData.switcher.fOneViewportWidth / g_iScreenWidth[CAIRO_DOCK_HORIZONTAL],
						(double) myData.switcher.fOneViewportHeight / g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
					cairo_set_source_surface (myDrawContext, pDesktopBgSurface,0, 0);
					cairo_paint(myDrawContext);
					cairo_restore (myDrawContext);
				}
				cairo_set_source_rgba(myDrawContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
				cairo_fill(myDrawContext);
				cairo_restore (myDrawContext);
			}
			else
			{
				//Ici on passe aux rectangle suivant de la premiere ligne.
				cd_message (" bureau 1ere ligne %d -- %d",myData.switcher.ScreenCurrentNums,i  );
				
				cairo_save(myDrawContext);
				
				cairo_rectangle(myDrawContext, myData.switcher.fOneViewportWidth*i, 0,myData.switcher.fOneViewportWidth , myData.switcher.fOneViewportHeight);
				if (myConfig.bInvertIndicator)
				{
					if (myData.switcher.ScreenCurrentNums == i)
					{
						cairo_stroke(myDrawContext);
						cairo_restore (myDrawContext);
					}
					else
					{
						cairo_stroke_preserve(myDrawContext);
						cairo_restore (myDrawContext);
					}
				}
				else
				{
					if (myData.switcher.ScreenCurrentNums == i)
					{
						cairo_stroke_preserve(myDrawContext);
						cairo_restore (myDrawContext);
					}
					else
					{
						cairo_stroke(myDrawContext);
						cairo_restore (myDrawContext);
					}
				}
				if (myConfig.bMapWallpaper)
				{
					cairo_save (myDrawContext);
					cairo_scale (myDrawContext,
						(double) myData.switcher.fOneViewportWidth / g_iScreenWidth[CAIRO_DOCK_HORIZONTAL],
						(double) myData.switcher.fOneViewportHeight / g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
					
					cairo_set_source_surface (myDrawContext, pDesktopBgSurface,g_iScreenWidth[CAIRO_DOCK_HORIZONTAL]*i, 0);
					cairo_paint(myDrawContext);
					cairo_restore (myDrawContext);
				}
				cairo_set_source_rgba(myDrawContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
				cairo_fill(myDrawContext);
				cairo_restore(myDrawContext);
			}
		}
		else
		{
			//Ici on passe aux rectangle de la deuxième ligne.
			cairo_save(myDrawContext);
			cd_message (" bureau 2eme ligne %d -- %d",myData.switcher.ScreenCurrentNums,i );
			cairo_save(myDrawContext);
			if (i-myData.switcher.iNbDesktopByLine==0)	//Test si c'est le premier de la ligne numero 2
			{
				cairo_rectangle(myDrawContext, 0, myData.switcher.fOneViewportHeight,myData.switcher.fOneViewportWidth , myData.switcher.fOneViewportHeight+myData.switcher.fOneViewportHeight);
				cairo_save(myDrawContext);
				if (myConfig.bInvertIndicator)
				{
					if (myData.switcher.ScreenCurrentNums == i)
					{
						cairo_stroke(myDrawContext);
						cairo_restore (myDrawContext);
					}
					else
					{
						cairo_stroke_preserve(myDrawContext);
						cairo_restore (myDrawContext);
					}
				}
				else
				{
					if (myData.switcher.ScreenCurrentNums == i)
					{
						cairo_stroke_preserve(myDrawContext);
						cairo_restore (myDrawContext);
					}
					else
					{
						cairo_stroke(myDrawContext);
						cairo_restore (myDrawContext);
					}
				}
				if (myConfig.bMapWallpaper)
				{
					cairo_save (myDrawContext);
					cairo_scale (myDrawContext,
						(double) myData.switcher.fOneViewportWidth / g_iScreenWidth[CAIRO_DOCK_HORIZONTAL],
						(double) myData.switcher.fOneViewportHeight / g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
					
					cairo_set_source_surface (myDrawContext, pDesktopBgSurface,0, g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
					cairo_paint(myDrawContext);
					cairo_restore (myDrawContext);
				}
				cairo_set_source_rgba(myDrawContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
				cairo_fill(myDrawContext);
				cairo_restore (myDrawContext);
			}
			else
			{
				// là on dessine le restant des rectangles
				
				cairo_save(myDrawContext);
				cairo_save(myDrawContext);
				cairo_rectangle(myDrawContext, myData.switcher.fOneViewportWidth*(i-myData.switcher.iNbDesktopByLine), myData.switcher.fOneViewportHeight,myData.switcher.fOneViewportWidth , myData.switcher.fOneViewportHeight+myData.switcher.fOneViewportHeight);
				if (myConfig.bInvertIndicator)
				{
					if (myData.switcher.ScreenCurrentNums == i)
					{
						cairo_stroke(myDrawContext);
						cairo_restore (myDrawContext);
					}
					else
					{
						cairo_stroke_preserve(myDrawContext);
						cairo_restore (myDrawContext);
					}
				}
				else
				{
					if (myData.switcher.ScreenCurrentNums == i)
					{
						cairo_stroke_preserve(myDrawContext);
						cairo_restore (myDrawContext);
					}
					else
					{
						cairo_stroke(myDrawContext);
						cairo_restore (myDrawContext);
					}
				}
				if (myConfig.bMapWallpaper)
				{
					cairo_save (myDrawContext);
					
					cairo_scale (myDrawContext,
					(double) myData.switcher.fOneViewportWidth / g_iScreenWidth[CAIRO_DOCK_HORIZONTAL],
					(double) myData.switcher.fOneViewportHeight / g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
					
					cairo_set_source_surface (myDrawContext,
						pDesktopBgSurface,
						(double) g_iScreenWidth[CAIRO_DOCK_HORIZONTAL]*(i-myData.switcher.iNbDesktopByLine),
						(double) g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
					cairo_paint(myDrawContext);
					cairo_restore (myDrawContext);
				}
			
				cairo_set_source_rgba(myDrawContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
				cairo_fill(myDrawContext);
				cairo_restore (myDrawContext);
				
				g_print ("myData.switcher.fOneViewportWidth*(i-myData.switcher.iNbDesktopByLine) : %f \n",myData.switcher.fOneViewportWidth*(i-myData.switcher.iNbDesktopByLine));
			}
		
		}
	
	}
	
	// On trace le contour carre de l'icone principale.
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_set_line_width (myDrawContext,myConfig.iLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
	
	cairo_rectangle(myDrawContext, 0, 0,myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
	cairo_stroke (myDrawContext);
	
	cairo_restore (myDrawContext);*/
}


/*Ici le dessin de l'icone principale en mode sous dock*/
// dessine le contour de l'icone, et le fond d'ecran dedans.
/*void switcher_draw_sub_dock_main_icon_back (cairo_t *myDrawContext, Icon *myIcon, CairoContainer *pContainer)
{
	cairo_save (myDrawContext);

	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	cairo_set_line_width (myDrawContext,myConfig.iLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
	cairo_rectangle(myDrawContext, 0, 0,myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
	cairo_stroke(myDrawContext);
	
	if (myConfig.bMapWallpaper)
	{
		cairo_surface_t *pDesktopBgSurface = cairo_dock_get_desktop_bg_surface ();
		if (pDesktopBgSurface != NULL)
		{
			cairo_scale (myDrawContext,
				(myIcon->fWidth * fMaxScale - 2*myConfig.iLineSize) / g_iScreenWidth[CAIRO_DOCK_HORIZONTAL],
				(myIcon->fWidth * fMaxScale - 2*myConfig.iLineSize) / g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
			
			cairo_set_source_surface (myDrawContext,
				pDesktopBgSurface,
				myConfig.iLineSize,
				myConfig.iLineSize);
			cairo_paint(myDrawContext);
			///cairo_fill(myDrawContext);
		}
	}
	
	cairo_restore (myDrawContext);
}*/



/*La fonction generale d'appel du dessin*/
/*gboolean switcher_draw_main_dock_icon (void )
{
	cairo_surface_t *pSurface = myData.pSurface;
	g_return_val_if_fail (pSurface != NULL, TRUE);
	
	cairo_dock_set_icon_surface (myDrawContext,pSurface);
	
	switcher_draw_main_dock_icon_back (myDrawContext, myIcon, myContainer);
	
	cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer);

	CD_APPLET_REDRAW_MY_ICON
}*/




void cd_switcher_draw_main_icon_expanded_mode (void)
{
	/*if (myConfig.bMapWallpaper)
	{
		cairo_surface_t *pDesktopBgSurface = cairo_dock_get_desktop_bg_surface ();
		if (pDesktopBgSurface != NULL)
		{
			double fMaxScale = cairo_dock_get_max_scale (myContainer);
			cairo_scale (myDrawContext,
				myIcon->fWidth * fMaxScale / g_iScreenWidth[CAIRO_DOCK_HORIZONTAL],
				myIcon->fWidth * fMaxScale / g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
			
			cairo_set_source_surface (myDrawContext,
				pDesktopBgSurface,
				0.,
				0.);
			cairo_paint(myDrawContext);
		}
	}
	else */if (myIcon->acFileName == NULL)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE)
	}
}
/*La fonction generale d'appel du dessin*/
/*gboolean switcher_draw_sub_dock_main_icon (void )
{
	cairo_surface_t *pSurfaceSDock = myData.pSurfaceSDock;
	g_return_val_if_fail (pSurfaceSDock != NULL, TRUE);
	
	cairo_dock_set_icon_surface (myDrawContext,pSurfaceSDock);
	if (myConfig.bMapWallpaper)
	{
		switcher_draw_sub_dock_main_icon_back (myDrawContext, myIcon, myContainer);
	}
	
	cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer);
	
	CD_APPLET_REDRAW_MY_ICON
}*/

/*Fonction de base pour toutes les autres*/
void cd_switcher_draw_main_icon (void)
{
	if (myConfig.bCompactView)
	{
		cd_switcher_draw_main_icon_compact_mode ();
	}
	else
	{
		cd_switcher_draw_main_icon_expanded_mode ();
	}
	
	cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer);
	CD_APPLET_REDRAW_MY_ICON
}


