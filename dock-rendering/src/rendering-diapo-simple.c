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

#include "rendering-diapo-simple.h"

extern gint     my_diapo_simple_iconGapX;
extern gint     my_diapo_simple_iconGapY;
extern gdouble  my_diapo_simple_fScaleMax;
extern gint     my_diapo_simple_sinW;
extern gboolean my_diapo_simple_lineaire;
extern gboolean  my_diapo_simple_wide_grid;
extern gboolean  my_diapo_simple_text_only_on_pointed;

extern gdouble  my_diapo_simple_color_frame_start[4];
extern gdouble  my_diapo_simple_color_frame_stop[4];
extern gboolean my_diapo_simple_fade2bottom;
extern gboolean my_diapo_simple_fade2right;
extern guint    my_diapo_simple_arrowWidth;
extern guint    my_diapo_simple_arrowHeight;
extern gdouble  my_diapo_simple_arrowShift;
extern guint    my_diapo_simple_lineWidth;
extern guint    my_diapo_simple_radius;
extern gdouble  my_diapo_simple_color_border_line[4];
extern gboolean my_diapo_simple_draw_background;
extern gboolean my_diapo_simple_display_all_icons;

const  gint X_BORDER_SPACE = 40;
const  gint Y_BORDER_SPACE = 40;
//const  gint  MaxTextWidthSimple = 125; 

/// On considere qu'on a my_diapo_simple_iconGapX entre chaque icone horizontalement, et my_diapo_simple_iconGapX/2 entre les icones et les bords (pour aerer un peu plus le dessin). Idem verticalement. X_BORDER_SPACE est la pour empecher que les icones debordent de la fenetre au zoom (?).

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
	   ///pDock->iMinDockWidth  = pDock->iMaxDockWidth  = nRowsX * (((Icon*)pDock->icons->data)->fWidth  + 2 * my_diapo_simple_iconGapX) + X_BORDER_SPACE * 2;
	   ///pDock->iMinDockHeight = pDock->iMaxDockHeight = nRowsY * (((Icon*)pDock->icons->data)->fHeight + 2 * my_diapo_simple_iconGapY) + Y_BORDER_SPACE * 3 + my_diapo_simple_arrowHeight-30;	// 30 -> pour que la fleche aille plus bas
	   pDock->iMinDockWidth = pDock->iMaxDockWidth = nRowsX * (((Icon*)pDock->icons->data)->fWidth  + my_diapo_simple_iconGapX) + X_BORDER_SPACE * 2;
	   pDock->iMinDockHeight = pDock->iMaxDockHeight = nRowsY * (((Icon*)pDock->icons->data)->fHeight + my_diapo_simple_iconGapY) + Y_BORDER_SPACE + my_diapo_simple_arrowHeight+10 + myLabels.iLabelSize;  // +10 -> pour que la fleche aille plus bas
        }
        else
        {
                ///pDock->iMaxDockWidth = pDock->iMaxDockHeight = pDock->iMinDockWidth = pDock->iMinDockHeight = 0;
                pDock->iMaxDockWidth = pDock->iMinDockWidth = X_BORDER_SPACE * 2 + 1;
                pDock->iMaxDockHeight = pDock->iMinDockHeight = Y_BORDER_SPACE + 1 + my_diapo_simple_arrowHeight+10;
        }

//////////////////////////////////////////////////////////////////////////////////////// Définition de la zone de déco avec Valérie Damidot - 0 car on s'en fout un peu comme l'emission de cette grognasse... :D
	pDock->iDecorationsHeight = 0;
	pDock->iDecorationsWidth  = 0;
	
//////////////////////////////////////////////////////////////////////////////////////// On affecte ca aussi au cas où
	pDock->fFlatDockWidth = pDock->iMinDockWidth;
	pDock->fMagnitudeMax = my_diapo_simple_fScaleMax / (1+g_fAmplitude);

}

GList *_get_first_drawn_element (GList *icons)
{
	Icon *icon;
	GList *ic;
	GList *pFirstDrawnElement = NULL;
	for (ic = icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->bPointed)
			break ;
	}
	
	if (ic == NULL || ic->next == NULL)  // derniere icone ou aucune pointee.
		pFirstDrawnElement = icons;
	else
		pFirstDrawnElement = ic->next;
	return pFirstDrawnElement;
}

void cd_rendering_render_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
	if(my_diapo_simple_draw_background)
        {
	        //\____________________ On trace le cadre.

	        cairo_save (pCairoContext);
	        cairo_dock_draw_frame_for_diapo_simple (pCairoContext, pDock);

	        //\____________________ On dessine les decorations dedans.

	        cairo_dock_render_decorations_in_frame_for_diapo_simple (pCairoContext, pDock);
	
	        //\____________________ On dessine le cadre.
	        if (my_diapo_simple_lineWidth > 0)
	        {
		        cairo_set_line_width (pCairoContext,  my_diapo_simple_lineWidth);
	                cairo_set_source_rgba (pCairoContext, my_diapo_simple_color_border_line[0],
	                                                      my_diapo_simple_color_border_line[1],
                                                              my_diapo_simple_color_border_line[2],
	                                                      my_diapo_simple_color_border_line[3] * (1. - pDock->fFoldingFactor));
		        cairo_stroke (pCairoContext);
	        }
	        else
	        	cairo_new_path (pCairoContext);
	        cairo_restore (pCairoContext);
        }
	
	if (pDock->icons == NULL)
		return;
	
	//\____________________ On dessine la ficelle qui les joint.
	//TODO Rendre joli !
	if (myIcons.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIcons.iStringLineWidth, TRUE, TRUE);
	
	//\____________________ On dessine les icones avec leurs etiquettes.
	// on determine la 1ere icone a tracer : l'icone suivant l'icone pointee.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	// on dessine les icones, l'icone pointee en dernier.
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, 1., FALSE);
		cairo_restore (pCairoContext);
		
//////////////////////////////////////////////////////////////////////////////////////// On affiche le texte !
		gdouble zoom;
		if(icon->pTextBuffer != NULL)
		{
			double fAlpha;
			if ((my_diapo_simple_text_only_on_pointed && icon->bPointed) || my_diapo_simple_display_all_icons)
				fAlpha = 1.;
			else if (!my_diapo_simple_text_only_on_pointed)
				fAlpha = 1. + (icon->fScale - my_diapo_simple_fScaleMax)/(my_diapo_simple_fScaleMax - 1);
			else
				fAlpha = 0.;
			if (fAlpha > 0)
			{
				cairo_save (pCairoContext);
				
				if (icon->iTextWidth > icon->fWidth * icon->fScale + my_diapo_simple_iconGapX)
				{
					cairo_translate (pCairoContext,
						icon->fDrawX - my_diapo_simple_iconGapX/2,
						icon->fDrawY + icon->fHeight * icon->fScale);
					
					cairo_set_source_surface (pCairoContext,
						icon->pTextBuffer,
						0.,
						0.);
					
					cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
						0.,
						icon->fWidth * icon->fScale + my_diapo_simple_iconGapX,
						0.);
					cairo_pattern_set_extend (pGradationPattern, icon->bPointed ? CAIRO_EXTEND_PAD : CAIRO_EXTEND_NONE);
					cairo_pattern_add_color_stop_rgba (pGradationPattern,
						0.,
						0.,
						0.,
						0.,
						fAlpha);
					cairo_pattern_add_color_stop_rgba (pGradationPattern,
						0.75,
						0.,
						0.,
						0.,
						fAlpha);
					cairo_pattern_add_color_stop_rgba (pGradationPattern,
						1.,
						0.,
						0.,
						0.,
						MIN (0.2, fAlpha/2));
					cairo_mask (pCairoContext, pGradationPattern);
					cairo_pattern_destroy (pGradationPattern);
				}
				else
				{
					cairo_translate (pCairoContext,
						icon->fDrawX + (icon->fWidth * icon->fScale - icon->iTextWidth) / 2,
						icon->fDrawY + icon->fHeight * icon->fScale);
					
					cairo_set_source_surface (pCairoContext,
						icon->pTextBuffer,
						0.,
						0.);
					if (fAlpha == 1)
						cairo_paint (pCairoContext);
					else
						cairo_paint_with_alpha (pCairoContext, fAlpha);
				}
				cairo_restore (pCairoContext);
			}
		}
               /*if(FALSE && icon->pTextBuffer != NULL)
                {
                	cairo_save (pCairoContext);
                	zoom = 1;
                	if(2*icon->fTextXOffset > MaxTextWidthSimple)
                	{
                	        zoom  = MaxTextWidthSimple / (2*icon->fTextXOffset);
	                        cairo_scale(pCairoContext, zoom, zoom);	
	                }
                        if (pDock->bHorizontalDock)
                        {
                        	cairo_set_source_surface (pCairoContext,
				        icon->pTextBuffer,
				        (icon->fDrawX + (icon->fWidth * icon->fScale)/2)/zoom - icon->fTextXOffset,
				        (icon->fDrawY +  (icon->fHeight * icon->fScale) + 0*(my_diapo_simple_iconGapY / 2)) / zoom - 6); // 6 ~= hauteur texte / 2
			}
			else
	                {
                        	cairo_set_source_surface (pCairoContext,
				        icon->pTextBuffer,  
				        (icon->fDrawY + (icon->fWidth * icon->fScale)/2)/zoom -icon->fTextXOffset,
				        (icon->fDrawX +  (icon->fHeight * icon->fScale)   + (my_diapo_simple_iconGapY / 2)  - 6 )/zoom); // 6 ~= hauteur texte / 2
			}

			if ((my_diapo_simple_text_only_on_pointed && icon->bPointed) || my_diapo_simple_display_all_icons)
			        cairo_paint (pCairoContext);
		        else if (!my_diapo_simple_text_only_on_pointed)
			        cairo_paint_with_alpha (pCairoContext, 1. + (icon->fScale - my_diapo_simple_fScaleMax)/(my_diapo_simple_fScaleMax - 1));
	
			cairo_restore (pCairoContext);
                }*/
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	}
	while (ic != pFirstDrawnElement);
}

static void _cd_rendering_check_if_mouse_inside_diapo_simple (CairoDock *pDock)
{
	if (! pDock->bInside)
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
	}
	else if ((pDock->iMouseX < 0) || (pDock->iMouseX > pDock->iMaxDockWidth - 0) || (pDock->iMouseY < 0) || (pDock->iMouseY > pDock->iMaxDockHeight - 0))  // (X_BORDER_SPACE/2)
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_ON_THE_EDGE;
	}
	else
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
	}
}
Icon *cd_rendering_calculate_icons_diapo_simple (CairoDock *pDock)
{
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
     

//////////////////////////////////////////////////////////////////////////////////////// On calcule la configuration de la grille :
        nIcones = cairo_dock_rendering_diapo_simple_guess_grid(pDock->icons, &nRowsX, &nRowsY);      
	

//////////////////////////////////////////////////////////////////////////////////////// On calcule les tailles des icones en fonction de la souris
	cairo_dock_calculate_wave_with_position_diapo_simple(pDock->icons, pDock->iMouseX, pDock->iMouseY, nRowsX);


//////////////////////////////////////////////////////////////////////////////////////// On calcule les positions des icones	
	Icon *pPointedIcon = cairo_dock_calculate_icons_position_for_diapo_simple(pDock, nRowsX, nRowsY, pDock->iMouseX, pDock->iMouseY);


	_cd_rendering_check_if_mouse_inside_diapo_simple (pDock);
	
	/// caluler bCanDrop ...
	
//////////////////////////////////////////////////////////////////////////////////////// On revoie l'icone pointee et NULL sinon
	return pPointedIcon;
}


void cd_rendering_register_diapo_simple_renderer (const gchar *cRendererName)
{
//////////////////////////////////////////////////////////////////////////////////////// On definit le renderer :
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);                                           //Nouvelle structure	
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-diapo-simple-view", MY_APPLET_SHARE_DATA_DIR);        //On affecte le readme
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-diapo-simple.png", MY_APPLET_SHARE_DATA_DIR);       // la preview
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_diapo_simple;                        //La fonction qui défini les bornes     
	pRenderer->calculate_icons = cd_rendering_calculate_icons_diapo_simple;                                        //qui calcule les param des icones      
	pRenderer->render = cd_rendering_render_diapo_simple;                                                          //qui initie le calcul du rendu         
	pRenderer->render_optimized = NULL;//cd_rendering_render_diapo_simple_optimized;                                      //pareil en mieux                       
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;                               // ?                                    
	pRenderer->render_opengl = cd_rendering_render_diapo_simple_opengl;
	
	pRenderer->bUseReflect = FALSE;                                                                         // On dit non au reflections
	
	cairo_dock_register_renderer (cRendererName, pRenderer);                                    //Puis on signale l'existence de notre rendu
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
		
//////////////////////////////////////////////////////////////////////////////////////// On va PAS se servir des fX fY comme d'index de la grille ailleurs qu'ici CAR le fY est changé dans des fonctions de drawing qui devrait pas !   ----> a confirmer mais ca ne devrait plus !
	        ///icon->fX = (icon->fWidth  + 2 * my_diapo_simple_iconGapX) * x + X_BORDER_SPACE;
	        ///icon->fY = (icon->fHeight + 2 * my_diapo_simple_iconGapY) * y + Y_BORDER_SPACE;
	        icon->fX = (icon->fWidth  + my_diapo_simple_iconGapX) * x + X_BORDER_SPACE + .5*my_diapo_simple_iconGapX ;
	        icon->fY = (icon->fHeight + my_diapo_simple_iconGapY) * y + Y_BORDER_SPACE + .5*my_diapo_simple_iconGapY;


//////////////////////////////////////////////////////////////////////////////////////// On passe au réferentiel de l'image :
	        icon->fXMin = icon->fXMax = icon->fXAtRest = //Ca on s'en sert pas encore
	        ///icon->fDrawX = icon->fX + my_diapo_simple_iconGapX + icon->fWidth  * (1. - icon->fScale) / 2;
	        ///icon->fDrawY = icon->fY + my_diapo_simple_iconGapY + icon->fHeight * (1. - icon->fScale) / 2;
	        icon->fDrawX = icon->fX + icon->fWidth  * (1. - icon->fScale) / 2;
	        icon->fDrawY = icon->fY + icon->fHeight * (1. - icon->fScale) / 2;

////////////////////////////////////////////////////////////////////////////////////////On va check de la mouse là :
                /**if((Mx > icon->fX) && 
                   (My > icon->fY) &&
                   (Mx < icon->fX + icon->fWidth  + 2 * my_diapo_simple_iconGapX) &&
                   (My < icon->fY + icon->fHeight + 2 * my_diapo_simple_iconGapY))*/
                   if((Mx > icon->fX - .5*my_diapo_simple_iconGapX) && 
                   (My > icon->fY - .5*my_diapo_simple_iconGapY) &&
                   (Mx < icon->fX + icon->fWidth  + .5*my_diapo_simple_iconGapX) &&
                   (My < icon->fY + icon->fHeight + .5*my_diapo_simple_iconGapY))
                {
                        icon->bPointed = TRUE;
                        pointed_ic = ic;
                        icon->fAlpha = 1.;
	        }
	        else
	        {
	                icon->bPointed = FALSE; 
	                icon->fAlpha = 0.75;
	        }

//////////////////////////////////////////////////////////////////////////////////////// On prépare pour la suivante :
	        i++;
	               
//////////////////////////////////////////////////////////////////////////////////////// On affecte tous les parametres qui n'ont pas été défini précédement
	        icon->fPhase = 0.;
	        icon->fOrientation = 0.;//2. * G_PI * pDock->fFoldingFactor;                // rotation de l'icone  -----> idee sympa ! par contre il faut placer l'icone du coup.
            icon->fWidthFactor = icon->fHeightFactor = 1. - pDock->fFoldingFactor;
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
                ///gdouble x2 = (icon->fWidth  + 2 * my_diapo_simple_iconGapX) * x + (icon->fWidth  / 2) + my_diapo_simple_iconGapX + X_BORDER_SPACE;
                gdouble x2 = (icon->fWidth + my_diapo_simple_iconGapX) * x + (icon->fWidth  / 2) + X_BORDER_SPACE + .5*my_diapo_simple_iconGapX;
                guint y1 = My;
                ///gdouble y2 = (icon->fHeight + 2 * my_diapo_simple_iconGapY) * y + (icon->fHeight / 2) + my_diapo_simple_iconGapY + Y_BORDER_SPACE;
                gdouble y2 = (icon->fHeight + my_diapo_simple_iconGapY) * y + (icon->fHeight / 2) + Y_BORDER_SPACE + .5*my_diapo_simple_iconGapY;
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



//////////////////////////////////////////////////////////////////////////////////////// Methodes de dessin :


static void cairo_dock_draw_frame_horizontal_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
        const gdouble arrow_dec = 2;
	gdouble fFrameWidth  = pDock->iMaxDockWidth-2*X_BORDER_SPACE;
	gdouble fFrameHeight = pDock->iMaxDockHeight - Y_BORDER_SPACE - (my_diapo_simple_arrowHeight+10); // +10->pour que la fleche aille plus bas...  -----> quel petit joueur, regarde les calculs de malade que je me suis tape pour la pointe des dialogues ! :-)
	gdouble fDockOffsetX = X_BORDER_SPACE;
	gdouble fDockOffsetY = (pDock->bDirectionUp ? Y_BORDER_SPACE : 10);
	
	

        cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);


        //HautGauche -> HautDroit
        if(pDock->bDirectionUp)
        {
                cairo_rel_line_to (pCairoContext, fFrameWidth, 0);
        }
        else
        {
               //On fait la fleche
                cairo_rel_line_to (pCairoContext,  (fFrameWidth/2 - my_diapo_simple_arrowWidth/2 - my_diapo_simple_arrowShift * fFrameWidth), 0);                //     _
                cairo_rel_line_to (pCairoContext, + my_diapo_simple_arrowWidth/2 + my_diapo_simple_arrowShift * fFrameWidth - my_diapo_simple_arrowShift * fFrameWidth / arrow_dec,  -my_diapo_simple_arrowHeight);       //  \. 
                cairo_rel_line_to (pCairoContext, + my_diapo_simple_arrowWidth/2 - my_diapo_simple_arrowShift * fFrameWidth + my_diapo_simple_arrowShift * fFrameWidth / arrow_dec, +my_diapo_simple_arrowHeight);        //    /     
                cairo_rel_line_to (pCairoContext, (fFrameWidth/2 - my_diapo_simple_arrowWidth/2 + my_diapo_simple_arrowShift * fFrameWidth) , 0);               // _     
        }
        //\_________________ Coin haut droit.
        cairo_rel_curve_to (pCairoContext,
                0, 0,
                my_diapo_simple_radius, 0,
                my_diapo_simple_radius, my_diapo_simple_radius );


        //HautDroit -> BasDroit
        cairo_rel_line_to (pCairoContext, 0, fFrameHeight + my_diapo_simple_lineWidth - my_diapo_simple_radius *  2 );
        //\_________________ Coin bas droit.
         cairo_rel_curve_to (pCairoContext,
                        0, 0,
                        0 , my_diapo_simple_radius,
                        -my_diapo_simple_radius , my_diapo_simple_radius);


        //BasDroit -> BasGauche        
        if(!pDock->bDirectionUp)
        {
                cairo_rel_line_to (pCairoContext, - fFrameWidth , 0);
        }
        else
        {
                //On fait la fleche
                cairo_rel_line_to (pCairoContext, - (fFrameWidth/2 - my_diapo_simple_arrowWidth/2 - my_diapo_simple_arrowShift * fFrameWidth), 0);                //     _
                cairo_rel_line_to (pCairoContext, - my_diapo_simple_arrowWidth/2 - my_diapo_simple_arrowShift * fFrameWidth + my_diapo_simple_arrowShift * fFrameWidth / arrow_dec, my_diapo_simple_arrowHeight);        //    /     
                cairo_rel_line_to (pCairoContext, - my_diapo_simple_arrowWidth/2 + my_diapo_simple_arrowShift * fFrameWidth - my_diapo_simple_arrowShift * fFrameWidth / arrow_dec, -my_diapo_simple_arrowHeight);       //  \. 
                cairo_rel_line_to (pCairoContext, - (fFrameWidth/2 - my_diapo_simple_arrowWidth/2 + my_diapo_simple_arrowShift * fFrameWidth) , 0);               // _      
        }
        //\_________________ Coin bas gauche.
        cairo_rel_curve_to (pCairoContext,
                        0, 0,
                        -my_diapo_simple_radius, 0,
                        -my_diapo_simple_radius, -my_diapo_simple_radius );
                        
                        
        //BasGauche -> HautGauche
        cairo_rel_line_to (pCairoContext, 0, - fFrameHeight - my_diapo_simple_lineWidth + my_diapo_simple_radius * 2);
        //\_________________ Coin haut gauche.
        cairo_rel_curve_to (pCairoContext,
                0, 0,
                0 , -my_diapo_simple_radius ,
                my_diapo_simple_radius, -my_diapo_simple_radius );

}


static void cairo_dock_draw_frame_vertical_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
        const gdouble arrow_dec = 2;
	gdouble fFrameWidth  = pDock->iMaxDockWidth - 2*X_BORDER_SPACE;
	gdouble fFrameHeight = pDock->iMaxDockHeight - Y_BORDER_SPACE - (my_diapo_simple_arrowHeight+10);
	gdouble fDockOffsetX = X_BORDER_SPACE;
	gdouble fDockOffsetY = (pDock->bDirectionUp ? Y_BORDER_SPACE : 10);;
	

        cairo_move_to (pCairoContext, fDockOffsetY, fDockOffsetX);

        if(pDock->bDirectionUp)
        {
                cairo_rel_line_to (pCairoContext, 0, fFrameWidth);
        }
        else
        {
                cairo_rel_line_to (pCairoContext,0,(fFrameWidth/2 - my_diapo_simple_arrowWidth/2 - my_diapo_simple_arrowShift * fFrameWidth));                //     _
                cairo_rel_line_to (pCairoContext, -my_diapo_simple_arrowHeight, my_diapo_simple_arrowWidth/2 + my_diapo_simple_arrowShift * fFrameWidth - my_diapo_simple_arrowShift * fFrameWidth / arrow_dec);       //  \. 
                cairo_rel_line_to (pCairoContext, my_diapo_simple_arrowHeight, + my_diapo_simple_arrowWidth/2 - my_diapo_simple_arrowShift * fFrameWidth + my_diapo_simple_arrowShift * fFrameWidth / arrow_dec);        //    /     
                cairo_rel_line_to (pCairoContext,0,(fFrameWidth/2 - my_diapo_simple_arrowWidth/2 + my_diapo_simple_arrowShift * fFrameWidth));               // _     
       } 
        //\_________________ Coin haut droit.
        cairo_rel_curve_to (pCairoContext,
                0, 0,
                0, my_diapo_simple_radius,
                my_diapo_simple_radius, my_diapo_simple_radius);
        cairo_rel_line_to (pCairoContext, fFrameHeight + my_diapo_simple_lineWidth - my_diapo_simple_radius * 2, 0);
        //\_________________ Coin bas droit.
         cairo_rel_curve_to (pCairoContext,
                        0, 0,
                        my_diapo_simple_radius, 0,
                        my_diapo_simple_radius, -my_diapo_simple_radius);
        if(!pDock->bDirectionUp)
        {
                cairo_rel_line_to (pCairoContext, 0, - fFrameWidth);
        }
        else
        {
                //On fait la fleche
                cairo_rel_line_to (pCairoContext, 0, - (fFrameWidth/2 - my_diapo_simple_arrowWidth/2 - my_diapo_simple_arrowShift * fFrameWidth));                 //     _
                cairo_rel_line_to (pCairoContext,  my_diapo_simple_arrowHeight, - my_diapo_simple_arrowWidth/2 - my_diapo_simple_arrowShift * fFrameWidth + my_diapo_simple_arrowShift * fFrameWidth / arrow_dec);        //    /     
                cairo_rel_line_to (pCairoContext, -my_diapo_simple_arrowHeight, - my_diapo_simple_arrowWidth/2 + my_diapo_simple_arrowShift * fFrameWidth - my_diapo_simple_arrowShift * fFrameWidth / arrow_dec );       //  \. 
                cairo_rel_line_to (pCairoContext, 0, - (fFrameWidth/2 - my_diapo_simple_arrowWidth/2 + my_diapo_simple_arrowShift * fFrameWidth));                 // _      
        }
        
        //\_________________ Coin bas gauche.
         cairo_rel_curve_to (pCairoContext,
                        0, 0,
                        0, -my_diapo_simple_radius,
                        -my_diapo_simple_radius, -my_diapo_simple_radius);
        cairo_rel_line_to (pCairoContext, - fFrameHeight - my_diapo_simple_lineWidth + my_diapo_simple_radius * 2, 0);
        //\_________________ Coin haut gauche.
        cairo_rel_curve_to (pCairoContext,
                0, 0,
                -my_diapo_simple_radius, 0,
                -my_diapo_simple_radius, my_diapo_simple_radius);
}




void cairo_dock_draw_frame_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
        if (pDock->bHorizontalDock)
                cairo_dock_draw_frame_horizontal_for_diapo_simple (pCairoContext, pDock);
        else
                cairo_dock_draw_frame_vertical_for_diapo_simple (pCairoContext, pDock);
}



void cairo_dock_render_decorations_in_frame_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock)
{
////////////////////////////////////////////////////////////////////////////////////////On se fait un beau pattern dégradé :
        cairo_pattern_t *mon_super_pattern;
        ///cairo_save (pCairoContext);       
        mon_super_pattern = cairo_pattern_create_linear (0.0, 0.0,
                                                my_diapo_simple_fade2right  ? pDock->iMaxDockWidth  : 0.0,      // Y'aurait surement des calculs complexes à faire mais 
                                                my_diapo_simple_fade2bottom ? pDock->iMaxDockHeight : 0.0);     //  a quelques pixels près pour un dégradé : OSEF !
                                                
        cairo_pattern_add_color_stop_rgba (mon_super_pattern, 0, my_diapo_simple_color_frame_start[0],                   
                                                                 my_diapo_simple_color_frame_start[1], 
                                                                 my_diapo_simple_color_frame_start[2], 
                                                                 my_diapo_simple_color_frame_start[3] * (1. - pDock->fFoldingFactor)); // On gère aussi l'anim de depliage parcequ'on est des dingues  ----> completement d'accord ! :-D
                                                
        cairo_pattern_add_color_stop_rgba (mon_super_pattern, 1, my_diapo_simple_color_frame_stop[0] ,                  
                                                                 my_diapo_simple_color_frame_stop[1] , 
                                                                 my_diapo_simple_color_frame_stop[2] , 
                                                                 my_diapo_simple_color_frame_stop[3]  * (1. - pDock->fFoldingFactor));
        cairo_set_source (pCairoContext, mon_super_pattern);
        
////////////////////////////////////////////////////////////////////////////////////////On remplit le contexte en le préservant -> pourquoi ?  ----> parce qu'on va tracer le contour plus tard ;-)
        cairo_fill_preserve (pCairoContext);
        cairo_pattern_destroy (mon_super_pattern);
        ///cairo_restore (pCairoContext);
}


void cd_rendering_render_diapo_simple_opengl (CairoDock *pDock)
{
	//\____________________ On initialise le cadre.
	int iNbVertex;
	GLfloat *pColorTab, *pVertexTab;
	
	double fRadius = my_diapo_simple_radius;
	double fFrameWidth  = pDock->iMaxDockWidth - 2*X_BORDER_SPACE;  // longueur du trait horizontal.
	double fFrameHeight = pDock->iMaxDockHeight- Y_BORDER_SPACE - (my_diapo_simple_arrowHeight+10);  // hauteur du cadre avec les rayons et sans la pointe.
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bHorizontalDock)
	{
		fDockOffsetX = X_BORDER_SPACE;
		fDockOffsetY = (!pDock->bDirectionUp ? Y_BORDER_SPACE : my_diapo_simple_arrowHeight+10);
		fFrameWidth  = pDock->iMaxDockWidth - 2*X_BORDER_SPACE;  // longueur du trait horizontal.
		fFrameHeight = pDock->iMaxDockHeight- Y_BORDER_SPACE - (my_diapo_simple_arrowHeight+10);  // hauteur du cadre avec les rayons et sans la pointe.
	}
	else
	{
		fDockOffsetY = X_BORDER_SPACE;
		fDockOffsetX = (!pDock->bDirectionUp ? Y_BORDER_SPACE : my_diapo_simple_arrowHeight+10);
		fFrameHeight = pDock->iMaxDockWidth - 2*X_BORDER_SPACE;  // longueur du trait horizontal.
		fFrameWidth = pDock->iMaxDockHeight- Y_BORDER_SPACE - (my_diapo_simple_arrowHeight+10);  // hauteur du cadre avec les rayons et sans la pointe.
	}
	
	glPushMatrix ();
	glTranslatef ((int) (fDockOffsetX + fFrameWidth/2), (int) (fDockOffsetY + fFrameHeight/2), -100);  // (int) -pDock->iMaxIconHeight * (1 + myIcons.fAmplitude) + 1
	glScalef (fFrameWidth, fFrameHeight, 1.);
	
	glEnable (GL_BLEND); // On active le blend
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	if (my_diapo_simple_draw_background)  // On remplit le cadre en 2 temps (avec des polygones convxes).
	{
		glPolygonMode (GL_FRONT, GL_FILL);
		glEnableClientState (GL_VERTEX_ARRAY);
		glEnableClientState (GL_COLOR_ARRAY);
		
		//\____________________ Le cadre sans la fleche.
		pVertexTab = cd_rendering_generate_path_for_diapo_simple_opengl_without_arrow (pDock, &pColorTab, &iNbVertex);
		
		//glVertexPointer (_CAIRO_DOCK_PATH_DIM, GL_FLOAT, 0, pVertexTab);
		_cairo_dock_set_vertex_pointer (pVertexTab);
		glColorPointer (4, GL_FLOAT, 0, pColorTab);
		glDrawArrays (GL_POLYGON, 0, iNbVertex);
		
		glDisableClientState (GL_COLOR_ARRAY);
		//\____________________ La fleche.
		GLfloat color[4];
		pVertexTab = cd_rendering_generate_arrow_path_for_diapo_simple_opengl (pDock, color);
		glColor4fv (color);
		
		//glVertexPointer (_CAIRO_DOCK_PATH_DIM, GL_FLOAT, 0, pVertexTab);
		_cairo_dock_set_vertex_pointer (pVertexTab);
		glDrawArrays (GL_POLYGON, 0, 4);
		glDisableClientState (GL_VERTEX_ARRAY);
	}
	
	//\____________________ On genere le chemin complet (avec la fleche).
	pVertexTab = cd_rendering_generate_path_for_diapo_simple_opengl (pDock, &iNbVertex);
	
	//glVertexPointer (_CAIRO_DOCK_PATH_DIM, GL_FLOAT, 0, pVertexTab);
	_cairo_dock_set_vertex_pointer (pVertexTab);
	cairo_dock_draw_current_path_opengl (my_diapo_simple_lineWidth, my_diapo_simple_color_border_line, iNbVertex);
	
	glPopMatrix ();
	if (pDock->icons == NULL)
		return ;
	
	//\____________________ On dessine la ficelle.
	if (myIcons.iStringLineWidth > 0)
		cairo_dock_draw_string_opengl (pDock, myIcons.iStringLineWidth, FALSE, FALSE);
	
	//\____________________ On dessine les icones.
	// on determine la 1ere icone a tracer : l'icone suivant l'icone pointee.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	// on dessine les icones, l'icone pointee en dernier.
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		glPushMatrix ();
		
		cairo_dock_render_one_icon_opengl (icon, pDock, 1., TRUE);
		
		glPopMatrix ();
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	}
	while (ic != pFirstDrawnElement);
}


static const double a = 2.5;  // definit combien la fleche est penchee.

#define RADIAN (G_PI / 180.0)  // Conversion Radian/Degres
#define DELTA_ROUND_DEGREE 5
#define TIP_OFFSET_FACTOR 2.
#define _recopy_prev_color(pColorTab, i) memcpy (&pColorTab[4*i], &pColorTab[4*(i-1)], 4*sizeof (GLfloat));
#define _copy_color(pColorTab, i, pDock, c) do { \
	pColorTab[4*i]   = c[0];\
	pColorTab[4*i+1] = c[1];\
	pColorTab[4*i+2] = c[2];\
	pColorTab[4*i+3] = c[3] * (1. - pDock->fFoldingFactor); } while (0)
#define _copy_mean_color(pColorTab, i, pDock, c1, c2, f) do { \
	pColorTab[4*i]   = c1[0]*f + c2[0]*(1-f);\
	pColorTab[4*i+1] = c1[1]*f + c2[1]*(1-f);\
	pColorTab[4*i+2] = c1[2]*f + c2[2]*(1-f);\
	pColorTab[4*i+3] = (c1[3]*f + c2[3]*(1-f)) * (1. - pDock->fFoldingFactor); } while (0)
GLfloat *cd_rendering_generate_path_for_diapo_simple_opengl (CairoDock *pDock, int *iNbPoints)
{
	//static GLfloat pVertexTab[((90/DELTA_ROUND_DEGREE+1)*4+1+3)*3];  // +3 pour la pointe.
	_cairo_dock_define_static_vertex_tab ((90/DELTA_ROUND_DEGREE+1)*4+1+3);  // +3 pour la pointe.
	double fRadius = my_diapo_simple_radius;
	double fFrameWidth  = pDock->iMaxDockWidth - 2*X_BORDER_SPACE - 2*fRadius;  // longueur du trait horizontal.
	double fFrameHeight = pDock->iMaxDockHeight- Y_BORDER_SPACE - (my_diapo_simple_arrowHeight+10);  // hauteur du cadre avec les rayons et sans la pointe.
	
	const gdouble arrow_dec = 2;
	double fTotalWidth = fFrameWidth + 2 * fRadius;
	double w = fFrameWidth / fTotalWidth / 2;
	double h = MAX (0, fFrameHeight - 2 * fRadius) / fFrameHeight / 2;
	double rw = fRadius / fTotalWidth;
	double rh = fRadius / fFrameHeight;
	int i=0, t;
	int iPrecision = DELTA_ROUND_DEGREE;
	double x,y;  // 1ere coordonnee de la pointe.
	
	for (t = 0;t <= 90;t += iPrecision, i++) // cote haut droit.
	{
		_cairo_dock_set_vertex_xy (i,
			w + rw * cos (t*RADIAN),
			h + rh * sin (t*RADIAN));
	}
	if (!pDock->bDirectionUp && pDock->bHorizontalDock)  // dessin de la pointe vers le haut.
	{
		x = 0. + my_diapo_simple_arrowShift * (fFrameWidth/2 - my_diapo_simple_arrowWidth/2)/fTotalWidth + my_diapo_simple_arrowWidth/2/fTotalWidth;
		y = h + rh;
		_cairo_dock_set_vertex_xy (i,
			x,
			y);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x - my_diapo_simple_arrowWidth/2 * (1 + a * my_diapo_simple_arrowShift)/fTotalWidth,
			y + my_diapo_simple_arrowHeight/fFrameHeight);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x - my_diapo_simple_arrowWidth/fTotalWidth,
			y);
		i ++;
	}
	for (t = 90;t <= 180;t += iPrecision, i++) // haut gauche.
	{
		_cairo_dock_set_vertex_xy (i,
			-w + rw * cos (t*RADIAN),
			h + rh * sin (t*RADIAN));
	}
	if (!pDock->bDirectionUp && !pDock->bHorizontalDock)  // dessin de la pointe vers la gauche.
	{
		x = -w - rw;
		y = 0. + my_diapo_simple_arrowShift * (fFrameHeight/2 - fRadius - my_diapo_simple_arrowWidth/2)/fFrameHeight + my_diapo_simple_arrowWidth/2/fFrameHeight;
		_cairo_dock_set_vertex_xy (i,
			x,
			y);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x - my_diapo_simple_arrowHeight/fFrameHeight,
			y - my_diapo_simple_arrowWidth/2 * (1 + a * my_diapo_simple_arrowShift)/fFrameHeight);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x,
			y - my_diapo_simple_arrowWidth/fFrameHeight);
		i ++;
	}
	for (t = 180;t <= 270;t += iPrecision, i++) // bas gauche.
	{
		_cairo_dock_set_vertex_xy (i,
			-w + rw * cos (t*RADIAN),
			-h + rh * sin (t*RADIAN));
	}
	if (pDock->bDirectionUp && pDock->bHorizontalDock)  // dessin de la pointe vers le bas.
	{
		x = 0. + my_diapo_simple_arrowShift * (fFrameWidth/2 - my_diapo_simple_arrowWidth/2)/fTotalWidth - my_diapo_simple_arrowWidth/2/fTotalWidth;
		y = - h - rh;
		_cairo_dock_set_vertex_xy (i,
			x,
			y);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x + my_diapo_simple_arrowWidth/2 * (1 - a * my_diapo_simple_arrowShift)/fTotalWidth,
			y - my_diapo_simple_arrowHeight/fFrameHeight);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x + my_diapo_simple_arrowWidth/fTotalWidth,
			y);
		i ++;
	}
	for (t = 270;t <= 360;t += iPrecision, i++) // bas droit.
	{
		_cairo_dock_set_vertex_xy (i,
			w + rw * cos (t*RADIAN),
			-h + rh * sin (t*RADIAN));
	}
	if (pDock->bDirectionUp && !pDock->bHorizontalDock)  // dessin de la pointe vers la droite.
	{
		x = w + rw;
		y = 0. + my_diapo_simple_arrowShift * (fFrameHeight/2 - fRadius - my_diapo_simple_arrowWidth/2)/fFrameHeight - my_diapo_simple_arrowWidth/2/fFrameHeight;
		_cairo_dock_set_vertex_xy (i,
			x,
			y);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x + my_diapo_simple_arrowHeight/fFrameHeight,
			y + my_diapo_simple_arrowWidth/2 * (1 - a * my_diapo_simple_arrowShift)/fFrameHeight);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x,
			y + my_diapo_simple_arrowWidth/fFrameHeight);
		i ++;
	}
	_cairo_dock_close_path(i);
	
	*iNbPoints = i+1;
	_cairo_dock_return_vertex_tab ();
}

GLfloat *cd_rendering_generate_path_for_diapo_simple_opengl_without_arrow (CairoDock *pDock, GLfloat **colors, int *iNbPoints)
{
	//static GLfloat pVertexTab[((90/DELTA_ROUND_DEGREE+1)*4+1+3)*3];  // +3 pour la pointe.
	_cairo_dock_define_static_vertex_tab ((90/DELTA_ROUND_DEGREE+1)*4+1+3);  // +3 pour la pointe.
	static GLfloat pColorTab[((90/DELTA_ROUND_DEGREE+1)*4+1+3)*4];  // +3 pour la pointe.
	double fRadius = my_diapo_simple_radius;
	double fFrameWidth  = pDock->iMaxDockWidth - 2*X_BORDER_SPACE - 2*fRadius;  // longueur du trait horizontal.
	double fFrameHeight = pDock->iMaxDockHeight- Y_BORDER_SPACE - (my_diapo_simple_arrowHeight+10);  // hauteur du cadre avec les rayons et sans la pointe.
	
	const gdouble arrow_dec = 2;
	double fTotalWidth = fFrameWidth + 2 * fRadius;
	double w = fFrameWidth / fTotalWidth / 2;
	double h = MAX (0, fFrameHeight - 2 * fRadius) / fFrameHeight / 2;
	double rw = fRadius / fTotalWidth;
	double rh = fRadius / fFrameHeight;
	int i=0, t;
	int iPrecision = DELTA_ROUND_DEGREE;
	
	double *pTopRightColor, *pTopLeftColor, *pBottomLeftColor, *pBottomRightColor;
	double pMeanColor[4] = {(my_diapo_simple_color_frame_start[0] + my_diapo_simple_color_frame_stop[0])/2,
		(my_diapo_simple_color_frame_start[1] + my_diapo_simple_color_frame_stop[1])/2,
		(my_diapo_simple_color_frame_start[2] + my_diapo_simple_color_frame_stop[2])/2,
		(my_diapo_simple_color_frame_start[3] + my_diapo_simple_color_frame_stop[3])/2};
	pTopLeftColor = my_diapo_simple_color_frame_start;
	if (my_diapo_simple_fade2bottom || my_diapo_simple_fade2right)
	{
		pBottomRightColor = my_diapo_simple_color_frame_stop;
		if (my_diapo_simple_fade2bottom && my_diapo_simple_fade2right)
		{
			pBottomLeftColor = pMeanColor;
			pTopRightColor = pMeanColor;
		}
		else if (my_diapo_simple_fade2bottom)
		{
			pBottomLeftColor = my_diapo_simple_color_frame_stop;
			pTopRightColor = my_diapo_simple_color_frame_start;
		}
		else
		{
			pBottomLeftColor = my_diapo_simple_color_frame_start;
			pTopRightColor = my_diapo_simple_color_frame_stop;
		}
	}
	else
	{
		pBottomRightColor = my_diapo_simple_color_frame_start;
		pBottomLeftColor = my_diapo_simple_color_frame_start;
		pTopRightColor = my_diapo_simple_color_frame_start;
	}
	
	for (t = 0;t <= 90;t += iPrecision, i++) // cote haut droit.
	{
		_cairo_dock_set_vertex_xy (i,
			w + rw * cos (t*RADIAN),
			h + rh * sin (t*RADIAN));
		//pVertexTab[3*i] = w + rw * cos (t*RADIAN);
		//pVertexTab[3*i+1] = h + rh * sin (t*RADIAN);
		_copy_color (pColorTab, i, pDock, pTopRightColor);
	}
	for (t = 90;t <= 180;t += iPrecision, i++) // haut gauche.
	{
		_cairo_dock_set_vertex_xy (i,
			-w + rw * cos (t*RADIAN),
			h + rh * sin (t*RADIAN));
		//pVertexTab[3*i] = -w + rw * cos (t*RADIAN);
		//pVertexTab[3*i+1] = h + rh * sin (t*RADIAN);
		_copy_color (pColorTab, i, pDock, pTopLeftColor);
	}
	for (t = 180;t <= 270;t += iPrecision, i++) // bas gauche.
	{
		_cairo_dock_set_vertex_xy (i,
			-w + rw * cos (t*RADIAN),
			-h + rh * sin (t*RADIAN));
		//pVertexTab[3*i] = -w + rw * cos (t*RADIAN);
		//pVertexTab[3*i+1] = -h + rh * sin (t*RADIAN);
		_copy_color (pColorTab, i, pDock, pBottomLeftColor);
	}
	for (t = 270;t <= 360;t += iPrecision, i++) // bas droit.
	{
		_cairo_dock_set_vertex_xy (i,
			w + rw * cos (t*RADIAN),
			-h + rh * sin (t*RADIAN));
		//pVertexTab[3*i] = w + rw * cos (t*RADIAN);
		//pVertexTab[3*i+1] = -h + rh * sin (t*RADIAN);
		_copy_color (pColorTab, i, pDock, pBottomRightColor);
	}
	_cairo_dock_close_path(i);
	//pVertexTab[3*i] = w + rw;  // on boucle.
	//pVertexTab[3*i+1] = h;
	memcpy (&pColorTab[4*i], &pColorTab[0], 4*sizeof (GLfloat));
	
	*iNbPoints = i+1;
	*colors = pColorTab;
	_cairo_dock_return_vertex_tab ();
}

#define _set_arrow_color(c1, c2, f, pDock, colors) do {\
	colors[0] = c1[0] * (f) + c2[0] * (1 - (f));\
	colors[1] = c1[1] * (f) + c2[1] * (1 - (f));\
	colors[2] = c1[2] * (f) + c2[2] * (1 - (f));\
	colors[3] = (c1[3] * (f) + c2[3] * (1 - (f))) * (1 - (pDock)->fFoldingFactor); } while (0)
GLfloat *cd_rendering_generate_arrow_path_for_diapo_simple_opengl (CairoDock *pDock, GLfloat *color)
{
	//static GLfloat pVertexTab[((90/DELTA_ROUND_DEGREE+1)*4+1+3)*3];  // +3 pour la pointe.
	_cairo_dock_define_static_vertex_tab ((90/DELTA_ROUND_DEGREE+1)*4+1+3);  // +3 pour la pointe.
	double fRadius = my_diapo_simple_radius;
	double fFrameWidth  = pDock->iMaxDockWidth - 2*X_BORDER_SPACE - 2*fRadius;  // longueur du trait horizontal.
	double fFrameHeight = pDock->iMaxDockHeight- Y_BORDER_SPACE - (my_diapo_simple_arrowHeight+10);  // hauteur du cadre avec les rayons et sans la pointe.
	
	const gdouble arrow_dec = 2;
	double fTotalWidth = fFrameWidth + 2 * fRadius;
	double w = fFrameWidth / fTotalWidth / 2;
	double h = MAX (0, fFrameHeight - 2 * fRadius) / fFrameHeight / 2;
	double rw = fRadius / fTotalWidth;
	double rh = fRadius / fFrameHeight;
	int i=0, t;
	int iPrecision = DELTA_ROUND_DEGREE;
	
	double *pTopRightColor, *pTopLeftColor, *pBottomLeftColor, *pBottomRightColor;
	double pMeanColor[4] = {(my_diapo_simple_color_frame_start[0] + my_diapo_simple_color_frame_stop[0])/2,
		(my_diapo_simple_color_frame_start[1] + my_diapo_simple_color_frame_stop[1])/2,
		(my_diapo_simple_color_frame_start[2] + my_diapo_simple_color_frame_stop[2])/2,
		(my_diapo_simple_color_frame_start[3] + my_diapo_simple_color_frame_stop[3])/2};
	pTopLeftColor = my_diapo_simple_color_frame_start;
	if (my_diapo_simple_fade2bottom || my_diapo_simple_fade2right)
	{
		pBottomRightColor = my_diapo_simple_color_frame_stop;
		if (my_diapo_simple_fade2bottom && my_diapo_simple_fade2right)
		{
			pBottomLeftColor = pMeanColor;
			pTopRightColor = pMeanColor;
		}
		else if (my_diapo_simple_fade2bottom)
		{
			pBottomLeftColor = my_diapo_simple_color_frame_stop;
			pTopRightColor = my_diapo_simple_color_frame_start;
		}
		else
		{
			pBottomLeftColor = my_diapo_simple_color_frame_start;
			pTopRightColor = my_diapo_simple_color_frame_stop;
		}
	}
	else
	{
		pBottomRightColor = my_diapo_simple_color_frame_start;
		pBottomLeftColor = my_diapo_simple_color_frame_start;
		pTopRightColor = my_diapo_simple_color_frame_start;
	}
	
	double x,y;  // 1ere coordonnee de la pointe.
	if (!pDock->bDirectionUp && pDock->bHorizontalDock)  // dessin de la pointe vers le haut.
	{
		x = 0. + my_diapo_simple_arrowShift * (fFrameWidth/2 - my_diapo_simple_arrowWidth/2)/fTotalWidth + my_diapo_simple_arrowWidth/2/fTotalWidth;
		y = h + rh;
		_cairo_dock_set_vertex_xy (i,
			x,
			y);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x - my_diapo_simple_arrowWidth/2 * (1 + a * my_diapo_simple_arrowShift)/fTotalWidth,
			y + my_diapo_simple_arrowHeight/fFrameHeight);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x - my_diapo_simple_arrowWidth/fTotalWidth,
			y);
		i ++;
		_set_arrow_color (pTopRightColor, pTopLeftColor, .5+my_diapo_simple_arrowShift/2, pDock, color);
	}
	else if (!pDock->bDirectionUp && !pDock->bHorizontalDock)  // dessin de la pointe vers la gauche.
	{
		x = -w - rw;
		y = 0. + my_diapo_simple_arrowShift * (fFrameHeight/2 - fRadius - my_diapo_simple_arrowWidth/2)/fFrameHeight + my_diapo_simple_arrowWidth/2/fFrameHeight;
		_cairo_dock_set_vertex_xy (i,
			x,
			y);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x - my_diapo_simple_arrowHeight/fFrameHeight,
			y - my_diapo_simple_arrowWidth/2 * (1 + a * my_diapo_simple_arrowShift)/fFrameHeight);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x,
			y - my_diapo_simple_arrowWidth/fFrameHeight);
		i ++;
		_set_arrow_color (pTopLeftColor, pBottomLeftColor, .5+my_diapo_simple_arrowShift/2, pDock, color);
	}
	else if (pDock->bDirectionUp && pDock->bHorizontalDock)  // dessin de la pointe vers le bas.
	{
		x = 0. + my_diapo_simple_arrowShift * (fFrameWidth/2 - my_diapo_simple_arrowWidth/2)/fTotalWidth - my_diapo_simple_arrowWidth/2/fTotalWidth;
		y = - h - rh;
		_cairo_dock_set_vertex_xy (i,
			x,
			y);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x + my_diapo_simple_arrowWidth/2 * (1 - a * my_diapo_simple_arrowShift)/fTotalWidth,
			y - my_diapo_simple_arrowHeight/fFrameHeight);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x + my_diapo_simple_arrowWidth/fTotalWidth,
			y);
		i ++;
		_set_arrow_color (pBottomRightColor, pBottomLeftColor, .5+my_diapo_simple_arrowShift/2, pDock, color);
	}
	else if (pDock->bDirectionUp && !pDock->bHorizontalDock)  // dessin de la pointe vers la droite.
	{
		x = w + rw;
		y = 0. + my_diapo_simple_arrowShift * (fFrameHeight/2 - fRadius - my_diapo_simple_arrowWidth/2)/fFrameHeight - my_diapo_simple_arrowWidth/2/fFrameHeight;
		_cairo_dock_set_vertex_xy (i,
			x,
			y);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x + my_diapo_simple_arrowHeight/fFrameHeight,
			y + my_diapo_simple_arrowWidth/2 * (1 - a * my_diapo_simple_arrowShift)/fFrameHeight);
		i ++;
		_cairo_dock_set_vertex_xy (i,
			x,
			y + my_diapo_simple_arrowWidth/fFrameHeight);
		i ++;
		_set_arrow_color (pTopRightColor, pBottomRightColor, .5+my_diapo_simple_arrowShift/2, pDock, color);
	}
	_cairo_dock_close_path (i);  // on boucle.
	_cairo_dock_return_vertex_tab ();
}
