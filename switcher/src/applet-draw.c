/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-read-data.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;
gboolean my_bRotateIconsOnEllipse = TRUE;







//void switcher_draw_main_dock_iconold (cairo_surface_t *pSurface)
void switcher_draw_main_dock_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer)
{
	cairo_save (pIconContext);


	cairo_restore (pIconContext);
	cairo_save (pIconContext);
	///double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);

	
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
cairo_select_font_face              (pIconContext,
                                                         "arial",
                                                         CAIRO_FONT_SLANT_ITALIC,
                                                         CAIRO_FONT_WEIGHT_BOLD);
cairo_set_font_size                 (pIconContext,
      14);

 // double defineline = pIcon->fHeight * fMaxScale / 2; //hauteur diviser par 2
 //
//myData.switcher.RedColor = myConfig.cColorBox;
printf("myData.switcher.iNbViewportX : %d \n",myData.switcher.iNbViewportX);
double fMaxScale = (myDock ? 1 + g_fAmplitude : 1); //coefficient Max icone Width
myData.switcher.MaxNbLigne = pIcon->fHeight * fMaxScale / 2; //hauteur diviser par 2
myData.switcher.NumDeskbyLigne = myData.switcher.iNbViewportX / 2; //Bureau diviser par 2 : obtenir nombre de bureau par ligne
myData.switcher.MaxWidthIcon = pIcon->fWidth * fMaxScale / myData.switcher.NumDeskbyLigne; //largeur icone
myData.switcher.MaxHeightIcon = pIcon->fHeight * fMaxScale / myData.switcher.MaxNbLigne; //hauteur icone

//cairo_set_source_rgba(pIconContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
cairo_set_line_width (pIconContext,myConfig.cLineSize);                                             
int i;
for (i = 0; i <myData.switcher.iNbViewportX; i ++)
	{
cd_message ("test i = %d-%d", i,myData.switcher.NumDeskbyLigne);
	if (i < myData.switcher.NumDeskbyLigne)
		{
			if (i <= 0 )
			{
			cd_message (" bureau 1 %d -- %d ",myData.switcher.NumDeskbyLigne,i );
			cairo_set_source_rgba(pIconContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
  			//cairo_rectangle(pIconContext, 0, 0,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne);
			cairo_save(pIconContext);
				if (myData.switcher.ScreenCurrentNums == i)
				{
cd_message (" bureau 0-1 ok %d " ,i );
cairo_rectangle(pIconContext, 0, 0,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne);
				cairo_stroke_preserve(pIconContext);
				cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 				cairo_fill(pIconContext);
				cairo_restore (pIconContext);
				}
				else
				{
cd_message (" bureau 0-2 pas ok  " ,i );
cairo_rectangle(pIconContext, 0, 0,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne);
  				cairo_stroke(pIconContext);
				cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
				cairo_fill(pIconContext);
				cairo_restore (pIconContext);


				}
			}
			else
			{
			cd_message (" bureau 1ere ligne %d -- %d",myData.switcher.ScreenCurrentNums,i  );
			cairo_set_source_rgba(pIconContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
  			//cairo_rectangle(pIconContext, myData.switcher.MaxWidthIcon*i, 0,myData.switcher.MaxWidthIcon*i , myData.switcher.MaxNbLigne);
			cairo_save(pIconContext);
				if (myData.switcher.ScreenCurrentNums == i)
				{cd_message (" bureau 1-1 ok  %d " ,i );
cairo_rectangle(pIconContext, myData.switcher.MaxWidthIcon*i, 0,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne);
				cairo_stroke_preserve(pIconContext);
				cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 				cairo_fill(pIconContext);
	cairo_restore(pIconContext);
				}
				else
				{cd_message (" bureau 1-2 pas ok  %d " ,i );
  			
cairo_rectangle(pIconContext, myData.switcher.MaxWidthIcon*i, 0,myData.switcher.MaxWidthIcon*i , myData.switcher.MaxNbLigne);
cairo_stroke(pIconContext);
				cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 				cairo_fill(pIconContext);
cairo_restore(pIconContext);
				}
			}
		}
			else
			{
			cd_message (" bureau 2eme ligne %d -- %d",myData.switcher.ScreenCurrentNums,i );
cairo_save(pIconContext);
				if (i-myData.switcher.NumDeskbyLigne==0)
				{
					
					cairo_set_source_rgba(pIconContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
 					//cairo_rectangle(pIconContext, 0, myData.switcher.MaxNbLigne,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne+myData.switcher.MaxNbLigne);
					cairo_save(pIconContext);
					if (myData.switcher.ScreenCurrentNums == i)
					{cd_message (" bureau 2-3 ok %d " ,i);
cairo_rectangle(pIconContext, 0, myData.switcher.MaxNbLigne,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne+myData.switcher.MaxNbLigne);
					cairo_stroke_preserve(pIconContext);
					cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 					cairo_fill(pIconContext);
					cairo_restore (pIconContext);
					}	
					else
					{cd_message (" bureau 2-3 pas ok  %d " ,i );
cairo_rectangle(pIconContext, 0, myData.switcher.MaxNbLigne,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne+myData.switcher.MaxNbLigne);
					cairo_stroke(pIconContext);
					cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 					cairo_fill(pIconContext);
					cairo_restore (pIconContext);
					}
				}
				else
				{
			cairo_set_source_rgba(pIconContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
  			
			cairo_save(pIconContext);
				if (myData.switcher.ScreenCurrentNums == i)
				{cd_message (" bureau 2-4 ok  %d " ,i );
					
cairo_rectangle(pIconContext, myData.switcher.MaxWidthIcon*(i-myData.switcher.NumDeskbyLigne), myData.switcher.MaxNbLigne,myData.switcher.MaxWidthIcon , myData.switcher.MaxNbLigne+myData.switcher.MaxNbLigne);
cairo_stroke_preserve(pIconContext);
					cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 					cairo_fill(pIconContext);
					cairo_restore (pIconContext);
					}
					else
					{cd_message (" bureau 2-4 pas ok  %d " ,i );
cairo_rectangle(pIconContext, myData.switcher.MaxWidthIcon*(i-myData.switcher.NumDeskbyLigne), myData.switcher.MaxNbLigne,myData.switcher.MaxWidthIcon*(i-myData.switcher.NumDeskbyLigne) , myData.switcher.MaxNbLigne+myData.switcher.MaxNbLigne);
  					cairo_stroke(pIconContext);
					cairo_set_source_rgba(pIconContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
 					cairo_fill(pIconContext);
					cairo_restore (pIconContext);
					}

				}
				
				//}
	

		}
	}

	
	//cairo_restore (pIconContext);

}
/*
void switcher_draw_main_dock_icon_backkkkk(cairo_t *pIconContext, Icon *pIcon, CairoDockContainer *pContainer)
{
	cairo_save (pIconContext);
	cairo_restore (pIconContext);
	cairo_save (pIconContext);

printf("myData.switcher.iNbViewportX : %d \n",myData.switcher.iNbViewportX);
double fMaxScale = (myDock ? 1 + g_fAmplitude : 1); //coefficient Max icone Width
myData.switcher.MaxNbLigne = pIcon->fHeight * fMaxScale / 2; //hauteur diviser par 2
myData.switcher.NumDeskbyLigne = myData.switcher.iNbViewportX / 2; //Bureau diviser par 2 : obtenir nombre de bureau par ligne
myData.switcher.MaxWidthIcon = pIcon->fWidth * fMaxScale / myData.switcher.NumDeskbyLigne; //largeur icone
myData.switcher.MaxHeightIcon = pIcon->fHeight * fMaxScale / myData.switcher.MaxNbLigne; //hauteur icone

cairo_set_source_rgba(pIconContext,0.9, 0.9, 0.9, 0.9);
cairo_set_line_width (pIconContext,1.0);

int i;
	for (i = 1; i <myData.switcher.iNbViewportX-1; i ++)
	{

	if (i < myData.switcher.NumDeskbyLigne)
		{
// dessin espace de travail 1ere ligne
        cairo_move_to (pIconContext,
                     	myData.switcher.MaxWidthIcon*i,
                       5);
        cairo_line_to (pIconContext,
                        myData.switcher.MaxWidthIcon*i,
                        myData.switcher.MaxNbLigne);
			
        cairo_stroke (pIconContext);
if (myData.switcher.ScreenCurrentNums == i)
	{
	
cairo_save(pIconContext);
cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.3);
cairo_rectangle (pIconContext, myData.switcher.MaxWidthIcon*i+4.5, 5, myData.switcher.MaxWidthIcon-10, myData.switcher.MaxNbLigne-10);
cairo_fill (pIconContext);
cairo_restore(pIconContext);
}
else
if (myData.switcher.ScreenCurrentNums == 0)
	{
cairo_save(pIconContext);
cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.3);
cairo_rectangle (pIconContext, 4.5, 5, myData.switcher.MaxWidthIcon-10, myData.switcher.MaxNbLigne-10);
cairo_fill (pIconContext);
cairo_restore(pIconContext);
}
		}
	else
		{
// dessin espace de travail 2eme ligne
  cairo_move_to (pIconContext,
                      	myData.switcher.MaxWidthIcon* (i-(myData.switcher.NumDeskbyLigne-1)),
                       myData.switcher.MaxNbLigne );
        cairo_line_to (pIconContext,
                        myData.switcher.MaxWidthIcon* (i-(myData.switcher.NumDeskbyLigne-1)),
                        (myData.switcher.MaxNbLigne+myData.switcher.MaxNbLigne) -5);
        cairo_stroke (pIconContext);
if (myData.switcher.ScreenCurrentNums == i)
	{
printf("i 1 : %d \n", i);
cairo_save(pIconContext);
cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.3);
cairo_rectangle (pIconContext, myData.switcher.MaxWidthIcon* (i-(myData.switcher.NumDeskbyLigne))+4, myData.switcher.MaxNbLigne+4.5, myData.switcher.MaxWidthIcon-10, myData.switcher.MaxNbLigne-10);
cairo_fill (pIconContext);
cairo_restore(pIconContext);
}
else
if (myData.switcher.ScreenCurrentNums  == i + 1)
	{
printf("i 2 : %d \n", i);
cairo_save(pIconContext);
cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.3);
cairo_rectangle (pIconContext, myData.switcher.MaxWidthIcon* ((i+1)-(myData.switcher.NumDeskbyLigne))+4, myData.switcher.MaxNbLigne+4.5, myData.switcher.MaxWidthIcon-10, myData.switcher.MaxNbLigne-10);
cairo_fill (pIconContext);
cairo_restore(pIconContext);
}

		}
	}

// dessin ligne horizontale
       cairo_move_to (pIconContext,
                        3,
                        myData.switcher.MaxNbLigne);
        cairo_line_to (pIconContext,
                        pIcon->fWidth * fMaxScale-5,
                        myData.switcher.MaxNbLigne);
        cairo_stroke (pIconContext);



cairo_restore (pIconContext);
}
*/




gboolean switcher_draw_main_dock_icon (gpointer data)
{
//if (myConfig.bCurrentView)
//		{

	//

cairo_surface_t *pSurface = myData.pSurface;

	g_return_val_if_fail (pSurface != NULL, TRUE);
	

	
		cairo_save (myDrawContext);

cairo_dock_set_icon_surface_full (myDrawContext, pSurface, 1., 1., myIcon, myContainer); 

switcher_draw_main_dock_icon_back  (myDrawContext, myIcon, myContainer);
	
		cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer); 
		//cairo_dock_redraw_my_icon (myIcon, myContainer);

//cairo_destroy (myDrawContext);
CD_APPLET_REDRAW_MY_ICON
//cairo_destroy (myDrawContext);



//}

}

gboolean _cd_switcher_check_for_redraw_cairo (gpointer data)
{
//cairo_surface_t *pSurface = (myData.pSurface);

if (myConfig.bCurrentView)
		{
//cairo_dock_get_nb_viewports (&myData.switcher.iNbViewportX, &myData.switcher.iNbViewportY);
switcher_draw_main_dock_icon (myData.pSurface);
CD_APPLET_REDRAW_MY_ICON
cd_message ("dessiner");

}
//else
//{
	if (myConfig.bShowSubDock)
		{
cd_message ("dessiner sous dock cairo");
//switcher_draw_subdock_icon (myData.pSurface);
}
else
{
cd_message ("pas de dessin");
}
}


