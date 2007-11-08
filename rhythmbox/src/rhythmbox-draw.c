#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#include "rhythmbox-draw.h"

//Inclusion des variables de dessins
extern Icon *rhythmbox_pIcon;
extern CairoDock *rhythmbox_pDock;
extern cairo_t *rhythmbox_pCairoContext;
extern int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS];
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;
extern double fImageWidth;
extern double fImageHeight;

//Inclusion des variables de configuration
extern int conf_changeAnimation;


//*********************************************************************************
// rhythmbox_iconWitness() : Effectue une animation de l'icone
//*********************************************************************************
void rhythmbox_iconWitness(int animationLenght)
{
	if(animationLenght > 0)
	{
		cairo_dock_arm_animation (rhythmbox_pIcon, conf_changeAnimation, animationLenght);
		cairo_dock_start_animation (rhythmbox_pIcon, rhythmbox_pDock);
	}
}


//*********************************************************************************
// rhythmbox_changeIconName() : Change l'image de l'icone
//*********************************************************************************
void rhythmbox_setIconSurface(cairo_surface_t *Surface)
{
	//Destrucion de l'image
	cairo_set_source_rgba (rhythmbox_pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (rhythmbox_pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (rhythmbox_pCairoContext);
	cairo_set_operator (rhythmbox_pCairoContext, CAIRO_OPERATOR_OVER);
	
	//Chargement de la nouvelle image
	cairo_set_source_surface (
		rhythmbox_pCairoContext,
		Surface,
		0,
		0);
	cairo_paint (rhythmbox_pCairoContext);
	
	cairo_dock_redraw_my_icon (rhythmbox_pIcon, rhythmbox_pDock);
}


//*********************************************************************************
// rhythmbox_changeIconName() : Change le nom de l'icone
//*********************************************************************************
void rhythmbox_setIconName(const gchar *IconName)
{
	g_free (rhythmbox_pIcon->acName);
	rhythmbox_pIcon->acName = g_strdup (IconName);
	cairo_dock_fill_one_text_buffer(
		rhythmbox_pIcon,
		rhythmbox_pCairoContext,
		g_iLabelSize,
		g_cLabelPolice,
		(g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : g_pMainDock->bHorizontalDock));
}
