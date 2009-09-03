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
	CD_SHOW_MOUSE_ON_DOCK=1,
	CD_SHOW_MOUSE_ON_DESKLET=2,
	CD_SHOW_MOUSE_ON_BOTH=3
	} CDShowMouseType;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gdouble fRotationSpeed;
	gint iParticleLifeTime;
	gint iNbParticles;
	gint iParticleSize;
	gdouble pColor1[3];
	gdouble pColor2[3];
	gboolean bMysticalFire;
	gint iNbSources;
	gdouble fScattering;
	CDShowMouseType iContainerType;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iTexture;
	CDShowMouseType iContainerType;
	} ;

typedef struct _CDShowMouseData {
	CairoParticleSystem *pSystem;
	gdouble fRotationAngle;
	gdouble fAlpha;
	gdouble *pSourceCoords;
	gdouble fRadius;
	} CDShowMouseData;

#endif
