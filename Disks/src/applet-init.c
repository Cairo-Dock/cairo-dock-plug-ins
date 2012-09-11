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

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-disks.h"


CD_APPLET_DEFINITION (N_("Disks"),
	2, 0, 5,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("<b><i>Monitors disks speed and space</i></b>.\n"
	"\n"
	"This applet show your disks informations. You can activate both options at once, but they're better separated in 2 or more instances of the applet.\n"
	"Using both in the same applet can cause problems with the speed display not scaling down.\n"
	"\n"
	"<b>Disk space</b> :  Show disk transfer rates. Up to 10 partitions\n"
	"  You need to configure the mount point of each monitored partition like / or /home\n"
	"  Better seen in a gauge, with 30 or 60s refresh rate and really short or no transition delay.\n"
	"\n"
	"<b>Disk speed</b> :  Show disk transfer rates. Up to 5 disks\n"
	"  You need to configure the name of each monitored disk with his device name like sda or sdb\n"
	"  Better seen in a graph, with 2 or 3s refresh rate and no delay between transitions.\n"
	"\n"
	"Second release, a lot more need to be done :\n"
	" <u>Free space :</u>\n"
	" - Popup with detailed informations.\n"
	" - Editable labels ?\n"
	" <u>Speed :</u>\n"
	" - All disks option.\n"
	" - Pop up showing disks info to know what to add in the list.\n"
	" - Editable labels ?\n"
	" - Get real block size for disks\n"
	" <u>Global</u>\n"
	" - Find actions : left, middle click, drop and wheel.\n"
	" - Fill menu with actions.\n"
	),
	"SQP");


static void _set_data_renderer (CairoDockModuleInstance *myApplet)
{
	CairoDataRendererAttribute *pRenderAttr = NULL;  // attributes for the global data-renderer.
	CairoGaugeAttribute aGaugeAttr;  // gauge attributes.
	CairoGraphAttribute aGraphAttr;  // graph attributes.
	if (myConfig.iDisplayType == CD_DISKS_GAUGE)
	{
		memset (&aGaugeAttr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGaugeAttr);
		pRenderAttr->cModelName = "gauge";
		pRenderAttr->iRotateTheme = myConfig.iRotateTheme;
		aGaugeAttr.cThemePath = myConfig.cGThemePath;
	}
	else if (myConfig.iDisplayType == CD_DISKS_GRAPH)
	{
		memset (&aGraphAttr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGraphAttr);
		pRenderAttr->cModelName = "graph";
		pRenderAttr->iMemorySize = (myIcon->fWidth > 1 ? myIcon->fWidth : 32);  // fWidth peut etre <= 1 en mode desklet au chargement.
		aGraphAttr.iType = myConfig.iGraphType;
		aGraphAttr.bMixGraphs = myConfig.bMixGraph;
		double fHighColor[CD_DISKS_NB_MAX_VALUES*3];
		double fLowColor[CD_DISKS_NB_MAX_VALUES*3];
		gsize i;
		for (i = 0; i < myData.iNumberDisks * 2; i+=2)
		{
			memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));
			memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));
			memcpy (&fHighColor[3*i+3], myConfig.fHigholor, 3*sizeof (double));
			memcpy (&fLowColor[3*i+3], myConfig.fLowColor, 3*sizeof (double));
		}
		aGraphAttr.fHighColor = fHighColor;
		aGraphAttr.fLowColor = fLowColor;
		memcpy (aGraphAttr.fBackGroundColor, myConfig.fBgColor, 4*sizeof (double));
	}

	const gchar *labels[CD_DISKS_NB_MAX_VALUES] = {};
	
	if (myConfig.iNumberParts > 0)
	{
		gsize i;
		for (i = 0; i < myConfig.iNumberParts; i++)
		{
		/// Ca non plus je suppose :)
			double *pSize;
			pSize = g_new0 (double, 1);
			myData.lParts = g_list_append (myData.lParts, pSize);
			labels[i] = myConfig.cParts[i];
		}
	}

	if (myData.iNumberDisks > 0)
	{
		gsize i;
		for (i = 0; i < myData.iNumberDisks; i++)
		{
			/// Ne pas faire ca ici !...
			CDDiskSpeedData *pSpeed;
			pSpeed = g_new0 (CDDiskSpeedData, 1);
			myData.lDisks = g_list_append (myData.lDisks, pSpeed);
			pSpeed->cName = g_strdup (myConfig.cDisks[i]);
			//~ pSpeed->uID = i;
			labels[2 * i] = pSpeed->cName;
		}
	}

	pRenderAttr->cLabels = (gchar **)labels;
	pRenderAttr->iLatencyTime = myConfig.iCheckInterval * 1000 * myConfig.fSmoothFactor;
	pRenderAttr->iNbValues = myConfig.iNumberParts + 2 * myData.iNumberDisks;
	
	/// Problem here : should be FALSE for size and TRUE for speed.
	/// This version force FALSE when size is monitored, so applets using both monitors
	/// could have some display problems on speed display.
	pRenderAttr->bUpdateMinMax = !(myConfig.iNumberParts > 0);

	if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
	{
		pRenderAttr->bWriteValues = TRUE;
		pRenderAttr->format_value = (CairoDataRendererFormatValueFunc)cd_disks_format_value_on_icon;
		pRenderAttr->pFormatData = myApplet;
	}

	CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
}

CD_APPLET_INIT_BEGIN
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	// Initialisation du rendu.
	myData.iNumberDisks = myConfig.iNumberDisks;
	_set_data_renderer (myApplet);
	
	// Initialisation de la tache periodique de mesure.
	myData.pClock = g_timer_new ();
	myData.pPeriodicTask = cairo_dock_new_task (myConfig.iCheckInterval,
		(CairoDockGetDataAsyncFunc) cd_disks_get_data,
		(CairoDockUpdateSyncFunc) cd_disks_update_from_data,
		myApplet);
	//~ myData.bAcquisitionOK = TRUE;
	cairo_dock_launch_task (myData.pPeriodicTask);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	// CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	// CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
		
		cd_disks_reset_parts_list (myApplet);
		cd_disks_reset_disks_list (myApplet);
		
		_set_data_renderer (myApplet);
		
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_ICON)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		}
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_LABEL)
		{
			if (myConfig.defaultTitle) // has another default name
				CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
			else
				CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		}
		
		cairo_dock_relaunch_task_immediately (myData.pPeriodicTask, myConfig.iCheckInterval);
	}
	else {  // on redessine juste l'icone.
		//CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		if (myConfig.iDisplayType == CD_DISKS_GRAPH)
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		
		/**if (! cairo_dock_task_is_running (myData.pPeriodicTask))
			cd_disks_update_from_data (myApplet);*/
	}
CD_APPLET_RELOAD_END
