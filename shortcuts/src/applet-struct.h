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


#ifndef __APPLET_STRUCT__
#define  __APPLET_STRUCT__

#include <cairo-dock.h>

#define SHORTCUTS_DEFAULT_NAME "_shortcuts_"


typedef enum {
	CD_SHOW_NOTHING=0,
	CD_SHOW_FREE_SPACE,
	CD_SHOW_USED_SPACE,
	CD_SHOW_FREE_SPACE_PERCENT,
	CD_SHOW_USED_SPACE_PERCENT,
	CD_NB_SHOW
} CDDiskUsageDisplayType;

typedef enum {
	CD_DESKLET_SLIDE=0,
	CD_DESKLET_TREE,
	CD_DESKLET_NB_RENDERER
} CDDeskletRendererType;

struct _AppletConfig {
	gboolean bListDrives;
	gboolean bListNetwork;
	gboolean bListBookmarks;
	gboolean bUseSeparator;
	CDDiskUsageDisplayType iDisplayType;
	gint iCheckInterval;
	gboolean bDrawBar;
	gchar *cRenderer;
	CDDeskletRendererType iDeskletRendererType;
	} ;

typedef struct _CDDiskUsage {
	long long iPrevAvail;
	long long iAvail;
	long long iFree;
	long long iTotal;
	long long iUsed;
	int iType;
	} CDDiskUsage;

struct _AppletData {
	CairoDockTask *pTask;
	// shared memory for the loading task
	GList *pIconList;
	// end of shared memory
	
	gchar *cDisksURI;
	gchar *cNetworkURI;
	gchar *cBookmarksURI;
	
	CairoDockTask *pDiskTask;  // tache non threadee.
	} ;


#endif
