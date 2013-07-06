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
#include <string.h>
#include <X11/XKBlib.h>
#include <libxklavier/xklavier.h>
#include <gdk/gdkx.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-xklavier.h"

// If the group changes
static void _state_changed (XklEngine *pEngine, XklEngineStateChange type,
                            gint iGroup, gboolean bRestore)
{
	XklState *state = xkl_engine_get_current_state (myData.pEngine);
	cd_debug ("State Changed: %d -> %d (%d) ; %d", myData.iCurrentGroup, state->group, iGroup, state->indicators);

	if (type == GROUP_CHANGED && myData.iCurrentGroup != state->group) // new keyboard layout
	{
		gchar *cShortGroupName = NULL;
		const gchar *cCurrentGroup = NULL;

		// Get the current num group
		guint n = xkl_engine_get_num_groups (myData.pEngine);
		g_return_if_fail (n > 0);

		int iNewGroup = MAX (0, MIN (n-1, state->group));  // workaround for 64bits to avoid strange numbers in 'state'
		const gchar **pGroupNames = xkl_engine_get_groups_names (myData.pEngine);
		g_return_if_fail (pGroupNames != NULL);

		cCurrentGroup = pGroupNames[iNewGroup];
		g_return_if_fail (cCurrentGroup != NULL);

		cd_debug (" group name : %s (%d groups)", cCurrentGroup, n);

		// build the displayed group name
		cShortGroupName = g_strndup (cCurrentGroup, myConfig.iNLetters);
		int index = 0;
		int i;
		for (i = 0; i < state->group; i ++)  // look for groups before us having the same name.
		{
			if (strncmp (cCurrentGroup, pGroupNames[i], myConfig.iNLetters) == 0)
				index ++;
		}
		if (index != 0)  // add a number if several groups have the same name.
		{
			gchar *tmp = cShortGroupName;
			cShortGroupName = g_strdup_printf ("%s%d", cShortGroupName, index+1);
			g_free (tmp);
		}

		myData.iCurrentGroup = state->group;
		cd_xkbd_update_icon (cCurrentGroup, cShortGroupName, TRUE);
		g_free (cShortGroupName);
	}
	else if (type == INDICATORS_CHANGED)
		cd_debug ("Indicators changed"); // according to libxklavier/tests/test_monitor.c, it should work... but I have to miss something
}

// A scroll on the icon
void cd_xkbd_set_prev_next_group (int iDelta)
{
	XklState *state = xkl_engine_get_current_state (myData.pEngine);

	cd_debug ("keyboard current state : %d;%d +%d", state->group, state->indicators, iDelta);
	
	int i = 0, n = xkl_engine_get_num_groups (myData.pEngine);
	g_return_if_fail (n > 0);
	int iCurrentGroup = MAX (0, MIN (n-1, state->group));  // on blinde car libxklavier peut bugger en 64bits.
	const gchar **pGroupNames = xkl_engine_get_groups_names (myData.pEngine);
	do  // on passe au groupe suivant/precedent en sautant les faux (-).
	{
		i ++;
		iCurrentGroup += iDelta;  // xkl_engine_get_next_group ne marche pas.
		if (iCurrentGroup == n)
			iCurrentGroup = 0;
		else if (iCurrentGroup < 0)
			iCurrentGroup = n - 1;
	} while (i < n && (pGroupNames[iCurrentGroup] == NULL || *pGroupNames[iCurrentGroup] == '-'));
	
	state->group = iCurrentGroup;
	cd_debug ("keyboard new state : %d", state->group);
	xkl_engine_allow_one_switch_to_secondary_group (myData.pEngine);  // sert a quoi ??
	
	Window Xid = xkl_engine_get_current_window (myData.pEngine);
	xkl_engine_save_state (myData.pEngine, Xid, state);
	xkl_engine_lock_group (myData.pEngine, state->group);  // sert a quoi ??
}

// Select the layout from the menu
void cd_xkbd_set_group (int iNumGroup)
{
	XklState *state = xkl_engine_get_current_state (myData.pEngine);
	cd_debug ("keyboard current state : %d;%d", state->group, state->indicators);
	
	state->group = iNumGroup;
	
	Window Xid = xkl_engine_get_current_window (myData.pEngine);
	xkl_engine_allow_one_switch_to_secondary_group (myData.pEngine);  // sert a quoi ??
	xkl_engine_save_state (myData.pEngine, Xid, state);
	xkl_engine_lock_group (myData.pEngine, state->group);  // sert a quoi ??
}

static guint32 _get_state_indicators ()
{
	Display *dpy = gdk_x11_get_default_xdisplay ();
	Bool st;
	guint32 indicators;
	Atom capsLock = XInternAtom(dpy, "Caps Lock", False);
	Atom numLock = XInternAtom(dpy, "Num Lock", False);
	// Atom scrollLock = XInternAtom(dpy, "Scroll Lock", False);

	XkbGetNamedIndicator (dpy, capsLock, NULL, &st, NULL, NULL);
	indicators = st;
	XkbGetNamedIndicator (dpy, numLock, NULL, &st, NULL, NULL);
	indicators |= st << 1;

	return indicators;
}

// only use to catch new state of the indicator
gboolean cd_xkbd_keyboard_state_changed (GldiModuleInstance *myApplet, Window *pWindow)
{
	CD_APPLET_ENTER;
	///if (pWindow == NULL)
	/// return GLDI_NOTIFICATION_LET_PASS;
	
	// Get the current state
	XklState *state = xkl_engine_get_current_state (myData.pEngine);
	guint32 indicators = _get_state_indicators ();
	
	cd_debug ("group : %d -> %d ; indic : %d -> %d (%d)",
		myData.iCurrentGroup, state->group,
		myData.iCurrentIndic, indicators, state->indicators);
	
	if (myData.iCurrentIndic == indicators)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);

	// Remember the current state
	myData.iCurrentIndic = indicators;
	
	cd_xkbd_update_icon (NULL, NULL, FALSE); // redraw only the indicators
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}

static GdkFilterReturn _filter_xevent (GdkXEvent *pGdkXEvent, GdkEvent *pEvent)
{
	xkl_engine_filter_events (myData.pEngine, (XEvent*) pGdkXEvent);

	return GDK_FILTER_CONTINUE;
}

void cd_xkbd_force_redraw ()
{
	// indicator
	Window Xid = xkl_engine_get_current_window (myData.pEngine);
	cd_xkbd_keyboard_state_changed (myApplet, &Xid); // force redraw

	// group
	myData.iCurrentGroup = -1;
	_state_changed (myData.pEngine, GROUP_CHANGED, -1, FALSE);
}

void cd_xkbd_init (Display *pDisplay)
{
	myData.pEngine = xkl_engine_get_instance (pDisplay);
	g_return_if_fail (myData.pEngine != NULL);

	g_signal_connect (myData.pEngine, "X-state-changed",
		G_CALLBACK(_state_changed), NULL); // notification for the group, we receive nothing for the indicators...

	gdk_window_add_filter (NULL, (GdkFilterFunc) _filter_xevent, NULL); // should connect to all type of windows
	/// gdk_window_add_filter (gdk_get_default_root_window (), (GdkFilterFunc) _filter_xevent, NULL);

	xkl_engine_start_listen (myData.pEngine, XKLL_TRACK_KEYBOARD_STATE);

	cd_xkbd_force_redraw ();
}

void cd_xkbd_stop ()
{
	g_return_if_fail (myData.pEngine != NULL);

	xkl_engine_stop_listen (myData.pEngine, XKLL_TRACK_KEYBOARD_STATE);

	gdk_window_remove_filter (NULL, (GdkFilterFunc) _filter_xevent, NULL);
	/// gdk_window_remove_filter (gdk_get_default_root_window (), (GdkFilterFunc) _filter_xevent, NULL);
}
