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

typedef enum {
	PENGUIN_DOWN = -1,
	PENGUIN_HORIZONTAL,
	PENGUIN_UP
	} PenguinDirectionType;


typedef struct {
	gchar *cFilePath;
	gint iNbDirections;
	gint iNbFrames;
	gint iSpeed;
	gint iAcceleration;
	gint iTerminalVelocity;
	gboolean bEnding;
	PenguinDirectionType iDirection;
	cairo_surface_t ***pSurfaces;
	int iFrameWidth, iFrameHeight;
	GLuint iTexture;
	} PenguinAnimation;

struct _AppletConfig {
	gchar *cThemePath;
	gint iDelayBetweenChanges;
	gdouble fAlpha;
	gboolean bFree;
	gint iGroundOffset;
	} ;


struct _AppletData {
	gint iCurrentAnimation;
	gint iCurrentPositionX, iCurrentPositionY;
	gint iCurrentSpeed;
	gint iCurrentDirection;
	gint iCurrentFrame;
	gint iCount;
	gdouble fFrameDelay;
	PenguinAnimation defaultAnimation;
	gint iNbAnimations;
	PenguinAnimation *pAnimations;
	gint iNbEndingAnimations;
	gint *pEndingAnimations;
	gint iNbBeginningAnimations;
	gint *pBeginningAnimations;
	gint iNbMovmentAnimations;
	gint *pMovmentAnimations;
	gint iNbGoUpAnimations;
	gint *pGoUpAnimations;
	gint iNbRestAnimations;
	gint *pRestAnimations;
	guint iSidRestartDelayed;
	gboolean bHasBeenStarted;
	gint iCountStep;
	gint iSleepingTime;
	} ;

#endif
