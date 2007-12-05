
#include "rhythmbox-draw.h"

//Inclusion des variables de dessins
extern Icon *myIcon;
extern CairoDock *myDock;
extern cairo_t *myDrawContext;

//Inclusion des variables de configuration
extern CairoDockAnimationType conf_changeAnimation;


//*********************************************************************************
// rhythmbox_iconWitness() : Effectue une animation de l'icone
//*********************************************************************************
void rhythmbox_iconWitness(int animationLenght)
{
	cairo_dock_animate_icon (myIcon, myDock, conf_changeAnimation, animationLenght);
}


//*********************************************************************************
// rhythmbox_changeIconName() : Change l'image de l'icone
//*********************************************************************************
void rhythmbox_setIconSurface(cairo_surface_t *pSurface)
{
	cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, myDock);
	
	cairo_dock_redraw_my_icon (myIcon, myDock);
}


//*********************************************************************************
// rhythmbox_changeIconName() : Change le nom de l'icone
//*********************************************************************************
void rhythmbox_setIconName(const gchar *cIconName)
{
	cairo_dock_set_icon_name (myDrawContext, cIconName, myIcon, myDock);
}
