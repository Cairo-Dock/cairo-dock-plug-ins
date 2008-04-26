#ifndef __APPLET_SILDER__
#define  __APPLET_SLIDER__


#include <cairo-dock.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

void cd_slider_get_files_from_dir(void);
gboolean cd_slider_draw_images(void);
void _printList (GList *pList);
void _slider_free_list(GList *pList);

#endif
