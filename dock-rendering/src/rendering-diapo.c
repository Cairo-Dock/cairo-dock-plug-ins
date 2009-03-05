/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

This rendering is (was) written by parAdOxxx_ZeRo, co mah blog : http://paradoxxx.zero.free.fr/ :D

*********************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "rendering-diapo.h"

extern gint     my_diapo_iconGapX;
extern gint     my_diapo_iconGapY;
extern gdouble  my_diapo_fScaleMax;
extern gint     my_diapo_sinW;
extern gboolean my_diapo_lineaire;
extern gboolean  my_diapo_wide_grid;
extern gboolean  my_diapo_text_only_on_pointed;

extern gdouble  my_diapo_color_frame_start[4];
extern gdouble  my_diapo_color_frame_stop[4];
extern gboolean my_diapo_fade2bottom;
extern gboolean my_diapo_fade2right;
extern guint    my_diapo_arrowWidth;
extern guint    my_diapo_arrowHeight;
extern gdouble  my_diapo_arrowShift;
extern guint    my_diapo_lineWidth;
extern guint    my_diapo_radius;
extern gdouble  my_diapo_color_border_line[4];
extern gboolean my_diapo_draw_background;
extern gboolean my_diapo_display_all_icons;

const  gint X_CONST_BORDER_SPACE = 40;
const  gint Y_CONST_BORDER_SPACE = 40;
const  gint  MaxTextWidth = 125; 

void cd_rendering_calculate_max_dock_size_diapo (CairoDock *pDock)
{
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
        
        
//////////////////////////////////////////////////////////////////////////////////////// On calcule la configuration de la grille :
        nIcones = cairo_dock_rendering_diapo_guess_grid(pDock->icons, &nRowsX, &nRowsY);   
        
        
        //////////////////////////////////////////////////////////////////////////////////////// On définit les tailles :
	if(nIcones != 0)
        {
                cairo_dock_calculate_icons_positions_at_rest_diapo (pDock->icons, &(pDock->iMinDockWidth), &(pDock->iMinDockHeight), nRowsX);
	        pDock->iMinDockWidth  += X_CONST_BORDER_SPACE * 2;
	        pDock->iMinDockHeight += Y_CONST_BORDER_SPACE * 3 + my_diapo_arrowHeight-60;	// 60 -> pour que la fleche aille plus bas
	        cairo_dock_rendering_diapo_calculate_max_dock_size (pDock->icons, pDock->iMinDockWidth, pDock->iMinDockHeight, &(pDock->iMaxDockWidth), &(pDock->iMaxDockHeight), nRowsX, nRowsY);
	        pDock->iMaxDockWidth  += X_CONST_BORDER_SPACE * 2;
	        pDock->iMaxDockHeight += Y_CONST_BORDER_SPACE * 3 + my_diapo_arrowHeight-60;	// 60 -> pour que la fleche aille plus bas
                pDock->iMinDockWidth  = pDock->iMaxDockWidth;
	        pDock->iMinDockHeight = pDock->iMaxDockHeight;
        }
        else
        {
                pDock->iMaxDockWidth = pDock->iMaxDockHeight = pDock->iMinDockWidth = pDock->iMinDockHeight = 0;
        }

        
//////////////////////////////////////////////////////////////////////////////////////// Définition de la zone de déco - 0 pour l'instant
	pDock->iDecorationsHeight = 0;
	pDock->iDecorationsWidth  = 0;
	
	
//////////////////////////////////////////////////////////////////////////////////////// On affecte ca aussi au cas où
	pDock->fFlatDockWidth = pDock->iMinDockWidth;
}


void cd_rendering_render_diapo (cairo_t *pCairoContext, CairoDock *pDock)
{
        if(my_diapo_draw_background)
        {
	        //\____________________ On trace le cadre.

	        cairo_save (pCairoContext);
	        cairo_dock_draw_frame_for_diapo (pCairoContext, pDock);

	        //\____________________ On dessine les decorations dedans.

	        cairo_dock_render_decorations_in_frame_for_diapo (pCairoContext, pDock);
	
	        //\____________________ On dessine le cadre.
	        if (my_diapo_lineWidth > 0)
	        {
		        cairo_set_line_width (pCairoContext,  my_diapo_lineWidth);
	                cairo_set_source_rgba (pCairoContext, my_diapo_color_border_line[0],
	                                                      my_diapo_color_border_line[1],
                                                              my_diapo_color_border_line[2],
	                                                      my_diapo_color_border_line[3] * (1. - pDock->fFoldingFactor));
		        cairo_stroke (pCairoContext);
	        }
	        cairo_restore (pCairoContext);
        }
        
	//\____________________ On dessine la ficelle qui les joint.
	//TODO Rendre joli !
	if (myIcons.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIcons.iStringLineWidth, FALSE, FALSE);
	//\____________________ On dessine les icones avec leurs etiquettes.
	
	
//////////////////////////////////////////////////////////////////////////////////////// Si y'en a pas on se barre (dock vide)
	//if (pFirstDrawnElement == NULL) return;
        if (pDock->icons == NULL) return;
	

//////////////////////////////////////////////////////////////////////////////////////// On parcourt la liste les icones :
	Icon *icon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
	
//////////////////////////////////////////////////////////////////////////////////////// On recupere la structure d'infos
		icon = ic->data;


//////////////////////////////////////////////////////////////////////////////////////// On sauvegarde le contexte de cairo
		cairo_save (pCairoContext);
		

//////////////////////////////////////////////////////////////////////////////////////// On affiche l'icone en cours avec les options :		
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, 0., FALSE);


//////////////////////////////////////////////////////////////////////////////////////// On restore le contexte de cairo
		cairo_restore (pCairoContext);
		
                gdouble zoom;
//////////////////////////////////////////////////////////////////////////////////////// On affiche le texte !
               if(icon->pTextBuffer != NULL)
                {
                	cairo_save (pCairoContext);
                	zoom = 1;
                	if(2*icon->fTextXOffset > MaxTextWidth)
                	{
                	        zoom  = MaxTextWidth / (2*icon->fTextXOffset);
	                        cairo_scale(pCairoContext, zoom, zoom);	
	                }
                        if (pDock->bHorizontalDock)
                        {
                        	cairo_set_source_surface (pCairoContext,
				        icon->pTextBuffer,                                        
				        (icon->fDrawX + (icon->fWidth * icon->fScale)/2)/zoom - icon->fTextXOffset,
				        (icon->fDrawY +  (icon->fHeight * icon->fScale)   + (my_diapo_iconGapY / 2)  - 6 )/zoom); // 6 ~= hauteur texte / 2
			}
			else
	                {
                        	cairo_set_source_surface (pCairoContext,
				        icon->pTextBuffer,  
				        (icon->fDrawY + (icon->fWidth * icon->fScale)/2)/zoom - icon->fTextXOffset,
				        (icon->fDrawX +  (icon->fHeight * icon->fScale)   + (my_diapo_iconGapY / 2)  - 6)/zoom); // 6 ~= hauteur texte / 2

			}
			if ((my_diapo_text_only_on_pointed && icon->bPointed) || my_diapo_display_all_icons)
			        cairo_paint (pCairoContext);
		        else if (!my_diapo_text_only_on_pointed)
			        cairo_paint_with_alpha (pCairoContext, 1. + (icon->fScale - my_diapo_fScaleMax)/(my_diapo_fScaleMax - 1));
			
			cairo_restore (pCairoContext);
                }
	}
}

static void _cd_rendering_check_if_mouse_inside_diapo (CairoDock *pDock)
{
	if (! pDock->bInside)
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
	}
	else if ((pDock->iMouseX < my_diapo_iconGapX) || (pDock->iMouseX > pDock->iMaxDockWidth - my_diapo_iconGapX) || (pDock->iMouseY < my_diapo_iconGapY) || (pDock->iMouseY > pDock->iMaxDockHeight - my_diapo_iconGapY))
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_ON_THE_EDGE;
	}
	else
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
	}
}
Icon *cd_rendering_calculate_icons_diapo (CairoDock *pDock)
{
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
        gint posMouseAbsX = 0;
        gint posMouseAbsY = 0;        


//////////////////////////////////////////////////////////////////////////////////////// On calcule la configuration de la grille :
        nIcones = cairo_dock_rendering_diapo_guess_grid(pDock->icons, &nRowsX, &nRowsY);      
	
	
	//\_______________ On calcule la position du curseur.
	posMouseAbsX = pDock->iMouseX; 
	posMouseAbsY = pDock->iMouseY;


//////////////////////////////////////////////////////////////////////////////////////// On calcule les tailles des icones en fonction de la souris
	cairo_dock_calculate_wave_with_position_diapo(pDock->icons, posMouseAbsX, posMouseAbsY, nRowsX);


//////////////////////////////////////////////////////////////////////////////////////// On calcule les positions des icones
	Icon *pPointedIcon = cairo_dock_calculate_icons_position_for_diapo(pDock, nRowsX, nRowsY, posMouseAbsX, posMouseAbsY);

	_cd_rendering_check_if_mouse_inside_diapo (pDock);
	
	
	/// caluler bCanDrop ...
	
//////////////////////////////////////////////////////////////////////////////////////// On revoie l'icone pointee et NULL sinon
	return pPointedIcon;
}


void cd_rendering_register_diapo_renderer (const gchar *cRendererName)
{
//////////////////////////////////////////////////////////////////////////////////////// On definit le renderer :
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);                                           //Nouvelle structure	
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-diapo-view", MY_APPLET_SHARE_DATA_DIR);        //On affecte le readme
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-diapo.png", MY_APPLET_SHARE_DATA_DIR);       // la preview
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_diapo;                        //La fonction qui défini les bornes     
	pRenderer->calculate_icons = cd_rendering_calculate_icons_diapo;                                        //qui calcule les param des icones      
	pRenderer->render = cd_rendering_render_diapo;                                                          //qui initie le calcul du rendu         
	pRenderer->render_optimized = NULL;                                                                     //pareil en mieux                       
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;                               // ?                                    
	
	pRenderer->bUseReflect = FALSE;                                                                         // On dit non au reflections
	
	cairo_dock_register_renderer (cRendererName, pRenderer);                                    //Puis on signale l'existence de notre rendu

}

guint cairo_dock_rendering_diapo_guess_grid(GList *pIconList, guint *nRowX, guint *nRowY)
{
//////////////////////////////////////////////////////////////////////////////////////// Calcul du nombre de ligne / colonne :
	guint count = g_list_length(pIconList);
	if(my_diapo_wide_grid)
	{
	        *nRowX = count  ? ceil(sqrt(count)) : 0;
        	*nRowY = count  ? ceil(((double) count) / *nRowX) : 0;
	}
	else
	{
	        *nRowY = count  ? ceil(sqrt(count)) : 0;
	        *nRowX = count  ? ceil(((double) count) / *nRowY) : 0;
        }
	return count;
}

Icon* cairo_dock_calculate_icons_position_for_diapo(CairoDock *pDock, guint nRowsX, guint nRowsY, gint Mx, gint My)
{
//////////////////////////////////////////////////////////////////////////////////////// On calcule la position de base pour toutes les icones :
        guint i = 0;
        guint x = 0;
        guint y = 0;
        gdouble iconeX = 0.;
        gdouble iconeY = 0.;
        guint maxWidth [nRowsX];
        guint maxHeight[nRowsY];
        guint curDockWidth  = 0;
        guint curDockHeight = 0;
        guint offsetX = 0;
        guint offsetY = 0;
        cairo_dock_rendering_diapo_calculate_max_icon_size(pDock->icons, (guint*) &maxWidth, (guint*)  &maxHeight, nRowsX, nRowsY);
       	GList *pointed_ic = NULL;
//////////////////////////////////////////////////////////////////////////////////////// On crée une liste d'icone des icones à parcourir :
	GList* ic;
	Icon* icon;
        i = 0;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
	
	
//////////////////////////////////////////////////////////////////////////////////////// On recupere la structure d'infos
		icon = ic->data;
		cairo_dock_rendering_diapo_get_gridXY_from_index(nRowsX, i, &x, &y);
		
		
//////////////////////////////////////////////////////////////////////////////////////// On affecte les parametres de dessin  :                
		
		
//////////////////////////////////////////////////////////////////////////////////////// On va PAS se servir des fX fY comme d'index de la grille ailleurs qu'ici CAR le fY est changé dans des fonctions de drawing qui devrait pas !
	        icon->fX = iconeX;
	        icon->fY = iconeY;	


//////////////////////////////////////////////////////////////////////////////////////// On passe au réferentiel de l'image :
	        icon->fXMin = icon->fXMax = icon->fXAtRest = //Ca on s'en sert pas encore
	        icon->fDrawX = iconeX + my_diapo_iconGapX + maxWidth [x] / 2  - (icon->fWidth  * icon->fScale) / 2 ;
	        icon->fDrawY = iconeY + my_diapo_iconGapY + maxHeight[y] / 2  - (icon->fHeight * icon->fScale) / 2 ;	    


//////////////////////////////////////////////////////////////////////////////////////// On prépare pour la suivante :
	        i++;
                if(!(i % nRowsX)) //  si on est à la fin d'une ligne on change
                {
                        curDockWidth = iconeX + maxWidth[x] + 2 * my_diapo_iconGapX;
                        iconeX  = 0.;
                        iconeY += maxHeight[y] + 2 * my_diapo_iconGapY;
                }
                else // sinon on bouge juste X
                {
	                iconeX  += maxWidth[x] + 2 * my_diapo_iconGapX;
	        }
	               
	                
//////////////////////////////////////////////////////////////////////////////////////// On affecte tous les parametres qui n'ont pas été défini précédement
	        icon->fPhase = 0.;
	        icon->fOrientation = 0.;//2. * G_PI * pDock->fFoldingFactor;                // rotation de l'icone
            icon->fWidthFactor = icon->fHeightFactor = 1. - pDock->fFoldingFactor;
        }
        if(iconeX != 0.)//cas de la rangé non terminée
        {
                iconeY += maxHeight[y] + 2 * my_diapo_iconGapY;
        }
        curDockHeight = iconeY;

        i=0;
//////////////////////////////////////////////////////////////////////////////////////// On calcule l'offset pour que ce soit centré :
        offsetX = (pDock->iMaxDockWidth  - curDockWidth ) / 2;
        offsetY = (pDock->iMaxDockHeight - curDockHeight) / 2;      
        pDock->iCurrentWidth  = curDockWidth;
        pDock->iCurrentHeight = curDockHeight;
        pDock->iDecorationsWidth = offsetX;   // Alors désolé mais ca c'est de la grosse feinte : on utilise les tailles de decoration inutilisés
        pDock->iDecorationsHeight = offsetY;  //  pour passer les offsets aux fonctions de dessin ... c'est sale mais ca marche !  
        for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
	
		cairo_dock_rendering_diapo_get_gridXY_from_index(nRowsX, i++, &x, &y);
//////////////////////////////////////////////////////////////////////////////////////// On recupere la structure d'infos
		icon = ic->data;
		icon->fDrawX += offsetX;
		icon->fDrawY += offsetY;

		////////////////////////////////////////////////////////////////////////////////////////On va check de la mouse là :
                if((Mx > icon->fDrawX - my_diapo_iconGapX) && 
                   (My > icon->fDrawY - my_diapo_iconGapY) &&
                   (Mx < icon->fDrawX + maxWidth[x]  + my_diapo_iconGapX) &&
                   (My < icon->fDrawY + maxHeight[y] +  my_diapo_iconGapY))
                {        
                        icon->bPointed = TRUE;    
                        pointed_ic = ic;
                        icon->fAlpha = 1.;                           
	        }
	        else
	        {
	                icon->bPointed = FALSE; 
	                icon->fAlpha = 0.9;
	        }
	}
	return pointed_ic == NULL ? NULL : pointed_ic->data;
}



void cairo_dock_calculate_wave_with_position_diapo(GList *pIconList, gint Mx, gint My, guint nRowsX)
{
        guint i = 0;
        guint x = 0;
        guint y = 0;
        if (pIconList == NULL) 
        {
                cd_debug("Rendering>Diapo -> pIconList == NULL Totaly uncool \n Returning badly...");
                return;
        }
        if(( (Icon*) pIconList->data ) == NULL)
        {
                cd_debug("Rendering>Diapo -> (Icon*) pIconList->data == NULL Totaly uncool \n Returning badly...");
                return;
        }
           
        GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cairo_dock_rendering_diapo_get_gridXY_from_index(nRowsX, i, &x, &y);
                guint x1 = Mx;
                gdouble x2 = icon->fDrawX + (icon->fWidth)  / 2 + (my_diapo_fScaleMax - 1) * 20; // formule empirique de chez empirique pour corriger le décalage incalculable... (si vous y arrivez envoyez moi un mail -> mapremierpartiedepseudo.ladeuxieme@gmail.com et je vous appellerais Dieu... merci ! Ateention ! Pas d'entourre les poules hein !)  // fDrawXAtRest
                guint y1 = My;
                gdouble y2 = icon->fDrawY + (icon->fHeight) / 2 + (my_diapo_fScaleMax - 1) * 20; // idem  // fDrawYAtRest

                gdouble distanceE = sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
                if(my_diapo_lineaire)
                {
                        gdouble eloignementMax = 3. * (icon->fWidth + icon->fHeight)  / 2; 
                        if(distanceE > eloignementMax)
                        {
                                icon->fScale = 1.;
                        }
                        else
                        {
                                icon->fScale = - (1./eloignementMax) * distanceE + my_diapo_fScaleMax;
                        }
                }
                else
                {
                        icon->fPhase = distanceE * G_PI / my_diapo_sinW + G_PI / 2.;
                        if (icon->fPhase < 0)
                        {
                                icon->fPhase = 0;
                        }
                        else if (icon->fPhase > G_PI)
                        {
                                icon->fPhase = G_PI;
                        }
                        icon->fScale = 1. + (my_diapo_fScaleMax-1.) * sin (icon->fPhase);                
                }
                i++;
 	}
}

	
//////////////////////////////////////////////////////////////////////////////////////// Fonction calculant la taille maximale du dock 
void cairo_dock_calculate_icons_positions_at_rest_diapo (GList *pIconList, gint* Wmin, gint* Hmin, guint nRowsX)
{
    gdouble iconeX = 0;
    gdouble iconeY = 0;
    guint i = 0;
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
	        icon->fDrawX = iconeX;  // fDrawXAtRest
	        icon->fDrawY = iconeY;  // fDrawYAtRest
                if(!(i % nRowsX)) //  si on est à la fin d'une ligne on change
                {
                        *Wmin = iconeX + icon->fWidth + 2 * my_diapo_iconGapX;
                        iconeX  = 0.;
                        iconeY += icon->fHeight + 2 * my_diapo_iconGapY;
                }
                else // sinon on bouge juste X
                {
	                iconeX  += icon->fWidth + 2 * my_diapo_iconGapX;
	        }
                i++;
	}
    if(iconeX != 0.)//cas de la rangée non terminée
    {
           iconeY += icon->fHeight + 2 * my_diapo_iconGapY;
    }
    *Hmin = iconeY;
}

//////////////////////////////////////////////////////////////////////////////////////// Fonction calculant sur chaque colonne et sur chaque ligne la taille d'icone maximale
void cairo_dock_rendering_diapo_calculate_max_icon_size(GList *pIconList, gint* maxWidth, gint* maxHeight, guint nRowsX, guint nRowsY)
{
    guint i = 0;
    guint x = 0;
    guint y = 0;
    for(i = 0 ;  i < nRowsX ; i++) maxWidth [i] = 0;
    for(i = 0 ;  i < nRowsY ; i++) maxHeight[i] = 0;
        
        
//////////////////////////////////////////////////////////////////////////////////////// On crée une liste d'icone des icones à parcourir :
	GList* ic;
	Icon* icon;
	i = 0;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
	
	
//////////////////////////////////////////////////////////////////////////////////////// On recupere la structure d'infos
		icon = ic->data;   
		cairo_dock_rendering_diapo_get_gridXY_from_index(nRowsX, i, &x, &y);
		guint W = icon->fWidth  * icon->fScale;
		guint H = icon->fHeight * icon->fScale;
		if(W > maxWidth[x])
		{
		        maxWidth[x] = W;
		}
		if(H > maxHeight[y])
		{
		        maxHeight[y] = H;
		}
		i++;
    }    
}         
         
//////////////////////////////////////////////////////////////////////////////////////// Fonction calculant la taille maximale du dock en placant une fausse souris au milieu de la vue
void cairo_dock_rendering_diapo_calculate_max_dock_size (GList *pIconList, gint Wmin, gint Hmin, gint* Wmax, gint* Hmax, guint nRowsX, guint nRowsY)
{
        *Hmax = *Wmax = 0;
        if (pIconList == NULL)
        	return ;
        gint maxWidth [nRowsX];
        gint maxHeight[nRowsY];
        guint i = 0;       
        
        cairo_dock_calculate_wave_with_position_diapo(pIconList, (Wmin / 2) + X_CONST_BORDER_SPACE, (Hmin / 2) + Y_CONST_BORDER_SPACE, nRowsX); 
        
        cairo_dock_rendering_diapo_calculate_max_icon_size(pIconList, (guint*)  &maxWidth, (guint*)  &maxHeight, nRowsX, nRowsY);
        
        
        for(i = 0 ;  i < nRowsX ; i++) *Wmax += maxWidth [i] + 2 * my_diapo_iconGapX;
        for(i = 0 ;  i < nRowsY ; i++) *Hmax += maxHeight[i] + 2 * my_diapo_iconGapY;
        *Wmax += + 2*X_CONST_BORDER_SPACE;
        *Hmax += + 2*Y_CONST_BORDER_SPACE;
}

//////////////////////////////////////////////////////////////////////////////////////// Fonctions utiles pour transformer l'index de la liste en couple (x,y) sur la grille
void cairo_dock_rendering_diapo_get_gridXY_from_index(guint nRowsX, guint index, guint* gridX, guint* gridY)
{
        *gridX = index % nRowsX;
        *gridY = (index - *gridX) / nRowsX;
}


//////////////////////////////////////////////////////////////////////////////////////// Et inversement (proportionnel)
guint cairo_dock_rendering_diapo_get_index_from_gridXY(guint nRowsX, guint gridX, guint gridY)
{
        return gridX + gridY * nRowsX;
}


//////////////////////////////////////////////////////////////////////////////////////// Methodes de dessin :


static void cairo_dock_draw_frame_horizontal_for_diapo (cairo_t *pCairoContext, CairoDock *pDock)
{
        const gdouble arrow_dec = 2;
	gint fFrameWidth  = pDock->iCurrentWidth;
	gint fFrameHeight = pDock->iCurrentHeight - my_diapo_arrowHeight + 60; //  60->pour que la fleche aille plus bas...
	gdouble fDockOffsetX = pDock->iDecorationsWidth;
	gdouble fDockOffsetY = pDock->iDecorationsHeight;

        cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);


        //HautGauche -> HautDroit
        if(pDock->bDirectionUp)
        {
                cairo_rel_line_to (pCairoContext, fFrameWidth, 0);
        }
        else
        {
               //On fait la fleche
                cairo_rel_line_to (pCairoContext,  (fFrameWidth/2 - my_diapo_arrowWidth/2 - my_diapo_arrowShift * fFrameWidth), 0);                //     _
                cairo_rel_line_to (pCairoContext, + my_diapo_arrowWidth/2 + my_diapo_arrowShift * fFrameWidth - my_diapo_arrowShift * fFrameWidth / arrow_dec,  -my_diapo_arrowHeight);       //  \. 
                cairo_rel_line_to (pCairoContext, + my_diapo_arrowWidth/2 - my_diapo_arrowShift * fFrameWidth + my_diapo_arrowShift * fFrameWidth / arrow_dec, +my_diapo_arrowHeight);        //    /     
                cairo_rel_line_to (pCairoContext, (fFrameWidth/2 - my_diapo_arrowWidth/2 + my_diapo_arrowShift * fFrameWidth) , 0);               // _     
        }
        //\_________________ Coin haut droit.
        cairo_rel_curve_to (pCairoContext,
                0, 0,
                my_diapo_radius, 0,
                my_diapo_radius, my_diapo_radius );


        //HautDroit -> BasDroit
        cairo_rel_line_to (pCairoContext, 0, fFrameHeight + my_diapo_lineWidth - my_diapo_radius *  2 );
        //\_________________ Coin bas droit.
         cairo_rel_curve_to (pCairoContext,
                        0, 0,
                        0 , my_diapo_radius,
                        -my_diapo_radius , my_diapo_radius);


        //BasDroit -> BasGauche
        if(!pDock->bDirectionUp)
        {
                cairo_rel_line_to (pCairoContext, - fFrameWidth , 0);
        }
        else
        {
                //On fait la fleche
                cairo_rel_line_to (pCairoContext, - (fFrameWidth/2 - my_diapo_arrowWidth/2 - my_diapo_arrowShift * fFrameWidth), 0);                //     _
                cairo_rel_line_to (pCairoContext, - my_diapo_arrowWidth/2 - my_diapo_arrowShift * fFrameWidth + my_diapo_arrowShift * fFrameWidth / arrow_dec, my_diapo_arrowHeight);        //    /     
                cairo_rel_line_to (pCairoContext, - my_diapo_arrowWidth/2 + my_diapo_arrowShift * fFrameWidth - my_diapo_arrowShift * fFrameWidth / arrow_dec, -my_diapo_arrowHeight);       //  \. 
                cairo_rel_line_to (pCairoContext, - (fFrameWidth/2 - my_diapo_arrowWidth/2 + my_diapo_arrowShift * fFrameWidth) , 0);               // _      
        }
        //\_________________ Coin bas gauche.
        cairo_rel_curve_to (pCairoContext,
                        0, 0,
                        -my_diapo_radius, 0,
                        -my_diapo_radius, -my_diapo_radius );
                        
                        
        //BasGauche -> HautGauche
        cairo_rel_line_to (pCairoContext, 0, - fFrameHeight - my_diapo_lineWidth + my_diapo_radius * 2);
        //\_________________ Coin haut gauche.
        cairo_rel_curve_to (pCairoContext,
                0, 0,
                0 , -my_diapo_radius ,
                my_diapo_radius, -my_diapo_radius );

}


static void cairo_dock_draw_frame_vertical_for_diapo (cairo_t *pCairoContext, CairoDock *pDock)
{
        const gdouble arrow_dec = 2;
	gint fFrameWidth  = pDock->iCurrentWidth;
	gint fFrameHeight = pDock->iCurrentHeight - my_diapo_arrowHeight + 60; //  60->pour que la fleche aille plus bas...
	gdouble fDockOffsetX = pDock->iDecorationsWidth;
	gdouble fDockOffsetY = pDock->iDecorationsHeight;
	

        cairo_move_to (pCairoContext, fDockOffsetY, fDockOffsetX);

        if(pDock->bDirectionUp)
        {
                cairo_rel_line_to (pCairoContext, 0, fFrameWidth);
        }
        else
        {
                cairo_rel_line_to (pCairoContext,0,(fFrameWidth/2 - my_diapo_arrowWidth/2 - my_diapo_arrowShift * fFrameWidth));                //     _
                cairo_rel_line_to (pCairoContext, -my_diapo_arrowHeight, my_diapo_arrowWidth/2 + my_diapo_arrowShift * fFrameWidth - my_diapo_arrowShift * fFrameWidth / arrow_dec);       //  \. 
                cairo_rel_line_to (pCairoContext, my_diapo_arrowHeight, + my_diapo_arrowWidth/2 - my_diapo_arrowShift * fFrameWidth + my_diapo_arrowShift * fFrameWidth / arrow_dec);        //    /     
                cairo_rel_line_to (pCairoContext,0,(fFrameWidth/2 - my_diapo_arrowWidth/2 + my_diapo_arrowShift * fFrameWidth));               // _     
       }
        //\_________________ Coin haut droit.
        cairo_rel_curve_to (pCairoContext,
                0, 0,
                0, my_diapo_radius,
                my_diapo_radius, my_diapo_radius);
        cairo_rel_line_to (pCairoContext, fFrameHeight + my_diapo_lineWidth - my_diapo_radius * 2, 0);
        //\_________________ Coin bas droit.
         cairo_rel_curve_to (pCairoContext,
                        0, 0,
                        my_diapo_radius, 0,
                        my_diapo_radius, -my_diapo_radius);
                        
        if(!pDock->bDirectionUp)
        {
                cairo_rel_line_to (pCairoContext, 0, - fFrameWidth);
        }
        else
        {
                //On fait la fleche
                cairo_rel_line_to (pCairoContext, 0, - (fFrameWidth/2 - my_diapo_arrowWidth/2 - my_diapo_arrowShift * fFrameWidth));                 //     _
                cairo_rel_line_to (pCairoContext,  my_diapo_arrowHeight, - my_diapo_arrowWidth/2 - my_diapo_arrowShift * fFrameWidth + my_diapo_arrowShift * fFrameWidth / arrow_dec);        //    /     
                cairo_rel_line_to (pCairoContext, -my_diapo_arrowHeight, - my_diapo_arrowWidth/2 + my_diapo_arrowShift * fFrameWidth - my_diapo_arrowShift * fFrameWidth / arrow_dec );       //  \. 
                cairo_rel_line_to (pCairoContext, 0, - (fFrameWidth/2 - my_diapo_arrowWidth/2 + my_diapo_arrowShift * fFrameWidth));                 // _      
        }

        //\_________________ Coin bas gauche.
         cairo_rel_curve_to (pCairoContext,
                        0, 0,
                        0, -my_diapo_radius,
                        -my_diapo_radius, -my_diapo_radius);
        cairo_rel_line_to (pCairoContext, - fFrameHeight - my_diapo_lineWidth + my_diapo_radius * 2, 0);
        //\_________________ Coin haut gauche.
        cairo_rel_curve_to (pCairoContext,
                0, 0,
                -my_diapo_radius, 0,
                -my_diapo_radius, my_diapo_radius);
}




void cairo_dock_draw_frame_for_diapo (cairo_t *pCairoContext, CairoDock *pDock)
{
        if (pDock->bHorizontalDock)
                cairo_dock_draw_frame_horizontal_for_diapo (pCairoContext, pDock);
        else
                cairo_dock_draw_frame_vertical_for_diapo (pCairoContext, pDock);
}



void cairo_dock_render_decorations_in_frame_for_diapo (cairo_t *pCairoContext, CairoDock *pDock)
{

////////////////////////////////////////////////////////////////////////////////////////On se fait un beau pattern dégradé :

        cairo_pattern_t *mon_super_pattern;
        cairo_save (pCairoContext);       
        mon_super_pattern = cairo_pattern_create_linear (0.0, 0.0,
                                                my_diapo_fade2right  ? pDock->iMaxDockWidth  : 0.0,      // Y'aurait surement des calculs complexes à faire mais 
                                                my_diapo_fade2bottom ? pDock->iMaxDockHeight : 0.0);     //  a quelques pixels près pour un dégradé : OSEF !
                                                
        cairo_pattern_add_color_stop_rgba (mon_super_pattern, 0, my_diapo_color_frame_start[0],                   
                                                                 my_diapo_color_frame_start[1], 
                                                                 my_diapo_color_frame_start[2], 
                                                                 my_diapo_color_frame_start[3] * (1. - pDock->fFoldingFactor)); // On gère aussi l'anim de depliage parcequ'on est des dingues
                                                
        cairo_pattern_add_color_stop_rgba (mon_super_pattern, 1, my_diapo_color_frame_stop[0] ,                  
                                                                 my_diapo_color_frame_stop[1] , 
                                                                 my_diapo_color_frame_stop[2] , 
                                                                 my_diapo_color_frame_stop[3]  * (1. - pDock->fFoldingFactor)); 
        cairo_set_source (pCairoContext, mon_super_pattern);
        
////////////////////////////////////////////////////////////////////////////////////////On remplit le contexte en le préservant -> pourquoi ?
        cairo_fill_preserve (pCairoContext);  
        cairo_pattern_destroy (mon_super_pattern);    
        cairo_restore (pCairoContext);

}


