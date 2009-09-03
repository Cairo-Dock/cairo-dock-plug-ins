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

#define COMPIZ_DEFAULT_NAME "_Compiz_"

typedef enum {
  COMPIZ_DEFAULT = 0,
  COMPIZ_BROKEN,
  COMPIZ_OTHER,
  COMPIZ_SETTING,
  COMPIZ_EMERALD,
  COMPIZ_RELOAD,
  COMPIZ_EXPOSITION,
  COMPIZ_WLAYER,
  COMPIZ_NB_ITEMS,
} compizIcons;

typedef enum {
	COMPIZ_NO_ACTION = 0,
	COMPIZ_SWITCH_WM,
	COMPIZ_LAYER,
	COMPIZ_EXPO,
	COMPIZ_SHOW_DESKTOP,
	COMPIZ_NB_ACTIONS
} compizAction;

typedef enum {
	DECORATOR_EMERALD = 0,
	DECORATOR_GTK,
	DECORATOR_KDE,
	DECORATOR_HELIODOR,
	DECORATOR_USER,
	COMPIZ_NB_DECORATORS
} compizDecorator;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean lBinding;
	gboolean iRendering;
	gboolean uLocalScreen;
	gboolean bSystemDecorator;
	gboolean bAutoReloadCompiz;
	gboolean bAutoReloadDecorator;
	///gboolean protectDecorator;
	gboolean forceConfig;
	///compizWM iWM;
	gchar *cRenderer;
	gchar *cUserWMCommand;
	gchar *cWindowDecorator;
	gchar *cUserImage[COMPIZ_NB_ITEMS];
	compizAction iActionOnMiddleClick;
	const gchar *cDecorators[COMPIZ_NB_DECORATORS];
	gboolean bStealTaskBarIcon;
	gboolean bScriptSubDock;
	gboolean bEmeraldIcon;
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	compizIcons iCompizIcon;
	gboolean bDecoratorIsRunning;
	gboolean bCompizIsRunning;
	gboolean bAcquisitionOK;
	CairoDockTask *pTask;
	gboolean bCompizRestarted;
	gboolean bDecoratorRestarted;
	int iCompizMajor, iCompizMinor, iCompizMicro;
} ;


#endif
