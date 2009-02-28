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
#include <math.h>

#ifdef HAVE_EXIF
#include <libexif/exif-data.h>
#endif

#include "cairo-dock.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-slider.h"

#define cd_slider_schedule_next_slide(myApplet) \
	if (myData.iTimerID == 0) myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_next_slide, (gpointer) myApplet);

#define cd_slider_next_slide_is_scheduled(myApplet) (myData.iTimerID != 0)

#define _cd_slider_erase_surface(myApplet) do { \
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);\
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);\
	cairo_paint (myDrawContext);\
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER); } while (0)

#define _cd_slider_add_background_to_slide(myApplet, fX, fY, alpha, slide) do { \
	if (myConfig.pBackgroundColor[3] != 0) {\
	cairo_set_source_rgba (myDrawContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], alpha * myConfig.pBackgroundColor[3]);\
	cairo_rectangle (myDrawContext, fX, fY, slide.fImgW, slide.fImgH);\
	cairo_fill (myDrawContext); } } while (0)
#define _cd_slider_add_background_to_current_slide(myApplet, fX, fY, alpha) _cd_slider_add_background_to_slide (myApplet, fX, fY, alpha, myData.slideArea)
#define _cd_slider_add_background_to_prev_slide(myApplet, fX, fY, alpha) _cd_slider_add_background_to_slide (myApplet, fX, fY, alpha, myData.prevSlideArea)

#define _cd_slider_add_background_to_slide_opengl(myApplet, fX, fY, alpha, slide) do { \
	glColor4f (myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], alpha * myConfig.pBackgroundColor[3]);\
	glPolygonMode (GL_FRONT, GL_FILL);\
	glEnable (GL_BLEND);\
	glBlendFunc (GL_ONE, GL_ZERO);\
	if (myConfig.pBackgroundColor[3] != 0) {\
		glBegin(GL_QUADS);\
		glVertex3f(fX - slide.fImgW/2, fY - slide.fImgH/2, 0.);\
		glVertex3f(fX + slide.fImgW/2, fY - slide.fImgH/2, 0.);\
		glVertex3f(fX + slide.fImgW/2, fY + slide.fImgH/2, 0.);\
		glVertex3f(fX - slide.fImgW/2, fY + slide.fImgH/2, 0.);\
		glEnd(); } } while (0)
#define _cd_slider_add_background_to_current_slide_opengl(myApplet, fX, fY, alpha) _cd_slider_add_background_to_slide_opengl (myApplet, fX, fY, alpha, myData.slideArea)
#define _cd_slider_add_background_to_prev_slide_opengl(myApplet, fX, fY, alpha) _cd_slider_add_background_to_slide_opengl (myApplet, fX, fY, alpha, myData.prevSlideArea)

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

static int _compare_images_order (SliderImage *image2, SliderImage *image1) {
	if (image1->cPath == NULL)
		return -1;
	if (image2->cPath == NULL)
		return 1;
	gchar *cURI_1 = g_ascii_strdown (image1->cPath, -1);
	gchar *cURI_2 = g_ascii_strdown (image2->cPath, -1);
	int iOrder = strcmp (cURI_1, cURI_2);
	g_free (cURI_1);
	g_free (cURI_2);
	return iOrder;
}
static GList *cd_slider_measure_directory (GList *pList, gchar *cDirectory, gboolean bRecursive, gboolean bSortAlpha) {
#ifdef HAVE_EXIF
	static gchar ebuf[1024];
	ExifData *pExifData;
	ExifEntry *pExifEntry;
	ExifByteOrder byteOrder;
#endif
	cd_debug ("%s (%s)", __func__, cDirectory);

	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirectory, 0, &erreur);
	if (erreur != NULL) {
		cd_warning ("Slider : %s", erreur->message);
		g_error_free (erreur);
		return pList;
	}
#ifdef HAVE_EXIF
	memset (ebuf, 0, 1024);
#endif
	struct stat buf;
	SliderImage *pImage;
	SliderImageFormat iFormat;
	const gchar *cFileName, *extension;
	GString *sFilePath = g_string_new ("");
	while ((cFileName = g_dir_read_name (dir)) != NULL) {
		g_string_printf (sFilePath, "%s/%s", cDirectory, cFileName);
		if (stat (sFilePath->str, &buf) != -1) {
			if (S_ISDIR (buf.st_mode) && bRecursive) {
				cd_debug ("Slider - %s is a directory, let's look", sFilePath->str);
				pList = cd_slider_measure_directory (pList, sFilePath->str, bRecursive, bSortAlpha);
			}
			else {
				extension = strrchr(cFileName,'.');
				if (extension != NULL) {
					iFormat = SLIDER_UNKNOWN_FORMAT;  // le but du format serait de definir un seuil de taille pour chaque format a partir duquel l'image devrait etre chargee par un thread.
					if (g_ascii_strcasecmp(extension, ".png") == 0)
						iFormat = SLIDER_PNG;
					else if (g_ascii_strcasecmp(extension, ".jpg") == 0 || g_ascii_strcasecmp(extension, ".jpeg") == 0)
						iFormat = SLIDER_JPG;
					else if (g_ascii_strcasecmp(extension, ".svg") == 0)
						iFormat = SLIDER_SVG;
					else if (g_ascii_strcasecmp(extension, ".gif") == 0)
						iFormat = SLIDER_GIF;
					else if (g_ascii_strcasecmp(extension, ".xpm") == 0)
						iFormat = SLIDER_XPM;
					
					if (iFormat != SLIDER_UNKNOWN_FORMAT) {
						cd_debug ("Slider - Adding %s to list", cFileName);
						pImage = g_new (SliderImage, 1);
						pImage->cPath = g_strdup (sFilePath->str);
						pImage->iSize = buf.st_size;
						pImage->iFormat = iFormat;
						pImage->iOrientation = 0;
						#ifdef HAVE_EXIF
						if (iFormat == SLIDER_JPG)
						{
							pExifData = exif_data_new_from_file (sFilePath->str);
							
							pExifEntry = exif_data_get_entry (pExifData, EXIF_TAG_ORIENTATION);
							if (pExifEntry != NULL)
							{
								byteOrder = exif_data_get_byte_order (pExifData);
								pImage->iOrientation = exif_get_short (pExifEntry->data, byteOrder);
								g_print ("iOrientation : %d\n", pImage->iOrientation);
							}
							
							exif_data_unref (pExifData);
						}
						#endif
						if (bSortAlpha)  // ordre alphabetique.
							pList = g_list_insert_sorted (pList, pImage, (GCompareFunc) _compare_images_order);
						else  // on randomise a la fin.
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
	
	myData.pList = cd_slider_measure_directory (NULL, myConfig.cDirectory, myConfig.bSubDirs, ! myConfig.bRandom); //Nouveau scan
	
	if (myConfig.bRandom) {
		//cd_debug ("Slider - Mixing images ...");
		GRand *pRandomGenerator = g_rand_new ();
		myData.pList = g_list_sort_with_data (myData.pList, (GCompareDataFunc) _cd_slider_random_compare, pRandomGenerator);
		g_rand_free (pRandomGenerator);
	}
}


void cd_slider_read_image (CairoDockModuleInstance *myApplet) {
	SliderImage *pImage = myData.pElement->data;
	gchar *cImagePath = pImage->cPath;
	cd_debug ("Slider - Displaying: %s (size %dbytes, orientation:%d)", cImagePath, pImage->iSize, pImage->iOrientation);
	
	//\_______________ On definit comment charger l'image.
	double fImgX, fImgY, fImgW=0, fImgH=0;
	CairoDockLoadImageModifier iLoadingModifier = CAIRO_DOCK_FILL_SPACE;
	if (pImage->iOrientation != 0)
		iLoadingModifier |= ((pImage->iOrientation-1) << 3);
	if (! myConfig.bFillIcon)
		iLoadingModifier |= CAIRO_DOCK_DONT_ZOOM_IN;
	if (myConfig.bNoStretch)
		iLoadingModifier |= CAIRO_DOCK_KEEP_RATIO;
	
	//\_______________ On cree la surface cairo.
	cairo_t *pCairoContext = cairo_dock_create_context_from_container (myContainer);
	myData.pCairoSurface = cairo_dock_create_surface_from_image (cImagePath,
		pCairoContext,  // myDrawContext
		1.,
		myData.iSurfaceWidth, myData.iSurfaceHeight,
		iLoadingModifier,
		&fImgW, &fImgH,
		NULL, NULL);
	cairo_destroy (pCairoContext);
	
	//\_______________ On cree la texture.
	if (g_bUseOpenGL)
	{
		myData.iTexture = cairo_dock_create_texture_from_surface (myData.pCairoSurface);
	}
	
	//\_______________ On garde l'aire de la surface/texture.
	fImgX = (myData.iSurfaceWidth - fImgW) / 2;
	fImgY = (myData.iSurfaceHeight - fImgH) / 2;
	myData.slideArea.fImgX = fImgX;
	myData.slideArea.fImgY = fImgY;
	myData.slideArea.fImgW = fImgW;
	myData.slideArea.fImgH = fImgH;
}


gboolean cd_slider_update_transition (CairoDockModuleInstance *myApplet) {
	//\______________________ On choisit la transition.
	if (myConfig.iAnimation == SLIDER_RANDOM) {
		srand(time(NULL));
		myData.iAnimation = 1 + (rand() % (SLIDER_RANDOM-1)); //Skip the default animation (1+)
	}
	else {
		myData.iAnimation = myConfig.iAnimation;
	}
	
	//\______________________ On initialise la transition.
	myData.iAnimCNT = 0;
	myData.sens = 1;
	
	//\______________________ On lance la transition.
	if (myConfig.iAnimation != SLIDER_DEFAULT)  // on lance l'animation.
	{
		cairo_dock_launch_animation (myContainer);
	}
	else  // on dessine tout de suite et on attend l'image suivante.
	{
		cd_slider_draw_default (myApplet);
		CD_APPLET_REDRAW_MY_ICON;
		cd_slider_schedule_next_slide (myApplet);
	}
	
	return TRUE;
}

gboolean cd_slider_next_slide (CairoDockModuleInstance *myApplet) {
	if (myData.bPause)  // on est en pause.
	{
		myData.iTimerID = 0;
		return FALSE;
	}
	//\___________________________ On recupere la nouvelle image a afficher.
	if (myData.pElement == NULL)  // debut
		myData.pElement = myData.pList;
	else
		myData.pElement = cairo_dock_get_next_element (myData.pElement, myData.pList);
	
	if (myData.pElement == NULL || myData.pElement->data == NULL) {
		
		cd_warning ("Slider stopped, list broken");
		myData.iTimerID = 0;
		return FALSE;
	}
	SliderImage *pImage = myData.pElement->data;
	cd_message (" >> %s", pImage->cPath);
	
	//\___________________________ On sauvegarde la surface actuelle.
	if (myData.pPrevCairoSurface != NULL && myData.pPrevCairoSurface != myData.pCairoSurface)
		cairo_surface_destroy (myData.pPrevCairoSurface);
	myData.pPrevCairoSurface = myData.pCairoSurface;
	
	if (myData.iPrevTexture != 0 && myData.iPrevTexture != myData.iTexture)
		glDeleteTextures (1, &myData.iPrevTexture);
	myData.iPrevTexture = myData.iTexture;
	
	myData.prevSlideArea = myData.slideArea;
	
	//\___________________________ On charge la nouvelle surface/texture et on lance l'animation de transition.
	if (myConfig.bUseThread && 
		((pImage->iFormat == SLIDER_SVG && pImage->iSize > 10e3) ||
		(pImage->iFormat == SLIDER_PNG && pImage->iSize > 100e3) ||
		(pImage->iFormat == SLIDER_JPG && pImage->iSize > 70e3) ||
		(pImage->iFormat == SLIDER_GIF && pImage->iSize > 100e3) ||
		(pImage->iFormat == SLIDER_XPM && pImage->iSize > 100e3))) {
		cd_debug ("Slider -   on threade");
		cairo_dock_launch_measure (myData.pMeasureImage);
	}
	else {
		cd_slider_read_image (myApplet);
		cd_slider_update_transition (myApplet);
	}
	
	if (myConfig.bImageName && myDesklet) {
		gchar *cFileName = g_strdup (pImage->cPath);
		gchar *strFileWithExtension = strrchr (cFileName, '/');
		strFileWithExtension++;
		gchar *strFileWithNoExtension = strrchr (strFileWithExtension, '.');
		*strFileWithNoExtension = '\0';
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (strFileWithExtension);
		//cd_debug ("Slider - Image path: %s", pImage->cPath);
		g_free (cFileName);
	}
	
	if (myConfig.iAnimation == SLIDER_DEFAULT)
	{
		return TRUE;  // pas d'animation => on ne quitte pas la boucle d'attente.
	}
	else
	{
		myData.iTimerID = 0;
		return FALSE;  // on quitte la boucle des images car on va effectuer une animation.
	}
}


void cd_slider_draw_default (CairoDockModuleInstance *myApplet)
{
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return ;
		
		_cd_slider_add_background_to_current_slide_opengl (myApplet, 0., 0., 1.);
		
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable (GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
		glColor4f (1., 1., 1., 1.);
		cairo_dock_apply_texture_at_size (myData.iTexture, myData.slideArea.fImgW, myData.slideArea.fImgH);
		
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		//\______________________ On efface le fond
		_cd_slider_erase_surface (myApplet);
		
		//\______________________ On empeche la transparence
		_cd_slider_add_background_to_current_slide (myApplet, myData.slideArea.fImgX, myData.slideArea.fImgY, 1.);
		
		//\______________________ On dessine la nouvelle surface.
		cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.slideArea.fImgX, myData.slideArea.fImgY);
		cairo_paint (myDrawContext);
	}
}

gboolean cd_slider_fade (CairoDockModuleInstance *myApplet) {
	myData.iAnimCNT ++;
	myData.fAnimAlpha = 1.*myData.iAnimCNT / myConfig.iNbAnimationStep;
	if (myData.fAnimAlpha > 1)
		myData.fAnimAlpha = 1;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return FALSE;
		
		//Fond précédent.
		if (myData.iPrevTexture != 0)
			_cd_slider_add_background_to_prev_slide_opengl (myApplet, 0., 0., 1 - myData.fAnimAlpha);
		
		//On empeche la transparence.
		_cd_slider_add_background_to_current_slide_opengl (myApplet, 0., 0., myData.fAnimAlpha);
		
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable (GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
		double fX, fY;
		
		//Image précédente
		if (myData.iPrevTexture != 0)
		{
			glColor4f (1., 1., 1., 1 - myData.fAnimAlpha);
			glPushMatrix ();
			cairo_dock_apply_texture_at_size (myData.iPrevTexture, myData.prevSlideArea.fImgW, myData.prevSlideArea.fImgH);
			glPopMatrix ();
		}
		
		glColor4f (1., 1., 1., myData.fAnimAlpha);
		cairo_dock_apply_texture_at_size (myData.iTexture, myData.slideArea.fImgW, myData.slideArea.fImgH);
		
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		//On efface le fond
		_cd_slider_erase_surface (myApplet);
		
		//Fond précédent.
		if (myData.pPrevCairoSurface != NULL)
			_cd_slider_add_background_to_prev_slide (myApplet, myData.prevSlideArea.fImgX, myData.prevSlideArea.fImgY, 1 - myData.fAnimAlpha);
		
		//On empeche la transparence.
		_cd_slider_add_background_to_current_slide (myApplet, myData.slideArea.fImgX, myData.slideArea.fImgY, myData.fAnimAlpha);
		
		//Image précédente
		if (myData.pPrevCairoSurface != NULL) {
			cairo_set_source_surface (myDrawContext, myData.pPrevCairoSurface, myData.prevSlideArea.fImgX, myData.prevSlideArea.fImgY);
			cairo_paint_with_alpha (myDrawContext, 1 - myData.fAnimAlpha);
		}
		
		cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.slideArea.fImgX, myData.slideArea.fImgY);
		cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
	}
	
	return (myData.fAnimAlpha < .99);
}

gboolean cd_slider_blank_fade (CairoDockModuleInstance *myApplet) {
	myData.iAnimCNT ++;
	myData.fAnimAlpha = 1 - 1.*myData.iAnimCNT / myConfig.iNbAnimationStep;
	if (myData.fAnimAlpha < 0)
		myData.fAnimAlpha = 0;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return FALSE;
		
		//On empeche la transparence
		_cd_slider_add_background_to_current_slide_opengl (myApplet, 0., 0., 1.);
		
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable (GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
		//Image
		glColor4f (1., 1., 1., 1.);
		cairo_dock_apply_texture_at_size (myData.iTexture, myData.slideArea.fImgW, myData.slideArea.fImgH);
		
		//Masque
		glColor4f (1., 1., 1., myData.fAnimAlpha);
		glBegin(GL_QUADS);
		glDisable (GL_TEXTURE_2D);
		glVertex3f(-.5,  .5, 0.);
		glVertex3f( .5,  .5, 0.);
		glVertex3f( .5, -.5, 0.);
		glVertex3f(-.5, -.5, 0.);
		glEnd();
		
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		//On efface le fond
		_cd_slider_erase_surface (myApplet);
		
		//On empeche la transparence
		_cd_slider_add_background_to_current_slide (myApplet, myData.slideArea.fImgX, myData.slideArea.fImgY, 1.);
		
		//Image
		cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.slideArea.fImgX, myData.slideArea.fImgY);
		cairo_paint (myDrawContext);
		
		//Masque
		cairo_set_source_rgba (myDrawContext, 1., 1., 1., myData.fAnimAlpha);
		cairo_rectangle(myDrawContext, 0., 0., myData.iSurfaceWidth, myData.iSurfaceHeight);
		cairo_fill(myDrawContext);
	}
	
	return (myData.fAnimAlpha > 0.01);
}

gboolean cd_slider_fade_in_out (CairoDockModuleInstance *myApplet) {
	myData.iAnimCNT ++;
	if (myData.iAnimCNT <= myConfig.iNbAnimationStep)  // courbe de alpha : \__/
		myData.fAnimAlpha = 1. * (myConfig.iNbAnimationStep - myData.iAnimCNT) / myConfig.iNbAnimationStep;
	else if (myData.iAnimCNT <= 1.5 * myConfig.iNbAnimationStep)
	{
		return TRUE;  // on ne fait rien, texture inchangee.
	}
	else
		myData.fAnimAlpha = 1. * (myData.iAnimCNT - 1.5 * myConfig.iNbAnimationStep) / myConfig.iNbAnimationStep;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return FALSE;
		
		if (myData.iAnimCNT < myConfig.iNbAnimationStep)  // image precedente en train de disparaitre
		{
			//On empeche la transparence
			_cd_slider_add_background_to_prev_slide_opengl (myApplet, 0., 0., myData.fAnimAlpha);
			
			//Image
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4f (1., 1., 1., myData.fAnimAlpha);
			cairo_dock_apply_texture_at_size (myData.iPrevTexture, myData.prevSlideArea.fImgW, myData.prevSlideArea.fImgH);
		}
		else if (myData.iAnimCNT > myConfig.iNbAnimationStep) // image courante en train d'apparaitre.
		{
			//On empeche la transparence
			_cd_slider_add_background_to_current_slide_opengl (myApplet, 0., 0., myData.fAnimAlpha);
			
			//Image
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4f (1., 1., 1., myData.fAnimAlpha);
			cairo_dock_apply_texture_at_size (myData.iTexture, myData.slideArea.fImgW, myData.slideArea.fImgH);
		}
		
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		//On efface le fond
		_cd_slider_erase_surface (myApplet);
		
		if (myData.iAnimCNT < myConfig.iNbAnimationStep)  // image precedente en train de disparaitre
		{
			//On empeche la transparence
			_cd_slider_add_background_to_prev_slide (myApplet, myData.prevSlideArea.fImgX, myData.prevSlideArea.fImgY, myData.fAnimAlpha);
			//Image
			cairo_set_source_surface (myDrawContext, myData.pPrevCairoSurface, myData.prevSlideArea.fImgX, myData.prevSlideArea.fImgY);
		}
		else if (myData.iAnimCNT > myConfig.iNbAnimationStep) // image courante en train d'apparaitre.
		{
			//On empeche la transparence
			_cd_slider_add_background_to_current_slide (myApplet, myData.slideArea.fImgX, myData.slideArea.fImgY, myData.fAnimAlpha);
			//Image
			cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.slideArea.fImgX, myData.slideArea.fImgY);
		}
		cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
	}
	
	return (myData.fAnimAlpha < .99);
}

gboolean cd_slider_side_kick (CairoDockModuleInstance *myApplet) {
	myData.iAnimCNT += myData.sens;
	int xcumul = myData.iAnimCNT * (myData.iAnimCNT + 1) / 2;
	xcumul *= (10./myConfig.iNbAnimationStep);  /// au pif, a calculer ...
	if (xcumul > myData.iSurfaceWidth)  // en fait il faudrait regarder x > (iSurfaceWidth + fImgW)/2, mais comme ca on se prend pas la tete avec la difference de fImgW et fprevImgW, et ca laisse une petite tampo pendant laquelle l'icone est vide, c'est bien.
		myData.sens = -1;  // donc le coup d'apres on sera a nouveau en-dessous du seuil.
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return FALSE;
		
		if (myData.sens == 1)  // image precedente qui part sur la gauche.
		{
			_cd_slider_add_background_to_prev_slide_opengl (myApplet, - xcumul, 0., 1.);
			
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4f (1., 1., 1., 1.);
			
			glTranslatef (- xcumul, 0., 0.);
			cairo_dock_apply_texture_at_size (myData.iPrevTexture, myData.prevSlideArea.fImgW, myData.prevSlideArea.fImgH);
		}
		else  // image courante qui vient de la droite.
		{
			_cd_slider_add_background_to_current_slide_opengl (myApplet, xcumul, 0., 1.);
			
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4f (1., 1., 1., 1.);
			
			glTranslatef (xcumul, 0., 0.);
			cairo_dock_apply_texture_at_size (myData.iTexture, myData.slideArea.fImgW, myData.slideArea.fImgH);
		}
		
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		//On efface le fond
		_cd_slider_erase_surface (myApplet);
		
		if (myData.sens == 1)  // image precedente qui part sur la gauche.
		{
			_cd_slider_add_background_to_prev_slide (myApplet, myData.prevSlideArea.fImgX - xcumul, myData.prevSlideArea.fImgY, 1.);
			
			cairo_set_source_surface (myDrawContext, myData.pPrevCairoSurface, myData.prevSlideArea.fImgX - xcumul, myData.prevSlideArea.fImgY);
		}
		else  // image courante qui vient de la droite.
		{
			_cd_slider_add_background_to_current_slide (myApplet, myData.slideArea.fImgX + xcumul, myData.slideArea.fImgY, 1.);
			
			cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.slideArea.fImgX + xcumul, myData.slideArea.fImgY);
		}
		cairo_paint (myDrawContext);
	}
	
	return (myData.iAnimCNT > 0);
}

gboolean cd_slider_diaporama (CairoDockModuleInstance *myApplet) {
	static double a = .75;
	myData.iAnimCNT ++;
	myData.fAnimAlpha = 1.*myData.iAnimCNT / myConfig.iNbAnimationStep;
	if (myData.fAnimAlpha > 1)
		myData.fAnimAlpha = 1;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return FALSE;
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, 1.0*(GLfloat)myData.iSurfaceWidth/(GLfloat)myData.iSurfaceHeight, 1., 4*myData.iSurfaceHeight);
		glMatrixMode (GL_MODELVIEW);
		
		glLoadIdentity ();
		gluLookAt (myData.iSurfaceWidth/2, myData.iSurfaceHeight/2, 3.,
			myData.iSurfaceWidth/2, myData.iSurfaceHeight/2, 0.,
			0.0f, 1.0f, 0.0f);
		glTranslatef (0.0f, 0.0f, -3);
		glTranslatef (myData.iSurfaceWidth/2, myData.iSurfaceHeight/2, -myData.iSurfaceHeight*(sqrt(3)/2));
		glScalef (1., -1., 1.);
		
		if (myData.iPrevTexture != 0 && myData.fAnimAlpha < a)
		{
			glPushMatrix ();
			
			glTranslatef (-myData.iSurfaceWidth/2, 0., 0.);
			glRotatef (120. * (myData.fAnimAlpha/a), 0., 1., 0.);
			glTranslatef (myData.iSurfaceWidth/2, 0., 0.);
			
			//On empeche la transparence
			_cd_slider_add_background_to_prev_slide_opengl (myApplet, 0., 0., 1.);
			
			//Image
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4f (1., 1., 1., 1.);
			cairo_dock_apply_texture_at_size (myData.iPrevTexture, myData.prevSlideArea.fImgW, myData.prevSlideArea.fImgH);
			
			glPopMatrix ();
		}
		
		if (myData.fAnimAlpha > 1-a)
		{
			glTranslatef (myData.iSurfaceWidth/2, 0., 0.);
			glRotatef (-120. * (1-myData.fAnimAlpha)/a, 0., 1., 0.);
			glTranslatef (-myData.iSurfaceWidth/2, 0., 0.);
			
			//On empeche la transparence
			_cd_slider_add_background_to_current_slide_opengl (myApplet, 0., 0., 1.);
			
			//Image
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4f (1., 1., 1., 1.);
			cairo_dock_apply_texture_at_size (myData.iTexture, myData.slideArea.fImgW, myData.slideArea.fImgH);
		}
		
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		//On efface le fond
		_cd_slider_erase_surface (myApplet);
		
		//Image précédante
		if (myData.pPrevCairoSurface != NULL)
		{
			_cd_slider_add_background_to_prev_slide (myApplet, myData.prevSlideArea.fImgX - myData.fAnimAlpha * myData.iSurfaceWidth, myData.prevSlideArea.fImgY, 1.);
			
			cairo_set_source_surface (myDrawContext, myData.pPrevCairoSurface, myData.prevSlideArea.fImgX - myData.fAnimAlpha * myData.iSurfaceWidth, myData.prevSlideArea.fImgY);
			cairo_paint(myDrawContext);
		}
		
		//Image courante.
		_cd_slider_add_background_to_current_slide (myApplet, myData.slideArea.fImgX + myData.iSurfaceWidth * (1 - myData.fAnimAlpha), myData.slideArea.fImgY, 1.);
		
		cairo_set_source_surface (myDrawContext, myData.pCairoSurface, myData.slideArea.fImgX + myData.iSurfaceWidth * (1 - myData.fAnimAlpha), myData.slideArea.fImgY);
		cairo_paint(myDrawContext);
	}
	
	return (myData.fAnimAlpha < .99);
}

gboolean cd_slider_grow_up (CairoDockModuleInstance *myApplet) {
	myData.iAnimCNT ++;
	myData.fAnimAlpha = 1.*myData.iAnimCNT / myConfig.iNbAnimationStep;
	if (myData.fAnimAlpha > 1)
		myData.fAnimAlpha = 1;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return FALSE;
		
		_cd_slider_add_background_to_current_slide_opengl (myApplet, 0., 0., myData.fAnimAlpha);
		
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable (GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
		glColor4f (1., 1., 1., 1.);
		cairo_dock_apply_texture_at_size (myData.iTexture, myData.slideArea.fImgW * myData.fAnimAlpha, myData.slideArea.fImgH * myData.fAnimAlpha);
		
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		//On efface le fond
		_cd_slider_erase_surface (myApplet);
		
		//On met a l'échelle en recentrant.
		cairo_save(myDrawContext);
		cairo_translate (myDrawContext, (myData.iSurfaceWidth - myData.slideArea.fImgW * myData.fAnimAlpha) / 2, (myData.iSurfaceHeight - myData.slideArea.fImgH * myData.fAnimAlpha) / 2);
		cairo_scale(myDrawContext, myData.fAnimAlpha, myData.fAnimAlpha);
		
		//On empeche la transparence et on affiche l'image
		_cd_slider_add_background_to_current_slide (myApplet, 0., 0., 1.);
		cairo_set_source_surface (myDrawContext, myData.pCairoSurface, 0., 0.);
		
		cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
		cairo_restore(myDrawContext);
	}
	
	return (myData.fAnimAlpha < .99);
}

gboolean cd_slider_shrink_down (CairoDockModuleInstance *myApplet) {
	myData.iAnimCNT ++;
	myData.fAnimAlpha = 2 - 1.*myData.iAnimCNT / myConfig.iNbAnimationStep;
	if (myData.fAnimAlpha < 1)
		myData.fAnimAlpha = 1;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return FALSE;
		
		_cd_slider_add_background_to_current_slide_opengl (myApplet, 0., 0., myData.fAnimAlpha);
		
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable (GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
		glColor4f (1., 1., 1., 1.);
		cairo_dock_apply_texture_at_size (myData.iTexture, myData.slideArea.fImgW * myData.fAnimAlpha, myData.slideArea.fImgH * myData.fAnimAlpha);
		
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		//On efface le fond
		_cd_slider_erase_surface (myApplet);
		
		//On met a l'échelle en recentrant.
		cairo_save(myDrawContext);
		cairo_translate (myDrawContext, (myData.iSurfaceWidth - myData.slideArea.fImgW * myData.fAnimAlpha) / 2, (myData.iSurfaceHeight - myData.slideArea.fImgH * myData.fAnimAlpha) / 2);
		cairo_scale(myDrawContext, myData.fAnimAlpha, myData.fAnimAlpha);
		
		//On empeche la transparence et on affiche l'image
		_cd_slider_add_background_to_current_slide (myApplet, 0., 0., 1.);
		cairo_set_source_surface (myDrawContext, myData.pCairoSurface, 0., 0.);
		
		cairo_paint_with_alpha (myDrawContext, myData.fAnimAlpha);
		cairo_restore(myDrawContext);
	}
	
	return (myData.fAnimAlpha > 1.01);
}

gboolean cd_slider_cube (CairoDockModuleInstance *myApplet) {
	myData.iAnimCNT ++;
	myData.fAnimAlpha = 1.*myData.iAnimCNT / myConfig.iNbAnimationStep;
	if (myData.fAnimAlpha > 1)
		myData.fAnimAlpha = 1;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return FALSE;
		
		
		
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	return (myData.fAnimAlpha < .99);
}

gboolean cd_slider_update_icon (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	if (pIcon != myIcon)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	if (cd_slider_next_slide_is_scheduled (myApplet))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	gboolean bContinueTransition = FALSE;
	switch (myData.iAnimation)
	{
		case SLIDER_FADE :
			bContinueTransition = cd_slider_fade (myApplet);
		break ;
		case SLIDER_BLANK_FADE :
			bContinueTransition = cd_slider_blank_fade (myApplet);
		break ;
		case SLIDER_FADE_IN_OUT :
			bContinueTransition = cd_slider_fade_in_out (myApplet);
		break ;
		case SLIDER_SIDE_KICK :
			bContinueTransition = cd_slider_side_kick (myApplet);
		break ;
		case SLIDER_DIAPORAMA :
			bContinueTransition = cd_slider_diaporama (myApplet);
		break ;
		case SLIDER_GROW_UP :
			bContinueTransition = cd_slider_grow_up (myApplet);
		break ;
		case SLIDER_SHRINK_DOWN :
			bContinueTransition = cd_slider_shrink_down (myApplet);
		break ;
		default :
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	CD_APPLET_REDRAW_MY_ICON;
	
	if (bContinueTransition)
		*bContinueAnimation = TRUE;
	else
	{
		cd_slider_schedule_next_slide (myApplet);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
