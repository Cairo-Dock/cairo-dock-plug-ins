/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the gauge-test applet\n made by Adrien Pilleboue for Cairo-Dock"))

void _gaugeTest_double_valeur(void)
{
	GList *pList = NULL;
	double *pValue;
	
	pValue = g_new (double, 1);
	*pValue = cairo_dock_show_value_and_wait("Valeur de la jauge",myIcon,myDock,0,1);
	pList = g_list_append (pList, pValue);
	pValue = g_new (double, 1);
	*pValue = cairo_dock_show_value_and_wait("Valeur de la jauge",myIcon,myDock,0,1);
	pList = g_list_append (pList, pValue);
	
	cairo_dock_render_gauge_multi_value(myDrawContext,myDock,myIcon,myData.pGauge,pList);
}

void _gaugeTest_triple_valeur(void)
{
	GList *pList = NULL;
	double *pValue;
	
	pValue = g_new (double, 1);
	*pValue = cairo_dock_show_value_and_wait("Valeur 1 de la jauge",myIcon,myDock,0,1);
	pList = g_list_append (pList, pValue);
	pValue = g_new (double, 1);
	*pValue = cairo_dock_show_value_and_wait("Valeur 2 de la jauge",myIcon,myDock,0,1);
	pList = g_list_append (pList, pValue);
	pValue = g_new (double, 1);
	*pValue = cairo_dock_show_value_and_wait("Valeur 3 de la jauge",myIcon,myDock,0,1);
	pList = g_list_append (pList, pValue);
	
	cairo_dock_render_gauge_multi_value(myDrawContext,myDock,myIcon,myData.pGauge,pList);
}

void _gaugeTest_heure(void)
{
	GList *pList = NULL;
	double *pValue;
	
	pValue = g_new (double, 1);
	*pValue = cairo_dock_show_value_and_wait("Heures",myIcon,myDock,0,23) / 23;
	pList = g_list_append (pList, pValue);
	pValue = g_new (double, 1);
	*pValue = cairo_dock_show_value_and_wait("Minutes",myIcon,myDock,0,59) / 59;
	pList = g_list_append (pList, pValue);
	pValue = g_new (double, 1);
	*pValue = cairo_dock_show_value_and_wait("Secondes",myIcon,myDock,0,59) / 59;
	pList = g_list_append (pList, pValue);
	
	cairo_dock_render_gauge_multi_value(myDrawContext,myDock,myIcon,myData.pGauge,pList);
}

CD_APPLET_ON_CLICK_BEGIN
	myData.gaugeValue = cairo_dock_show_value_and_wait("Valeur de la jauge",myIcon,myDock,myData.gaugeValue,1);
	cairo_dock_render_gauge(myDrawContext,myDock,myIcon,myData.pGauge,myData.gaugeValue);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cairo_dock_free_gauge(myData.pGauge);
	double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
	myData.pGauge = cairo_dock_load_gauge(myDrawContext,myConfig.cThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
	cairo_dock_render_gauge(myDrawContext,myDock,myIcon,myData.pGauge,0);
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("gauge-test", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
		
		CD_APPLET_ADD_IN_MENU ("Double valeur", _gaugeTest_double_valeur, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU ("Triple valeur", _gaugeTest_triple_valeur, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU ("Heure", _gaugeTest_heure, CD_APPLET_MY_MENU)
CD_APPLET_ON_BUILD_MENU_END
