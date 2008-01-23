/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <cairo-dock.h>
/* #include "cairo-dock-icons.h" */
/* #include "cairo-dock-dock-factory.h" */
/* #include "cairo-dock-load.h" */
/* #include "cairo-dock-draw.h" */
#include "cairo-applet.h"

/* static GSList *s_pDialogList = NULL; */
/* static GStaticRWLock s_mDialogsMutex = G_STATIC_RW_LOCK_INIT; */

extern CairoDock *g_pMainDock;
extern gint g_iScreenWidth[2], g_iScreenHeight[2];
extern gboolean g_bSticky;
extern gboolean g_bKeepAbove;
extern gboolean g_bAutoHide;
extern int g_iVisibleZoneWidth, g_iVisibleZoneHeight;

extern int g_iDockLineWidth;
extern int g_iDockRadius;
extern double g_fLineColor[4];

extern int g_iDialogButtonWidth;
extern int g_iDialogButtonHeight;
extern double g_fDialogColor[4];
extern int g_iDialogIconSize;
extern double g_fDialogTextColor[4];

extern int g_iDialogMessageSize;
extern gchar *g_cDialogMessagePolice;
extern int g_iDialogMessageWeight;
extern int g_iDialogMessageStyle;

static gboolean applet_on_enter_dialog (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDockDialog *pDialog)
{
  if (!pDialog)
    return FALSE;
  pDialog->bInside = TRUE;
  return FALSE;
}

static gboolean applet_on_leave_dialog (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDockDialog *pDialog)
{
  if (!pDialog)
    return FALSE;
  pDialog->bInside = FALSE;
  return FALSE;
}

static gboolean applet_on_button_press_dialog (GtkWidget* pWidget,
                                        GdkEventButton* pButton,
                                        CairoDockDialog *pDialog)
{
	return FALSE;
}

static gboolean applet_on_expose_dialog (GtkWidget *pWidget,
                                  GdkEventExpose *pExpose,
                                  CairoDockDialog *pDialog)
{
	g_print ("%s ()\n", __func__);
	if (!pDialog)
          return FALSE;

	double fLineWidth = g_iDockLineWidth;
	double fRadius = pDialog->fRadius;

	cairo_t *pCairoContext = gdk_cairo_create (pWidget->window);
	if (cairo_status (pCairoContext) != CAIRO_STATUS_SUCCESS, FALSE)
	{
		cairo_destroy (pCairoContext);
		return FALSE;
	}
	cairo_set_source_rgba (pCairoContext, 0., 0., 0., 0.);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	if (! pDialog->bBuildComplete)
	{
		cairo_destroy (pCairoContext);
		return FALSE;
	}


	cairo_save (pCairoContext);
	double fOffsetX = fRadius + fLineWidth / 2;
	double fOffsetY = (pDialog->bDirectionUp ? fLineWidth / 2 : pDialog->iHeight - .5*fLineWidth);
	int sens = (pDialog->bDirectionUp ? 1 : -1);
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	int iWidth = pDialog->iWidth;

/* 	GtkRequisition requisition; */
/*         gtk_widget_size_request(pDialog->, &requisition); */

	cairo_rel_line_to (pCairoContext, iWidth - (2 * fRadius + fLineWidth), 0);
	// Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens * (pDialog->iTextHeight + fLineWidth - fRadius * 2));
	// Coin bas droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, sens * fRadius,
		-fRadius, sens * fRadius);
	// La pointe.
/* 	double fDeltaMargin; */
/* 	if (pDialog->bRight) */
/* 	{ */
/* 		fDeltaMargin = pDialog->iAimedX - pDialog->iPositionX - fRadius - fLineWidth / 2;  // >= 0 */
/* 		//g_print ("fDeltaMargin : %.2f\n", fDeltaMargin); */
/* 		cairo_rel_line_to (pCairoContext, -iWidth + fDeltaMargin + fLineWidth + 2. * fRadius + APPLET_DIALOG_TIP_MARGIN + APPLET_DIALOG_TIP_BASE + APPLET_DIALOG_TIP_ROUNDING_MARGIN , 0); */
/* 		cairo_rel_curve_to (pCairoContext, */
/* 			0, 0, */
/* 			- APPLET_DIALOG_TIP_ROUNDING_MARGIN, 0, */
/* 			- (APPLET_DIALOG_TIP_ROUNDING_MARGIN + APPLET_DIALOG_TIP_MARGIN + APPLET_DIALOG_TIP_BASE), sens * pDialog->fTipHeight); */
/* 		cairo_rel_curve_to (pCairoContext, */
/* 			0, 0, */
/* 			APPLET_DIALOG_TIP_MARGIN, - sens * pDialog->fTipHeight, */
/* 			APPLET_DIALOG_TIP_MARGIN - APPLET_DIALOG_TIP_ROUNDING_MARGIN, - sens * pDialog->fTipHeight); */
/* 		cairo_rel_line_to (pCairoContext, - APPLET_DIALOG_TIP_MARGIN - fDeltaMargin + CAIRO_DOCK_DIALOG_TIP_ROUNDING_MARGIN, 0); */
/* 	} */
/* 	else */
/* 	{ */
/* 		fDeltaMargin = pDialog->iPositionX + pDialog->iWidth - fRadius - fLineWidth / 2 - pDialog->iAimedX;  // >= 0. */
/* 		//g_print ("fDeltaMargin : %.2f\n", fDeltaMargin); */
/* 		cairo_rel_line_to (pCairoContext, - (CAIRO_DOCK_DIALOG_TIP_MARGIN + fDeltaMargin) + CAIRO_DOCK_DIALOG_TIP_ROUNDING_MARGIN, 0); */
/* 		cairo_rel_curve_to (pCairoContext, */
/* 			0, 0, */
/* 			-CAIRO_DOCK_DIALOG_TIP_ROUNDING_MARGIN, 0, */
/* 			CAIRO_DOCK_DIALOG_TIP_MARGIN - CAIRO_DOCK_DIALOG_TIP_ROUNDING_MARGIN, sens * pDialog->fTipHeight); */
/* 		cairo_rel_curve_to (pCairoContext, */
/* 			0, 0, */
/* 			- (CAIRO_DOCK_DIALOG_TIP_MARGIN + CAIRO_DOCK_DIALOG_TIP_BASE), - sens * pDialog->fTipHeight, */
/* 			- (CAIRO_DOCK_DIALOG_TIP_MARGIN + CAIRO_DOCK_DIALOG_TIP_BASE) - CAIRO_DOCK_DIALOG_TIP_ROUNDING_MARGIN, - sens * pDialog->fTipHeight); */
/* 		cairo_rel_line_to (pCairoContext, -iWidth + fDeltaMargin + fLineWidth + 2 * fRadius + CAIRO_DOCK_DIALOG_TIP_MARGIN + CAIRO_DOCK_DIALOG_TIP_BASE + CAIRO_DOCK_DIALOG_TIP_ROUNDING_MARGIN, 0); */
/* 	} */

	// Coin bas gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-fRadius, 0,
		-fRadius, -sens * fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens * (- pDialog->iTextHeight - fLineWidth + fRadius * 2));
	// Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);

	cairo_save (pCairoContext);
	cairo_set_source_rgba (pCairoContext, g_fDialogColor[0], g_fDialogColor[1], g_fDialogColor[2], g_fDialogColor[3]);
	cairo_fill_preserve (pCairoContext);
	cairo_restore (pCairoContext);

	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	cairo_stroke (pCairoContext);
	cairo_restore (pCairoContext);  // retour au contexte initial.

/* 	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER); */
/* 	cairo_set_source_surface (pCairoContext, pDialog->pTextBuffer, fOffsetX + CAIRO_DOCK_DIALOG_TEXT_MARGIN, fOffsetY + fLineWidth / 2 + CAIRO_DOCK_DIALOG_TEXT_MARGIN - (pDialog->bDirectionUp ? 0 : fLineWidth + pDialog->iTextHeight)); */
/* 	cairo_paint (pCairoContext); */

/* 	if (pDialog->iButtonsType != GTK_BUTTONS_NONE) */
/* 	{ */
/* 		GtkRequisition requisition = {0, 0}; */
/* 		if (pDialog->pInteractiveWidget != NULL) */
/* 			gtk_widget_size_request (pDialog->pInteractiveWidget, &requisition); */
/* 		//g_print (" pInteractiveWidget : %dx%d\n", requisition.width, requisition.height); */

/* 		int iButtonY = fLineWidth + pDialog->iMessageHeight + requisition.height + APPLET_DIALOG_VGAP; */
/* 		if (! pDialog->bDirectionUp) */
/* 			iButtonY +=  pDialog->iHeight - pDialog->iTextHeight - g_iDockLineWidth; */

/* 		cairo_set_source_surface (pCairoContext, s_pButtonOkSurface, .5*pDialog->iWidth - g_iDialogButtonWidth - .5*APPLET_DIALOG_BUTTON_GAP + pDialog->iButtonOkOffset, iButtonY + pDialog->iButtonOkOffset); */
/* 		cairo_paint (pCairoContext); */

/* 		cairo_set_source_surface (pCairoContext, s_pButtonCancelSurface, .5*pDialog->iWidth + .5*CAIRO_DOCK_DIALOG_BUTTON_GAP + pDialog->iButtonCancelOffset, iButtonY + pDialog->iButtonCancelOffset); */
/* 		cairo_paint (pCairoContext); */
/* 	} */

	cairo_destroy (pCairoContext);
	return FALSE;
}

static gboolean applet_on_configure_dialog (GtkWidget* pWidget,
	GdkEventConfigure* pEvent,
	CairoDockDialog *pDialog)
{
	g_print ("%s (%dx%d)\n", __func__, pEvent->width, pEvent->height);
	if (!pDialog)
		return FALSE;

	pDialog->bBuildComplete = (pDialog->iWidth == pEvent->width && pDialog->iHeight == pEvent->height);  // pour empecher un clignotement intempestif lors de la creation de la fenetre, on la dessine en transparent lorsqu'elle n'est pas encore completement finie.

	return FALSE;
}


void applet_isolate_dialog (CairoDockDialog *pDialog)
{
	if (pDialog == NULL)
		return ;

	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_expose_dialog, NULL);
	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_button_press_dialog, NULL);
	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_configure_dialog, NULL);
	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_enter_dialog, NULL);
	g_signal_handlers_disconnect_by_func (pDialog->pWidget, applet_on_leave_dialog, NULL);
}


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
	g_signal_connect (G_OBJECT (pWindow),
		"expose-event",
		G_CALLBACK (applet_on_expose_dialog),
 		pDialog);
/* 	g_signal_connect (G_OBJECT (pWindow), */
/* 		"configure-event", */
/* 		G_CALLBACK (applet_on_configure_dialog), */
/* 		pDialog); */
	g_signal_connect (G_OBJECT (pWindow),
		"button-press-event",
		G_CALLBACK (applet_on_button_press_dialog),
		pDialog);
	g_signal_connect (G_OBJECT (pWindow),
		"button-release-event",
		G_CALLBACK (applet_on_button_press_dialog),
		pDialog);
	g_signal_connect (G_OBJECT (pWindow),
		"enter-notify-event",
		G_CALLBACK (applet_on_enter_dialog),
		pDialog);
	g_signal_connect (G_OBJECT (pWindow),
		"leave-notify-event",
		G_CALLBACK (applet_on_leave_dialog),
		pDialog);

/* 	GtkRequisition requisition; */
        pDialog->pInteractiveWidget = pInteractiveWidget;
	if (pInteractiveWidget != NULL)
	{
          gtk_container_add (GTK_CONTAINER (pWindow), pInteractiveWidget);
          gtk_widget_set_size_request (pWindow, 640, 480);

/*           gtk_widget_size_request (pInteractiveWidget, &requisition); */
/* void                gtk_window_get_size                 (GtkWindow *window, */
/*                                                          gint *width, */
/*                                                          gint *height); */
/*           g_print (" pInteractiveWidget : %dx%d\n", requisition.width, requisition.height); */

          pDialog->iWidth = 800;
          gtk_widget_show_all (pInteractiveWidget);
	}
	return pDialog;
}


void applet_hide_dialog(CairoDockDialog *pDialog)
{
  if (!pDialog)
    return ;
  gtk_widget_hide (pDialog->pWidget);
}

void applet_unhide_dialog (CairoDockDialog *pDialog)
{
  if (!pDialog)
    return;
  gtk_window_present(GTK_WINDOW(pDialog->pWidget));
}
