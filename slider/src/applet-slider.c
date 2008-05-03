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
		cd_message("Now searching in %s for images files", myConfig.cDirectory);
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
					cd_message("Adding %s to list", cFileName);
					pFilePath = g_strconcat (myConfig.cDirectory, "/", cFileName, NULL);
					myData.pList = g_list_prepend (myData.pList, pFilePath);
					myData.iImagesNumber++;
				}
				else {
					cd_message("%s not handeled, ignoring...", cFileName);
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
	
	if (myData.pElement == NULL || myData.pElement->data == NULL) {
		cd_warning ("Slider stopped, list broken");
		return FALSE;
 	}
	gchar *cImagePath = myData.pElement->data;
	cd_message("Displaying: %s\n", cImagePath);
	
	double fImgX, fImgY, fImgW=0, fImgH=0;
	myData.pCairoSurface = cairo_dock_create_surface_from_image (cImagePath, myDrawContext, cairo_dock_get_max_scale (myContainer), myIcon->fWidth, myIcon->fHeight, &fImgW, &fImgH, myConfig.bNoStrench);  // keep ratio.
  	
  fImgX = (myIcon->fWidth - fImgW) / 2;
  fImgY = (myIcon->fHeight - fImgH) / 2;

  cd_message("X Y: %.02f %.02f - Ratio W: %.02f - Ratio H: %.02f - W: %.02f - H: %.02f", fImgX, fImgY, myIcon->fWidth/ fImgW, myIcon->fHeight / fImgH, fImgW ,fImgH);
		
	myData.pImgL.fImgX = fImgX;
	myData.pImgL.fImgY = fImgY;
	myData.pImgL.fImgW = fImgW;
	myData.pImgL.fImgH = fImgH;
		
	if (myData.iAnimTimerID != 0) {
		g_source_remove(myData.iAnimTimerID);
		myData.iAnimTimerID = 0;
	}
		
	cairo_save (myDrawContext);
	//On efface le fond
	cairo_set_source_rgba (myDrawContext, 1., 1., 1., 0.);

	switch (myConfig.pAnimation) {
		case SLIDER_DEFAULT: default:
			cd_debug("Affichage par défaut");
			//On efface le fond
			cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
			cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
			cairo_paint (myDrawContext);
			cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
			
			//On empeche la transparence
			cairo_save (myDrawContext);
			cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myConfig.pBackgroundColor[3]);
			cairo_rectangle(myDrawContext, myData.pImgL.fImgX, myData.pImgL.fImgY, myData.pImgL.fImgW, myData.pImgL.fImgH);
			cairo_fill(myDrawContext);
			cairo_restore (myDrawContext);

			cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
			
			cairo_paint (myDrawContext);
			cairo_surface_destroy(myData.pCairoSurface);
 			//cairo_destroy (myDrawContext); //Pas de fuite mémoire
 			
 			myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
 			
 			CD_APPLET_REDRAW_MY_ICON
		break;
		case SLIDER_FADE:
			cd_debug("Affichage par fade");
			myData.fAnimAlpha = 0.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_fade, (gpointer) NULL);
		break;
		case SLIDER_BLANK_FADE:
			cd_debug("Affichage par blank fade");
			myData.fAnimAlpha = 1.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_blank_fade, (gpointer) NULL);
		break;
		case SLIDER_FADE_IN_OUT:
			cd_debug("Affichage par fade in out");
			myData.iAnimCNT = 0;
			myData.fAnimAlpha = 0.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_fade_in_out, (gpointer) NULL);
		break;
		case SLIDER_SIDE_KICK:
			cd_debug("Affichage par side kick");
			myData.fAnimCNT = -myData.pImgL.fImgW;
			myData.iAnimTimerID = g_timeout_add (70, (GSourceFunc) cd_slider_side_kick, (gpointer) NULL);
		break;
		case SLIDER_DIAPORAMA:
			cd_debug("Affichage par diaporama");
			myData.fAnimCNT = -myData.pImgL.fImgW - 10;
			myData.pPrevCairoSurface = cd_slider_get_previous_img_surface(myData.pList, myData.pElement);
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_diaporama, (gpointer) NULL);
		break;
		case SLIDER_GROW_UP:
			cd_debug("Affichage par grow up");
			myData.iAnimCNT = 0;
			myData.fAnimAlpha = 0.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_fade_in_out, (gpointer) NULL);
		break;
		case SLIDER_SHRINK_DOWN:
			cd_debug("Affichage par shrink down");
			myData.iAnimCNT = 0;
			myData.fAnimAlpha = 0.;
			myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_fade_in_out, (gpointer) NULL);
		break;
	}

	cairo_restore (myDrawContext);
	//cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer);
  myData.pElement = cairo_dock_get_next_element (myData.pElement, myData.pList);
  
  cd_message("Next Image: %s\n", myData.pElement->data);
  return FALSE;
}

static void _cd_slider_add_background_to_current_slide (double fX, double fY) {
	cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myConfig.pBackgroundColor[3]);
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	cairo_rectangle (myDrawContext, fX, fY, myData.pImgL.fImgW * fMaxScale, myData.pImgL.fImgH * fMaxScale);
	cairo_fill (myDrawContext);
}

gboolean cd_slider_fade (void) {
	myData.fAnimAlpha = myData.fAnimAlpha +.1;
	
	//On efface le fond
	cairo_set_operator (myData.pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myData.pCairoContext);
  cairo_set_operator (myData.pCairoContext, CAIRO_OPERATOR_OVER);
	
	//Image précédante
	if (myData.pPrevCairoSurface != NULL) {
		cairo_set_source_surface (myData.pCairoContext, myData.pPrevCairoSurface, myData.fAnimCNT + myData.pImgL.fImgW + 10 + myData.pImgL.fImgX, myData.pImgL.fImgY);
	}
	
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.pImgL.fImgX, myData.pImgL.fImgY);
	
	cairo_set_source_surface (myData.pCairoContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
	cairo_paint_with_alpha (myData.pCairoContext, myData.fAnimAlpha);
	
	CD_APPLET_REDRAW_MY_ICON
				
	if (myData.fAnimAlpha >= 1) {
		//cairo_surface_destroy(myData.pCairoSurface);
  	//cairo_destroy (myData.pCairoContext); //Pas de fuite mémoire
  	myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		return FALSE;
	}
		
	return TRUE;
}

gboolean cd_slider_blank_fade (void) {
	myData.fAnimAlpha = myData.fAnimAlpha -.1;
	
	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
  cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.pImgL.fImgX, myData.pImgL.fImgY);
	
	//Image
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
  cairo_paint (myDrawContext);
	
	//Masque
	cairo_set_source_rgba (myDrawContext, 1., 1., 1., myData.fAnimAlpha);
	cairo_rectangle(myDrawContext, 0, 0, myIcon->fWidth, myIcon->fHeight);
	cairo_fill(myDrawContext);
	
	cairo_paint (myDrawContext);
	CD_APPLET_REDRAW_MY_ICON
				
	if (myData.fAnimAlpha <= 0) {
		cairo_surface_destroy(myData.pCairoSurface);
  	//cairo_destroy (myDrawContext); //Pas de fuite mémoire
  	myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		return FALSE;
	}
		
	return TRUE;
}

gboolean cd_slider_fade_in_out (void) {
	
	if (myData.fAnimAlpha <= 1 && myData.iAnimCNT == 0) { //On augmente l'alpha
		myData.fAnimAlpha += .1;
	}
	if (myData.fAnimAlpha >= 1 &&myData.iAnimCNT <= 100) { //On attent 100/50 ms
		myData.iAnimCNT += 1;
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
		
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.pImgL.fImgX, myData.pImgL.fImgY);
	
	//Image
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
	cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
	
	CD_APPLET_REDRAW_MY_ICON
	
	if (myData.fAnimAlpha <= 0  && myData.iAnimCNT >= 1) { //On arrete l'animation
		cairo_surface_destroy(myData.pCairoSurface);
  	//cairo_destroy (myDrawContext); //Pas de fuite mémoire
  	myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_side_kick (void) {
	
	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
  cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
		
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.fAnimCNT, myData.pImgL.fImgY);
	
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.fAnimCNT, myData.pImgL.fImgY);
  cairo_paint (myDrawContext);
  
	CD_APPLET_REDRAW_MY_ICON
	
	//Effet d'arrivee rapide, passage lent, sortie rapide comme un coup de pied
	if (myData.fAnimCNT >= (-myData.pImgL.fImgW / 2) && myData.fAnimCNT <= (myIcon->fWidth / 2)) {
		myData.fAnimCNT = myData.fAnimCNT +.5;
	}
	else {
		myData.fAnimCNT = myData.fAnimCNT +5.;
	}
	
	if (myData.fAnimCNT >= myIcon->fWidth+5) {
		cairo_surface_destroy(myData.pCairoSurface);
  	//cairo_destroy (myDrawContext); //Pas de fuite mémoire
  	myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		return FALSE;
	}
		
	return TRUE;
}

gboolean cd_slider_diaporama (void) {
	myData.fAnimCNT = myData.fAnimCNT +.5;

	//On efface le fond
	cairo_set_operator (myData.pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myData.pCairoContext);
  cairo_set_operator (myData.pCairoContext, CAIRO_OPERATOR_OVER);
		
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.pImgL.fImgX, myData.pImgL.fImgY);
	
	cairo_set_source_surface (myData.pCairoContext, myData.pCairoSurface, myData.fAnimCNT, myData.pImgL.fImgY);
	cairo_paint(myData.pCairoContext);
	
	//Image précédante
	if (myData.pPrevCairoSurface != NULL) {
		cairo_set_source_surface (myData.pCairoContext, myData.pPrevCairoSurface, myData.fAnimCNT + myData.pImgL.fImgW + 10 + myData.pImgL.fImgX, myData.pImgL.fImgY);
	}
	
  cairo_paint (myData.pCairoContext);
  
	CD_APPLET_REDRAW_MY_ICON

	if (myData.fAnimCNT >= myData.pImgL.fImgX) {
		cairo_surface_destroy(myData.pCairoSurface);
  	cairo_destroy (myData.pCairoContext); //Pas de fuite mémoire
  	myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		return FALSE;
	}
		
	return TRUE;
}

gboolean cd_slider_grow_up (void) {
	myData.fAnimAlpha = myData.fAnimAlpha - 0.1;

	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
  cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);

	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.fAnimCNT, myData.pImgL.fImgY);
	
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.fAnimCNT, myData.pImgL.fImgY);
  cairo_paint (myDrawContext);

	if (myData.fAnimAlpha <= 0) {
		cairo_surface_destroy(myData.pCairoSurface);
  	//cairo_destroy (myDrawContext); //Pas de fuite mémoire
  	myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		return FALSE;
	}
	return TRUE;
}

gboolean cd_slider_shrink_down (void) {
	myData.fAnimAlpha = myData.fAnimAlpha - 0.1;

	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
  cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);

	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myData.fAnimCNT, myData.pImgL.fImgY);
	
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.fAnimCNT, myData.pImgL.fImgY);
  cairo_paint (myDrawContext);

	if (myData.fAnimAlpha <= 0) {
		cairo_surface_destroy(myData.pCairoSurface);
  	//cairo_destroy (myDrawContext); //Pas de fuite mémoire
  	myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
		return FALSE;
	}
	return TRUE;
}

GList* cd_slider_get_previous_img(GList *pList, GList *pImg) {
	GList *pPrevious=NULL, *pElement=pList;
  while (pElement != NULL) {
		if (strcmp(pElement->data, pImg->data) == 0)
    	break;
    	
    pPrevious = pElement;
    pElement = pElement->next;
  }
  
	return pPrevious;
}

cairo_surface_t* cd_slider_get_previous_img_surface(GList *pList, GList *pImg) {
	GList *pElement = cd_slider_get_previous_img(pList, pImg);
	
	if (pElement == NULL)
		return NULL;
	
	gchar *pValue = pElement->data;
	
 	cairo_surface_t* surface;
 	cairo_t* context;
	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1400, 900); //1400x900 pour que l'image se charge dans sa totalité
 	context = cairo_create (surface);
 	
 	double fImgX, fImgY, fImgW, fImgH;
 	surface = cairo_dock_create_surface_from_image(pValue, context, 1., NULL, NULL, &fImgW, &fImgH, TRUE);
 	cd_debug("Image width: %.02f height: %.02f", fImgW, fImgH);
 	cairo_surface_destroy(surface);
 	cairo_destroy (context);
 	
 	context = cairo_create (myIcon->pIconBuffer);
 	if (fImgW < fImgH) { //H dominant: Portrait, il faut calculer le ratio imgH/iconH et l'utiliser sur W
 		if (myIcon->fHeight < fImgH) { //On réduit H a celle de l'icône et on scale
 			fImgW = (double) (myIcon->fHeight / fImgH) * fImgW;
 			fImgH = myIcon->fHeight;
 		}
 	}
 	else { //W dominant: Paysage, il faut calculer le ratio imgW/iconW et l'utiliser sur H
 		if (myIcon->fWidth < fImgW) { //On réduit W a celle de l'icône et on scale
 			fImgH = (double) (myIcon->fWidth/ fImgW) * fImgH;
 			fImgW = myIcon->fWidth;
 		}
 	}
 	
 	surface = cairo_dock_create_surface_from_image(pValue, context, 1., fImgW, fImgH, &fImgW, &fImgH, TRUE);
 	
	myData.pPrevImgL.fImgW = fImgW;
	myData.pPrevImgL.fImgH = fImgH;
	
	return surface;
}

//Sert au débug, a retirer pour un vrai release
void _printList(GList *pList) {
  GList *pElement;
  gchar *pValue;
  for (pElement = pList; pElement != NULL; pElement = pElement->next) {
    pValue = pElement->data;
    cd_message("Listed: %s\n", pValue);
  }
}

GList* _slider_random_image(void) {
  GList *pElement=NULL;
  srand(time(NULL));
  int i=0, j = rand() % myData.iImagesNumber;
  for (pElement = myData.pList; pElement != NULL; pElement = pElement->next) {
  	if (i == j) {
  		return pElement;
		}
		i++;
  }
}

void _slider_free_list(GList *pList) {
	if (pList != NULL) {
  	g_list_foreach (pList, (GFunc) g_free, NULL);
  	g_list_free (pList);
  	pList = NULL;
  }
}
