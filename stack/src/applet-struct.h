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

#define CD_STACK_DEFAULT_NAME "Stack"

typedef enum {
	CD_STACK_SORT_BY_NAME=0,
	CD_STACK_SORT_BY_DATE,
	CD_STACK_SORT_BY_TYPE,
	CD_STACK_SORT_MANUALLY,
	CD_STACK_NB_SORT
	} CDStackSortType;


typedef enum {
	CD_DESKLET_SLIDE=0,
	CD_DESKLET_TREE,
	CD_DESKLET_NB_RENDERER
} CDDeskletRendererType;

typedef struct _CDHtmlLink
{
	CairoDockModuleInstance *pApplet;
	gchar *cURL;
	gchar *cTitle;
	gchar *cFaviconPath;
	gchar *cConfFilePath;
	CairoDockTask *pTask;
} CDHtmlLink;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar **cMimeTypes;
	gchar *cRenderer;
	gboolean bFilter;
	CDStackSortType iSortType;
	gchar *cTextIcon;
	gchar *cUrlIcon;
	gboolean bSelectionClipBoard;
	gchar *cStackDir;
	CDDeskletRendererType iDeskletRendererType;
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gint no_data;
	GList *pGetPageTaskList;
} ;


#endif
