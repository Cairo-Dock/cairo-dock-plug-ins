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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>
#ifdef HAVE_GIO
#include <gio/gio.h>
#else
#define GIcon gpointer
#endif

typedef struct {
	GtkWidget    *pixmap;
	const char   *stock_id;
	GIcon        *gicon;
	char         *image;
	char         *fallback_image;
	GtkIconTheme *icon_theme;
	GtkIconSize   icon_size;
} IconToLoad;

typedef struct {
	GtkWidget   *image;
	const char  *stock_id;
	GdkPixbuf   *pixbuf;
	GtkIconSize  icon_size;
} IconToAdd;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bHasIcons;
	gboolean bShowRecent;
	gchar *cMenuShortkey;
	gchar *cQuickLaunchShortkey;
	gchar *cConfigureMenuCommand;
	gchar *cRecentRootDirFilter;
	gint iRecentAge;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GtkWidget *pMenu;
	CairoDialog *pQuickLaunchDialog;
	gboolean bIconsLoaded;
	guint iSidFakeMenuIdle;
	guint iSidCreateMenuIdle;
	guint iSidTreeChangeIdle;
	
	GtkRecentManager *pRecentManager;
	GtkWidget *pRecentMenuItem;
	GtkRecentFilter *pRecentFilter;
	
	GHashTable *dir_hash;
	GList *possible_executables;
	GList *completion_items;
	GCompletion *completion;
	gboolean completion_started;
	} ;


#endif
