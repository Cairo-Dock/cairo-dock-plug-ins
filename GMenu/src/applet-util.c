/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>
#include <gio/gio.h>

#include "applet-struct.h"
#include "applet-util.h"



char * panel_util_get_icon_name_from_g_icon (GIcon *gicon)
{
	const char * const *names;
	GtkIconTheme *icon_theme;
	int i;

	if (!G_IS_THEMED_ICON (gicon))
		return NULL;

	names = g_themed_icon_get_names (G_THEMED_ICON (gicon));
	icon_theme = gtk_icon_theme_get_default ();

	for (i = 0; names[i] != NULL; i++) {
		if (gtk_icon_theme_has_icon (icon_theme, names[i]))
			return g_strdup (names[i]);
	}

	return NULL;
}

GdkPixbuf * panel_util_get_pixbuf_from_g_loadable_icon (GIcon *gicon,
					    int    size)
{
	GdkPixbuf    *pixbuf;
	GInputStream *stream;

	if (!G_IS_LOADABLE_ICON (gicon))
		return NULL;

	pixbuf = NULL;

	stream = g_loadable_icon_load (G_LOADABLE_ICON (gicon),
				       size,
				       NULL, NULL, NULL);
	if (stream) {
		pixbuf = panel_util_gdk_pixbuf_load_from_stream (stream);
		g_object_unref (stream);
	}

	if (pixbuf) {
		gint width, height;

		width = gdk_pixbuf_get_width (pixbuf);
		height = gdk_pixbuf_get_height (pixbuf);

		if (width > size || height > size) {
			GdkPixbuf *tmp;

			tmp = gdk_pixbuf_scale_simple (pixbuf, size, size,
						       GDK_INTERP_BILINEAR);

			g_object_unref (pixbuf);
			pixbuf = tmp;
		}
	}

	return pixbuf;
}


static void _launch_from_file (const gchar *cDesktopFilePath)
{
	GError *erreur = NULL;
        GKeyFile* keyfile = g_key_file_new();
        g_key_file_load_from_file (keyfile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
        if (erreur != NULL)
        {
                cd_warning ("Attention : while trying to load %s : %s", cDesktopFilePath, erreur->message);
                g_error_free (erreur);
                return ;
        }
	gchar *cCommand = g_key_file_get_string (keyfile, "Desktop Entry", "Exec", &erreur);
        if (erreur != NULL)
        {
                cd_warning ("Attention : while trying to load %s : %s", cDesktopFilePath, erreur->message);
                g_error_free (erreur);
                erreur = NULL;
        }
	gchar *cWorkingDirectory = NULL;
	if (cCommand != NULL)
        {
                g_free (cWorkingDirectory);
                cWorkingDirectory = g_key_file_get_string (keyfile, "Desktop Entry", "Path", NULL);
                if (cWorkingDirectory != NULL && *cWorkingDirectory == '\0')
                {
                        g_free (cWorkingDirectory);
                        cWorkingDirectory = NULL;
                }
        }
	cairo_dock_launch_command_full (cCommand, cWorkingDirectory);
	g_free (cCommand);
	g_free (cWorkingDirectory);
} 
static void _launch_from_basename (const gchar *cDesktopFileName)
{
	gchar *cName = g_strdup (cDesktopFileName);
	gchar *str = strrchr (cName, '.');
	if (str != NULL)
		str = '\0';
	cairo_dock_launch_command (cName);
	g_free (cName);
}

void
panel_launch_desktop_file (const char  *desktop_file,
			   const char  *fallback_exec,
			   GdkScreen   *screen,
			   GError     **error)
{
	//GnomeDesktopItem *ditem;

	if (g_path_is_absolute (desktop_file))
		//ditem = gnome_desktop_item_new_from_file (desktop_file, 0, error);
		_launch_from_file (desktop_file);
	else
		//ditem = gnome_desktop_item_new_from_basename (desktop_file, 0, error);
		_launch_from_basename (desktop_file);

	/*if (ditem != NULL) {
		panel_ditem_launch (ditem, NULL, screen, error);
		gnome_desktop_item_unref (ditem);
	} else if (fallback_exec != NULL) {
		char *argv [2] = {(char *)fallback_exec, NULL};

		if (*error) {
			g_error_free (*error);
			*error = NULL;
		}

		gdk_spawn_on_screen (screen, NULL, argv, NULL,
				     G_SPAWN_SEARCH_PATH,
				     NULL, NULL, NULL, error);
	}*/
}
void panel_menu_item_activate_desktop_file (GtkWidget  *menuitem,
				       const char *path)
{
	GError *error;

	error = NULL;
	panel_launch_desktop_file (path, NULL,
				   NULL/*menuitem_to_screen (menuitem)*/, &error);
	if (error) {
		/*panel_error_dialog (NULL, menuitem_to_screen (menuitem),
				    "cannot_launch_entry", TRUE,
				    _("Could not launch menu item"),
				    error->message);*/
		cd_warning (error->message);
		g_error_free (error);
	}
}

char * panel_util_icon_remove_extension (const char *icon)
{
	char *icon_no_extension;
	char *p;

	icon_no_extension = g_strdup (icon);
	p = strrchr (icon_no_extension, '.');
	if (p &&
	    (strcmp (p, ".png") == 0 ||
	     strcmp (p, ".xpm") == 0 ||
	     strcmp (p, ".svg") == 0)) {
	    *p = 0;
	}

	return icon_no_extension;
}

static gboolean
panel_util_query_tooltip_cb (GtkWidget  *widget,
			     gint        x,
			     gint        y,
			     gboolean    keyboard_tip,
			     GtkTooltip *tooltip,
			     const char *text)
{
	/*if (!panel_global_config_get_tooltips_enabled ())
		return FALSE;*/

	gtk_tooltip_set_text (tooltip, text);
	return TRUE;
}
void panel_util_set_tooltip_text (GtkWidget  *widget,
			     const char *text)
{
        g_signal_handlers_disconnect_matched (widget,
					      G_SIGNAL_MATCH_FUNC,
					      0, 0, NULL,
					      panel_util_query_tooltip_cb,
					      NULL);

	if (string_empty (text)) {
		g_object_set (widget, "has-tooltip", FALSE, NULL);
		return;
	}

	g_object_set (widget, "has-tooltip", TRUE, NULL);
	g_signal_connect_data (widget, "query-tooltip",
			       G_CALLBACK (panel_util_query_tooltip_cb),
			       g_strdup (text), (GClosureNotify) g_free, 0);
}

char * menu_escape_underscores_and_prepend (const char *text)
{
	GString    *escaped_text;
	const char *src;
	int         inserted;
	
	if (!text)
		return g_strdup (text);

	escaped_text = g_string_sized_new (strlen (text) + 1);
	g_string_printf (escaped_text, "_%s", text);

	src = text;
	inserted = 1;

	while (*src) {
		gunichar c;

		c = g_utf8_get_char (src);

		if (c == (gunichar)-1) {
			g_warning ("Invalid input string for underscore escaping");
			return g_strdup (text);
		} else if (c == '_') {
			g_string_insert_c (escaped_text,
					   src - text + inserted, '_');
			inserted++;
		}

		src = g_utf8_next_char (src);
	}

	return g_string_free (escaped_text, FALSE);
}

char * panel_find_icon (GtkIconTheme  *icon_theme,
		 const char    *icon_name,
		 gint           size)
{
	GtkIconInfo *info;
	char        *retval;
	char        *icon_no_extension;

	if (icon_name == NULL || strcmp (icon_name, "") == 0)
		return NULL;

	if (g_path_is_absolute (icon_name)) {
		if (g_file_test (icon_name, G_FILE_TEST_EXISTS)) {
			return g_strdup (icon_name);
		} else {
			char *basename;

			basename = g_path_get_basename (icon_name);
			retval = panel_find_icon (icon_theme, basename,
						  size);
			g_free (basename);

			return retval;
		}
	}

	/* This is needed because some .desktop files have an icon name *and*
	 * an extension as icon */
	icon_no_extension = panel_util_icon_remove_extension (icon_name);

	info = gtk_icon_theme_lookup_icon (icon_theme, icon_no_extension,
					   size, 0);

	g_free (icon_no_extension);

	if (info) {
		retval = g_strdup (gtk_icon_info_get_filename (info));
		gtk_icon_info_free (info);
	} else
		retval = NULL;

	return retval;
}

/* TODO: kill this when we can depend on GTK+ 2.14 */
GdkPixbuf * panel_util_gdk_pixbuf_load_from_stream (GInputStream  *stream)
{
#define LOAD_BUFFER_SIZE 65536
	unsigned char buffer[LOAD_BUFFER_SIZE];
	gssize bytes_read;
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	gboolean got_eos;
	

	g_return_val_if_fail (stream != NULL, NULL);

	got_eos = FALSE;
	loader = gdk_pixbuf_loader_new ();
	while (1) {
		bytes_read = g_input_stream_read (stream, buffer, sizeof (buffer),
						  NULL, NULL);
		
		if (bytes_read < 0) {
			break;
		}
		if (bytes_read == 0) {
			got_eos = TRUE;
			break;
		}
		if (!gdk_pixbuf_loader_write (loader,
					      buffer,
					      bytes_read,
					      NULL)) {
			break;
		}
	}

	g_input_stream_close (stream, NULL, NULL);
	gdk_pixbuf_loader_close (loader, NULL);

	pixbuf = NULL;
	if (got_eos) {
		pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
		if (pixbuf != NULL) {
			g_object_ref (pixbuf);
		}
	}

	g_object_unref (loader);

	return pixbuf;
}

