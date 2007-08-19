/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include "stdlib.h"

#include "cd-clock-struct.h"
#include "cd-clock-draw.h"
#include "cd-clock-config.h"
#include "cd-clock-menu-functions.h"
#include "cd-clock-init.h"


#define CD_CLOCK_CONF_FILE "clock.conf"
#define CD_CLOCK_USER_DATA_DIR "clock"


gchar *my_cConfFilePath = NULL;
gboolean my_bShowDate;
gboolean my_bShowSeconds;
gboolean my_b24Mode;
gboolean my_bOldStyle;
int my_iTheme = 0;

int my_iSidUpdateClock = 0;
Icon *my_pIcon = NULL;
GtkWidget *my_pWidget = NULL;
cairo_t *my_pCairoContext = NULL;
GHashTable *my_pThemeTable = NULL;

cairo_surface_t* g_pBackgroundSurface = NULL;
cairo_surface_t* g_pForegroundSurface = NULL;
RsvgDimensionData my_DimensionData;
RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];


char my_cFileNames[CLOCK_ELEMENTS][30] =
{
	"clock-drop-shadow.svg",
	"clock-face.svg",
	"clock-marks.svg",
	"clock-hour-hand-shadow.svg",
	"clock-minute-hand-shadow.svg",
	"clock-second-hand-shadow.svg",
	"clock-hour-hand.svg",
	"clock-minute-hand.svg",
	"clock-second-hand.svg",
	"clock-face-shadow.svg",
	"clock-glass.svg",
	"clock-frame.svg"
};


Icon *cd_clock_init (GtkWidget *pWidget, gchar **cConfFilePath, GError **erreur)
{
	//g_print ("%s ()\n", __func__);
	gchar *cUserDataDirPath = g_strdup_printf ("%s/plug-in/%s", g_cCurrentThemePath, CD_CLOCK_USER_DATA_DIR);
	if (! g_file_test (cUserDataDirPath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		g_print ("directory %s doesn't exist, trying to fix it ...\n", cUserDataDirPath);
		
		gchar *command = g_strdup_printf ("mkdir -p %s", cUserDataDirPath);
		system (command);
		g_free (command);
		
		command = g_strdup_printf ("cp %s/%s %s", CD_CLOCK_SHARE_DATA_DIR, CD_CLOCK_CONF_FILE, cUserDataDirPath);
		system (command);
		g_free (command);
	}
	
	//\_______________ On charge la liste des themes disponibles.
	GError *tmp_erreur = NULL;
	gchar *cThemesDir = g_strdup_printf ("%s/themes", CD_CLOCK_SHARE_DATA_DIR);
	my_pThemeTable = cairo_dock_list_themes (cThemesDir, NULL, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	my_cConfFilePath = g_strdup_printf ("%s/%s", cUserDataDirPath, CD_CLOCK_CONF_FILE);
	g_free (cUserDataDirPath);
	cairo_dock_update_conf_file_with_hash_table (my_cConfFilePath, my_pThemeTable, "MODULE", "theme", 1, "Theme (for analogic display only) :");
	
	int i;
	for (i = 0; i < CLOCK_ELEMENTS; i ++)
	{
		my_pSvgHandles[i] = NULL;
	}
	
	//\_______________ On lit le fichier de conf.
	int iOriginalWidth = 1, iOriginalHeight = 1;
	gchar *cName = NULL;
	cd_clock_read_conf_file (my_cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cName);
	
	//\_______________ On cree nos entrees dans le menu qui sera appele lors d'un clic droit.
	GtkWidget *pModuleMenu = gtk_menu_new ();
	GtkWidget *menu_item;
	
	menu_item = gtk_menu_item_new_with_label ("Set up time and date");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_clock_launch_time_admin), NULL);
	
	menu_item = gtk_menu_item_new_with_label ("About");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (cd_clock_about), NULL);
	
	//\_______________ On cree notre icone.
	cairo_t *pSourceContext = cairo_dock_create_context_from_window (pWidget->window);
	my_pIcon = cairo_dock_create_icon_for_applet (pSourceContext, iOriginalWidth, iOriginalHeight, cName, pModuleMenu);
	cairo_destroy (pSourceContext);
	
	my_pWidget = pWidget;
	my_pCairoContext = cairo_create (my_pIcon->pIconBuffer);
	g_return_val_if_fail (my_pCairoContext != NULL, NULL);
	rsvg_handle_get_dimensions (my_pSvgHandles[CLOCK_DROP_SHADOW], &my_DimensionData);
	
	//\_______________ On charge les surfaces d'arriere-plan et d'avant-plan.
	cairo_set_operator (my_pCairoContext, CAIRO_OPERATOR_SOURCE);
	g_pBackgroundSurface = update_surface (g_pBackgroundSurface,
		my_pCairoContext,
		my_pIcon->fWidth * (1 + g_fAmplitude),
		my_pIcon->fHeight * (1 + g_fAmplitude),
		KIND_BACKGROUND);
	g_pForegroundSurface = update_surface (g_pForegroundSurface,
		my_pCairoContext,
		my_pIcon->fWidth * (1 + g_fAmplitude),
		my_pIcon->fHeight * (1 + g_fAmplitude),
		KIND_FOREGROUND);
	
	//\_______________ On lance le timer.
	cd_clock_update_with_time (my_pIcon);
	my_iSidUpdateClock = g_timeout_add ((my_bShowSeconds ? 1000 : 60000), (GSourceFunc) cd_clock_update_with_time, (gpointer) my_pIcon);
	
	g_free (cName);
	*cConfFilePath = g_strdup (my_cConfFilePath);
	return my_pIcon;
}

void cd_clock_stop (void)
{
	//g_print ("%s ()\n", __func__);
	g_source_remove (my_iSidUpdateClock);
	my_iSidUpdateClock = 0;
	my_pIcon = NULL;
	cairo_destroy (my_pCairoContext);
	my_pCairoContext = NULL;
	g_free (my_cConfFilePath);
	my_cConfFilePath = NULL;
	int i;
	for (i = 0; i < CLOCK_ELEMENTS; i ++)
	{
		rsvg_handle_free (my_pSvgHandles[i]);
		my_pSvgHandles[i] = NULL;
	}
	g_hash_table_destroy (my_pThemeTable);
	my_pThemeTable = NULL;
}

gboolean cd_clock_action (void)
{
	//g_print ("%s ()\n", __func__);
	GtkWidget *pDialog = gtk_dialog_new ();
	
	GtkWidget *pCalendar = gtk_calendar_new ();
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pDialog)->vbox), pCalendar);
	gtk_widget_show (pCalendar);
	gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);
	
	return TRUE;
}

