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
#include <zeitgeist.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar *cShortkey;
	gint iNbResultsMax;
	gint iNbRelatedFilesMax;
	gboolean b24Mode;
	} ;

typedef void (* CDOnGetEventsFunc) (ZeitgeistResultSet *pEvents, gpointer data);
typedef void (* CDOnDeleteEventsFunc) (int iNbEvents, gpointer data);
typedef enum {
	CD_EVENT_ALL,
	CD_EVENT_APPLICATION,
	CD_EVENT_DOCUMENT,
	///CD_EVENT_FOLDER,  // marche pas avec zeitgeist, et puis n'est pas super utile.
	CD_EVENT_IMAGE,
	CD_EVENT_AUDIO,
	CD_EVENT_VIDEO,
	CD_EVENT_WEB,
	CD_EVENT_OTHER,
	CD_EVENT_TOP_RESULTS,
	CD_NB_EVENT_TYPES
	} CDEventType;

typedef enum {
	CD_MODEL_NAME,
	CD_MODEL_URI,
	CD_MODEL_PATH,
	CD_MODEL_ICON,
	CD_MODEL_DATE,
	CD_MODEL_ID,
	CD_MODEL_NB_COLUMNS
	} CDModelColumn;
//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	ZeitgeistEvent **pEvents;
	ZeitgeistLog *pLog;
	ZeitgeistIndex *pIndex;
	GList *pAppList;
	gchar *cCurrentUri;
	GtkWidget *pEntry;
	GtkListStore *pModel;
	CairoDialog *pDialog;
	CDEventType iCurrentCaterogy;
	GldiShortkey *pKeyBinding;
	gint iDesiredIconSize;
	guint iSidTryDialog;
	gint iNbTries;
	gchar *cQuery;
	} ;


#endif
