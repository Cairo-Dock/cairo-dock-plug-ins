/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "cairo-dock.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-slider.h"

CD_APPLET_INCLUDE_MY_VARS


static int _cd_slider_random_compare (gconstpointer a, gconstpointer b, GRand *pRandomGenerator) {
	return (g_rand_boolean (pRandomGenerator) ? 1 : -1);
}

void cd_slider_free_image (SliderImage *pImage) {
	if (pImage == NULL)
		return;
	g_free (pImage->cPath);
	g_free (pImage);
}

void cd_slider_free_images_list (GList *pList) {
	g_list_foreach (pList, (GFunc) cd_slider_free_image, NULL);
	g_list_free (pList);
}

static GList *cd_slider_measure_directory (GList *pList, gchar *cDirectory, gboolean bRecursive) {
	cd_debug ("%s (%s)", __func__, cDirectory);

	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirectory, 0, &erreur);
	if (erreur != NULL) {
		cd_warning ("Slider : %s", erreur->message);
		g_error_free (erreur);
		return pList;
	}
	
	struct stat buf;
	SliderImage *pImage;
	SliderImageFormat iFormat;
	const gchar *cFileName, *extension;
	GString *sFilePath = g_string_new ("");
	while ((cFileName = g_dir_read_name (dir)) != NULL) {
		g_string_printf (sFilePath, "%s/%s", cDirectory, cFileName);
		if (stat (sFilePath->str, &buf) != -1) {
			if (S_ISDIR (buf.st_mode)) {
				cd_debug ("%s is a directory, let's look", sFilePath->str);
				if (bRecursive)
					pList = cd_slider_measure_directory (pList, sFilePath->str, bRecursive);
			}
			else {
			  extension = strrchr(cFileName,'.');
				if (extension != NULL) {
					iFormat = SLIDER_UNKNOWN_FORMAT;  // le but du format serait de definir un seuil de taille pour chaque format a partir duquel l'image devrait etre chargee par un thread.
					if (g_ascii_strcasecmp(extension, ".png") == 0)
						iFormat = SLIDER_PNG;
					else if (g_ascii_strcasecmp(extension, ".jpg") == 0)
						iFormat = SLIDER_JPG;
					else if (g_ascii_strcasecmp(extension, ".svg") == 0)
						iFormat = SLIDER_SVG;
					else if (g_ascii_strcasecmp(extension, ".gif") == 0)
						iFormat = SLIDER_GIF;
					else if (g_ascii_strcasecmp(extension, ".xpm") == 0)
						iFormat = SLIDER_XPM;
					if (iFormat != SLIDER_UNKNOWN_FORMAT) {
						cd_debug ("Adding %s to list", cFileName);
						pImage = g_new (SliderImage, 1);
						pImage->cPath = g_strdup (sFilePath->str);
						pImage->iSize = buf.st_size;
						pImage->iFormat = iFormat;
						pList = g_list_prepend (pList, pImage);
					}
				}
			}
		}
	}
	
	g_string_free (sFilePath, TRUE);
	g_dir_close (dir);
	return pList;
}

void cd_slider_get_files_from_dir(CairoDockModuleInstance *myApplet) {
	if (myConfig.cDirectory == NULL) {
	  ///Et si on scannait le dossier image du home a la place? => bonne idee, mais comment trouver son nom ? il depend de la locale.
	  ///Il devrai y avoir une var d'environement qui le permet, je vais chercher laquelle, ou sinon c'est dans la config de gnome.
		cd_warning ("Slider : No directory to scan, halt.");
		return;
	}
	
	myData.pList = cd_slider_measure_directory (NULL, myConfig.cDirectory, myConfig.bSubDirs); //Nouveau scan
	
	if (myConfig.bRandom) {
		//cd_debug ("Mixing images ...");
		GRand *pRandomGenerator = g_rand_new ();
		myData.pList = g_list_sort_with_data (myData.pList, (GCompareDataFunc) _cd_slider_random_compare, pRandomGenerator);
		g_rand_free (pRandomGenerator);
	}
	else {
		myData.pList = g_list_reverse (myData.pList);
	}
}

void cd_slider_read_directory (CairoDockModuleInstance *myApplet) {
	cd_slider_get_files_from_dir (myApplet);
}

gboolean cd_slider_launch_slides (CairoDockModuleInstance *myApplet) {
	myData.pElement = myData.pList;
	cd_slider_draw_images(myApplet);
	return TRUE;
}



void cd_slider_read_image (CairoDockModuleInstance *myApplet) {
	//\___________________________ On sauvegarde la surface actuelle.
	cairo_surface_destroy (myData.pPrevCairoSurface);
	myData.pPrevCairoSurface = myData.pCairoSurface;
	myData.pPrevImgL = myData.pImgL;
	
	//\___________________________ On charge la nouvelle surface.
	SliderImage *pImage = myData.pElement->data;
	gchar *cImagePath = pImage->cPath;
	//cd_debug ("Displaying: %s (size %dbytes)", cImagePath, pImage->iSize);
	
	double fImgX, fImgY, fImgW=0, fImgH=0;
	CairoDockLoadImageModifier iLoadingModifier = CAIRO_DOCK_FILL_SPACE;
	if (! myConfig.bFillIcon)
		iLoadingModifier |= CAIRO_DOCK_DONT_ZOOM_IN;
	if (myConfig.bNoStretch)
		iLoadingModifier |= CAIRO_DOCK_KEEP_RATIO;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
	myData.pCairoSurface = cairo_dock_create_surface_from_image (cImagePath,
		pCairoContext,  // myDrawContext
		1.,
		myData.fSurfaceWidth, myData.fSurfaceHeight,
		iLoadingModifier,
		&fImgW, &fImgH,
		NULL, NULL);
	cairo_destroy (pCairoContext);
	
	fImgX = (myData.fSurfaceWidth - fImgW) / 2;
	fImgY = (myData.fSurfaceHeight - fImgH) / 2;
	myData.pImgL.fImgX = fImgX;
	myData.pImgL.fImgY = fImgY;
	myData.pImgL.fImgW = fImgW;
	myData.pImgL.fImgH = fImgH;
}

gboolean cd_slider_update_slide (CairoDockModuleInstance *myApplet) {
	
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	
	if (myConfig.iAnimation == SLIDER_RANDOM) {
		srand(time(NULL));
		myData.iAnimation = 1 + (rand() % (SLIDER_RANDOM-1)); //Skip the default animation (1+) /// pourquoi on saute celle par defaut ?
		//Elle bloque le slide, et je n'arrive pas a trouver pourquoi :/ et puis ca coupe les effets de voir une image arriver "Blip" sans rien => oki :-)
	}
	else {
		myData.iAnimation = myConfig.iAnimation ;
	}
	
	switch (myData.iAnimation) {
		case SLIDER_DEFAULT: default:
			//cd_debug("Displaying with default");
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
			//cd_debug("Displaying with fade");
			myData.fAnimAlpha = 0.;
			myData.fAnimCNT = 1.;
			if (myData.iAnimTimerID == 0)
				myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_fade, (gpointer) myApplet);
		break;
		case SLIDER_BLANK_FADE:
			//cd_debug("Displaying with blank fade");
			myData.fAnimAlpha = 1.;
			if (myData.iAnimTimerID == 0)
				myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_blank_fade, (gpointer) myApplet);
		break;
		case SLIDER_FADE_IN_OUT:
			//cd_debug("Displaying with fade in out");
			myData.iAnimCNT = 0;
			myData.fAnimAlpha = 0.;
			if (myData.iAnimTimerID == 0)
				myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_fade_in_out, (gpointer) myApplet);
		break;
		case SLIDER_SIDE_KICK:
			//cd_debug("Displaying with side kick");
			myData.fAnimCNT = -myData.pImgL.fImgW;
			if (myData.iAnimTimerID == 0)
				myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_side_kick, (gpointer) myApplet);
		break;
		case SLIDER_DIAPORAMA:
			//cd_debug("Displaying with diaporama");
			myData.fAnimCNT = -myData.pImgL.fImgW - 10;
			if (myData.iAnimTimerID == 0)
				myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_diaporama, (gpointer) myApplet);
		break;
		case SLIDER_GROW_UP:
			//cd_debug("Displaying with grow up");
			myData.fAnimAlpha = 0.;
			if (myData.iAnimTimerID == 0)
				myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_grow_up, (gpointer) myApplet);
		break;
		case SLIDER_SHRINK_DOWN:
			//cd_debug("Displaying with shrink down");
			myData.fAnimAlpha = 2.5;
			myData.fAnimCNT = .3;
			if (myData.iAnimTimerID == 0)
				myData.iAnimTimerID = g_timeout_add (50, (GSourceFunc) cd_slider_shrink_down, (gpointer) myApplet);
		break;
	}
	
	//\______________________ On passe a l'image suivante.
	myData.pElement = cairo_dock_get_next_element (myData.pElement, myData.pList);
	
	if (myConfig.iAnimation == SLIDER_DEFAULT)
	{
		myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) myApplet);
	}
	else
		myData.iTimerID = 0;
	
	return TRUE;
}

gboolean cd_slider_draw_images(CairoDockModuleInstance *myApplet) {
	if (myData.bPause == TRUE)
		return FALSE;
	
	//\___________________________ On recupere la nouvelle image a afficher.
	if (myData.pElement == NULL || myData.pElement->data == NULL) {
		cd_warning ("Slider stopped, list broken");
		return FALSE;
	}
	SliderImage *pImage = myData.pElement->data;
	cd_message (" >> %s", pImage->cPath);
	
	//\___________________________ On arrete l'animation precedente si elle n'etait pas finie (ne devrait pas arriver).
	if (myData.iAnimTimerID != 0) {
		cd_warning ("slider : previous animation didn't finish before the new one begins.");
		g_source_remove(myData.iAnimTimerID);
		myData.iAnimTimerID = 0;
	}
	
	//\___________________________ On charge la nouvelle et on lance l'animation de transition.
	if (myConfig.bUseThread && 
		((pImage->iFormat == SLIDER_SVG && pImage->iSize > 10e3) ||
		(pImage->iFormat == SLIDER_PNG && pImage->iSize > 100e3) ||
		(pImage->iFormat == SLIDER_JPG && pImage->iSize > 70e3) ||
		(pImage->iFormat == SLIDER_GIF && pImage->iSize > 100e3) ||
		(pImage->iFormat == SLIDER_XPM && pImage->iSize > 100e3))) {
		cd_debug ("  on threade");
		cairo_dock_launch_measure (myData.pMeasureImage);
	}
	else {
		cd_slider_read_image (myApplet);
		cd_slider_update_slide (myApplet);
	}
	
	return FALSE;  // on quitte la boucle des images car on va effectuer une animation.
}


static void _cd_slider_add_background_to_current_slide (CairoDockModuleInstance *myApplet, double fX, double fY) {
	cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myConfig.pBackgroundColor[3]);
	cairo_rectangle (myDrawContext, fX, fY, myData.pImgL.fImgW, myData.pImgL.fImgH);
	cairo_fill (myDrawContext);
}

gboolean cd_slider_fade (CairoDockModuleInstance *myApplet) {
	myData.fAnimAlpha = myData.fAnimAlpha +.1;
	myData.fAnimCNT = myData.fAnimCNT -.1;
	
	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	
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
		if (myData.iTimerID == 0)
			myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) myApplet);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_blank_fade (CairoDockModuleInstance *myApplet) {
	myData.fAnimAlpha = myData.fAnimAlpha -.1;
	
	//On efface le fond
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myApplet, myData.pImgL.fImgX, myData.pImgL.fImgY);
	
	//Image
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.pImgL.fImgX, myData.pImgL.fImgY);
	cairo_paint (myDrawContext);
	
	//Masque
	cairo_set_source_rgba (myDrawContext, 1., 1., 1., myData.fAnimAlpha);
	cairo_rectangle(myDrawContext, 0., 0., myData.fSurfaceWidth, myData.fSurfaceHeight);
	cairo_fill(myDrawContext);
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimAlpha <= 0.01) {
		if (myData.iTimerID == 0)
			myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) myApplet);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_fade_in_out (CairoDockModuleInstance *myApplet) {
	
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
		if (myData.iTimerID == 0)
			myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) myApplet);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_side_kick (CairoDockModuleInstance *myApplet) {
	
	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myApplet, myData.fAnimCNT, myData.pImgL.fImgY);
	
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.fAnimCNT, myData.pImgL.fImgY);
	cairo_paint (myDrawContext);
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	//Effet d'arrivee rapide, passage lent, sortie rapide comme un coup de pied
	if (myData.fAnimCNT >= (-myData.pImgL.fImgW / 2) && myData.fAnimCNT <= (myData.fSurfaceWidth / 2)) {
		myData.fAnimCNT = myData.fAnimCNT +1.;
	}
	else {
		myData.fAnimCNT = myData.fAnimCNT +5.;
	}
	
	if (myData.fAnimCNT >= myData.fSurfaceWidth +5) {
		if (myData.iTimerID == 0)
			myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) myApplet);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
		
	return TRUE;
}

gboolean cd_slider_diaporama (CairoDockModuleInstance *myApplet) {
	myData.fAnimCNT = myData.fAnimCNT +1;

	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	
	//On empeche la transparence
	_cd_slider_add_background_to_current_slide (myApplet, myData.fAnimCNT, myData.pImgL.fImgY);
	
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
		if (myData.iTimerID == 0)
			myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) myApplet);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_grow_up (CairoDockModuleInstance *myApplet) {
	myData.fAnimAlpha += 0.1;

	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	
	//On met a l'échelle en recentrant.
	cairo_translate (myDrawContext, (myData.fSurfaceWidth - myData.pImgL.fImgW * myData.fAnimAlpha) / 2, (myData.fSurfaceHeight - myData.pImgL.fImgH * myData.fAnimAlpha) / 2);
	cairo_scale(myDrawContext, myData.fAnimAlpha, myData.fAnimAlpha);
	
	//On empeche la transparence et on affiche l'image
	_cd_slider_add_background_to_current_slide (myApplet, 0., 0.);
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, 0., 0.);
	
	cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimAlpha >= .99) {
		if (myData.iTimerID == 0)
			myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) myApplet);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}

gboolean cd_slider_shrink_down (CairoDockModuleInstance *myApplet) {
	myData.fAnimAlpha = myData.fAnimAlpha - 0.1;
	myData.fAnimCNT += 0.1;
	
	//On efface le fond
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	cairo_save(myDrawContext);
	
	
	//On met a l'échelle en recentrant.
	cairo_translate (myDrawContext, (myData.fSurfaceWidth - myData.pImgL.fImgW * myData.fAnimAlpha) / 2, (myData.fSurfaceHeight - myData.pImgL.fImgH * myData.fAnimAlpha) / 2);
	cairo_scale(myDrawContext, myData.fAnimAlpha, myData.fAnimAlpha);
	
	//On empeche la transparence et on affiche l'image
	_cd_slider_add_background_to_current_slide (myApplet, 0., 0.);
	cairo_set_source_surface (myDrawContext, myData.pCairoSurface, 0., 0.);
	
	cairo_paint_with_alpha (myDrawContext, myData.fAnimCNT);
	
	CD_APPLET_REDRAW_MY_ICON
	cairo_restore(myDrawContext);
	
	if (myData.fAnimAlpha <= 1.01) {
		if (myData.iTimerID == 0)
			myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) myApplet);
		myData.iAnimTimerID = 0;
		return FALSE;
	}
	
	return TRUE;
}
