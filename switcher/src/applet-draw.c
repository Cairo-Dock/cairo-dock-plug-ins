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
void switcher_draw_main_dock_icon_backdfdfdf (cairo_t *pIconContext, Icon *pIcon, CairoDockContainer *pContainer)
{
	cairo_save (pIconContext);


	cairo_restore (pIconContext);
	cairo_save (pIconContext);
	double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);

	
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
cairo_select_font_face              (pIconContext,
                                                         "arial",
                                                         CAIRO_FONT_SLANT_ITALIC,
                                                         CAIRO_FONT_WEIGHT_BOLD);
cairo_set_font_size                 (pIconContext,
      14);

  double defineline = pIcon->fHeight * fMaxScale / 2; //hauteur diviser par 2
 //

int definedeskparline = g_iNbDesktops / 2; //
double maxparline = pIcon->fWidth * fMaxScale / definedeskparline;
double scren = g_iNbDesktops / maxparline;
double screenHW = pIcon->fWidth * fMaxScale / definedeskparline; //largeur icone                                                 
int i;
for (i = 0; i < g_iNbDesktops; i ++)
	{

//printf("i : %d \n", i);

//printf("fHeight : %f \n", scren);
//printf("fHeight : %f \n", screenHW);
//printf("max par ligne : %f \n", definedeskparline);
//printf("max par ligne : %f \n", pIcon->fWidth * fMaxScale);
//printf("nombre desktop : %d \n", g_iNbDesktops);
int j;

  for (j = 0; j < definedeskparline; j ++)
	{

if (myData.switcher.ScreenCurrentNums != i)
	{
cd_message (" mauvais bureau " );
cd_message (" j : %d",j );
cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
  cairo_rectangle(pIconContext, screenHW*j, 0,screenHW , defineline);
  cairo_stroke(pIconContext);
cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
 cairo_fill(pIconContext);
}
else
{
cairo_save(pIconContext);
cd_message (" bon bureau : %d", i );
cd_message (" j : %d",j );
cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
  cairo_rectangle(pIconContext, screenHW*j, 0,screenHW , defineline);
cairo_stroke_preserve(pIconContext);
cairo_set_source_rgba(pIconContext,0.9, 0.9, 0.9, 0.9);
 cairo_fill(pIconContext);
//cairo_restore (pIconContext);
}
//cairo_save(pIconContext);

//
// cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
// cairo_fill_preserve(pIconContext);
//
//	}
}
//int i;
//for (i = 0; i < g_iNbDesktops; i ++)
//	{

//printf("i : %d \n", i);

//printf("fHeight : %f \n", scren);
//printf("fHeight : %f \n", screenHW);
//printf("max par ligne : %f \n", definedeskparline);
//printf("max par ligne : %f \n", pIcon->fWidth * fMaxScale);
//printf("nombre desktop : %d \n", g_iNbDesktops);
//int j;

  for (j = 0; j < definedeskparline; j ++)

	{
//pango_cairo_show_layout (pIconContext, pLayout);
//cairo_text_path(pIconContext,"2");
//cairo_save (pIconContext);
if (myData.switcher.ScreenCurrentNums != i)
	{
	 cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
  cairo_rectangle(pIconContext,screenHW*j, defineline, screenHW, defineline);
//cairo_save(pIconContext);
cd_message (" mauvais bureau " );
cd_message (" j : %d",j );
cairo_stroke(pIconContext);
//cairo_restore (pIconContext);
 cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
  cairo_fill(pIconContext);
}
else
{
cairo_save(pIconContext);
cd_message (" bon bureau : %d", i );
cd_message (" j : %d",j );
	 cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
  cairo_rectangle(pIconContext,screenHW*j, defineline, screenHW, defineline);
//cairo_save (pIconContext);
cairo_stroke_preserve(pIconContext);
//cairo_restore (pIconContext);
 cairo_set_source_rgba(pIconContext,0.9, 0.9, 0.9, 0.9);
  cairo_fill(pIconContext);
//cairo_restore (pIconContext);
}

//cairo_save(pIconContext);
  //cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
  //cairo_fill_preserve(pIconContext);


	}
	}
	//}
//pango_cairo_show_layout (myDrawContext, pLayout);
	
	cairo_restore (pIconContext);

}
void switcher_draw_main_dock_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoDockContainer *pContainer)
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



void switcher_draw_subdock_icon_back (cairo_surface_t *pSurface)
{
	cairo_save (myDrawContext);
	//cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, myContainer);  // on n'utilise pas la macro car on ne veut pas du redraw.
	
	cairo_restore (myDrawContext);
	cairo_save (myDrawContext);
	double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);
	//double fMaxScale = 50.00;
cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
GList *pMainList = NULL;
Icon *pIcon;
	//cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
int i;
for (i = 0; i < myData.switcher.iNbViewportX; i ++)
	{
	
//while (i < g_iNbDesktops +1)
//	{
printf("i : %d \n", i);





	pIcon = g_new0 (Icon, 1);
	if (myData.switcher.ScreenCurrentNums == i)
		{
	pIcon->cQuickInfo = g_strdup_printf ("%d",i);
	pIcon->fScale = 1.;
	pIcon->fAlpha = .1;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->cParentDockName = g_strdup (myIcon->acName);
}
else
		{
	pIcon->cQuickInfo = g_strdup_printf ("%d",i);
	pIcon->fScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->cParentDockName = g_strdup (myIcon->acName);
	}
	pMainList = g_list_append (pMainList, pIcon);

//cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
  //cairo_rectangle(myDrawContext, screenHW*j, 0,screenHW , defineline);
  //cairo_stroke(myDrawContext);
 //cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
 //cairo_fill(myDrawContext);
//cairo_translate(myDrawContext, 0, 0);
//printf("taille pas encore atteinte : %f \n", myIcon->fWidth);



}


  cairo_destroy(myDrawContext);
	
	//cairo_move_to (myDrawContext, 3, myIcon->fHeight * fMaxScale - 3);
	//cairo_rel_line_to (myDrawContext, (myIcon->fWidth * fMaxScale - 6) * 1000 * .01, 0);
	
	//cairo_set_source (myDrawContext, pGradationPattern);
	//cairo_stroke (myDrawContext);
	
	//cairo_pattern_destroy (pGradationPattern);
	
	cairo_restore (myDrawContext);
		
}




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
gboolean switcher_draw_subdock_icon (gpointer data)
{
//if (myConfig.bCurrentView)
//		{

	//

cairo_surface_t *pSurface = myData.pSurface;

	g_return_val_if_fail (pSurface != NULL, TRUE);
	

	
		cairo_save (myDrawContext);

cairo_dock_set_icon_surface_full (myDrawContext, pSurface, 1., 1., myIcon, myContainer); 

switcher_draw_subdock_icon_back  (NULL);
	
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
cairo_dock_get_nb_viewports (&myData.switcher.iNbViewportX, &myData.switcher.iNbViewportY);
switcher_draw_main_dock_icon (myData.pSurface);
CD_APPLET_REDRAW_MY_ICON
cd_message ("dessiner");
	if (myConfig.bShowSubDock)
		{
//switcher_draw_subdock_icon (myData.pBrokenSurface);
}
}
else
{
cd_message ("pas de dessin");
}
}


