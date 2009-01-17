/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-xgamma.h"


double xgamma_get_gamma (XF86VidModeGamma *pGamma)
{
	g_return_val_if_fail (pGamma != NULL, 1);
	const Display *dpy = cairo_dock_get_Xdisplay ();
	
	g_return_val_if_fail (XF86VidModeGetGamma != NULL, 1.);
	if (!XF86VidModeGetGamma (dpy, DefaultScreen (dpy), pGamma))
	{
		cd_warning ("Xgamma : unable to query gamma correction");
		return 1.;
	}
	return (pGamma->red + pGamma->blue + pGamma->green) / 3;
}


void xgamma_set_gamma (XF86VidModeGamma *pGamma)
{
	g_return_if_fail (pGamma != NULL);
	const Display *dpy = cairo_dock_get_Xdisplay ();
	
	g_return_if_fail (XF86VidModeSetGamma != NULL);
	if (!XF86VidModeSetGamma(dpy, DefaultScreen (dpy), pGamma))
	{
		cd_warning ("Xgamma : unable to set gamma correction");
	}
}



static void on_scale_value_changed (GtkRange *range, gpointer data)
{
	int iChannelNumber = GPOINTER_TO_INT (data);
	cd_message ("%s (%d, %.2f)", __func__, iChannelNumber, gtk_range_get_value (GTK_RANGE (range)));
	
	switch (iChannelNumber)
	{
		case 0 :
		{
			double fOldGamma = (myData.Xgamma.red + myData.Xgamma.blue + myData.Xgamma.green) / 3;
			double fNewGamma = gtk_range_get_value (GTK_RANGE (range));
			double fDeltaGamma = fNewGamma - fOldGamma;
			
			myData.Xgamma.red += fDeltaGamma;
			myData.Xgamma.red = MAX (GAMMA_MIN, MIN (GAMMA_MAX, myData.Xgamma.red));
			myData.Xgamma.green += fDeltaGamma;
			myData.Xgamma.green = MAX (GAMMA_MIN, MIN (GAMMA_MAX, myData.Xgamma.green));
			myData.Xgamma.blue += fDeltaGamma;
			myData.Xgamma.blue = MAX (GAMMA_MIN, MIN (GAMMA_MAX, myData.Xgamma.blue));
			
			g_signal_handler_block (myData.pRedScale, myData.iRedScaleSignalID);
			g_signal_handler_block (myData.pGreenScale, myData.iGreenScaleSignalID);
			g_signal_handler_block (myData.pBlueScale, myData.iBlueScaleSignalID);
			
			gtk_range_set_value (GTK_RANGE (myData.pRedScale), myData.Xgamma.red);
			gtk_range_set_value (GTK_RANGE (myData.pGreenScale), myData.Xgamma.green);
			gtk_range_set_value (GTK_RANGE (myData.pBlueScale), myData.Xgamma.blue);
			
			g_signal_handler_unblock (myData.pRedScale, myData.iRedScaleSignalID);
			g_signal_handler_unblock (myData.pGreenScale, myData.iGreenScaleSignalID);
			g_signal_handler_unblock (myData.pBlueScale, myData.iBlueScaleSignalID);
		}
		break ;
		
		case 1 :
			myData.Xgamma.red = gtk_range_get_value (GTK_RANGE (range));
		break ;
		
		case 2 :
			myData.Xgamma.blue = gtk_range_get_value (GTK_RANGE (range));
		break ;
		
		case 3 :
			myData.Xgamma.green = gtk_range_get_value (GTK_RANGE (range));
		break ;
		
	}
	xgamma_set_gamma (&myData.Xgamma);
}
static GtkWidget *_xgamma_add_channel_widget (GtkWidget *pInteractiveWidget, gchar *cLabel, int iChannelNumber, guint *iSignalID, double fChannelGamma)
{
	GtkWidget *pLabel = gtk_label_new (cLabel);
	gtk_table_attach_defaults (GTK_TABLE (pInteractiveWidget), pLabel, 0, 1, iChannelNumber, iChannelNumber+1);
	
	GtkWidget *pHScale = gtk_hscale_new_with_range (GAMMA_MIN, GAMMA_MAX, .02);
	gtk_scale_set_digits (GTK_SCALE (pHScale), 2);
	gtk_range_set_value (GTK_RANGE (pHScale), fChannelGamma);
	gtk_widget_set (pHScale, "width-request", 150, NULL);
	
	*iSignalID = g_signal_connect (G_OBJECT (pHScale),
		"value-changed",
		G_CALLBACK (on_scale_value_changed),
		GINT_TO_POINTER (iChannelNumber));
	gtk_table_attach_defaults (GTK_TABLE (pInteractiveWidget), pHScale, 1, 2, iChannelNumber, iChannelNumber+1);
	
	return pHScale;
}
void xgamma_create_scales_widget (double fGamma, XF86VidModeGamma *pGamma)
{
	myData.pWidget = gtk_table_new (4, 2, FALSE);
	
	myData.pGlobalScale = _xgamma_add_channel_widget (myData.pWidget, "Gamma :", 0, &myData.iGloalScaleSignalID, fGamma);
	
	myData.pRedScale = _xgamma_add_channel_widget (myData.pWidget, "Red :", 1, &myData.iRedScaleSignalID, pGamma->red);
	
	myData.pGreenScale = _xgamma_add_channel_widget (myData.pWidget, "Green :", 2, &myData.iGreenScaleSignalID, pGamma->green);
	
	myData.pBlueScale = _xgamma_add_channel_widget (myData.pWidget, "Blue :", 3, &myData.iBlueScaleSignalID, pGamma->blue);
	
	gtk_widget_show_all (myData.pWidget);
}


void xgamma_apply_values (int iAnswer, GtkWidget *pWidget, gpointer data)
{
	if (iAnswer == GTK_RESPONSE_OK)
	{
		cd_message ("%s (ok)");
	}
	else
	{
		cd_message ("%s (cancel)", __func__);
		myData.Xgamma = myData.XoldGamma;
		xgamma_set_gamma (&myData.Xgamma);
	}
	cairo_dock_hide_dialog (myData.pDialog);  // apres cette fonction, ref --
	cairo_dock_dialog_reference (myData.pDialog);
	
}

CairoDialog *xgamma_build_dialog (void)
{
	CairoDialogAttribute attr;
	memset (&attr, 0, sizeof (CairoDialogAttribute));
	attr.cText = D_("Set up gamma :");
	attr.pInteractiveWidget = myData.pWidget;
	attr.iButtonsType = GTK_BUTTONS_OK_CANCEL;
	attr.pActionFunc = (CairoDockActionOnAnswerFunc) xgamma_apply_values;
	attr.pUserData = myApplet;
	return cairo_dock_build_dialog (&attr, myIcon, myContainer);
}

void xgamma_build_and_show_widget (void)
{
	cd_message ("");
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	g_return_if_fail (fGamma > 0);
	
	xgamma_create_scales_widget (fGamma, &myData.Xgamma);
	
	if (myDock)
	{
		myData.pDialog = xgamma_build_dialog ();
		/*myData.pDialog = cairo_dock_build_dialog (D_("Set up gamma :"),
			myIcon,
			myContainer,
			NULL,
			myData.pWidget,
			GTK_BUTTONS_OK_CANCEL,
			(CairoDockActionOnAnswerFunc) xgamma_apply_values,
			NULL,
			NULL);*/
	}
	else
	{
		cairo_dock_add_interactive_widget_to_desklet (myData.pWidget, myDesklet);
		cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
}
