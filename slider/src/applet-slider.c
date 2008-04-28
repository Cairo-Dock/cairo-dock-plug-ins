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

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-silder.h"

CD_APPLET_INCLUDE_MY_VARS

void cd_slider_get_files_from_dir(void) {
	if (myConfig.cDirectory == NULL)
		return;
	
  DIR *d;
  struct dirent *dir;
  gchar *extension=NULL;
  
  cd_message("Opening %s...", myConfig.cDirectory);
  d = opendir(myConfig.cDirectory);
  myData.pList=NULL;
  char *pFile=NULL;
  char *File=NULL;
  if (d) {
  	cd_message("Now searching in %s for images files", myConfig.cDirectory);
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, ".") == 0) continue;
      if (strcmp(dir->d_name, "..") == 0) continue; 
      File = dir->d_name;
      extension = strchr(dir->d_name,'.');
      if (extension != NULL) {
       if (strcmp(extension, ".png") == 0 || strcmp(extension, ".jpg") == 0 || strcmp(extension, ".svg") == 0 || strcmp(extension, ".xpm") == 0) {
         cd_message("Adding %s to list\n", File);
         pFile = g_strdup(File);
         myData.pList = g_list_append (myData.pList, pFile);
        }
        else {
         	cd_message("%s not handeled, ignoring...\n", File);
        }
      }
    }
    closedir(d);
  }
  myData.pElement = myData.pList;
  _printList(myData.pList);
  cd_slider_draw_images();
}


//A optimiser!
gboolean cd_slider_draw_images(void) {
  gchar *pValue=NULL;
  if (myData.pElement == NULL || myData.bPause == TRUE)
  	return FALSE;
  
  if (myData.pElement->data != NULL) {
    pValue = myData.pElement->data;
  }
  
  CD_APPLET_REDRAW_MY_ICON
  cd_message("Displaying: %s\n", pValue);
  pValue = g_strdup_printf ("%s/%s",myConfig.cDirectory , pValue);
  
  if (myDesklet && myConfig.bNoStrench) {
  	cairo_t* pCairoContext;
  	cairo_surface_t* pCairoSurface;
  	
  	//On créer un context différent pour charger l'image, On évite que la taille max soit celle du desklet.
  	cairo_surface_t* surface;
  	cairo_t* context;
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1400, 900); //1400x900 pour que l'image se charge dans sa totalité
  	context = cairo_create (surface);
  	
  	CD_APPLET_SET_SURFACE_ON_MY_ICON(surface); //Surface pour vider le fond
  	
  	double fImgX, fImgY, fImgW, fImgH;
  	surface = cairo_dock_create_surface_from_image(pValue, context, 1., NULL, NULL, &fImgW, &fImgH, TRUE);
  	cd_message("Image width: %.02f height: %.02f", fImgW, fImgH);
  	cairo_surface_destroy(surface);
  	cairo_destroy (context);
  	
  	pCairoContext = cairo_create (myIcon->pIconBuffer);
  	if (fImgW < fImgH) { //H dominant: Portrait, il faut calculer le ratio imgH/iconH et l'utiliser sur W
  		if (myIcon->fHeight < fImgH) { //On réduit H a celle de l'icône et on scale
  			fImgW = (double) (myIcon->fHeight / fImgH) * fImgW;
  			fImgH = myIcon->fHeight;
  		}
  		pCairoSurface = cairo_dock_create_surface_from_image(pValue, pCairoContext, 1., fImgW, fImgH, &fImgW, &fImgH, TRUE);
  	}
  	else { //W dominant: Paysage, il faut calculer le ratio imgW/iconW et l'utiliser sur H
  		if (myIcon->fWidth < fImgW) { //On réduit W a celle de l'icône et on scale
  			fImgH = (double) (myIcon->fWidth/ fImgW) * fImgH;
  			fImgW = myIcon->fWidth;
  		}
  		pCairoSurface = cairo_dock_create_surface_from_image(pValue, pCairoContext, 1., fImgW, fImgH, &fImgW, &fImgH, TRUE);
  	}
  	
  	fImgX = (myIcon->fWidth - fImgW) / 2;
  	fImgY = (myIcon->fHeight - fImgH) / 2;
  	
  	cairo_translate (pCairoContext, fImgX, fImgY);
		
  	cd_message("X Y: %.02f %.02f - Ratio W: %.02f - Ratio H: %.02f - W: %.02f - H: %.02f", fImgX, fImgY, myIcon->fWidth/ fImgW, myIcon->fHeight / fImgH, fImgW ,fImgH);
		
  	cairo_set_source_surface (pCairoContext, pCairoSurface, 0, 0);
  	cairo_paint (pCairoContext);
  	
  	cairo_surface_destroy(pCairoSurface);
  	cairo_destroy (pCairoContext); //Pas de fuite mémoire
  }
  else {
    CD_APPLET_SET_IMAGE_ON_MY_ICON (pValue);
  }

  g_free(pValue); //Pas de fuite mémoire.
    
  myData.pElement = myData.pElement->next;
  if (myData.pElement == NULL) {
    myData.pElement = myData.pList;
  }
  cd_message("Next Image: %s\n", myData.pElement->data);
  myData.iTimerID = g_timeout_add (myConfig.dSlideTime, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
  return FALSE;
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

void _slider_free_list(GList *pList) {
	if (pList != NULL) {
  	g_list_foreach (pList, (GFunc) g_free, NULL);
  	g_list_free (pList);
  	pList = NULL;
  }
}
