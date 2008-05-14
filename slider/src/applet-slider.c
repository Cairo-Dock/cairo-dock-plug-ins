/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>
#include <dirent.h> 
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <time.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-silder.h"

CD_APPLET_INCLUDE_MY_VARS

double fMaxScale;

static int _cd_slider_random_compare (gconstpointer a, gconstpointer b, GRand *pRandomGenerator) {
	return (g_rand_boolean (pRandomGenerator) ? 1 : -1);
}

void cd_slider_get_files_from_dir(void) {
	if (myConfig.cDirectory == NULL)
		return;
	
	DIR *d;
	struct dirent *dir;
	gchar *pFilePath;
	const gchar *cFileName, *extension;
	
	cd_message("Opening %s...", myConfig.cDirectory);  /// le faire par recurrence
	myData.pList=NULL;
	myData.iImagesNumber=0;
	d = opendir(myConfig.cDirectory);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (strcmp(dir->d_name, ".") == 0) continue;
			if (strcmp(dir->d_name, "..") == 0) continue;
			cFileName = dir->d_name;
			extension = strchr(cFileName,'.');
			if (extension != NULL) {
				if (g_ascii_strcasecmp(extension, ".png") == 0
				|| g_ascii_strcasecmp(extension, ".jpg") == 0
				|| g_ascii_strcasecmp(extension, ".svg") == 0
				|| g_ascii_strcasecmp(extension, ".xpm") == 0) {
					cd_debug ("  Adding %s to list", cFileName);
					pFilePath = g_strconcat (myConfig.cDirectory, "/", cFileName, NULL);
					myData.pList = g_list_prepend (myData.pList, pFilePath);
					myData.iImagesNumber++;
				}
				else {
					cd_debug ("%s not handeled, ignoring...", cFileName);
				}
			}
		}
		closedir(d);
	}
	if (myConfig.bRandom) {
		cd_message ("Mixing images ...");
		GRand *pRandomGenerator = g_rand_new ();
		myData.pList = g_list_sort_with_data (myData.pList, (GCompareDataFunc) _cd_slider_random_compare, pRandomGenerator);
		g_rand_free (pRandomGenerator);
	}
	else {
		myData.pList = g_list_reverse (myData.pList);
	}
	
	myData.pElement = myData.pList;
	//_printList(myData.pList);
	cd_slider_draw_images();
}


//A optimiser!
gboolean cd_slider_draw_images(void) {
	if (myData.bPause == TRUE)
		return FALSE;
	
	//\___________________________ On recupere la nouvelle image a afficher.
	if (myData.pElement == NULL || myData.pElement->data == NULL) {
		cd_warning ("Slider stopped, list broken");
		return FALSE;
 	}
	gchar *cImagePath = myData.pElement->data;
	cd_message("Displaying: %s\n", cImagePath);
	
	//\___________________________ On sauvegarde la surface actuelle et on charge la nouvelle surface.
	cairo_surface_destroy (myData.pPrevCairoSurface);
	myData.pPrevCairoSurface = myData.pCairoSurface;
	myData.pPrevImgL = myData.pImgL;
	
	double fImgX=myConfig.pFrameOffset, fImgY=myConfig.pFrameOffset, fImgW=0., fImgH=0., fW = myIcon->fWidth - (myConfig.pFrameOffset * 2.), fH = myIcon->fHeight - (myConfig.pFrameOffset * 2.);
	CairoDockLoadImageModifier iLoadingModifier = CAIRO_DOCK_FILL_SPACE;
	
	if (! myConfig.bFillIcon)
		iLoadingModifier |= CAIRO_DOCK_DONT_ZOOM_IN;
	if (myConfig.bNoStrench)
		iLoadingModifier |= CAIRO_DOCK_KEEP_RATIO;
	
	myData.pCairoSurface = cairo_dock_create_surface_from_image (cImagePath, myDrawContext, cairo_dock_get_max_scale (myContainer), fW, fH, &fImgW, &fImgH, iLoadingModifier);
	
	myData.pImgL.fImgX = fImgX;
	myData.pImgL.fImgY = fImgY;
	myData.pImgL.fImgW = fImgW;
	myData.pImgL.fImgH = fImgH;
	
	//\___________________________ On arrete l'animation precedente si elle n'etait pas finie (ne devrait pas arriver).
	if (myData.iAnimTimerID != 0) {
		cd_warning ("slider : previous animation didn't finish before the new one begins.");
		g_source_remove(myData.iAnimTimerID);
		myData.iAnimTimerID = 0;
	}
	
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	
	switch (myConfig.iAnimation) {
		case SLIDER_DEFAULT: default:
			cd_debug("Displaying with default");
			//\______________________ On efface le fond
			cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
			cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
			cairo_paint (myDrawContext);
			cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
			
			//\______________________ On empeche la transparence
			cairo_save (myDrawContext);
			cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myConfig.pBackgroundColor[3]);
			cairo_rectangle(myDrawContext, myData.pImgL.fImgX, myData.pImgL.fImgY, myData.pImgL.fImgW, myData.pImgL.fImgH);
			cairo_fill(myDrawContext);
			cairo_restore (myDrawContext);
			
			//\______________________ On dessine la nouvelle surface.
			cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
			cairo_paint (myDrawContext);
			
 			CD_APPLET_REDRAW_MY_ICON
		break;
		case SLIDER_FADE:
			cd_debug("Displaying with fade");
			myData.fAnimAlpha = 0.;
			myData.fAnimCNT = 1.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_fade, (gpointer) NULL);
		break;
		case SLIDER_BLANK_FADE:
			cd_debug("Displaying with blank fade");
			myData.fAnimAlpha = 1.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_blank_fade, (gpointer) NULL);
		break;
		case SLIDER_FADE_IN_OUT:
			cd_debug("Displaying with fade in out");
			myData.iAnimCNT = 0;
			myData.fAnimAlpha = 0.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_fade_in_out, (gpointer) NULL);
		break;
		case SLIDER_SIDE_KICK:
			cd_debug("Displaying with side kick");
			myData.fAnimCNT = -myData.pImgL.fImgW;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_side_kick, (gpointer) NULL);
		break;
		case SLIDER_DIAPORAMA:
			cd_debug("Displaying with diaporama");
			myData.fAnimCNT = -myData.pImgL.fImgW - 10;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_diaporama, (gpointer) NULL);
		break;
		case SLIDER_GROW_UP:
			cd_debug("Displaying with grow up");
			myData.fAnimAlpha = 0.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_grow_up, (gpointer) NULL);
		break;
		case SLIDER_SHRINK_DOWN:
			cd_debug("Displaying with shrink down");
			myData.fAnimAlpha = 2.5;
			myData.fAnimCNT = .3;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_shrink_down, (gpointer) NULL);
		break;
	}
	
	/// Afficher le reflet... Pourquoi pas?
	//cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer);
	
	//\______________________ On passe a l'image suivante.
	myData.pElement = cairo_dock_get_next_element (myData.pElement, myData.pList);
	cd_debug ("Next Image: %s\n", myData.pElement->data);
	
	if (myConfig.iAnimation == SLIDER_DEFAULT) {
		if (myData.iTimerID == 0)
			myData.iTimerID = g_timeout_add (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		return TRUE;  // on reboucle tout de suite (pas d'animation).
	}
	else {
		myData.iTimerID = 0;
		return FALSE;  // on quitte la boucle des images car on va effectuer une animation.
	}
}

static void _cd_slider_add_frame_to_current_slide (void) {
	cairo_set_source_surface (myDrawContext, myData.pCairoFrameSurface, 0., 0.);
	cairo_paint_with_alpha (myDrawContext, myConfig.pFrameAlpha);
}

static void _cd_slider_add_background_to_current_slide (double fX, double fY) {
	cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myConfig.pBackgroundColor[3]);
	fMaxScale = cairo_dock_get_max_scale (myContainer);
	cairo_rectangle (myDrawContext, fX, fY, myData.pImgL.fImgW * fMaxScale, myData.pImgL.fImgH * fMaxScale);
	cairo_fill (myDrawContext);
}

gboolean cd_slider_fade (void) {
	myData.fAnimAlpha = myData.fAnimAlpha +.1;
	myData.fAnimCNT = myData.fAnimCNT -.1;
	
	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	_cd_slider_add_frame_to_current_slide(); //Add background frame
	
	//Fond précédent.
	if (myData.pPrevCairoSurface != NULL) {
		cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myData.fAnimCNT * myConfig.pBackgroundColor[3]);
		cairo_rectangle (myDrawContext, myData.pPrevImgL.fImgX, myData.pPrevImgL.fImgY, myData.pPrevImgL.fImgW, myData.pPrevImgL.fImgH);
		cairo_fill (myDrawContext);
	}
	
	//On empeche la transparence.
	cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myData.fAnimAlpha * myConfig.pBackgroundColor[3]);
	cairo_rectangle (myDrawContext, myData.pImgL.fImgX, myData.pImgL.fImgY, myData.pImgL.fImgW, myData.pImgL.fImgH);
	cairo_fill (myDrawContext);
	
	//Image précédente
	if (myData.pPrevCairoSurface != NULL) {
		cairo_set_source_surface (myDrawContext, myData.pPrevCairoSurface, myData.pPrevImgL.fImgX, myData.pPrevImgL.fImgY);
		cairo_paint_with_alpha (myDrawContext, myData.fAnimCNT);
	}
	
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
	cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimAlpha >= .99) {
		myData.iTimerID = g_timeout_add (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_blank_fade (void) {
	myData.fAnimAlpha = myData.fAnimAlpha -.1;
	
	//On efface le fond
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	_cd_slider_add_frame_to_current_slide(); //Add background frame
	
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.pImgL.fImgX, myData.pImgL.fImgY);
	
	//Image
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
	cairo_paint (myDrawContext);
	
	//Masque
	cairo_set_source_rgba (myDrawContext, 1., 1., 1., myData.fAnimAlpha);
	cairo_rectangle(myDrawContext, 0., 0., myIcon->fWidth, myIcon->fHeight);
	cairo_fill(myDrawContext);
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimAlpha <= 0.01) {
		myData.iTimerID = g_timeout_add (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_fade_in_out (void) {
	
	if (myData.fAnimAlpha <= 1 && myData.iAnimCNT == 0) { //On augmente l'alpha
		myData.fAnimAlpha += .1;
	}
	if (myData.fAnimAlpha >= 1 &&myData.iAnimCNT <= 700) { //On attent 100/50 ms
		myData.iAnimCNT += 10;
		return TRUE;
	}
	if (myData.iAnimCNT >= 100) {
		myData.fAnimAlpha = myData.fAnimAlpha - 0.1; //On diminue l'alpha
	}
	
	//On efface le fond
	cairo_set_source_rgba (myDrawContext, 1., 1., 1., 0.);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	_cd_slider_add_frame_to_current_slide(); //Add background frame
	
	//On empeche la transparence
	cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myData.fAnimAlpha * myConfig.pBackgroundColor[3]);
	cairo_rectangle (myDrawContext, myData.pImgL.fImgX, myData.pImgL.fImgY, myData.pImgL.fImgW, myData.pImgL.fImgH);
	cairo_fill (myDrawContext);
	
	//Image
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
	cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimAlpha <= 0.01  && myData.iAnimCNT >= .99) { //On arrete l'animation
		myData.iTimerID = g_timeout_add (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_side_kick (void) {
	
	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	_cd_slider_add_frame_to_current_slide(); //Add background frame
	
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.fAnimCNT, myData.pImgL.fImgY);
	
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.fAnimCNT, myData.pImgL.fImgY);
	cairo_paint (myDrawContext);
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	//Effet d'arrivee rapide, passage lent, sortie rapide comme un coup de pied
	if (myData.fAnimCNT >= (-myData.pImgL.fImgW / 2) && myData.fAnimCNT <= (myIcon->fWidth / 2)) {
		myData.fAnimCNT = myData.fAnimCNT +1.;
	}
	else {
		myData.fAnimCNT = myData.fAnimCNT +5.;
	}
	
	if (myData.fAnimCNT >= myIcon->fWidth+5) {
		myData.iTimerID = g_timeout_add (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
		
	return TRUE;
}

gboolean cd_slider_diaporama (void) {
	myData.fAnimCNT = myData.fAnimCNT +.5;

	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	_cd_slider_add_frame_to_current_slide(); //Add background frame
	
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.fAnimCNT, myData.pImgL.fImgY);
	
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.fAnimCNT, myData.pImgL.fImgY);
	cairo_paint(myDrawContext);
	
	//Image précédante
	if (myData.pPrevCairoSurface != NULL) {
		cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myConfig.pBackgroundColor[3]);
		double fX = myData.fAnimCNT + myData.pImgL.fImgW + 10 + myData.pPrevImgL.fImgX, fY = myData.pPrevImgL.fImgY;
		cairo_rectangle (myDrawContext, fX, fY, myData.pPrevImgL.fImgW, myData.pPrevImgL.fImgH);
		cairo_fill (myDrawContext);
		
		cairo_set_source_surface (myDrawContext, myData.pPrevCairoSurface, fX, fY);
		cairo_paint(myDrawContext);
	}
  
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimCNT >= myData.pImgL.fImgX) {
		myData.iTimerID = g_timeout_add (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
		
	return TRUE;
}

gboolean cd_slider_grow_up (void) {
	myData.fAnimAlpha += 0.1;

	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	_cd_slider_add_frame_to_current_slide(); //Add background frame
	
	//On met a l'échelle en recentrant.
	cairo_translate (myDrawContext, (myIcon->fWidth - myData.pImgL.fImgW * myData.fAnimAlpha) / 2 * fMaxScale, (myIcon->fHeight - myData.pImgL.fImgH * myData.fAnimAlpha) / 2 * fMaxScale);
	cairo_scale(myDrawContext, myData.fAnimAlpha, myData.fAnimAlpha);
	
	//On empeche la transparence et on affiche l'image
	_cd_slider_add_background_to_current_slide (myData.pImgL.fImgX, myData.pImgL.fImgY);
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
	
	cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
	
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimAlpha >= .99) {
		myData.iTimerID = g_timeout_add (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_shrink_down (void) {
	myData.fAnimAlpha = myData.fAnimAlpha - 0.1;
	myData.fAnimCNT += 0.1;
	
	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	_cd_slider_add_frame_to_current_slide(); //Add background frame
	
	//On met a l'échelle en recentrant.
	cairo_translate (myDrawContext, (myIcon->fWidth - myData.pImgL.fImgW * myData.fAnimAlpha) / 2 * fMaxScale, (myIcon->fHeight - myData.pImgL.fImgH * myData.fAnimAlpha) / 2 * fMaxScale);
	cairo_scale(myDrawContext, myData.fAnimAlpha, myData.fAnimAlpha);
	
	//On empeche la transparence et on affiche l'image
	_cd_slider_add_background_to_current_slide (myData.pImgL.fImgX, myData.pImgL.fImgY);
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
	
	cairo_paint_with_alpha (myDrawContext, myData.fAnimCNT);
	

	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimAlpha <= .99) {
		myData.iTimerID = g_timeout_add (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}
