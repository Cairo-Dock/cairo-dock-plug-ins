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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct _CDWM {
	const gchar *cName;
	const gchar *cCommand;
	const gchar *cConfigTool;
	gboolean bIsAvailable;
	void (*activate_composite) (gboolean bActive);
} CDWM;

typedef enum {
	CD_COMPIZ,
	CD_KWIN,
	CD_XFWM,
	CD_METACITY,
	CD_CUSTOM_WMFB,
	CD_CUSTOM_WMC,
	NB_WM
} CDWMIndex;

#define NB_COMPOSITE_WM 4  // 4 of them can do composite
#define NB_FALLBACK_WM 3  // 3 of them can do without

typedef enum {
	CD_EDIT_CONFIG,
	CD_SHOW_DESKTOP,
	CD_EXPOSE_DESKTOPS,
	CD_EXPOSE_WINDOWS,
	CD_SHOW_WIDGET_LAYER,
	CD_NB_ACTIONS
	} CDWMAction;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar *cWmCompositor;
	gchar *cWmFallback;
	gchar *cIconCompositeON;
	gchar *cIconCompositeOFF;
	gboolean bAskBeforeSwitching;
	CDWMAction iActionOnMiddleClick;
	gchar *cShortCut;
} ;

typedef struct {
	gchar *which;
	gchar *ps;
	} CDSharedMemory;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	CairoDockTask *pTask;
	CDWM pWmList[NB_WM];
	gboolean bIsComposited;
	CDWM *wmc;
	CDWM *wmfb;
} ;


#endif
