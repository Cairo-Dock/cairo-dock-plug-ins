/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

static void _cd_slider_get_exif_props (SliderImage *pImage);


  //////////////
 /// IMAGES ///
//////////////

static void cd_slider_free_image (SliderImage *pImage) {
	if (pImage == NULL)
		return;
	g_free (pImage->cPath);
	g_free (pImage);
}

static void cd_slider_free_images_list (GList *pList) {
	g_list_foreach (pList, (GFunc) cd_slider_free_image, NULL);
	g_list_free (pList);
}


static void _cd_slider_load_image (GldiModuleInstance *myApplet)
{
	g_return_if_fail (myData.pElement != NULL);
	SliderImage *pImage = myData.pElement->data;
	gchar *cImagePath = pImage->cPath;
	if (!pImage->bGotExifData && myData.iSidExifIdle == 0)  // no exif data yet and no process currently retrieving them.
		_cd_slider_get_exif_props (pImage);
	cd_debug ("  Slider - loading %s (size %dbytes, orientation:%d)", cImagePath, pImage->iSize, pImage->iOrientation);
	//\_______________ On definit comment charger l'image.
	double fImgX, fImgY, fImgW=0, fImgH=0;
	CairoDockLoadImageModifier iLoadingModifier = 0;  // CAIRO_DOCK_FILL_SPACE
	if (pImage->iOrientation != 0)
		iLoadingModifier |= ((pImage->iOrientation-1) << 3);
	if (! myConfig.bFillIcon)
		iLoadingModifier |= CAIRO_DOCK_DONT_ZOOM_IN;
	if (myConfig.bNoStretch)
		iLoadingModifier |= CAIRO_DOCK_KEEP_RATIO;
	
	//\_______________ On cree la surface cairo.
	int iLineWidth = 0;
	if (myConfig.iBackgroundType == 2)
	{
		iLineWidth = _get_frame_linewidth (myApplet);
	}
	
	myData.pCairoSurface = cairo_dock_create_surface_from_image (cImagePath,
		1.,
		myData.iSurfaceWidth - iLineWidth,  // iLineWidth/2 de chaque cote
		myData.iSurfaceHeight - iLineWidth,  // idem
		iLoadingModifier,
		&fImgW, &fImgH,
		NULL, NULL);
	
	//\_______________ On garde l'aire de la surface/texture.
	fImgX = (myData.iSurfaceWidth - fImgW) / 2;
	fImgY = (myData.iSurfaceHeight - fImgH) / 2;
	myData.slideArea.fImgX = fImgX;
	myData.slideArea.fImgY = fImgY;
	myData.slideArea.fImgW = fImgW;
	myData.slideArea.fImgH = fImgH;
	cd_debug ("  %s loaded", cImagePath);
}

static gboolean _cd_slider_display_image (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
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
		cd_slider_draw_default (myApplet);
		CD_APPLET_REDRAW_MY_ICON;
		cd_slider_schedule_next_slide (myApplet);
	}
	CD_APPLET_LEAVE (FALSE);
}

void cd_slider_jump_to_next_slide (GldiModuleInstance *myApplet)
{
	//\___________________________ stop any pending action.
	if (myData.iTimerID != 0)
	{
		g_source_remove (myData.iTimerID);
		myData.iTimerID = 0;
	}
	cairo_dock_stop_task (myData.pMeasureImage);
	
	//\___________________________ On recupere la nouvelle image a afficher.
	if (myData.pElement == NULL)  // debut
		myData.pElement = myData.pList;
	else
		myData.pElement = cairo_dock_get_next_element (myData.pElement, myData.pList);
	
	if (myData.pElement == NULL || myData.pElement->data == NULL)
	{
		cd_warning ("Slider stopped, empty list");
		return;
	}
	SliderImage *pImage = myData.pElement->data;
	cd_message ("Slider - load %s", pImage->cPath);
	
	//\___________________________ On sauvegarde la surface actuelle.
	if (myData.pPrevCairoSurface != NULL && myData.pPrevCairoSurface != myData.pCairoSurface)
		cairo_surface_destroy (myData.pPrevCairoSurface);
	myData.pPrevCairoSurface = myData.pCairoSurface;
	myData.pCairoSurface = NULL;
	
	if (myData.iPrevTexture != 0 && myData.iPrevTexture != myData.iTexture)
		glDeleteTextures (1, &myData.iPrevTexture);
	myData.iPrevTexture = myData.iTexture;
	myData.iTexture = 0;
	
	myData.prevSlideArea = myData.slideArea;
	
	//\___________________________ On ecrit le nom de la nouvelle image en info.
	if (myConfig.bImageName && myDesklet)
	{
		gchar *cFilePath = g_strdup (pImage->cPath);
		gchar *cFileName = strrchr (cFilePath, '/');
		if (cFileName)
			cFileName ++;
		else
			cFileName = cFilePath;
		gchar *ext = strrchr (cFileName, '.');
		if (ext)
			*ext = '\0';
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (cFileName);
		//cd_debug ("Slider - Image path: %s", pImage->cPath);
		g_free (cFilePath);
	}
	
	//\___________________________ On charge la nouvelle surface/texture et on lance l'animation de transition.
	if (myConfig.bUseThread && CD_APPLET_MY_CONTAINER_IS_OPENGL && pImage->iFormat != SLIDER_SVG &&  // pour certains SVG, ca plante dans X :-(
		((pImage->iFormat == SLIDER_PNG && pImage->iSize > 100e3)
		|| (pImage->iFormat == SLIDER_JPG && pImage->iSize > 70e3)
		|| (pImage->iFormat == SLIDER_GIF && pImage->iSize > 100e3)
		|| (pImage->iFormat == SLIDER_XPM && pImage->iSize > 100e3)) )
	{
		cd_debug ("Slider - launch thread");
		cairo_dock_launch_task (myData.pMeasureImage);
	}
	else
	{
		_cd_slider_load_image (myApplet);
		_cd_slider_display_image (myApplet);
	}
}


static gboolean _next_slide (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	
	myData.iTimerID = 0;  // we'll quit this loop anyway, so reset the timer now so that it doesn't disturb us.
	
	if (! myData.bPause)  // if paused, don't go to next slide.
	{
		cd_slider_jump_to_next_slide (myApplet);
	}
	
	CD_APPLET_LEAVE (FALSE);
}
void cd_slider_schedule_next_slide (GldiModuleInstance *myApplet)
{
	if (myData.iTimerID == 0)
		myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) _next_slide, (gpointer) myApplet);
}


  /////////////////
 /// EXIF DATA ///
/////////////////

static void _cd_slider_get_exif_props (SliderImage *pImage)
{
#ifdef HAVE_EXIF
	ExifData *pExifData;
	ExifEntry *pExifEntry;
	ExifByteOrder byteOrder;
	
	if (pImage->iFormat == SLIDER_JPG)
	{
		//g_print ("Slider : %s\n", pImage->cPath);
		pExifData = exif_data_new_from_file (pImage->cPath);
		if (pExifData != NULL)
		{
			pExifEntry = exif_data_get_entry (pExifData, EXIF_TAG_ORIENTATION);
			if (pExifEntry != NULL)
			{
				byteOrder = exif_data_get_byte_order (pExifData);
				pImage->iOrientation = exif_get_short (pExifEntry->data, byteOrder);
				//g_print ("Slider : %s -> orientation %d\n", pImage->cPath, pImage->iOrientation);
			}
			
			exif_data_unref (pExifData);
		}
	}
#endif
	pImage->bGotExifData = TRUE;
}

static gboolean _cd_slider_get_exif_props_idle (GldiModuleInstance *myApplet)
{
#ifdef HAVE_EXIF
	if (myData.pExifElement == NULL)
	{
		myData.iSidExifIdle = 0;
		return FALSE;
	}
	
	SliderImage *pImage = myData.pExifElement->data;
	_cd_slider_get_exif_props (pImage);
	
	myData.pExifElement = myData.pExifElement->next;
	return TRUE;
#else
	myData.pExifElement = NULL;
	myData.iSidExifIdle = 0;
	return FALSE;
#endif
}


  //////////////////////
 /// FOLDER PARSING ///
//////////////////////

typedef struct {
	gchar *cDirectory;
	gboolean bSubDirs;
	gboolean bRandom;
	GList *pList;  // the result
	GldiModuleInstance *pApplet;
	} CDListSharedMemory;

static void _free_shared_memory (CDListSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->cDirectory);
	// cd_slider_free_images_list (pSharedMemory->pList); // should be the same as myData.pList
	g_free (pSharedMemory);
}

static int _compare_images_order (SliderImage *image2, SliderImage *image1)
{
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
static int _cd_slider_random_compare (gconstpointer a, gconstpointer b, GRand *pRandomGenerator)
{
	return (g_rand_boolean (pRandomGenerator) ? 1 : -1);
}
static GList *_list_directory (GList *pList, gchar *cDirectory, gboolean bRecursive, gboolean bSortAlpha)
{
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
				pList = _list_directory (pList, sFilePath->str, bRecursive, bSortAlpha);
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
						pImage = g_new0 (SliderImage, 1);
						pImage->cPath = g_strdup (sFilePath->str);
						pImage->iSize = buf.st_size;
						pImage->iFormat = iFormat;
						// exif orientation is got later.
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

static void cd_slider_get_files_from_dir (CDListSharedMemory *pSharedMemory)
{
	if (pSharedMemory->cDirectory == NULL)
	{
		cd_warning ("Slider : No directory to scan, halt.");
		return;
	}
	
	// recursively scan the folder
	pSharedMemory->pList = _list_directory (NULL, pSharedMemory->cDirectory, pSharedMemory->bSubDirs, ! pSharedMemory->bRandom);
	
	// randomize the list if necessary
	if (pSharedMemory->bRandom)
	{
		GRand *pRandomGenerator = g_rand_new ();
		pSharedMemory->pList = g_list_sort_with_data (pSharedMemory->pList, (GCompareDataFunc) _cd_slider_random_compare, pRandomGenerator);
		g_rand_free (pRandomGenerator);
	}
}

static gboolean cd_slider_start_slide (CDListSharedMemory *pSharedMemory)
{
	// grab the result
	GldiModuleInstance *myApplet = pSharedMemory->pApplet;
	myData.pList = pSharedMemory->pList;
	
	// if needed, get the EXIF data now
	if (myData.iSidExifIdle == 0 && myConfig.bGetExifDataAtOnce)
	{
		myData.pExifElement = myData.pList;
		myData.iSidExifIdle = g_idle_add_full (G_PRIORITY_LOW,  // on ne veut pas que ca vienne ralentir le chargement des icones, qui lui a une priorite normale.
			(GSourceFunc) _cd_slider_get_exif_props_idle,
			myApplet,
			NULL);
	}
	
	// create the loading task that will be used later
	myData.pMeasureImage = cairo_dock_new_task (0,
		(CairoDockGetDataAsyncFunc) _cd_slider_load_image,
		(CairoDockUpdateSyncFunc) _cd_slider_display_image,
		myApplet);  // 0 <=> one shot task.
	
	// display the first slide.
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
	if (myData.pList != NULL)
	{
		cd_slider_jump_to_next_slide (myApplet);
	}
	else  // no images found or folder not defined => no need to launch the loop, display a 'missing' icon
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (GLDI_ICON_NAME_MISSING_IMAGE);
	}
	return FALSE;
}


void cd_slider_start (GldiModuleInstance *myApplet, gboolean bDelay)
{
	cairo_dock_discard_task (myData.pMeasureDirectory);
	
	// remember the current params.
	g_free (myData.cDirectory);
	myData.cDirectory = g_strdup (myConfig.cDirectory);
	myData.bSubDirs = myConfig.bSubDirs;
	myData.bRandom = myConfig.bRandom;
	
	// create a task to parse the folder.
	CDListSharedMemory *pSharedMemory = g_new0 (CDListSharedMemory, 1);
	pSharedMemory->bSubDirs = myConfig.bSubDirs;
	pSharedMemory->bRandom = myConfig.bRandom;
	pSharedMemory->cDirectory = g_strdup (myConfig.cDirectory);
	pSharedMemory->pApplet = myApplet;
	
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("...");  // Note: we could launch a 'busy' animation, but it's maybe not a good idea to add more CPU load (anyway, the animation should be launched with a g_idle_add, because animations may not be inited yet).
	myData.pMeasureDirectory = cairo_dock_new_task_full (0,
		(CairoDockGetDataAsyncFunc) cd_slider_get_files_from_dir,
		(CairoDockUpdateSyncFunc) cd_slider_start_slide,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory); // 0 <=> one shot task.
	
	// launch the parsing.
	if (bDelay)
		cairo_dock_launch_task_delayed (myData.pMeasureDirectory, cairo_dock_is_loading () ? 1500. : 0.);  // launch with a delay or just in the next main loop event.
	else
		cairo_dock_launch_task (myData.pMeasureDirectory);
}

void cd_slider_stop (GldiModuleInstance *myApplet)
{
	//Stop all processes
	cairo_dock_free_task (myData.pMeasureImage);  // since it needs a cairo context, we can't let it live.
	myData.pMeasureImage = NULL;
	cairo_dock_discard_task (myData.pMeasureDirectory);
	myData.pMeasureDirectory = NULL;
	if (myData.iSidExifIdle != 0)
	{
		g_source_remove(myData.iSidExifIdle);
		myData.iSidExifIdle = 0;
	}
	if (myData.iScrollID != 0)
	{
		g_source_remove (myData.iScrollID);
		myData.iScrollID = 0;
	}
	if (myData.iTimerID != 0)
	{
		g_source_remove(myData.iTimerID);
		myData.iTimerID = 0;
	}
	
	// destroy current buffers.
	if (myData.pCairoSurface)
	{
		cairo_surface_destroy (myData.pCairoSurface);
		myData.pCairoSurface = NULL;
	}
	if (myData.pPrevCairoSurface)
	{
		cairo_surface_destroy (myData.pPrevCairoSurface);
		myData.pPrevCairoSurface = NULL;
	}
	if (myData.iPrevTexture != 0)
	{
		_cairo_dock_delete_texture (myData.iPrevTexture);
		myData.iPrevTexture = 0;
	}
	if (myData.iTexture != 0)
	{
		_cairo_dock_delete_texture (myData.iTexture);
		myData.iTexture = 0;
	}
	
	cd_slider_free_images_list (myData.pList);
	myData.pList = NULL;
	myData.pElement = NULL;
	myData.pExifElement = NULL;
	myData.bPause = FALSE;
}
