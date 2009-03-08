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

#ifdef HAVE_EXIF
#include <libexif/exif-data.h>
#endif

#include "cairo-dock.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-transitions.h"
#include "applet-slider.h"


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
static int _cd_slider_random_compare (gconstpointer a, gconstpointer b, GRand *pRandomGenerator) {
	return (g_rand_boolean (pRandomGenerator) ? 1 : -1);
}
static GList *cd_slider_measure_directory (GList *pList, gchar *cDirectory, gboolean bRecursive, gboolean bSortAlpha) {
#ifdef HAVE_EXIF
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
	g_print ("  chargement %s...\n", cImagePath);
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
	
	//\_______________ On garde l'aire de la surface/texture.
	fImgX = (myData.iSurfaceWidth - fImgW) / 2;
	fImgY = (myData.iSurfaceHeight - fImgH) / 2;
	myData.slideArea.fImgX = fImgX;
	myData.slideArea.fImgY = fImgY;
	myData.slideArea.fImgW = fImgW;
	myData.slideArea.fImgH = fImgH;
	g_print ("  %s chargee\n", cImagePath);
}


gboolean cd_slider_update_transition (CairoDockModuleInstance *myApplet) {
	g_print ("%s ()\n", __func__);
	//\_______________ On cree la texture (en-dehors du thread).
	if (g_bUseOpenGL)
	{
		myData.iTexture = cairo_dock_create_texture_from_surface (myData.pCairoSurface);
	}
	
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
		g_print ("---default animation\n");
		cd_slider_draw_default (myApplet);
		CD_APPLET_REDRAW_MY_ICON;
		cd_slider_schedule_next_slide (myApplet);
	}
	
	return FALSE;
}

gboolean cd_slider_next_slide (CairoDockModuleInstance *myApplet) {
	g_print ("%s (%d)\n", __func__, myData.iTimerID);
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
	
	//\___________________________ On ecrit le nom de la nouvelle image en info.
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
	
	//\___________________________ On charge la nouvelle surface/texture et on lance l'animation de transition.
	if (myConfig.bUseThread && CD_APPLET_MY_CONTAINER_IS_OPENGL && pImage->iFormat != SLIDER_SVG &&  // pour certains SVG, ca plante dans X :-(
		((pImage->iFormat == SLIDER_PNG && pImage->iSize > 100e3) ||
		(pImage->iFormat == SLIDER_JPG && pImage->iSize > 70e3) ||
		(pImage->iFormat == SLIDER_GIF && pImage->iSize > 100e3) ||
		(pImage->iFormat == SLIDER_XPM && pImage->iSize > 100e3))) {
		cd_debug ("Slider -   on threade");
		cairo_dock_launch_measure (myData.pMeasureImage);
		myData.iTimerID = 0;
		g_print ("bye\n");
		return FALSE;  // on quitte la boucle d'attente car on va effectuer une animation.
	}
	else {
		cd_slider_read_image (myApplet);
		cd_slider_update_transition (myApplet);
		
		if (myConfig.iAnimation == SLIDER_DEFAULT)
		{
			return TRUE;  // pas d'animation => on ne quitte pas la boucle d'attente.
		}
		else
		{
			myData.iTimerID = 0;
			return FALSE;  // on quitte la boucle d'attente car on va effectuer une animation.
		}
	}
}
