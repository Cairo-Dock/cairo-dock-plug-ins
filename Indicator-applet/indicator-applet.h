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


#ifndef __CD_INDICATOR_APPLET__
#define  __CD_INDICATOR_APPLET__

#include <cairo-dock.h>

#include <libindicator/indicator.h>
///#include <libindicator/indicator-object.h>
#include <libindicator/indicator-service-manager.h>

#if (GTK_MAJOR_VERSION < 3)
#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>
#else
#include <libdbusmenu-gtk3/menuitem.h>
#include <libdbusmenu-gtk3/menu.h>
#endif

typedef struct _CDAppletIndicator CDAppletIndicator;

struct _CDAppletIndicator {
	IndicatorServiceManager *service;
	DBusGProxy * pServiceProxy;
	DbusmenuGtkMenu *pMenu;
	guint iSidGetMenuOnce;
	guint iSidCheckIndicator;
	gchar *cStatusIcon;
	gboolean bConnected;
	const gchar *cBusName;
	const gchar *cServiceObject;
	const gchar *cServiceInterface;
	const gchar *cMenuObject;
	CairoDockModuleInstance *pApplet;
	void (*on_connect) (CairoDockModuleInstance *pApplet);
	void (*on_disconnect) (CairoDockModuleInstance *pApplet);
	void (*get_initial_values) (CairoDockModuleInstance *pApplet);
	void (*add_menu_handler) (DbusmenuGtkClient * client);
	};

#define INDICATOR_APPLET_DEFAULT_VERSION 1


CDAppletIndicator *cd_indicator_new (CairoDockModuleInstance *pApplet, const gchar *cBusName, const gchar *cObjectName, const gchar *cServiceInterface, const gchar *cMenuObject, int iVersion);

void cd_indicator_destroy (CDAppletIndicator *pIndicator);

void cd_indicator_set_icon (CDAppletIndicator *pIndicator, const gchar *cStatusIcon);

void cd_indicator_reload_icon (CDAppletIndicator *pIndicator);

gboolean cd_indicator_show_menu (CDAppletIndicator *pIndicator);


#endif
