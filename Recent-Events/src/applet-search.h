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

#ifndef __APPLET_SEARCH__
#define  __APPLET_SEARCH__

#include <cairo-dock.h>
#include "applet-struct.h"


void cd_find_recent_related_files (const gchar **cMimeTypes, CDOnGetEventsFunc pCallback, gpointer data);


void cd_find_recent_events (CDEventType iEventType, int iSortType, CDOnGetEventsFunc pCallback, gpointer data);


void cd_search_events (const gchar *cQuery, CDEventType iEventType, CDOnGetEventsFunc pCallback, gpointer data);


void cd_delete_recent_events (int iNbDays, CDOnDeleteEventsFunc pCallback, gpointer data);

void cd_delete_event (const gchar *cUri);


#endif
