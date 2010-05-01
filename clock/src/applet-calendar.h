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


#ifndef __CD_CLOCK_CALENDAR__
#define  __CD_CLOCK_CALENDAR__

#include <cairo-dock.h>
#include "applet-struct.h"


void cd_clock_register_backend (CairoDockModuleInstance *myApplet, const gchar *cBackendName, CDClockTaskBackend *pBackend);

CDClockTaskBackend *cd_clock_get_backend (CairoDockModuleInstance *myApplet, const gchar *cBackendName);

void cd_clock_set_current_backend (CairoDockModuleInstance *myApplet);


void cd_clock_list_tasks (CairoDockModuleInstance *myApplet);

void cd_clock_add_task_to_list (CDClockTask *pTask, CairoDockModuleInstance *myApplet);

void cd_clock_remove_task_from_list (CDClockTask *pTask, CairoDockModuleInstance *myApplet);

void cd_clock_free_task (CDClockTask *pTask);

void cd_clock_reset_tasks_list (CairoDockModuleInstance *myApplet);

CDClockTask *cd_clock_get_task_by_id (const gchar *cID, CairoDockModuleInstance *myApplet);

gchar *cd_clock_get_tasks_for_today (CairoDockModuleInstance *myApplet);

gchar *cd_clock_get_tasks_for_this_week (CairoDockModuleInstance *myApplet);

CDClockTask *cd_clock_get_next_scheduled_task (CairoDockModuleInstance *myApplet);

CDClockTask *cd_clock_get_next_anniversary (CairoDockModuleInstance *myApplet);


void cd_clock_update_calendar_marks (CairoDockModuleInstance *myApplet);

void cd_clock_hide_dialogs (CairoDockModuleInstance *myApplet);

void cd_clock_show_hide_calendar (CairoDockModuleInstance *myApplet);


#endif

