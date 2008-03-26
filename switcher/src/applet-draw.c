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
void switcher_draw_main_dock_icon_back (cairo_t *pIconContext, Icon *pIcon, CairoDockContainer *pContainer)
{
	cairo_save (pIconContext);

	PangoLayout *pLayout = pango_cairo_create_layout (pIconContext);
	cairo_restore (pIconContext);
	cairo_save (pIconContext);
	double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);

	
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);

int i;
for (i = 0; i < g_iNbDesktops; i ++)
	{

printf("i : %d \n", i);
double defineline = pIcon->fHeight * fMaxScale / 2; //hauteur diviser par 2
 //

int definedeskparline = g_iNbDesktops / 2; //
double maxparline = pIcon->fWidth * fMaxScale / definedeskparline;
double scren = g_iNbDesktops / maxparline;
double screenHW = pIcon->fWidth * fMaxScale / definedeskparline; //largeur icone
printf("fHeight : %f \n", scren);
printf("fHeight : %f \n", screenHW);
printf("max par ligne : %f \n", definedeskparline);
printf("max par ligne : %f \n", pIcon->fWidth * fMaxScale);
printf("nombre desktop : %d \n", g_iNbDesktops);
int j;
//pango_layout_set_text (pLayout, &i, -1);	
	
  for (j = 0; j < maxparline; j ++)
	{
if (pIcon->fWidth * i < pIcon->fWidth * fMaxScale)
	{

cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
//pango_cairo_show_layout (pIconContext, pLayout);
  cairo_rectangle(pIconContext, screenHW*j, 0,screenHW , defineline);
  cairo_stroke(pIconContext);
 cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
 cairo_fill_preserve(pIconContext);
//
}
else
{

	 cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
  cairo_rectangle(pIconContext,screenHW*j, defineline, screenHW, defineline);
//pango_cairo_show_layout (pIconContext, pLayout);
  cairo_stroke(pIconContext);
  cairo_set_source_rgba(pIconContext,0.6, 0.6, 0.6, 0.6);
  cairo_fill_preserve(pIconContext);


}

}
}	
	cairo_restore (pIconContext);

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
gboolean _cd_switcher_check_for_redraw_cairo (gpointer data)
{
//cairo_surface_t *pSurface = (myData.pSurface);

if (myConfig.bCurrentView)
		{
switcher_draw_main_dock_icon (myData.pSurface);
CD_APPLET_REDRAW_MY_ICON
cd_message ("dessiner");
}
else
{
cd_message ("pas de dessin");
}
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
  cairo_fill(myDrawContext);CD_APPLET_REDRAW_MY_ICON
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
