/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <cairo-dock.h>
#include "cairo-applet.h"


static gboolean applet_on_expose_dialog_dummy (GtkWidget *pWidget,
                                               GdkEventExpose *pExpose,
                                               gpointer pDialog)
{
  return FALSE;
}

static gboolean applet_on_expose_dialog (GtkWidget *pWidget,
                                  GdkEventExpose *pExpose,
                                  CairoDockDialog *pDialog)
{
  gint w = 0, h = 0;
  int border = 7;

  if (!pDialog)
    return FALSE;

  cairo_t *pCairoContext = gdk_cairo_create (pWidget->window);
  if (cairo_status (pCairoContext) != CAIRO_STATUS_SUCCESS, FALSE) {
    cairo_destroy (pCairoContext);
    return FALSE;
  }

  //erase the background
  cairo_set_source_rgba (pCairoContext, 0., 0., 0., 0.);
  cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
  cairo_paint (pCairoContext);

  //set the color
  cairo_set_source_rgba (pCairoContext, 1., 1., 1., 0.7);
  cairo_set_line_width (pCairoContext, 10.0);
  cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_ROUND);

  //draw a rounded square
  gtk_window_get_size(GTK_WINDOW(pDialog->pWidget), &w, &h);
  cairo_move_to (pCairoContext, border, border);
  cairo_rel_line_to (pCairoContext, w - (border << 1), 0);
  cairo_rel_line_to (pCairoContext, 0, h - (border << 1));
  cairo_rel_line_to (pCairoContext, -(w - (border << 1)) , 0);
  cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
  cairo_close_path (pCairoContext);
  cairo_stroke (pCairoContext);

  cairo_destroy (pCairoContext);
  return FALSE;
}

/* void applet_isolate_dialog (CairoDockDialog *pDialog) */
/* { */
/* 	if (pDialog == NULL) */
/* 		return ; */

/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_expose_dialog, NULL); */
/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_button_press_dialog, NULL); */
/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_configure_dialog, NULL); */
/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_enter_dialog, NULL); */
/* 	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_leave_dialog, NULL); */
/* } */


void applet_free_dialog (CairoDockDialog *pDialog)
{
  if (pDialog == NULL)
		return ;

	g_print ("%s ()\n", __func__);

	cairo_surface_destroy (pDialog->pTextBuffer);
	pDialog->pTextBuffer = NULL;

	gtk_widget_destroy (pDialog->pWidget);  // detruit aussi le widget interactif.
	pDialog->pWidget = NULL;

	if (pDialog->pUserData != NULL && pDialog->pFreeUserDataFunc != NULL)
		pDialog->pFreeUserDataFunc (pDialog->pUserData);

	g_free (pDialog);
}


CairoDockDialog *applet_build_dialog (CairoDock *pDock, GtkWidget *pInteractiveWidget, gpointer data)
{

	//\________________ On cree un dialogue qu'on insere immediatement dans la liste.
	CairoDockDialog *pDialog = g_new0 (CairoDockDialog, 1);
	GtkWidget* pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);

        pDialog->pWidget = pWindow;

        gtk_window_stick(GTK_WINDOW(pWindow));
	gtk_window_set_keep_above(GTK_WINDOW(pWindow), TRUE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(pWindow), TRUE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pWindow), TRUE);
	gtk_window_set_gravity(GTK_WINDOW(pWindow), GDK_GRAVITY_STATIC);

/* 	gtk_window_set_type_hint (GTK_WINDOW (pWindow), GDK_WINDOW_TYPE_HINT_MENU); */
        //GTK_WIDGET_SET_FLAGS (pWindow, GTK_CAN_FOCUS);  // a priori inutile mais bon.

	cairo_dock_set_colormap_for_window(pWindow);

	gtk_widget_set_app_paintable(pWindow, TRUE);
	gtk_window_set_decorated(GTK_WINDOW(pWindow), FALSE);
	gtk_window_set_resizable(GTK_WINDOW(pWindow), TRUE);
	gtk_window_set_title(GTK_WINDOW(pWindow), "cairo-dock-dialog");

	gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	gtk_widget_show_all(pWindow);


	//\________________ On ajoute les widgets necessaires aux interactions avec l'utilisateur.
	pDialog->pUserData = data;

	//\________________ On connecte les signaux utiles.
/* 	g_signal_connect (G_OBJECT (pWindow), */
/* 		"configure-event", */
/* 		G_CALLBACK (applet_on_configure_dialog), */
/* 		pDialog); */
/* 	g_signal_connect (G_OBJECT (pWindow), */
/* 		"button-press-event", */
/* 		G_CALLBACK (applet_on_button_press_dialog), */
/* 		pDialog); */
/* 	g_signal_connect (G_OBJECT (pWindow), */
/* 		"button-release-event", */
/* 		G_CALLBACK (applet_on_button_press_dialog), */
/* 		pDialog); */
/* 	g_signal_connect (G_OBJECT (pWindow), */
/* 		"enter-notify-event", */
/* 		G_CALLBACK (applet_on_enter_dialog), */
/* 		pDialog); */
/* 	g_signal_connect (G_OBJECT (pWindow), */
/* 		"leave-notify-event", */
/* 		G_CALLBACK (applet_on_leave_dialog), */
/* 		pDialog); */

        pDialog->pInteractiveWidget = pInteractiveWidget;
	if (pInteractiveWidget != NULL)
	{
          GtkWidget *pFrame = gtk_frame_new(0);
          gtk_container_set_border_width  (GTK_CONTAINER(pFrame), 10);
          gtk_container_add(GTK_CONTAINER(pWindow), pFrame);
          gtk_container_add(GTK_CONTAINER(pFrame), pInteractiveWidget);
          g_signal_connect (G_OBJECT (pFrame), "expose-event",
                            G_CALLBACK (applet_on_expose_dialog), pDialog);

          //gtk_widget_set_size_request(pWindow, 0, 0);

          gtk_widget_show_all (pFrame);
	}
	return pDialog;
}


//TODO lol
static gint root_x = 0, root_y = 0;

void applet_hide_dialog(CairoDockDialog *pDialog)
{
  if (!pDialog)
    return ;
  gtk_window_get_position(GTK_WINDOW(pDialog->pWidget), &root_x, &root_y);
  gtk_widget_hide (pDialog->pWidget);
}

void applet_unhide_dialog (CairoDockDialog *pDialog)
{
  if (!pDialog)
    return;
  gtk_window_present(GTK_WINDOW(pDialog->pWidget));
  gtk_window_move(GTK_WINDOW(pDialog->pWidget), root_x, root_y);
}
