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
const  gint  MaxTextWidthSimple = 125; 

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
	   pDock->iMinDockHeight = pDock->iMaxDockHeight = nRowsY * (((Icon*)pDock->icons->data)->fHeight + my_diapo_simple_iconGapY) + Y_BORDER_SPACE * 2 + my_diapo_simple_arrowHeight-30 + myLabels.iconTextDescription.iSize;  // 30 -> pour que la fleche aille plus bas
        }
        else
        {
                ///pDock->iMaxDockWidth = pDock->iMaxDockHeight = pDock->iMinDockWidth = pDock->iMinDockHeight = 0;
                pDock->iMaxDockWidth = pDock->iMinDockWidth = X_BORDER_SPACE * 2;
                pDock->iMaxDockHeight = pDock->iMinDockHeight = Y_BORDER_SPACE * 2 + my_diapo_simple_arrowHeight-30;
        }

//////////////////////////////////////////////////////////////////////////////////////// Définition de la zone de déco avec Valérie Damidot - 0 car on s'en fout un peu comme l'emission de cette grognasse... :D
	pDock->iDecorationsHeight = 0;
	pDock->iDecorationsWidth  = 0;
	
//////////////////////////////////////////////////////////////////////////////////////// On affecte ca aussi au cas où
	pDock->fFlatDockWidth = pDock->iMinDockWidth;

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
	//\____________________ On dessine la ficelle qui les joint.*/
	//TODO Rendre joli !
	if (myIcons.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIcons.iStringLineWidth, TRUE, TRUE);
	
	//\____________________ On dessine les icones avec leurs etiquettes.
	
//////////////////////////////////////////////////////////////////////////////////////// Si y'en a pas on se barre (dock vide)
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
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, 1., FALSE);

//////////////////////////////////////////////////////////////////////////////////////// On restore le contexte de cairo
		cairo_restore (pCairoContext);
		
                gdouble zoom;
//////////////////////////////////////////////////////////////////////////////////////// On affiche le texte !
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
               if(FALSE && icon->pTextBuffer != NULL)
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
                }
	}
}

static void _cd_rendering_check_if_mouse_inside_diapo_simple (CairoDock *pDock)
{
	if (! pDock->bInside)
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
	}
	else if ((pDock->iMouseX < (X_BORDER_SPACE/2)) || (pDock->iMouseX > pDock->iMaxDockWidth - (X_BORDER_SPACE/2)) || (pDock->iMouseY < (Y_BORDER_SPACE/2)) || (pDock->iMouseY > pDock->iMaxDockHeight - (Y_BORDER_SPACE/2)))
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
	gdouble fFrameHeight = pDock->iMaxDockHeight-2*Y_BORDER_SPACE - my_diapo_simple_arrowHeight + 30; //  30->pour que la fleche aille plus bas...  -----> quel petit joueur, regarde les calculs de malade que je me suis tape pour la pointe des dialogues ! :-)
	gdouble fDockOffsetX = X_BORDER_SPACE;
	gdouble fDockOffsetY = Y_BORDER_SPACE;
	
	

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
	gdouble fFrameWidth  = pDock->iMaxDockWidth-2*X_BORDER_SPACE;
	gdouble fFrameHeight = pDock->iMaxDockHeight-2*Y_BORDER_SPACE - my_diapo_simple_arrowHeight;
	gdouble fDockOffsetX = X_BORDER_SPACE;
	gdouble fDockOffsetY = Y_BORDER_SPACE;
	

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
        cairo_save (pCairoContext);       
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
        cairo_restore (pCairoContext);
}
