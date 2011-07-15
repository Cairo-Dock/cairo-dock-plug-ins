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

#include <stdlib.h>
#include <cairo-dock.h>

#include "powermanager-struct.h"
#include "powermanager-config.h"
#include "powermanager-common.h"
#include "powermanager-upower.h"
#include "powermanager-menu-functions.h"
#include "powermanager-draw.h"
#include "powermanager-init.h"


CD_APPLET_DEFINITION (N_("PowerManager"),
	2, 3, 0,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet displays the current state of your <b>laptop battery</b>: charge, time remaining, etc\n"
	"<b>Click</b> on the icon to have useful inforamtion,\n"
	"<b>Right-click</b> on the icon to hibernate or suspend the system."),
	"Necropotame (Adrien Pilleboue) and Fabounet")


static void _set_data_renderer (CairoDockModuleInstance *myApplet, gboolean bReload)
{
	CairoDataRendererAttribute *pRenderAttr = NULL;  // les attributs du data-renderer global.
	if (myConfig.iDisplayType == CD_POWERMANAGER_GAUGE)
	{
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		attr.cThemePath = myConfig.cGThemePath;
	}
	else if (myConfig.iDisplayType == CD_POWERMANAGER_GRAPH)
	{
		CairoGraphAttribute attr;  // les attributs du graphe.
		memset (&attr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "graph";
		pRenderAttr->iMemorySize = (myIcon->fWidth > 1 ? myIcon->fWidth : 32);  // fWidth peut etre <= 1 en mode desklet au chargement.
		attr.iType = myConfig.iGraphType;
		attr.iRadius = 10;
		attr.fHighColor = myConfig.fHigholor;
		attr.fLowColor = myConfig.fLowColor;
		memcpy (attr.fBackGroundColor, myConfig.fBgColor, 4*sizeof (double));
	}
	else if (myConfig.iDisplayType == CD_POWERMANAGER_ICONS)
	{
		
	}
	if (pRenderAttr != NULL)
	{
		if (myConfig.quickInfoType != 0)
		{
			pRenderAttr->bWriteValues = TRUE;
			pRenderAttr->format_value = (CairoDataRendererFormatValueFunc)cd_powermanager_format_value;
			pRenderAttr->pFormatData = myApplet;
		}
		
		if (! bReload)
			CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
		else
			CD_APPLET_RELOAD_MY_DATA_RENDERER (pRenderAttr);
	}
}


CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	_set_data_renderer (myApplet, FALSE);
	if (myConfig.cEmblemIconName == NULL)
		myData.pEmblem = CD_APPLET_MAKE_EMBLEM (MY_APPLET_SHARE_DATA_DIR"/charge.svg");
	else
		myData.pEmblem = CD_APPLET_MAKE_EMBLEM (myConfig.cEmblemIconName);
	cairo_dock_set_emblem_position (myData.pEmblem, CAIRO_DOCK_EMBLEM_MIDDLE);
	
	cd_powermanager_start ();
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	cairo_dock_discard_task (myData.pTask);
	
	if (myData.pUPowerClient != NULL)
	{
		g_object_unref (myData.pUPowerClient);
	}
	
	if (myData.checkLoop != 0)
	{
		g_source_remove (myData.checkLoop);
	}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	cairo_dock_free_emblem (myData.pEmblem);
	if (myConfig.cEmblemIconName == NULL)
		myData.pEmblem = CD_APPLET_MAKE_EMBLEM (MY_APPLET_SHARE_DATA_DIR"/charge.svg");
	else
		myData.pEmblem = CD_APPLET_MAKE_EMBLEM (myConfig.cEmblemIconName);
	cairo_dock_set_emblem_position (myData.pEmblem, CAIRO_DOCK_EMBLEM_MIDDLE);
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
		
		_set_data_renderer (myApplet, TRUE);
		
		cd_powermanager_change_loop_frequency (myConfig.iCheckInterval);
	}
	else
	{
		CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		if (myConfig.iDisplayType == CD_POWERMANAGER_GRAPH)
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
	}
	
	//\_______________ On redessine notre icone.
	if (myData.cBatteryStateFilePath || myData.pUPowerClient != NULL)
	{
		if (myConfig.iDisplayType == CD_POWERMANAGER_GAUGE || myConfig.iDisplayType == CD_POWERMANAGER_GRAPH)  // On recharge la jauge.
		{
			double fPercent = (double) myData.iPercentage / 100.;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			
			//Embleme sur notre icône
			//CD_APPLET_DRAW_EMBLEM ((myData.bOnBattery ? CAIRO_DOCK_EMBLEM_BLANK : CAIRO_DOCK_EMBLEM_CHARGE), CAIRO_DOCK_EMBLEM_MIDDLE);
			if (! myData.bOnBattery)
				CD_APPLET_DRAW_EMBLEM_ON_MY_ICON (myData.pEmblem);
		}
		else if (myConfig.iDisplayType == CD_POWERMANAGER_ICONS)
			cd_powermanager_draw_icon_with_effect (myData.bOnBattery);
		
		if (!myData.bOnBattery && myData.iPercentage < 100)
			myData.bAlerted = FALSE; //We will alert when battery charge reach 100%
		if (myData.bOnBattery)
		{
			if (myData.iPercentage > myConfig.lowBatteryValue)
				myData.bAlerted = FALSE; //We will alert when battery charge is under myConfig.lowBatteryValue
			
			if (myData.iPercentage > 4)
				myData.bCritical = FALSE; //We will alert when battery charge is critical (under 4%)
		}
		
		myData.iPrevPercentage = -1;
		myData.iPrevTime = -1;
		update_icon();

	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg");
	
CD_APPLET_RELOAD_END
