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


#ifndef __SCREENSHOT_STRUCT__
#define  __SCREENSHOT_STRUCT__

#include <cairo-dock.h>


struct _AppletConfig {
	gchar *cShortkey;
	gchar *cDirPath;
	} ;

typedef struct {
	gint iDelay;
	gboolean bActiveWindow;
	gchar *cFolder;
	gchar *cName;
	} CDScreenshotOptions;
	
struct _AppletData {
	gchar *cCurrentUri;
	GList *pAppList;
	guint iSidTakeWithDelay;
	CDScreenshotOptions *pOptions;
	gint iCountdown;
	CairoDockImageBuffer *pCurrentImage;
	CairoDockImageBuffer *pOldImage;
	CairoKeyBinding *pKeyBinding;
	gboolean bFromShortkey;
	CairoDialog *pDialog;
	} ;

#endif
