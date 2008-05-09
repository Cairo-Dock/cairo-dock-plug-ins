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

void cd_rendering_calculate_max_dock_size_diapo (CairoDock *pDock)
{
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
// On cherche le premier élément :
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_diapo (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);

	nIcones = cairo_dock_rendering_diapo_guess_grid(pDock->icons, &nRowsX, &nRowsY);
	
	cd_message("grepme > n : %d - x:%d - y:%d", nIcones, nRowsX, nRowsY);
// Définition de la zone de déco - 0 pour l'instant
	pDock->iDecorationsHeight = 0;                          //pDock->iMaxIconHeight + 2 * g_iFrameMargin;
	pDock->iDecorationsWidth  = 0;                           //pDock->iMaxDockWidth;
	
// Rayon des coins + calcul de la place en plus (?))
	//double fRadius = MIN (g_iDockRadius, (pDock->iDecorationsHeight + g_iDockLineWidth) / 2 - 1);
	//double fExtraWidth = g_iDockLineWidth + 2 * (fRadius + g_iFrameMargin);
	if(nIcones != 0)
	{
//On obtient les hauteur/largeur max
	        pDock->iMaxDockWidth = nRowsX * ((Icon*)pDock->icons->data)->fWidth;
	        pDock->iMaxDockHeight = nRowsY * ((Icon*)pDock->icons->data)->fHeight;
//Et les hauteur/largeur min
	        pDock->iMinDockWidth = nRowsX * ((Icon*)pDock->icons->data)->fWidth;
	        pDock->iMinDockHeight = nRowsY * ((Icon*)pDock->icons->data)->fHeight;
	}
	else
	{
	        pDock->iMaxDockWidth = 0;
	        pDock->iMaxDockHeight = 0;
	        pDock->iMinDockWidth = 0;
	        pDock->iMinDockHeight = 0;
	}
}

void cairo_dock_set_subdock_position_linear (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;

	int iX = pPointedIcon->fXAtRest - (pDock->fFlatDockWidth - pDock->iMaxDockWidth) / 2 + pPointedIcon->fWidth / 2;

//On définit les paramètres du sous dock à partir de l'icone pointée :
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
	
//On définit les paramètres de dessin (par defaut pour l'instant)
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
	
//On définit le ratio entre les icones
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
//On redefinit avec la valeur du dock
	fRatio = pDock->fRatio;
	
//On (re)cherche le premier élément :
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
//Si y'en a pas on se barre (dock vide)
	if (pFirstDrawnElement == NULL)
		return;
//On calcule la magnitude ---> ?
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex)/** * pDock->fMagnitudeMax*/;
//On parcourt la liste les icones :
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
//On recupere la structure d'infos
		icon = ic->data;

//On sauvegarde le contexte -> ?
		cairo_save (pCairoContext);

//On affiche l'icone en cour avec les options :		
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE, pDock->iCurrentWidth, pDock->bDirectionUp);

//On restore le contexte --> ?
		cairo_restore (pCairoContext);

//On passe à l'icone suivante
		ic = cairo_dock_get_next_element (ic, pDock->icons);

//Si c'est la première (?) on s'arrete
	} while (ic != pFirstDrawnElement);
	
//On regle son compte au contexte
	cairo_destroy (pCairoContext);
	
//Si on est --glitz alors on s'en sert :
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}

Icon *cd_rendering_calculate_icons_diapo (CairoDock *pDock)
{
//On recherche l'icone pointée completement à l'arrache
        guint nRowsX = 0;
        guint nRowsY = 0;
        guint nIcones = 0;
        
	//TODO : L'icone pointée en 3D (cairo-dock-icons.c:1096)
	//\_______________ On calcule la position du curseur dans le referentiel du dock a plat.
	int dx = pDock->iMouseX - pDock->iCurrentWidth / 2;  // ecart par rapport au milieu du dock a plat.
	int x_abs = dx + pDock->fFlatDockWidth / 2;  // ecart par rapport a la gauche du dock minimal  plat.

	//\_______________ On calcule l'ensemble des parametres des icones.
	double fMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex) * pDock->fMagnitudeMax;
	Icon *pPointedIcon ;//= cairo_dock_calculate_wave_with_position_diapo (pDock->icons, pDock->pFirstDrawnElement, x_abs, fMagnitude, pDock->fFlatDockWidth, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->fAlign, pDock->fFoldingFactor, pDock->bDirectionUp);   TODO icone pointée

        nIcones = cairo_dock_rendering_diapo_guess_grid(pDock->icons, &nRowsX, &nRowsY);
        
//On regarde si la souris est à l'interieur
	CairoDockMousePositionType iMousePositionType = cairo_dock_check_if_mouse_inside_linear (pDock); //TODO : L'icone pointée en 3D (cairo-dock-icons.c:1108)
//En fonction on fait des trucs : TODO comprendre
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);

	//\____________________ On calcule les position/etirements/alpha des icones.
	cairo_dock_mark_avoiding_mouse_icons_linear (pDock);

//On crée un pointeur d'icone
	Icon* icon;
//On crée une liste d'icone des icone à parcourir :
	GList* ic;
//Pour toutes les icones 
        gint i = 0;
        double firstX = 0.;
        double firstY = 0.;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
//On recupere la structure d'infos
		icon = ic->data;
//On affecte les parametres de dessin  :

	        icon->fDrawX = firstX + icon->fWidth * (i % nRowsX);                // l'abscisse
	        icon->fDrawY = firstY + icon->fHeight * (int) (i / nRowsX);                // l'ordonnée
	        cd_message("grepme > x:%.2f- y:%.2f - i:%d", icon->fDrawX, icon->fDrawY, i);
	        icon->fWidthFactor = 1.;                // zoom sur largeur
	        icon->fHeightFactor = 1.;               // zoom sur hauteur
	        icon->fOrientation = 0.;                // rotation de l'icone
//Transparence de l'icone :
	        if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->iCurrentWidth)
	        {
		        icon->fAlpha = 1;
	        }
	        else
	        {
		        icon->fAlpha = .25;
	        }
//On laisse le dock s'occuper des animations
		cairo_dock_manage_animations (icon, pDock);
	        i++;
	}
//On revoie l'icone pointee et NULL sinon
	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
}


void cd_rendering_register_diapo_renderer (void)
{
        //On definit le renderer :
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


Icon * cairo_dock_calculate_wave_with_position_diapo (GList *pIconList, GList *pFirstDrawnElementGiven, int x_abs, gdouble fMagnitude, double fFlatDockWidth, int iWidth, int iHeight, double fAlign, double fFoldingFactor, gboolean bDirectionUp)
{
	/*if (x_abs < 0 && iWidth > 0)  // ces cas limite sont la pour empecher les icones de retrecir trop rapidement quand on sort par les cotes.
		x_abs = -1;
	else if (x_abs > fFlatDockWidth && iWidth > 0)
		x_abs = fFlatDockWidth+1;
	if (pIconList == NULL)
		return NULL;

	float x_cumulated = 0, fXMiddle, fDeltaExtremum;
	GList* ic, *pointed_ic;
	Icon *icon, *prev_icon;

	GList *pFirstDrawnElement = (pFirstDrawnElementGiven != NULL ? pFirstDrawnElementGiven : pIconList);
	ic = pFirstDrawnElement;
	pointed_ic = (x_abs < 0 ? ic : NULL);
	do
	{
		icon = ic->data;
		x_cumulated = icon->fXAtRest;
		fXMiddle = icon->fXAtRest + icon->fWidth / 2;

		//\_______________ On calcule sa phase (pi/2 au niveau du curseur).
		icon->fPhase = (fXMiddle - x_abs) / g_iSinusoidWidth * G_PI + G_PI / 2;
		if (icon->fPhase < 0)
		{
			icon->fPhase = 0;
		}
		else if (icon->fPhase > G_PI)
		{
			icon->fPhase = G_PI;
		}
		
		//\_______________ On en deduit l'amplitude de la sinusoide au niveau de cette icone, et donc son echelle.
		icon->fScale = 1 + fMagnitude * g_fAmplitude * sin (icon->fPhase);
		if (icon->fPersonnalScale > 0 && iWidth > 0)
		{
			icon->fPersonnalScale *= .85;
			icon->fScale *= icon->fPersonnalScale;
			if (icon->fPersonnalScale < 0.05)
				icon->fPersonnalScale = 0.05;
		}
		else if (icon->fPersonnalScale < 0 && iWidth > 0)
		{
			icon->fPersonnalScale *= .85;
			icon->fScale *= (1 + icon->fPersonnalScale);
			if (icon->fPersonnalScale > -0.05)
				icon->fPersonnalScale = -0.05;
		}
		icon->fY = (bDirectionUp ? iHeight - g_iDockLineWidth - g_iFrameMargin - icon->fScale * icon->fHeight : g_iDockLineWidth + g_iFrameMargin);
		
		//\_______________ Si on avait deja defini l'icone pointee, on peut placer l'icone courante par rapport a la precedente.
		if (pointed_ic != NULL)
		{
			if (ic == pFirstDrawnElement)  // peut arriver si on est en dehors a gauche du dock.
			{
				icon->fX = x_cumulated - 1. * (fFlatDockWidth - iWidth) / 2;
				//g_print ("  en dehors a gauche : icon->fX = %.2f (%.2f)\n", icon->fX, x_cumulated);
			}
			else
			{
				prev_icon = (ic->prev != NULL ? ic->prev->data : cairo_dock_get_last_icon (pIconList));
				icon->fX = prev_icon->fX + (prev_icon->fWidth + g_iIconGap) * prev_icon->fScale;

				if (icon->fX + icon->fWidth * icon->fScale > icon->fXMax - g_fAmplitude * fMagnitude * (icon->fWidth + 1.5*g_iIconGap) / 8 && iWidth != 0)
				{
					//g_print ("  on contraint %s (fXMax=%.2f , fX=%.2f\n", prev_icon->acName, prev_icon->fXMax, prev_icon->fX);
					fDeltaExtremum = icon->fX + icon->fWidth * icon->fScale - (icon->fXMax - g_fAmplitude * fMagnitude * (icon->fWidth + 1.5*g_iIconGap) / 16);
					icon->fX -= fDeltaExtremum * (1 - (icon->fScale - 1) / g_fAmplitude) * fMagnitude;
				}
			}
			icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1. - fFoldingFactor);
			//g_print ("  a droite : icon->fX = %.2f (%.2f)\n", icon->fX, x_cumulated);
		}
		
		//\_______________ On regarde si on pointe sur cette icone.
		if (x_cumulated + icon->fWidth + .5*g_iIconGap >= x_abs && x_cumulated - .5*g_iIconGap <= x_abs && pointed_ic == NULL)  // on a trouve l'icone sur laquelle on pointe.
		{
			pointed_ic = ic;
			icon->bPointed = TRUE;
			icon->fX = x_cumulated - (fFlatDockWidth - iWidth) / 2 + (1 - icon->fScale) * (x_abs - x_cumulated + .5*g_iIconGap);
			icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1. - fFoldingFactor);
			//g_print ("  icone pointee : fX = %.2f (%.2f)\n", icon->fX, x_cumulated);
		}
		else
			icon->bPointed = FALSE;
			
		ic = cairo_dock_get_next_element (ic, pIconList);
	} while (ic != pFirstDrawnElement);
	
	//\_______________ On place les icones precedant l'icone pointee par rapport a celle-ci.
	if (pointed_ic == NULL)  // on est a droite des icones.
	{
		pointed_ic = (pFirstDrawnElement->prev == NULL ? g_list_last (pIconList) : pFirstDrawnElement->prev);
		icon = pointed_ic->data;
		icon->fX = x_cumulated - (fFlatDockWidth - iWidth) / 2 + (1 - icon->fScale) * (icon->fWidth + .5*g_iIconGap);
		icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1 - fFoldingFactor);
		//g_print ("  en dehors a droite : icon->fX = %.2f (%.2f)\n", icon->fX, x_cumulated);
	}
	
	ic = pointed_ic;
	while (ic != pFirstDrawnElement)
	{
		icon = ic->data;
		
		ic = ic->prev;
		if (ic == NULL)
			ic = g_list_last (pIconList);
			
		prev_icon = ic->data;
		
		prev_icon->fX = icon->fX - (prev_icon->fWidth + g_iIconGap) * prev_icon->fScale;
		//g_print ("fX <- %.2f; fXMin : %.2f\n", prev_icon->fX, prev_icon->fXMin);
		if (prev_icon->fX < prev_icon->fXMin + g_fAmplitude * fMagnitude * (prev_icon->fWidth + 1.5*g_iIconGap) / 8 && iWidth != 0 && x_abs < iWidth && fMagnitude > 0)  /// && prev_icon->fPhase == 0   // on rajoute 'fMagnitude > 0' sinon il y'a un leger "saut" du aux contraintes a gauche de l'icone pointee.
		{
			//g_print ("  on contraint %s (fXMin=%.2f , fX=%.2f\n", prev_icon->acName, prev_icon->fXMin, prev_icon->fX);
			fDeltaExtremum = prev_icon->fX - (prev_icon->fXMin + g_fAmplitude * fMagnitude * (prev_icon->fWidth + 1.5*g_iIconGap) / 16);
			prev_icon->fX -= fDeltaExtremum * (1 - (prev_icon->fScale - 1) / g_fAmplitude) * fMagnitude;
		}
		prev_icon->fX = fAlign * iWidth + (prev_icon->fX - fAlign * iWidth) * (1. - fFoldingFactor);
		//g_print ("  prev_icon->fX : %.2f\n", prev_icon->fX);
	}

	return pointed_ic->data;*/
}

GList *cairo_dock_calculate_icons_positions_at_rest_diapo (GList *pIconList, double fFlatDockWidth, int iXOffset)
{
	//g_print ("%s (%d, +%d)\n", __func__, fFlatDockWidth, iXOffset);
	double x_cumulated = iXOffset;
	double fXMin = 99999;
	GList* ic, *pFirstDrawnElement = NULL;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{

		icon = ic->data;

		if (x_cumulated + icon->fWidth / 2 < 0)
			icon->fXAtRest = x_cumulated + fFlatDockWidth;
		else if (x_cumulated + icon->fWidth / 2 > fFlatDockWidth)
			icon->fXAtRest = x_cumulated - fFlatDockWidth;
		else
			icon->fXAtRest = x_cumulated;

		if (icon->fXAtRest < fXMin)
		{
			fXMin = icon->fXAtRest;
			pFirstDrawnElement = ic;
		}
		//g_print ("%s : fXAtRest = %.2f\n", icon->acName, icon->fXAtRest);

		x_cumulated += icon->fWidth + g_iIconGap;
	}


	return pFirstDrawnElement;
}

guint cairo_dock_rendering_diapo_guess_grid(GList *pIconList, guint *nRowX, guint *nRowY)
{
	//Calcul du nombre de ligne / colonne :
	guint count = g_list_length(pIconList);
	*nRowY = count  ? ceil(sqrt(count)) : 0;
	*nRowX = count  ? ceil(((double) count) / *nRowY) : 0;
	return count;
}

