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

#include <gdk/gdkx.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-xgamma.h"

static gboolean s_bUseXf86VidMode = FALSE;

static gboolean _xf86vidmode_supported (void)
{
	static gboolean s_bXf86VidModeChecked = FALSE;
	if (s_bXf86VidModeChecked)
		return s_bUseXf86VidMode;

	int event_base, error_base;
	Display *dpy = gdk_x11_get_default_xdisplay ();
	if (! XF86VidModeQueryExtension (dpy, &event_base, &error_base))  // on regarde si le serveur X supporte l'extension.
	{
		cd_warning ("XF86VidMode extension not available.");
		s_bUseXf86VidMode = FALSE;
	}
	else
		s_bUseXf86VidMode = TRUE;
	s_bXf86VidModeChecked = TRUE;
	return s_bUseXf86VidMode;
}

static inline double _gamma_to_percent (double fGamma)
{
	if (fGamma < GAMMA_MIN)
		fGamma = GAMMA_MIN;
	if (fGamma > GAMMA_MAX)
		fGamma = GAMMA_MAX;
	return 100. * (fGamma - GAMMA_MIN) / (GAMMA_MAX - GAMMA_MIN);
}

static inline double _percent_to_gamma (double fGammaPercent)
{
	if (fGammaPercent < 0)
		fGammaPercent = 0;
	if (fGammaPercent > 100)
		fGammaPercent = 100;
	return GAMMA_MIN + fGammaPercent / 100. * (GAMMA_MAX - GAMMA_MIN);
}

void xgamma_add_gamma (XF86VidModeGamma *pGamma, gint iNbSteps)
{
	if (iNbSteps == 0)
		return;
	double fGamma = xgamma_get_gamma (pGamma);
	double fGammaPercent = _gamma_to_percent (fGamma);
	fGammaPercent += iNbSteps * myConfig.iScrollVariation;
	double fNewGamma = _percent_to_gamma (fGammaPercent);
	double f = fNewGamma / fGamma;
	myData.Xgamma.red *= f;
	myData.Xgamma.green *= f;
	myData.Xgamma.blue *= f;
	
	xgamma_set_gamma (&myData.Xgamma);
}

double xgamma_get_gamma (XF86VidModeGamma *pGamma)
{
	g_return_val_if_fail (pGamma != NULL, 1);
	Display *dpy = gdk_x11_get_default_xdisplay ();
	
	g_return_val_if_fail (_xf86vidmode_supported (), 1.);
	if (!XF86VidModeGetGamma (dpy, DefaultScreen (dpy), pGamma))
	{
		cd_warning ("Xgamma : unable to query gamma correction");
		return 1.;
	}
	double fGamma = (pGamma->red + pGamma->blue + pGamma->green) / 3;
	cd_debug ("Gamma: %f, %f, %f, %f",
		pGamma->red, pGamma->blue, pGamma->green, fGamma);
	return fGamma;
}


void xgamma_set_gamma (XF86VidModeGamma *pGamma)
{
	g_return_if_fail (pGamma != NULL);
	Display *dpy = gdk_x11_get_default_xdisplay ();
	
	g_return_if_fail (_xf86vidmode_supported ());
	if (!XF86VidModeSetGamma(dpy, DefaultScreen (dpy), pGamma))
	{
		cd_warning ("Xgamma : unable to set gamma correction");
	}
	else
	{
		if (myConfig.cDefaultTitle == NULL)
		{
			double fGamma = (pGamma->red + pGamma->blue + pGamma->green) / 3;
			cd_gamma_display_gamma_on_label (fGamma);
		}
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
static GtkWidget *_xgamma_add_channel_widget (GtkWidget *pInteractiveWidget, const gchar *cLabel, const gchar *cColor, int iChannelNumber, guint *iSignalID, double fChannelGamma)
{
	GtkWidget *pLabel = gtk_label_new (NULL);
	if (cColor)
	{
		gchar *cText = g_strdup_printf ("<span color=\"%s\">%s</span>", cColor, cLabel);
		gtk_label_set_markup (GTK_LABEL (pLabel), cText);
		g_free (cText);
	}
	else
	{
		gtk_label_set_text (GTK_LABEL (pLabel), cLabel);
		gldi_dialog_set_widget_text_color (pLabel); // default colour
	}

	gtk_grid_attach (GTK_GRID (pInteractiveWidget),
		pLabel,
		1,
		iChannelNumber+1,
		1,
		1);

	GtkWidget *pHScale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, GAMMA_MIN, GAMMA_MAX, .02);
	gtk_scale_set_digits (GTK_SCALE (pHScale), 2);
	gtk_range_set_value (GTK_RANGE (pHScale), fChannelGamma);
	g_object_set (pHScale, "width-request", 150, NULL);
	
	*iSignalID = g_signal_connect (G_OBJECT (pHScale),
		"value-changed",
		G_CALLBACK (on_scale_value_changed),
		GINT_TO_POINTER (iChannelNumber));
	gtk_grid_attach (GTK_GRID (pInteractiveWidget),
		pHScale,
		2,
		iChannelNumber+1,
		1,
		1);

	return pHScale;
}
void xgamma_create_scales_widget (double fGamma, XF86VidModeGamma *pGamma)
{
	myData.pWidget = gtk_grid_new ();

	myData.pGlobalScale = _xgamma_add_channel_widget (myData.pWidget, D_("Gamma :"), NULL, 0, &myData.iGloalScaleSignalID, fGamma);

	myData.pRedScale = _xgamma_add_channel_widget (myData.pWidget, D_("Red :"), "red", 1, &myData.iRedScaleSignalID, pGamma->red);

	myData.pGreenScale = _xgamma_add_channel_widget (myData.pWidget, D_("Green :"), "green", 2, &myData.iGreenScaleSignalID, pGamma->green);

	myData.pBlueScale = _xgamma_add_channel_widget (myData.pWidget, D_("Blue :"), "blue", 3, &myData.iBlueScaleSignalID, pGamma->blue);

	gtk_widget_show_all (myData.pWidget);
}


static void _xgamma_apply_values (int iClickedButton, GtkWidget *pWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		cd_message ("%s (ok)", __func__);
	}
	else
	{
		cd_message ("%s (cancel)", __func__);
		myData.Xgamma = myData.XoldGamma;
		xgamma_set_gamma (&myData.Xgamma);
	}
	gldi_dialog_hide (myData.pDialog);
	gldi_object_ref (GLDI_OBJECT(myData.pDialog));  // pour garder notre dialogue en vie.
	
}
CairoDialog *xgamma_build_dialog (void)
{
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	attr.cText = D_("Set up gamma:");
	attr.pInteractiveWidget = myData.pWidget;
	const gchar *cButtons[3] = {"ok", "cancel", NULL};
	attr.cButtonsImage = cButtons;
	attr.pActionFunc = (CairoDockActionOnAnswerFunc) _xgamma_apply_values;
	attr.pUserData = myApplet;
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;
	return gldi_dialog_new (&attr);
}

void xgamma_build_and_show_widget (void)
{
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	g_return_if_fail (fGamma >= 0);
	
	xgamma_create_scales_widget (fGamma, &myData.Xgamma);
	
	if (myDock)
	{
		myData.pDialog = xgamma_build_dialog ();
	}
	else
	{
		gldi_desklet_add_interactive_widget (myDesklet, myData.pWidget);
		CD_APPLET_SET_DESKLET_RENDERER (NULL);  // pour empecher le clignotement du au double-buffer.
		CD_APPLET_SET_STATIC_DESKLET;
	}
}

static void on_scale_value_changed_simple (GtkRange *range, gpointer data)
{
	double fGammaPercent = gtk_range_get_value (GTK_RANGE (range));
	double fGamma = _percent_to_gamma (fGammaPercent);
	
	myData.Xgamma.red = fGamma;
	myData.Xgamma.blue = fGamma;
	myData.Xgamma.green = fGamma;
	xgamma_set_gamma (&myData.Xgamma);
}
static void _xgamma_apply_value_simple (int iClickedButton, GtkWidget *pWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		cd_message ("%s (ok)", __func__);
	}
	else
	{
		cd_message ("%s (cancel)", __func__);
		myData.Xgamma = myData.XoldGamma;
		xgamma_set_gamma (&myData.Xgamma);
	}
}
CairoDialog *xgamma_build_dialog_simple (void)
{
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	g_return_val_if_fail (fGamma >= 0, NULL);
	double fGammaPercent = _gamma_to_percent (fGamma);
	myData.XoldGamma = myData.Xgamma;
	
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));

	GtkWidget *pHScale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 100., 1.);
	gtk_scale_set_digits (GTK_SCALE (pHScale), 0);
	gtk_range_set_value (GTK_RANGE (pHScale), fGammaPercent);
	g_object_set (pHScale, "width-request", 150, NULL);
	g_signal_connect (G_OBJECT (pHScale),
		"value-changed",
		G_CALLBACK (on_scale_value_changed_simple),
		NULL);
	gldi_dialog_set_widget_text_color (pHScale);
	
	attr.cText = D_("Set up gamma:");
	attr.pInteractiveWidget = pHScale;
	const gchar *cButtons[3] = {"ok", "cancel", NULL};
	attr.cButtonsImage = cButtons;
	attr.pActionFunc = (CairoDockActionOnAnswerFunc) _xgamma_apply_value_simple;
	attr.pUserData = myApplet;
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;
	return gldi_dialog_new (&attr);
}


void cd_gamma_display_gamma_on_label (double fGamma)
{
	double fGammaPercent = _gamma_to_percent (fGamma);
	gchar *cLabel = g_strdup_printf ("%s: %d%%", D_("Luminosity"), (int)fGammaPercent);
	gldi_icon_set_name (myIcon, cLabel);
	g_free (cLabel);
}
