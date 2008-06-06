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
#include "applet-read-data.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;
extern cairo_surface_t *g_pDesktopBgSurface;
extern int g_iScreenWidth[2], g_iScreenHeight[2];
gboolean my_bRotateIconsOnEllipse = TRUE;







// Dessin de l'icone principale avec division suivant le nombre de bureau configurer dans compiz .Tous les dessins sont divisés en deux parties, si c'est le bureau courant on preserve le rectangle pour afficher l'indicateur.Je pense que cette fonction est à épurer même si l'ai beaucoup eclairci je la trouve encore fouilli.

void switcher_draw_main_dock_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer)
{
	cairo_save (pIconContext);


	cairo_restore (pIconContext);
	cairo_save (pIconContext);


	
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);


double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
myData.switcher.MaxNbLigne = pIcon->fHeight * fMaxScale / 2; //hauteur diviser par 2
myData.switcher.NumDeskbyLigne = myData.switcher.iNbViewportX / 2; //Bureau diviser par 2 : obtenir nombre de bureau par ligne
myData.switcher.MaxWidthIcon = pIcon->fWidth * fMaxScale / myData.switcher.NumDeskbyLigne; //largeur icone
myData.switcher.MaxHeightIcon = pIcon->fHeight * fMaxScale / myData.switcher.MaxNbLigne; //hauteur icone

if (g_pDesktopBgSurface == NULL)			// ici on teste si le wallpaper a été initialiser.Si non on lance la fonction qui va bien
{
printf("Background null Reload Applet /n");
//cairo_dock_load_desktop_background_surface ();

}


  

int i;
for (i = 0; i <myData.switcher.iNbViewportX; i ++)
	{
							//définition de la taille et de la couleurs des lignes par rapport aux variable de la conf.
cairo_set_line_width (pIconContext,myConfig.cInLineSize);     
cairo_set_source_rgba(pIconContext,myConfig.RGBInLineColors[0],myConfig.RGBInLineColors[1],myConfig.RGBInLineColors[2],myConfig.RGBInLineColors[3]);  
cairo_save(pIconContext);  
cd_message ("test i = %d-%d", i,myData.switcher.NumDeskbyLigne);
	if (i < myData.switcher.NumDeskbyLigne)
		{ 					// Pour dessiner sur deux ligne je teste si i est inferieur au nombre de bureau divisé par 2

			if (i <= 0 ) 
			{ 											// Si il est égale à 0 ( bureau 1)
			cd_message (" bureau 1 %d -- %d ",myData.switcher.NumDeskbyLigne,i );

							/* Pour tous les rectangles c'est identique :
							je dessine le rectangle puis je teste si c'est le bureau courant si oui alors on preserve le rectangle ce qui nous fait l'indicateur.
							Ensuite je redimenssione la surface de wallpaper aux dimension du rectangle je l'applique et je la peint.
							Enfin J'applique la couleur de l'indicateurs.
							C'est dans le calcul pour tracer le rectangle que j'ai mis beaucoup de temps.Surtout pour la deuxieme ligne.*/

			cairo_save(pIconContext);
cairo_rectangle(pIconContext, 0, 0,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne);
				
/* Ici on teste si le booleen de configuration pour inverser l'identification du bureau courant est valide et on inverse l'indication. Fonctionnalité soumise par Nochka*/
if (myConfig.bInvertIndicator)
{
if (myData.switcher.ScreenCurrentNums == i)
{
cairo_stroke(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_stroke_preserve(pIconContext);
cairo_restore (pIconContext);
}
}
else
{
if (myData.switcher.ScreenCurrentNums == i)
{
cairo_stroke_preserve(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_stroke(pIconContext);
cairo_restore (pIconContext);
}
}
if (myConfig.bMapWallpaper)
{
cairo_save (pIconContext);
cairo_scale (pIconContext, myData.switcher.MaxWidthIcon/g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] , myData.switcher.MaxNbLigne/g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
cairo_set_source_surface (pIconContext, g_pDesktopBgSurface,0, 0);
cairo_paint(pIconContext);
cairo_restore (pIconContext);
}
				cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);


 				cairo_fill(pIconContext);
			cairo_restore (pIconContext);


				
			}
			else
			{
									//Ici on passe aux rectangle suivant de la premiere ligne.
			cd_message (" bureau 1ere ligne %d -- %d",myData.switcher.ScreenCurrentNums,i  );
		 
cairo_save(pIconContext);
		
cairo_rectangle(pIconContext, myData.switcher.MaxWidthIcon*i, 0,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne);
				if (myConfig.bInvertIndicator)
{
if (myData.switcher.ScreenCurrentNums == i)
{
cairo_stroke(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_stroke_preserve(pIconContext);
cairo_restore (pIconContext);
}
}
else
{
if (myData.switcher.ScreenCurrentNums == i)
{
cairo_stroke_preserve(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_stroke(pIconContext);
cairo_restore (pIconContext);
}
}
if (myConfig.bMapWallpaper)
{
cairo_save (pIconContext);
cairo_scale (pIconContext, myData.switcher.MaxWidthIcon/g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] , myData.switcher.MaxNbLigne/g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);

cairo_set_source_surface (pIconContext, g_pDesktopBgSurface,g_iScreenWidth[CAIRO_DOCK_HORIZONTAL]*i, 0);
cairo_paint(pIconContext);
cairo_restore (pIconContext);
}
				cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 				cairo_fill(pIconContext);
	cairo_restore(pIconContext);
				
			}
		}
			else
			{
 						//Ici on passe aux rectangle de la deuxième ligne.
cairo_save(pIconContext);
			cd_message (" bureau 2eme ligne %d -- %d",myData.switcher.ScreenCurrentNums,i );
cairo_save(pIconContext);
				if (i-myData.switcher.NumDeskbyLigne==0)	//Test si c'est le premier de la ligne numero 2
				{
				
cairo_rectangle(pIconContext, 0, myData.switcher.MaxNbLigne,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne+myData.switcher.MaxNbLigne);
					cairo_save(pIconContext);
					if (myConfig.bInvertIndicator)
{
if (myData.switcher.ScreenCurrentNums == i)
{
cairo_stroke(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_stroke_preserve(pIconContext);
cairo_restore (pIconContext);
}
}
else
{
if (myData.switcher.ScreenCurrentNums == i)
{
cairo_stroke_preserve(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_stroke(pIconContext);
cairo_restore (pIconContext);
}
}
if (myConfig.bMapWallpaper)
{
cairo_save (pIconContext);
cairo_scale (pIconContext, myData.switcher.MaxWidthIcon/g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] , myData.switcher.MaxNbLigne/g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);

cairo_set_source_surface (pIconContext, g_pDesktopBgSurface,0, g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
cairo_paint(pIconContext);
cairo_restore (pIconContext);
}
					cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 					cairo_fill(pIconContext);
					cairo_restore (pIconContext);
					
				}
				else
				{
	// là on dessine le restant des rectangles

cairo_save(pIconContext);
			cairo_save(pIconContext);
cairo_rectangle(pIconContext, myData.switcher.MaxWidthIcon*(i-myData.switcher.NumDeskbyLigne), myData.switcher.MaxNbLigne,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne+myData.switcher.MaxNbLigne);
				if (myConfig.bInvertIndicator)
{
if (myData.switcher.ScreenCurrentNums == i)
{
cairo_stroke(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_stroke_preserve(pIconContext);
cairo_restore (pIconContext);
}
}
else
{
if (myData.switcher.ScreenCurrentNums == i)
{
cairo_stroke_preserve(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_stroke(pIconContext);
cairo_restore (pIconContext);
}
}
if (myConfig.bMapWallpaper)
{
cairo_save (pIconContext);

cairo_scale (pIconContext, myData.switcher.MaxWidthIcon/(double)g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] , myData.switcher.MaxNbLigne/(double)g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);

cairo_set_source_surface (pIconContext, g_pDesktopBgSurface,(double)g_iScreenWidth[CAIRO_DOCK_HORIZONTAL]*(i-myData.switcher.NumDeskbyLigne), (double)g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
cairo_paint(pIconContext);
cairo_restore (pIconContext);
}

					cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 					cairo_fill(pIconContext);
					cairo_restore (pIconContext);
printf("myData.switcher.MaxWidthIcon*(i-myData.switcher.NumDeskbyLigne) : %f \n",myData.switcher.MaxWidthIcon*(i-myData.switcher.NumDeskbyLigne));
					

				}
				
		}

	}

//cairo_save(pIconContext);
if (myDesklet != NULL)
{
cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
cairo_set_line_width (pIconContext,myConfig.cLineSize);     
cairo_set_source_rgba(pIconContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);  
 
cairo_rectangle(pIconContext, 0, 0,pIcon->fWidth, pIcon->fHeight);
cairo_stroke (pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
cairo_set_line_width (pIconContext,myConfig.cLineSize);     
cairo_set_source_rgba(pIconContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);  
 
cairo_rectangle(pIconContext, 0, 0,pIcon->fWidth*2, pIcon->fHeight*2);
cairo_stroke (pIconContext);
cairo_restore (pIconContext);
}

}


/*Ici le dessin de l'icone principale en mode sous dock*/

void switcher_draw_sub_dock_main_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer)
{
	cairo_save (pIconContext);


	cairo_restore (pIconContext);
	cairo_save (pIconContext);

cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
double fMaxScale = cairo_dock_get_max_scale (myContainer);
cairo_set_line_width (pIconContext,myConfig.cLineSize);     
cairo_set_source_rgba(pIconContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);  
cairo_save(pIconContext);   
//cairo_rectangle(pIconContext, 0, 0,pIcon->fWidth*2, pIcon->fHeight*2);
//cairo_stroke(pIconContext);	
//cairo_restore (pIconContext);
//cairo_save (pIconContext);

if (myDesklet != NULL)
{
cairo_rectangle(pIconContext, 0, 0,pIcon->fWidth, pIcon->fHeight);
cairo_stroke(pIconContext);	
cairo_restore (pIconContext);
cairo_save (pIconContext);
cairo_scale (pIconContext, pIcon->fWidth/(double)g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] , pIcon->fWidth/(double)g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);

cairo_set_source_surface (pIconContext, g_pDesktopBgSurface,0,0);
cairo_paint(pIconContext);
cairo_fill(pIconContext);
cairo_restore (pIconContext);
}
else
{
cairo_rectangle(pIconContext, 0, 0,pIcon->fWidth*2, pIcon->fHeight*2);
cairo_stroke(pIconContext);	
cairo_restore (pIconContext);
cairo_save (pIconContext);
cairo_scale (pIconContext, pIcon->fWidth*2/(double)g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] , pIcon->fWidth*2/(double)g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);

cairo_set_source_surface (pIconContext, g_pDesktopBgSurface,0,0);
cairo_paint(pIconContext);
cairo_fill(pIconContext);
cairo_restore (pIconContext);
}
}



/*La fonction generale d'appel du dessin*/
gboolean switcher_draw_main_dock_icon (gpointer data)
{


cairo_surface_t *pSurface = myData.pSurface;

	g_return_val_if_fail (pSurface != NULL, TRUE);

	
		cairo_save (myDrawContext);

cairo_dock_set_icon_surface_full (myDrawContext,pSurface, 1., 1., myIcon, myContainer); 
	

switcher_draw_main_dock_icon_back  (myDrawContext, myIcon, myContainer);
	
		cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer); 

CD_APPLET_REDRAW_MY_ICON


}



/*La fonction generale d'appel du dessin*/
gboolean switcher_draw_sub_dock_main_icon (gpointer data)
{

cairo_surface_t *pSurfaceSDock = myData.pSurfaceSDock;

	g_return_val_if_fail (pSurfaceSDock != NULL, TRUE);
	

	
		cairo_save (myDrawContext);

cairo_dock_set_icon_surface_full (myDrawContext,pSurfaceSDock, 1., 1., myIcon, myContainer); 
if (myConfig.bMapWallpaper){

switcher_draw_sub_dock_main_icon_back  (myDrawContext, myIcon, myContainer);
}
	
		cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer); 

CD_APPLET_REDRAW_MY_ICON


}

/*Fonction de base pour toutes les autres*/
gboolean _cd_switcher_check_for_redraw_cairo (gpointer data)
{

if (myConfig.bCurrentView)
	{


switcher_draw_main_dock_icon (myData.pSurface);

CD_APPLET_REDRAW_MY_ICON

}
else
{
switcher_draw_sub_dock_main_icon(myData.pSurfaceSDock);
CD_APPLET_REDRAW_MY_ICON
}
}


