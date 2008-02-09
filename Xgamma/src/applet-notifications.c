/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-xgamma.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (_D("This is the Xgamma applet\n made by Fabrice Rey for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	if (fGamma > 0)
	{
		fGamma = cairo_dock_show_value_and_wait (_D("Set up gamma :"), myIcon, myDock, fGamma, GAMMA_MAX);
		if (fGamma > 0)
		{
			fGamma = MAX (fGamma, GAMMA_MIN);
			
			myData.Xgamma.red = fGamma;
			myData.Xgamma.blue = fGamma;
			myData.Xgamma.green = fGamma;
			xgamma_set_gamma (&myData.Xgamma);
		}
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Xgamma", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END


static void _xgamma_apply_values (int iAnswer, GtkWidget *pWidget, gpointer data)
{
	if (iAnswer == GTK_RESPONSE_OK)
	{
		cd_message ("%s (ok)\n");
	}
	else
	{
		cd_message ("%s (cancel)\n", __func__);
		myData.Xgamma = myData.XoldGamma;
		xgamma_set_gamma (&myData.Xgamma);
	}
	cairo_dock_hide_dialog (myData.pDialog);  // apres cette fonction, ref --
}
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	if (fGamma > 0)
	{
		//\___________________ On construit notre widget si c'est la 1ere fois.
		if (myData.pWidget == NULL)
			xgamma_create_scales_widget ();
		
		//\___________________ On lui met les valeurs a jour, sans appeler les callbacks.
		g_signal_handler_block (myData.pGlobalScale, myData.iGloalScaleSignalID);
		g_signal_handler_block (myData.pRedScale, myData.iRedScaleSignalID);
		g_signal_handler_block (myData.pGreenScale, myData.iGreenScaleSignalID);
		g_signal_handler_block (myData.pBlueScale, myData.iBlueScaleSignalID);
		
		gtk_range_set_value (GTK_RANGE (myData.pGlobalScale), fGamma);
		gtk_range_set_value (GTK_RANGE (myData.pRedScale), myData.Xgamma.red);
		gtk_range_set_value (GTK_RANGE (myData.pGreenScale), myData.Xgamma.green);
		gtk_range_set_value (GTK_RANGE (myData.pBlueScale), myData.Xgamma.blue);
		myData.XoldGamma = myData.Xgamma;
		
		g_signal_handler_unblock (myData.pGlobalScale, myData.iGloalScaleSignalID);
		g_signal_handler_unblock (myData.pRedScale, myData.iRedScaleSignalID);
		g_signal_handler_unblock (myData.pGreenScale, myData.iGreenScaleSignalID);
		g_signal_handler_unblock (myData.pBlueScale, myData.iBlueScaleSignalID);
		
		//\___________________ On construit notre dialogue si necessaire.
		if (myData.pDialog == NULL)
		{
			myData.pDialog = cairo_dock_build_dialog (_D("Set up gamma :"),
				myIcon,
				myDock,
				NULL,
				myData.pWidget,
				GTK_BUTTONS_OK_CANCEL,
				(CairoDockActionOnAnswerFunc) _xgamma_apply_values,
				NULL,
				NULL);
		}
		else
		{
			cairo_dock_unhide_dialog (myData.pDialog);
		}
		cairo_dock_dialog_reference (myData.pDialog);  // on montre notre dialogue --> ref + +
	}
CD_APPLET_ON_MIDDLE_CLICK_END
