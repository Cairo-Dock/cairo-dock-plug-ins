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
  walker(myConfig.cDirectory);
}

gboolean cd_slider_draw_images(void) {
  gchar *pValue=NULL;
  if (myData.pElement->data != NULL) {
    pValue = myData.pElement->data;
  }
  if (pValue != NULL) {
    printf("Displaying: %s\n", pValue);
    pValue = g_strdup_printf ("%s/%s",myConfig.cDirectory , pValue);
    CD_APPLET_SET_IMAGE_ON_MY_ICON (pValue);
    myData.pElement = myData.pElement->next;
  }
  if (myData.pElement == NULL) {
    myData.pElement = myData.pList;
  }
  g_timeout_add (3000, (GSourceFunc) cd_slider_draw_images, (gpointer) NULL);
  CD_APPLET_REDRAW_MY_ICON
  return FALSE;
}

void _printList(GList *pList) {
  GList *pElement;
  gchar *pValue;
  for (pElement = pList; pElement != NULL; pElement = pElement->next) {
    pValue = pElement->data;
    printf("Listed: %s\n", pValue);
  }
}

void walker(const gchar *path) {
  DIR *d;
  struct dirent *dir;
  gchar *extension=NULL;
  d = opendir(path);
  myData.pList=NULL;
  char *pFile=NULL;
  char *File=NULL;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, ".") == 0) continue;
      if (strcmp(dir->d_name, "..") == 0) continue; 
      File = dir->d_name;
      extension = strchr(dir->d_name,'.');
      if (extension != NULL) {
       if (strcmp(extension, ".png") == 0 || strcmp(extension, ".jpg") == 0 || strcmp(extension, ".svg") == 0 || strcmp(extension, ".xpm") == 0) {
         printf("Adding %s to list\n", File);
         pFile = File;
         myData.pList = g_list_append (myData.pList, pFile);
        }
        else {
          printf("%s not handeled\n", File);
        }
      }
    }
    closedir(d);
  }
  myData.pElement = myData.pList;
  _printList(myData.pList);
  cd_slider_draw_images();
}

void _slider_free_list(GList *pList) {
  g_list_foreach (pList, (GFunc) g_free, NULL);
  g_list_free (pList);
  pList = NULL;
}
