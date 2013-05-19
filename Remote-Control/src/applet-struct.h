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

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar *cShortkey;
	gchar *cDockName;
	gchar *cIconAnimation;
	gint iCloseDuration;
	} ;

typedef enum {
	CD_SESSION_NONE,
	CD_SESSION_CLOSING,
	CD_SESSION_RUNNING,
	CD_SESSION_NB_STATES
} CDSesssionState;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	CDSesssionState iSessionState;
	GString *sCurrentText;
	CairoDockImageBuffer *pArrowImage;
	gint iPromptAnimationCount;
	gint iCloseTime;
	Window iPreviouslyActiveWindow;
	Icon *pCurrentIcon;
	CairoDock *pCurrentDock;
	gboolean bIgnoreIconState;
	gboolean bIgnoreClick;
	int iPrevMouseX, iPrevMouseY;
	int iMouseX, iMouseY;
	int iMotionCount;
	GldiShortkey *pKeyBinding;
	} ;


#endif
