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


void switcher_draw_main_dock_iconsold (cairo_surface_t *pSurface)
{
	cairo_save (myDrawContext);
	//cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, myContainer);  // on n'utilise pas la macro car on ne veut pas du redraw.
	
	cairo_restore (myDrawContext);
	cairo_save (myDrawContext);
	double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);
	//double fMaxScale = 50.00;
//cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
//cairo_paint (myDrawContext);
int i;
for (i = 0; i < g_iNbDesktops; i ++)
	{
	
//while (i < g_iNbDesktops +1)
//	{
printf("i : %d \n", i);
double defineline = myIcon->fHeight * fMaxScale / 2; //hauteur diviser par 2
 //

double definedeskparline = g_iNbDesktops / 2; //
double maxparline = myIcon->fWidth * fMaxScale / definedeskparline;
double scren = g_iNbDesktops / maxparline;
//double definedeskWline = myIcon->fWidth * fMaxScale / 
double screenHW = myIcon->fWidth * fMaxScale / definedeskparline; //largeur icone
printf("fHeight : %f \n", scren);
printf("fHeight : %f \n", screenHW);
int j;
  for (j = 0; j < maxparline; j ++)
	{
if (myIcon->fWidth * i < myIcon->fWidth * fMaxScale)
	{
//cairo_save (myDrawContext);
//cairo_restore (myDrawContext);
cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
  cairo_rectangle(myDrawContext, screenHW*j, 0,screenHW , defineline);
  cairo_stroke(myDrawContext);
 cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
 cairo_fill(myDrawContext);
//cairo_translate(myDrawContext, 0, 0);
//printf("taille pas encore atteinte : %f \n", myIcon->fWidth);
//cairo_save (myDrawContext);
}
else
{
//cairo_restore (myDrawContext);
	 cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
  cairo_rectangle(myDrawContext,screenHW*j, defineline, screenHW, defineline);
  cairo_stroke(myDrawContext);
  cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
  cairo_fill(myDrawContext);
  cairo_translate(myDrawContext, 0, 0);
//cairo_save (myDrawContext);
//printf("taille atteinte : %f \n", myIcon->fWidth);
}
 //i++;
}
}
//CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", myData.switcher.ScreenCurrentNums)
cairo_save (myDrawContext);
cairo_destroy(myDrawContext);
 // cairo_destroy(myDrawContext);
	
	//cairo_move_to (myDrawContext, 3, myIcon->fHeight * fMaxScale - 3);
	//cairo_rel_line_to (myDrawContext, (myIcon->fWidth * fMaxScale - 6) * 1000 * .01, 0);
	
	//cairo_set_source (myDrawContext, pGradationPattern);
	//cairo_stroke (myDrawContext);
	
	//cairo_pattern_destroy (pGradationPattern);
	//
	//cairo_restore (myDrawContext);
//cairo_save (myDrawContext);
//CD_APPLET_REDRAW_MY_ICON
}


void switcher_draw_main_dock_icon (cairo_surface_t *pSurface)
{
//if (myConfig.bCurrentView)
//		{
cairo_save (myDrawContext);

	cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, myContainer);  // on n'utilise pas la macro car on ne veut pas du redraw.
	cairo_restore (myDrawContext);
	cairo_save (myDrawContext);
	cd_message ("ok%d ",myData.switcher.ScreenCurrentNums);
cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	

cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
  cairo_rectangle(myDrawContext,0, 0,25 , 25);
  cairo_stroke_preserve(myDrawContext);
 cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
 cairo_fill_preserve(myDrawContext);
cairo_paint (myDrawContext);


	cairo_restore (myDrawContext);
	CD_APPLET_REDRAW_MY_ICON



//}

}


void switcher_draw_subdock_icon (cairo_surface_t *pSurface)
{
	cairo_save (myDrawContext);
	cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, myContainer);  // on n'utilise pas la macro car on ne veut pas du redraw.
	
	cairo_restore (myDrawContext);
	cairo_save (myDrawContext);
	double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);
	//double fMaxScale = 50.00;
cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
	//cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
int i;
for (i = 0; i < g_iNbDesktops; i ++)
	{
	
//while (i < g_iNbDesktops +1)
//	{
printf("i : %d \n", i);
double defineline = myIcon->fHeight * fMaxScale / 2; //hauteur diviser par 2
 //

double definedeskparline = g_iNbDesktops / 2; //
double maxparline = myIcon->fWidth * fMaxScale / definedeskparline;
double scren = g_iNbDesktops / maxparline;
//double definedeskWline = myIcon->fWidth * fMaxScale / 
double screenHW = myIcon->fWidth * fMaxScale / definedeskparline; //largeur icone
printf("fHeight : %f \n", scren);
printf("fHeight : %f \n", screenHW);
int j;
  for (j = 0; j < maxparline; j ++)
	{
if (myIcon->fWidth * i < myIcon->fWidth * fMaxScale)
	{

cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
  cairo_rectangle(myDrawContext, screenHW*j, 0,screenHW , defineline);
  cairo_stroke(myDrawContext);
 cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
 cairo_fill(myDrawContext);
//cairo_translate(myDrawContext, 0, 0);
//printf("taille pas encore atteinte : %f \n", myIcon->fWidth);

}
else
{
	 cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
  cairo_rectangle(myDrawContext,screenHW*j, defineline, screenHW, defineline);
  cairo_stroke(myDrawContext);
  cairo_set_source_rgb(myDrawContext, 0.6, 0.6, 0.6);
  cairo_fill(myDrawContext);
  cairo_translate(myDrawContext, 0, 0);
//printf("taille atteinte : %f \n", myIcon->fWidth);
}
 //i++;
}
}


  cairo_destroy(myDrawContext);
	
	//cairo_move_to (myDrawContext, 3, myIcon->fHeight * fMaxScale - 3);
	//cairo_rel_line_to (myDrawContext, (myIcon->fWidth * fMaxScale - 6) * 1000 * .01, 0);
	
	//cairo_set_source (myDrawContext, pGradationPattern);
	//cairo_stroke (myDrawContext);
	
	//cairo_pattern_destroy (pGradationPattern);
	
	cairo_restore (myDrawContext);
		
}
