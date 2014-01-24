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

#include <string.h>

#include "applet-struct.h"
#include "applet-logout.h"
#include "applet-reboot-required.h"

static gboolean s_bRebootRequired = FALSE;
static gboolean s_bMonitored = FALSE;

static const gchar * _get_default_message (void)
{
	if (myConfig.cDefaultLabel) // has another default name
		return myConfig.cDefaultLabel;
	else
		return myApplet->pModule->pVisitCard->cTitle;
}

static gchar * _get_reboot_message (void)
{
	gchar *cMessage = NULL;
	gsize length = 0;
	g_file_get_contents (CD_REBOOT_NEEDED_FILE,
		&cMessage,
		&length,
		NULL);
	if (cMessage != NULL)
	{
		int len = strlen (cMessage);
		if (cMessage[len-1] == '\n')
			cMessage[len-1] = '\0';
	}

	return (cMessage);
}

static void _notify_action_required (void)
{
	CD_APPLET_DEMANDS_ATTENTION ("pulse", 20);
	gldi_dialogs_remove_on_icon (myIcon);

	// it's not a good idea to reboot the computer before the end of the update ;)
	gchar *cName = g_strdup_printf ("%s\n%s", myIcon->cName,
		D_("Please do that at the end of the update."));

	gldi_dialog_show_temporary_with_icon (cName, myIcon, myContainer, 15e3, "same icon");

	g_free (cName);

	gint iIconSize = MAX (myIcon->image.iWidth, myIcon->image.iHeight);
	gchar *cImagePath = cd_logout_check_icon (myConfig.cEmblemPath,
		(myConfig.iRebootNeededImage == CD_DISPLAY_EMBLEM ?
				iIconSize / 2 :
				iIconSize));
	if (! cImagePath)
	{
		cImagePath = cd_logout_check_icon (GTK_STOCK_REFRESH,
			(myConfig.iRebootNeededImage == CD_DISPLAY_EMBLEM ?
				iIconSize / 2 :
				iIconSize));
		if (! cImagePath)
			cImagePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/system-restart.svg");
	}

	if (myConfig.iRebootNeededImage == CD_DISPLAY_EMBLEM)
		CD_APPLET_PRINT_OVERLAY_ON_MY_ICON (cImagePath, CAIRO_OVERLAY_UPPER_RIGHT);
	else
		CD_APPLET_SET_IMAGE_ON_MY_ICON (cImagePath);
	g_free (cImagePath);
}

static void _stop_notify_action_required (void)
{
	 // should not happen... mainly for the tests.
	gldi_dialogs_remove_on_icon (myIcon);
	if (myConfig.iRebootNeededImage == CD_DISPLAY_EMBLEM)
		CD_APPLET_PRINT_OVERLAY_ON_MY_ICON (NULL, CAIRO_OVERLAY_UPPER_RIGHT);
	else
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cDefaultIcon);
	CD_APPLET_STOP_DEMANDING_ATTENTION;
}

static gboolean _notify_reboot_requiered (gpointer pData)
{
	if (! myApplet || ! s_bRebootRequired)
	{
		s_bMonitored = FALSE;
		return FALSE;
	}

	CairoDockFMEventType iEventType = GPOINTER_TO_INT (pData);

	gchar *cMessage = _get_reboot_message ();
	if (cMessage && *cMessage != '\0')
		CD_APPLET_SET_NAME_FOR_MY_ICON (cMessage);
	else
		CD_APPLET_SET_NAME_FOR_MY_ICON (_get_default_message ());
	if (iEventType == CAIRO_DOCK_FILE_CREATED)
		_notify_action_required ();

	g_free (cMessage);
	s_bMonitored = FALSE;

	return FALSE;
}

void cd_logout_check_reboot_required (CairoDockFMEventType iEventType, const gchar *cURI)
{
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_MODIFIED: // new message
		case CAIRO_DOCK_FILE_CREATED:  // reboot required
			s_bRebootRequired = TRUE;
			if (! s_bMonitored)
			{
				s_bMonitored = TRUE;
				gpointer pEventType = GINT_TO_POINTER (iEventType);
				#ifdef END_INSTALLATION_PID
				cairo_dock_fm_monitor_pid (END_INSTALLATION_PID, FALSE,
					_notify_reboot_requiered, TRUE, pEventType);
				#else
				_notify_reboot_requiered (pEventType);
				#endif
			}
		break;
		
		case CAIRO_DOCK_FILE_DELETED:  // reboot/logout no more required (shouldn't happen)
			s_bRebootRequired = FALSE;
			_stop_notify_action_required (); // default icon
			CD_APPLET_SET_NAME_FOR_MY_ICON (_get_default_message ());
		break;
		default:
		break;
	}
}

void cd_logout_check_reboot_required_init (void)
{
	if (g_file_test (CD_REBOOT_NEEDED_FILE, G_FILE_TEST_EXISTS))
	{
		cd_logout_check_reboot_required (CAIRO_DOCK_FILE_CREATED, CD_REBOOT_NEEDED_FILE);
	}
}