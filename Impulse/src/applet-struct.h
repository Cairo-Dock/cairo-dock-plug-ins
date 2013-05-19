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
	gchar *cIconAnimation;
	gint iNbAnimations;
	double fMinValueToAnim;
	gint iLoopTime;
	CairoDock *pDock;
	gboolean bStopAnimations;
	gboolean bLaunchAtStartup;
	gboolean bFree;
	gchar *cIconImpulseON;
	gchar *cIconImpulseOFF;
	gchar *cIconImpulseERROR;
	gint iSourceIndex;
	} ;

typedef struct {
	GList *pIconsList; // without separators
	gboolean bIsUpdatingIconsList;
	gchar *cIconAnimation;
	gint iNbAnimations;
	double fMinValueToAnim;
	gboolean bStopAnimations;
	gboolean bNeedRefreshIfNotAnimated; // if bStopAnimations and there is no animation after, the icon is not refreshed
	CairoDock *pDock;
	//GldiModuleInstance *pApplet;
	} CDSharedMemory;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	// CairoDockTask *pTask;  // task for the animation
	CDSharedMemory *pSharedMemory;
	// gboolean isRunning;
	gboolean bPulseLaunched; // can we stop the pulseaudio client/server?
	guint iSidAnimate;
	guint iSidRestartDelayed;
	guint iSidCheckStatus;
	gboolean bHasBeenStarted;
	} ;


#endif
