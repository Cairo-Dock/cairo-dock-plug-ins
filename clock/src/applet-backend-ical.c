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

#include <stdlib.h>
#include <math.h>
#define __USE_POSIX
#include <signal.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-calendar.h"

#ifdef CD_CLOCK_ICAL_SUPPORT
#include <libical/icalfileset.h>
#include <libical/icalset.h>
#include <libical/icalcomponent.h>
#include <libical/icalrecur.h>
#include <libical/icalproperty.h>
#include "applet-backend-ical.h"

typedef struct {
	icalset *piCalSet;
	icalcomponent *piCalCalendar;
	} CDClockIcalBackendData;

static CDClockIcalBackendData *_pBackendData = NULL;

static void backend_ical_init(CairoDockModuleInstance *myApplet)
{
	cd_debug("Backend initialization.");
	gchar *cDirPath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, "clock");
	if (! g_file_test (cDirPath, G_FILE_TEST_EXISTS))
	{
		if (g_mkdir (cDirPath, 7*8*8+7*8+5) != 0)
		{
			cd_warning ("couldn't create directory %s", cDirPath);
			g_free (cDirPath);
			return;
		}
	}
	
	gchar *cFile = g_strdup_printf ("%s/%s", cDirPath, "tasks.ics");
	icalset *piCalSet = icalfileset_new(cFile);
	cd_debug("Backend initialization: iCal file stored in %s. icalfileset_new returned %x", cFile, piCalSet);

	if( piCalSet )
	{
		_pBackendData = g_new0 (CDClockIcalBackendData, 1);
		_pBackendData->piCalSet = piCalSet;
		// find the calendar
		for (_pBackendData->piCalCalendar = icalfileset_get_first_component(_pBackendData->piCalSet);
				 _pBackendData->piCalCalendar;
				 _pBackendData->piCalCalendar = icalfileset_get_next_component(_pBackendData->piCalSet))
		{
			if( ICAL_VCALENDAR_COMPONENT == icalcomponent_isa(_pBackendData->piCalCalendar) ) break;
		}
		// if there is no calendar, create one
		if( _pBackendData->piCalCalendar == NULL )
		{
			_pBackendData->piCalCalendar = icalcomponent_new_vcalendar();
			cd_debug("Adding new calendar to iCal file...");
			icalerrorenum error = icalfileset_add_component(_pBackendData->piCalSet, _pBackendData->piCalCalendar);
			if( error != ICAL_NO_ERROR )
			{
				cd_debug(" --> %s\n", icalerror_strerror(error));
			}
		}
	}

	g_free (cFile);
	g_free (cDirPath);
}

static void backend_ical_stop(CairoDockModuleInstance *myApplet)
{
	if( _pBackendData )
	{
		if( _pBackendData->piCalSet )
		{
			icalfileset_commit(_pBackendData->piCalSet);
			icalfileset_free( _pBackendData->piCalSet );
		}
		g_free( _pBackendData ); _pBackendData = NULL;
	}
}

static gboolean _assert_data(void)
{
	if (_pBackendData == NULL ||
	    _pBackendData->piCalSet == NULL ||
	    _pBackendData->piCalCalendar == NULL)  // comment ca, "NULL" ??
	{
		if( _pBackendData == NULL )
		{
			cd_error("ERROR in Clock plugin with iCal: _pBackendData is NULL");
		}
		else
		{
			cd_error("ERROR in Clock plugin with iCal: _pBackendData is corrupted");
		}
		return FALSE;
	}

	return TRUE;
}

static icalcomponent *find_task(const char* uid)
{
	if( !_assert_data() || uid == NULL ) return NULL;
	
	gchar *cTaskID = NULL;
	icalcomponent *piCalComponent = NULL;

	for (piCalComponent = icalcomponent_get_first_component(_pBackendData->piCalCalendar, ICAL_ANY_COMPONENT);
	     piCalComponent;
	     piCalComponent = icalcomponent_get_next_component(_pBackendData->piCalCalendar, ICAL_ANY_COMPONENT))
	{
		//if( ICAL_VCALENDAR_COMPONENT != icalcomponent_isa(piCalComponent) ) continue;
		cTaskID = g_strdup(icalcomponent_get_uid(piCalComponent));
		g_print( "...Found task ID=%s\n", cTaskID );
		if( cTaskID != NULL && strcmp( uid, cTaskID ) == 0 )
		{
			break;
		}
	}

	return piCalComponent;
}

static GList *get_tasks (CairoDockModuleInstance *myApplet)
{
	if( !_assert_data() ) return NULL;
	
	CDClockTask *pTask = NULL;
	gchar *cTaskID = NULL;
	GList *pTaskList = NULL;

	icalcomponent *piCalComponent = NULL;

	for (piCalComponent = icalcomponent_get_first_component(_pBackendData->piCalCalendar, ICAL_ANY_COMPONENT);
	     piCalComponent;
	     piCalComponent = icalcomponent_get_next_component(_pBackendData->piCalCalendar, ICAL_ANY_COMPONENT))
	{
		//if( ICAL_VCALENDAR_COMPONENT != icalcomponent_isa(piCalComponent) ) continue;
		cd_debug( "Fetching iCal component of kind: %s", icalcomponent_kind_to_string(icalcomponent_isa(piCalComponent)) );
		
		cTaskID = g_strdup(icalcomponent_get_uid(piCalComponent));
		pTask = g_new0 (CDClockTask, 1);
		cd_debug ("+ task %s\n", cTaskID);

		struct icaltimetype liCalStartDate = icalcomponent_get_dtstart(piCalComponent);
		// struct icaldurationtype liCalDuration = icalcomponent_get_duration(piCalComponent); //ignored until Clock manages tasks duration
		
		pTask->cID = cTaskID;
		pTask->iDay = liCalStartDate.day;
		pTask->iMonth = liCalStartDate.month;
		pTask->iYear = liCalStartDate.year;
		pTask->iHour = liCalStartDate.hour;
		pTask->iMinute = liCalStartDate.minute;

		pTask->iFrequency = CD_TASK_DONT_REPEAT;
		// TODO: really do the frequency management. If possible.
		icalproperty *rrule = NULL;
		struct icalrecurrencetype recur;
		rrule = icalcomponent_get_first_property(piCalComponent,ICAL_RRULE_PROPERTY);
		recur = icalproperty_get_rrule(rrule);
		switch( recur.freq )
		{
			case ICAL_MONTHLY_RECURRENCE:pTask->iFrequency = CD_TASK_EACH_MONTH; break;
			case ICAL_YEARLY_RECURRENCE:pTask->iFrequency = CD_TASK_EACH_YEAR; break;
			default:pTask->iFrequency = CD_TASK_DONT_REPEAT; break;
		}

		pTask->cTitle = g_strdup(icalcomponent_get_summary(piCalComponent));
		pTask->cText = g_strdup(icalcomponent_get_description(piCalComponent));
		pTask->cTags = g_strdup(icalcomponent_get_comment(piCalComponent));
		
		pTaskList = g_list_prepend (pTaskList, pTask);
	}
	
	return pTaskList;
}

static int s_iCounter = 0;
static gboolean create_task (CDClockTask *pTask, CairoDockModuleInstance *myApplet)
{
	if( !_assert_data() ) return FALSE;
	if( pTask == NULL ) return FALSE;

	icalcomponent *piCalComponent = NULL;
	gboolean bIsModification = FALSE;

	if( pTask->cID == NULL )
	{
		// find the first free slot for the ID
		do {
			if( pTask->cID ) g_free(pTask->cID);
			pTask->cID = g_strdup_printf ("%d", ++s_iCounter);
		} while( find_task(pTask->cID) != NULL );

		piCalComponent = icalcomponent_new_vtodo();
		if( piCalComponent == NULL ) return FALSE;		
		icalcomponent_set_uid(piCalComponent, pTask->cID);
	}
	else
	{
		// ID is already set, so it is a modification
		piCalComponent = find_task(pTask->cID);
		cd_warning("Trying to modify task ID=%s, but didn't find it in the iCal database!", pTask->cID);
		if( piCalComponent == NULL ) return FALSE;
		bIsModification = TRUE;
	}


	struct icaltimetype liCalStartDate = icaltime_null_time();
	// struct icaldurationtype liCalDuration = icalcomponent_get_duration(piCalComponent); //ignored until Clock manages tasks duration
	
	liCalStartDate.day = pTask->iDay;
	liCalStartDate.month = pTask->iMonth;
	liCalStartDate.year = pTask->iYear;
	liCalStartDate.hour = pTask->iHour;
	liCalStartDate.minute = pTask->iMinute;

	icalcomponent_set_dtstart(piCalComponent, liCalStartDate);
	
	icalproperty *rrule = NULL;
	struct icalrecurrencetype recur;
	switch( pTask->iFrequency )
	{
		case CD_TASK_EACH_MONTH:
			recur = icalrecurrencetype_from_string("FREQ=MONTHLY");
			rrule = icalproperty_new_rrule(recur);
			break;
		case CD_TASK_EACH_YEAR:
			recur = icalrecurrencetype_from_string("FREQ=YEARLY");
			rrule = icalproperty_new_rrule(recur);
			break;
		default: break;
	}
	if( bIsModification == TRUE )
	{
		// we should remove the previous RECUR property
		icalproperty *previous_rrule = icalcomponent_get_first_property( piCalComponent, ICAL_RRULE_PROPERTY );
		if( previous_rrule )
		{
			icalcomponent_remove_property(piCalComponent, previous_rrule);
		}
	}
	if( rrule )
	{
		icalcomponent_add_property(piCalComponent, rrule);
	}

	if( pTask->cTitle )
	{
		icalcomponent_set_summary(piCalComponent, pTask->cTitle);
	}
	if( pTask->cText )
	{
		icalcomponent_set_description(piCalComponent, pTask->cText);
	}
	if( pTask->cTags )
	{
		icalcomponent_set_comment(piCalComponent, pTask->cTags);
	}

	if( !bIsModification )
	{
		cd_debug("Adding component (ID=%s,Title=%s) to iCal file...", pTask->cID, pTask->cTitle);
		icalcomponent_add_component(_pBackendData->piCalCalendar, piCalComponent);
	}
	icalfileset_mark(_pBackendData->piCalSet);
	icalfileset_commit(_pBackendData->piCalSet);
	
	return TRUE;
}

static gboolean delete_task (CDClockTask *pTask, CairoDockModuleInstance *myApplet)
{
	g_print ("%s (%s)\n", __func__, pTask->cTitle);

	if( !_assert_data() ) return FALSE;
	if( pTask == NULL ) return FALSE;

	icalcomponent *piCalComponent = find_task(pTask->cID);
	if( piCalComponent == NULL )
	{
		cd_warning("Trying to delete task ID=%s, but didn't find it in the iCal database!", pTask->cID);
		return FALSE;
	}

	icalcomponent_remove_component(_pBackendData->piCalCalendar, piCalComponent);
	icalfileset_mark(_pBackendData->piCalSet);
	icalfileset_commit(_pBackendData->piCalSet);
	
	return TRUE;
}

static gboolean update_task (CDClockTask *pTask, CairoDockModuleInstance *myApplet)
{
	g_print ("%s (%s, '%s')\n", __func__, pTask->cTitle, pTask->cText);
	//if( !delete_task (pTask, myApplet) ) return FALSE;
	if( !create_task (pTask, myApplet) ) return FALSE;
		
	return TRUE;
}
#endif

void cd_clock_register_backend_ical (CairoDockModuleInstance *myApplet)
{
#ifdef CD_CLOCK_ICAL_SUPPORT
	CDClockTaskBackend *pBackend = g_new0 (CDClockTaskBackend, 1);
	pBackend->init = backend_ical_init;
	pBackend->stop = backend_ical_stop;
	pBackend->get_tasks = get_tasks;
	pBackend->create_task = create_task;
	pBackend->delete_task = delete_task;
	pBackend->update_task = update_task;
	
	cd_clock_register_backend (myApplet, "iCal", pBackend);
#endif
}
