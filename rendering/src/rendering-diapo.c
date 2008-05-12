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

void cd_rendering_calculate_max_dock_size_diapo (CairoDock *pDock)
{
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
        
        
//////////////////////////////////////////////////////////////////////////////////////// On calcule la configuration de la grille :
        nIcones = cairo_dock_rendering_diapo_guess_grid(pDock->icons, &nRowsX, &nRowsY);   
        
        
//////////////////////////////////////////////////////////////////////////////////////// On cherche le premier élément :
	cairo_dock_calculate_icons_positions_at_rest_diapo (pDock->icons, &(pDock->iMinDockWidth), &(pDock->iMinDockHeight), nRowsX);
	cairo_dock_rendering_diapo_calculate_max_dock_size (pDock->icons, pDock->iMinDockWidth, pDock->iMinDockHeight, &(pDock->iMaxDockWidth), &(pDock->iMaxDockHeight), nRowsX, nRowsY);
	/*cd_message("grepme > nico %d - x %d - y %d", nIcones, nRowsX, nRowsY);
	cd_message("grepme > minw %d - minh %d", pDock->iMinDockWidth, pDock->iMinDockHeight);
	cd_message("grepme > maxw %d - maxh %d", pDock->iMaxDockWidth, pDock->iMaxDockHeight);*/


//////////////////////////////////////////////////////////////////////////////////////// Définition de la zone de déco - 0 pour l'instant
	pDock->iDecorationsHeight = 0;//pDock->iMinDockHeight;
	pDock->iDecorationsWidth  = 0;//pDock->iMinDockWidth;
	
	
//////////////////////////////////////////////////////////////////////////////////////// On affecte ca aussi au cas où
	pDock->fFlatDockWidth = pDock->iMinDockWidth;
}

void cairo_dock_set_subdock_position_linear (Icon *pPointedIcon, CairoDock *pDock)
{

	CairoDock *pSubDock = pPointedIcon->pSubDock;

	int iX = pPointedIcon->fXAtRest - (pDock->fFlatDockWidth - pDock->iMaxDockWidth) / 2 + pPointedIcon->fWidth / 2;


//////////////////////////////////////////////////////////////////////////////////////// On définit les paramètres du sous dock à partir de l'icone pointée :
	if (pSubDock->bHorizontalDock == pDock->bHorizontalDock)
	{
		pSubDock->fAlign = 0.5;
		pSubDock->iGapX = iX + pDock->iWindowPositionX - g_iScreenWidth[pDock->bHorizontalDock] / 2;
		pSubDock->iGapY = pDock->iGapY + pDock->iMaxDockHeight;
	}
	else
	{
		pSubDock->fAlign = (pDock->bDirectionUp ? 1 : 0);
		pSubDock->iGapX = (pDock->iGapY + pDock->iMaxDockHeight) * (pDock->bDirectionUp ? -1 : 1);
		if (pDock->bDirectionUp)
			pSubDock->iGapY = g_iScreenWidth[pDock->bHorizontalDock] - (iX + pDock->iWindowPositionX) - pSubDock->iMaxDockHeight / 2;
		else
			pSubDock->iGapY = iX + pDock->iWindowPositionX - pSubDock->iMaxDockHeight / 2;
	}
}


void cd_rendering_render_diapo (CairoDock *pDock)
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
	//TODO
	//\____________________ On dessine les icones avec leurs etiquettes.
	
	
//////////////////////////////////////////////////////////////////////////////////////// On définit le ratio entre les icones
	//double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	
	
//////////////////////////////////////////////////////////////////////////////////////// On redefinit avec la valeur du dock
	//fRatio = pDock->fRatio;
	
	
//////////////////////////////////////////////////////////////////////////////////////// On (re)cherche le premier élément :
	//GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	
	
//////////////////////////////////////////////////////////////////////////////////////// Si y'en a pas on se barre (dock vide)
	//if (pFirstDrawnElement == NULL) return;
        if (pDock->icons == NULL) return;
	
	
//////////////////////////////////////////////////////////////////////////////////////// On calcule la magnitude ---> ?
	//double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex)/** * pDock->fMagnitudeMax*/;
	
	
//////////////////////////////////////////////////////////////////////////////////////// On parcourt la liste les icones :
	Icon *icon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
	
//////////////////////////////////////////////////////////////////////////////////////// On recupere la structure d'infos
		icon = ic->data;


//////////////////////////////////////////////////////////////////////////////////////// On sauvegarde le contexte de cairo
		cairo_save (pCairoContext);
		

//////////////////////////////////////////////////////////////////////////////////////// On affiche l'icone en cour avec les options :		
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, pDock->fRatio, 0, pDock->bUseReflect, FALSE, pDock->iCurrentWidth, pDock->bDirectionUp);


//////////////////////////////////////////////////////////////////////////////////////// On restore le contexte de cairo
		cairo_restore (pCairoContext);
		
               //double fDockMagnitude = 1.;//1. + (icon->fScale - my_diapo_fScaleMax)/(my_diapo_fScaleMax - 1);
//////////////////////////////////////////////////////////////////////////////////////// On affiche le texte !
               if(icon->pTextBuffer != NULL)
                {
                	cairo_save (pCairoContext);
                	/*if (fRatio < 1)  // on met le texte à l'échelle de l'icone...
			        cairo_scale (pCairoContext,
			        	fRatio,
			        	fRatio); */
                	cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,                                        // TODO récupérer vrai valeurs
				icon->fDrawX + (icon->fWidth * icon->fScale)/2                   -icon->fTextXOffset,
				icon->fDrawY +  (icon->fHeight * icon->fScale)   + (my_diapo_iconGapY / 2)  - 5  ); // 5 = hauteur texte / 2
			
			if (my_diapo_text_only_on_pointed && icon->bPointed)
			        cairo_paint (pCairoContext);
		        else if (!my_diapo_text_only_on_pointed)
			        cairo_paint_with_alpha (pCairoContext, 1. + (icon->fScale - my_diapo_fScaleMax)/(my_diapo_fScaleMax - 1));
			
			cairo_restore (pCairoContext);
                }


//////////////////////////////////////////////////////////////////////////////////////// On passe à l'icone suivante
		//ic = cairo_dock_get_next_element (ic, pDock->icons);
	}
	
	
//////////////////////////////////////////////////////////////////////////////////////// On regle son compte au contexte
	cairo_destroy (pCairoContext);
	
	
//////////////////////////////////////////////////////////////////////////////////////// Si on est --glitz alors on s'en sert :
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}

Icon *cd_rendering_calculate_icons_diapo (CairoDock *pDock)
{


//////////////////////////////////////////////////////////////////////////////////////// On recherche l'icone pointée completement à l'arrache
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
        gint posMouseAbsX = 0;
        gint posMouseAbsY = 0;        
        
        
//////////////////////////////////////////////////////////////////////////////////////// On calcule la configuration de la grille :
        nIcones = cairo_dock_rendering_diapo_guess_grid(pDock->icons, &nRowsX, &nRowsY);      
	
	
	//\_______________ On calcule la position du curseur.
////////////////////////////////////////////////////////////////////////////////////////  pos abs = ecart par rapport au milieu du dock + ecart par rapport a la gauche du dock min.	
	posMouseAbsX = pDock->iMouseX; //- pDock->iCurrentWidth  / 2 + pDock->iMinDockWidth  / 2 - (pDock->iMaxDockWidth  - pDock->iCurrentWidth  ) / 2;
	posMouseAbsY = pDock->iMouseY;// - pDock->iCurrentHeight / 2 + pDock->iMinDockHeight / 2 - (pDock->iMaxDockHeight - pDock->iCurrentHeight ) / 2;
        /*cd_message("grep --x-- %d %d %d %d",  pDock->iMouseX, pDock->iCurrentWidth ,  pDock->iMinDockWidth , pDock->iMaxDockWidth );
        cd_message("grep --y-- %d %d %d %d",  pDock->iMouseY, pDock->iCurrentHeight,  pDock->iMinDockHeight, pDock->iMaxDockHeight);*/
        //\_______________ On calcule l'ensemble des parametres des icones.
	//double fMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex) * pDock->fMagnitudeMax;
	cairo_dock_calculate_wave_with_position_diapo(pDock->icons, posMouseAbsX, posMouseAbsY, nRowsX);
	
	Icon *pPointedIcon = cairo_dock_calculate_icons_position_for_diapo(pDock, nRowsX, nRowsY, posMouseAbsX, posMouseAbsY);
//(pDock->icons, pDock->pFirstDrawnElement, x_abs, fMagnitude, pDock->fFlatDockWidth, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->fAlign, pDock->fFoldingFactor, pDock->bDirectionUp);   //TODO icone pointée


        
//////////////////////////////////////////////////////////////////////////////////////// On regarde si la souris est à l'interieur
	//CairoDockMousePositionType iMousePositionType = cairo_dock_check_if_mouse_inside_linear (pDock); //TODO : L'icone pointée en 3D (cairo-dock-icons.c:1108)
	
	
//////////////////////////////////////////////////////////////////////////////////////// En fonction on fait des trucs : TODO comprendre
	//cairo_dock_manage_mouse_position (pDock, iMousePositionType);


	//\____________________ On calcule les position/etirements/alpha des icones.
	//cairo_dock_mark_avoiding_mouse_icons_linear (pDock);



//////////////////////////////////////////////////////////////////////////////////////// On revoie l'icone pointee et NULL sinon

	return pPointedIcon/*(iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL)*/;
}


void cd_rendering_register_diapo_renderer (void)
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
	
	cairo_dock_register_renderer (MY_APPLET_DIAPO_VIEW_NAME, pRenderer);                                    //Puis on signale l'existence de notre rendu

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
        Icon *pointedIcon ;
        cairo_dock_rendering_diapo_calculate_max_icon_size(pDock->icons, (guint*) &maxWidth, (guint*)  &maxHeight, nRowsX, nRowsY);
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

    	        
	       
	        //if(i == 0) cd_message("grep --x-- %lf %lf %d  --y-- %lf %lf %d", icon->fDrawXAtRest, icon->fDrawX, Mx, icon->fDrawYAtRest, icon->fDrawY, My);
////////////////////////////////////////////////////////////////////////////////////////On va check de la mouse là :
                if((Mx > icon->fX) && 
                   (My > icon->fY) &&
                   (Mx < icon->fX + maxWidth[x]  + 2 * my_diapo_iconGapX) &&
                   (My < icon->fY + maxHeight[y] + 2 * my_diapo_iconGapY))
                {        
                        icon->bPointed = TRUE;    
                        *pointedIcon = *icon;
                        icon->fAlpha = 1.;                           
	        }
	        else
	        {
	                icon->bPointed = FALSE; 
	                icon->fAlpha = 0.9;
	        }

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


//////////////////////////////////////////////////////////////////////////////////////// On calcule l'offset pour que ce soit centré :
        offsetX = (pDock->iMaxDockWidth  - /*pDock->iCurrentWidth */curDockWidth ) / 2;
        offsetY = (pDock->iMaxDockHeight - /*pDock->iCurrentHeight*/curDockHeight) / 2;      
                cd_message("grep > %d", pDock->icons);
        for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
	
	
//////////////////////////////////////////////////////////////////////////////////////// On recupere la structure d'infos
		icon = ic->data;
		icon->fDrawX += offsetX;
		icon->fDrawY += offsetY;

		
//////////////////////////////////////////////////////////////////////////////////////// On laisse le dock s'occuper des animations
		cairo_dock_manage_animations (icon, pDock);
	}
	return pointedIcon;
}



void cairo_dock_calculate_wave_with_position_diapo(GList *pIconList, gint Mx, gint My, guint nRowsX)
{
        guint i = 0;
        guint x = 0;
        guint y = 0;
        guint indexXforMouse = 0; 
        guint indexYforMouse = 0; 
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
        indexXforMouse = Mx / ( ( (Icon*) pIconList->data )->fWidth  + 2 * my_diapo_iconGapX );
        indexYforMouse = My / ( ( (Icon*) pIconList->data )->fHeight + 2 * my_diapo_iconGapY );
        //Icon *pointedIcon = g_list_nth_data (pIconList, cairo_dock_rendering_diapo_get_index_from_gridXY(nRowsX, indexXforMouse, indexYforMouse));
              
        GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cairo_dock_rendering_diapo_get_gridXY_from_index(nRowsX, i, &x, &y);
                guint x1 = Mx;
                gdouble x2 = icon->fDrawXAtRest + (icon->fWidth) / 2;
                guint y1 = My;
                gdouble y2 = icon->fDrawYAtRest + (icon->fHeight) / 2;
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
      //  return pointedIcon;
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
	        icon->fDrawXAtRest = iconeX;
	        icon->fDrawYAtRest = iconeY;
	        
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
        gint maxWidth [nRowsX];
        gint maxHeight[nRowsY];
        guint i = 0;       
        
        cairo_dock_calculate_wave_with_position_diapo(pIconList, (Wmin / 2), (Hmin / 2), nRowsX); 
        
        cairo_dock_rendering_diapo_calculate_max_icon_size(pIconList, (guint*)  &maxWidth, (guint*)  &maxHeight, nRowsX, nRowsY);
        
        *Hmax = *Wmax = 0;
        for(i = 0 ;  i < nRowsX ; i++) *Wmax += maxWidth [i] + 2 * my_diapo_iconGapX;
        for(i = 0 ;  i < nRowsY ; i++) *Hmax += maxHeight[i] + 2 * my_diapo_iconGapY;
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

