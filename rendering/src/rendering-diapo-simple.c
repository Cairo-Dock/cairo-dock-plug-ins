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

#include "rendering-diapo-simple.h"

extern gint     my_diapo_simple_iconGapX;
extern gint     my_diapo_simple_iconGapY;
extern gdouble  my_diapo_simple_fScaleMax;
extern gint     my_diapo_simple_sinW;
extern gboolean my_diapo_simple_lineaire;
extern gboolean  my_diapo_simple_wide_grid;
extern gboolean  my_diapo_simple_text_only_on_pointed;

const  gint X_BORDER_SPACE = 40;
const  gint Y_BORDER_SPACE = 40;

void cd_rendering_calculate_max_dock_size_diapo_simple (CairoDock *pDock)
{	
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
        
        
//////////////////////////////////////////////////////////////////////////////////////// On calcule la configuration de la grille :
        nIcones = cairo_dock_rendering_diapo_simple_guess_grid(pDock->icons, &nRowsX, &nRowsY);   
        
        
//////////////////////////////////////////////////////////////////////////////////////// On calcule la taille de l'affichage
        if(nIcones != 0)
        {
	   pDock->iMinDockWidth  = pDock->iMaxDockWidth  = nRowsX * (((Icon*)pDock->icons->data)->fWidth  + 2 * my_diapo_simple_iconGapX) + X_BORDER_SPACE * 2;
	   pDock->iMinDockHeight = pDock->iMaxDockHeight = nRowsY * (((Icon*)pDock->icons->data)->fHeight + 2 * my_diapo_simple_iconGapY) + Y_BORDER_SPACE * 2;	
        }
        else
        {
                pDock->iMaxDockWidth = pDock->iMaxDockHeight = pDock->iMinDockWidth = pDock->iMinDockHeight = 0;
        }

//////////////////////////////////////////////////////////////////////////////////////// Définition de la zone de déco avec Valérie Damidot - 0 pour l'instant
	pDock->iDecorationsHeight = pDock->iMaxDockHeight;
	pDock->iDecorationsWidth  = pDock->iMaxDockWidth;
	
	
//////////////////////////////////////////////////////////////////////////////////////// On affecte ca aussi au cas où
	pDock->fFlatDockWidth = pDock->iMinDockWidth;

}

void cd_rendering_render_diapo_simple (CairoDock *pDock)
{
	//\____________________ On cree le contexte du dessin.
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	
//////////////////////////////////////////////////////////////////////////////////////// On définit les paramètres de dessin (par defaut pour l'instant)
	cairo_set_tolerance (pCairoContext, 0.5); 
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	//\____________________ On trace le cadre.
	//TODO
	//\____________________ On dessine les decorations dedans.
	//TODO
	//\____________________ On dessine le cadre.
	//TODO
	
		//\____________________ On trace le cadre.
	/*double fChangeAxes = 0.5 * (pDock->iCurrentWidth - pDock->iMaxDockWidth);
	*/double fLineWidth = g_iDockLineWidth;/*
	double fMargin = g_iFrameMargin;*/
	double fRadius = (pDock->iMaxDockHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDock->iMaxDockHeight + fLineWidth) / 2 - 1);
	/*double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + 0 - fMargin : fRadius + fLineWidth / 2);  // fChangeAxes
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iDecorationsHeight + fLineWidth;
	}
	*/
	cairo_save (pCairoContext);
	gboolean bRoundedBottomCorner_temp = g_bRoundedBottomCorner;
	g_bRoundedBottomCorner = TRUE;

	cairo_dock_draw_frame (pCairoContext, fRadius, 1, pDock->iMaxDockWidth-2*X_BORDER_SPACE, pDock->iMaxDockHeight-2*Y_BORDER_SPACE, X_BORDER_SPACE, Y_BORDER_SPACE, 1, 0, pDock->bHorizontalDock);  // fLineWidth

	//\____________________ On dessine les decorations dedans.
	//fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, Y_BORDER_SPACE);
	
	//\____________________ On dessine le cadre.
	// (fLineWidth > 0)
	//{
	//	cairo_set_line_width (pCairoContext, fLineWidth);
	//	cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	//	cairo_stroke (pCairoContext);/*
	//}
	g_bRoundedBottomCorner = bRoundedBottomCorner_temp;
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine la ficelle qui les joint.*/
	//TODO Rendre joli !
	if (g_iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, g_iStringLineWidth, TRUE, TRUE);
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
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, pDock->fRatio, 0, pDock->bUseReflect, FALSE, pDock->iCurrentWidth, pDock->bDirectionUp);


//////////////////////////////////////////////////////////////////////////////////////// On restore le contexte de cairo
		cairo_restore (pCairoContext);
		

//////////////////////////////////////////////////////////////////////////////////////// On affiche le texte !
               if(icon->pTextBuffer != NULL)
                {
                	cairo_save (pCairoContext);

                	cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,                                        
				icon->fDrawX + (icon->fWidth * icon->fScale)/2 -icon->fTextXOffset,
				icon->fDrawY +  (icon->fHeight * icon->fScale)   + (my_diapo_simple_iconGapY / 2)  - 6 ); // 6 ~= hauteur texte / 2
			
			if (my_diapo_simple_text_only_on_pointed && icon->bPointed)
			        cairo_paint (pCairoContext);
		        else if (!my_diapo_simple_text_only_on_pointed)
			        cairo_paint_with_alpha (pCairoContext, 1. + (icon->fScale - my_diapo_simple_fScaleMax)/(my_diapo_simple_fScaleMax - 1));
			
			cairo_restore (pCairoContext);
                }
	}
	
	
//////////////////////////////////////////////////////////////////////////////////////// On regle son compte au contexte
	cairo_destroy (pCairoContext);
	
	
//////////////////////////////////////////////////////////////////////////////////////// Si on est --glitz alors on s'en sert pour bien que ca plante :
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}

void cd_rendering_render_diapo_simple_optimized (CairoDock *pDock)
{
	//\____________________ On cree le contexte du dessin.
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	
//////////////////////////////////////////////////////////////////////////////////////// On définit les paramètres de dessin (par defaut pour l'instant)
	cairo_set_tolerance (pCairoContext, 0.5); 
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	//\____________________ On trace le cadre.
	//TODO
	//\____________________ On dessine les decorations dedans.
	//TODO
	//\____________________ On dessine le cadre.
	//TODO
	//\____________________ On dessine la ficelle qui les joint.
	//TODO Rendre joli !
	if (g_iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, g_iStringLineWidth, FALSE, FALSE);
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

                if(icon->fScale != 1.)
                {
//////////////////////////////////////////////////////////////////////////////////////// On sauvegarde le contexte de cairo
        		cairo_save (pCairoContext);
		

//////////////////////////////////////////////////////////////////////////////////////// On affiche l'icone en cours avec les options :		
        		cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, pDock->fRatio, 0, pDock->bUseReflect, FALSE, pDock->iCurrentWidth, pDock->bDirectionUp);


//////////////////////////////////////////////////////////////////////////////////////// On restore le contexte de cairo
        		cairo_restore (pCairoContext);
		

//////////////////////////////////////////////////////////////////////////////////////// On affiche le texte !
                       if(icon->pTextBuffer != NULL)
                        {
                        	cairo_save (pCairoContext);

                        	cairo_set_source_surface (pCairoContext,
				        icon->pTextBuffer,                                        
				        icon->fDrawX + (icon->fWidth * icon->fScale)/2 -icon->fTextXOffset,
				        icon->fDrawY +  (icon->fHeight * icon->fScale)   + (my_diapo_simple_iconGapY / 2)  - 6 ); // 6 ~= hauteur texte / 2
			
			        if (my_diapo_simple_text_only_on_pointed && icon->bPointed)
			                cairo_paint (pCairoContext);
		                else if (!my_diapo_simple_text_only_on_pointed)
			                cairo_paint_with_alpha (pCairoContext, 1. + (icon->fScale - my_diapo_simple_fScaleMax)/(my_diapo_simple_fScaleMax - 1));
			
			        cairo_restore (pCairoContext);
                        }
                }
	}
	
	
//////////////////////////////////////////////////////////////////////////////////////// On regle son compte au contexte
	cairo_destroy (pCairoContext);
	
	
//////////////////////////////////////////////////////////////////////////////////////// Si on est --glitz alors on s'en sert pour bien que ca plante :
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}

Icon *cd_rendering_calculate_icons_diapo_simple (CairoDock *pDock)
{
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
     

//////////////////////////////////////////////////////////////////////////////////////// On calcule la configuration de la grille :
        nIcones = cairo_dock_rendering_diapo_simple_guess_grid(pDock->icons, &nRowsX, &nRowsY);      
	
	
//cd_message("grep x%d y%d", pDock->iMouseX, pDock->iMouseY);

//////////////////////////////////////////////////////////////////////////////////////// On calcule les tailles des icones en fonction de la souris
	cairo_dock_calculate_wave_with_position_diapo_simple(pDock->icons, pDock->iMouseX, pDock->iMouseY, nRowsX);


//////////////////////////////////////////////////////////////////////////////////////// On calcule les positions des icones	
	Icon *pPointedIcon = cairo_dock_calculate_icons_position_for_diapo_simple(pDock, nRowsX, nRowsY, pDock->iMouseX, pDock->iMouseY);


	CairoDockMousePositionType iMousePositionType;
	if (! pDock->bInside)
	{
		iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
	}
	else if ((pDock->iMouseX < (X_BORDER_SPACE/2)) || (pDock->iMouseX > pDock->iMaxDockWidth - (X_BORDER_SPACE/2)) || (pDock->iMouseY < (Y_BORDER_SPACE/2)) || (pDock->iMouseY > pDock->iMaxDockHeight - (Y_BORDER_SPACE/2)))
	{
		iMousePositionType = CAIRO_DOCK_MOUSE_ON_THE_EDGE;
	}
	else
	{
		iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
	}
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	cairo_dock_mark_avoiding_mouse_icons_linear (pDock);

//////////////////////////////////////////////////////////////////////////////////////// On revoie l'icone pointee et NULL sinon
	return pPointedIcon;
}


void cd_rendering_register_diapo_simple_renderer (void)
{


//////////////////////////////////////////////////////////////////////////////////////// On definit le renderer :
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);                                           //Nouvelle structure	
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-diapo-simple-view", MY_APPLET_SHARE_DATA_DIR);        //On affecte le readme
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-diapo-simple.png", MY_APPLET_SHARE_DATA_DIR);       // la preview
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_diapo_simple;                        //La fonction qui défini les bornes     
	pRenderer->calculate_icons = cd_rendering_calculate_icons_diapo_simple;                                        //qui calcule les param des icones      
	pRenderer->render = cd_rendering_render_diapo_simple;                                                          //qui initie le calcul du rendu         
	pRenderer->render_optimized = cd_rendering_render_diapo_simple_optimized;                                      //pareil en mieux                       
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;                               // ?                                    
	
	pRenderer->bUseReflect = FALSE;                                                                         // On dit non au reflections
	
	cairo_dock_register_renderer (MY_APPLET_DIAPO_SIMPLE_VIEW_NAME, pRenderer);                                    //Puis on signale l'existence de notre rendu
}


guint cairo_dock_rendering_diapo_simple_guess_grid(GList *pIconList, guint *nRowX, guint *nRowY)
{	

//////////////////////////////////////////////////////////////////////////////////////// Calcul du nombre de ligne / colonne :
	guint count = g_list_length(pIconList);
	if(my_diapo_simple_wide_grid)
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

Icon* cairo_dock_calculate_icons_position_for_diapo_simple(CairoDock *pDock, guint nRowsX, guint nRowsY, gint Mx, gint My)
{


//////////////////////////////////////////////////////////////////////////////////////// On calcule la position de base pour toutes les icones :
        guint i = 0;
        guint x = 0;
        guint y = 0;
        
       	GList *pointed_ic = NULL;
//////////////////////////////////////////////////////////////////////////////////////// On crée une liste d'icone des icones à parcourir :
	GList* ic;
	Icon* icon;
        i = 0;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
	
	
//////////////////////////////////////////////////////////////////////////////////////// On recupere la structure d'infos
		icon = ic->data;
		cairo_dock_rendering_diapo_simple_get_gridXY_from_index(nRowsX, i, &x, &y);
		
		
//////////////////////////////////////////////////////////////////////////////////////// On affecte les parametres de dessin  :                
		
		
//////////////////////////////////////////////////////////////////////////////////////// On va PAS se servir des fX fY comme d'index de la grille ailleurs qu'ici CAR le fY est changé dans des fonctions de drawing qui devrait pas !
	        icon->fX = (icon->fWidth  + 2 * my_diapo_simple_iconGapX) * x + X_BORDER_SPACE;
	        icon->fY = (icon->fHeight + 2 * my_diapo_simple_iconGapY) * y + Y_BORDER_SPACE;;


//////////////////////////////////////////////////////////////////////////////////////// On passe au réferentiel de l'image :
	        icon->fXMin = icon->fXMax = icon->fXAtRest = //Ca on s'en sert pas encore
	        icon->fDrawX = icon->fX + my_diapo_simple_iconGapX + icon->fWidth  * (1. - icon->fScale) / 2;
	        icon->fDrawY = icon->fY + my_diapo_simple_iconGapY + icon->fHeight * (1. - icon->fScale) / 2;	    	        


////////////////////////////////////////////////////////////////////////////////////////On va check de la mouse là :
                if((Mx > icon->fX) && 
                   (My > icon->fY) &&
                   (Mx < icon->fX + icon->fWidth  + 2 * my_diapo_simple_iconGapX) &&
                   (My < icon->fY + icon->fHeight + 2 * my_diapo_simple_iconGapY))
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


//////////////////////////////////////////////////////////////////////////////////////// On prépare pour la suivante :
	        i++;
	               
	                
//////////////////////////////////////////////////////////////////////////////////////// On affecte tous les parametres qui n'ont pas été défini précédement
	        icon->fPhase = 0.;
	        icon->fOrientation = 0.;//2. * G_PI * pDock->fFoldingFactor;                // rotation de l'icone
            icon->fWidthFactor = icon->fHeightFactor = 1. - pDock->fFoldingFactor;

//////////////////////////////////////////////////////////////////////////////////////// On laisse le dock s'occuper des animations
		cairo_dock_manage_animations (icon, pDock);
	}
	return pointed_ic == NULL ? NULL : pointed_ic->data;
}



void cairo_dock_calculate_wave_with_position_diapo_simple(GList *pIconList, gint Mx, gint My, guint nRowsX)
{
        guint i = 0;
        guint x = 0;
        guint y = 0;
              
        GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cairo_dock_rendering_diapo_simple_get_gridXY_from_index(nRowsX, i, &x, &y);
                guint x1 = Mx;
                gdouble x2 = (icon->fWidth  + 2 * my_diapo_simple_iconGapX) * x + (icon->fWidth  / 2) + my_diapo_simple_iconGapX + X_BORDER_SPACE;
                guint y1 = My;
                gdouble y2 = (icon->fHeight + 2 * my_diapo_simple_iconGapY) * y + (icon->fHeight / 2) + my_diapo_simple_iconGapY + Y_BORDER_SPACE;
                gdouble distanceE = sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
              
                if(my_diapo_simple_lineaire)
                {
                        gdouble eloignementMax = 3. * (icon->fWidth + icon->fHeight)  / 2; 
                        if(distanceE > eloignementMax)
                        {
                                icon->fScale = 1.;
                        }
                        else
                        {
                                icon->fScale = - (1./eloignementMax) * distanceE + my_diapo_simple_fScaleMax;
                        }
                }
                else
                {
                        icon->fPhase = distanceE * G_PI / my_diapo_simple_sinW + G_PI / 2.;
                        if (icon->fPhase < 0)
                        {
                                icon->fPhase = 0;
                        }
                        else if (icon->fPhase > G_PI)
                        {
                                icon->fPhase = G_PI;
                        }
                        icon->fScale = 1. + (my_diapo_simple_fScaleMax-1.) * sin (icon->fPhase);                
                }
                i++;
 	}
}


//////////////////////////////////////////////////////////////////////////////////////// Fonctions utiles pour transformer l'index de la liste en couple (x,y) sur la grille
void cairo_dock_rendering_diapo_simple_get_gridXY_from_index(guint nRowsX, guint index, guint* gridX, guint* gridY)
{
        *gridX = index % nRowsX;
        *gridY = (index - *gridX) / nRowsX;
}


//////////////////////////////////////////////////////////////////////////////////////// Et inversement (proportionnel)
guint cairo_dock_rendering_diapo_simple_get_index_from_gridXY(guint nRowsX, guint gridX, guint gridY)
{
        return gridX + gridY * nRowsX;
}

