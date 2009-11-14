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


#ifndef __APPLET_SPOT__
#define  __APPLET_SPOT__


#include <cairo-dock.h>

#define cd_animation_load_halo_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("halo.png")
#define cd_animation_load_spot_texture(...) CD_APPLET_LOAD_TEXTURE_WITH_DEFAULT (myConfig.cSpotImage, "spot.png")
#define cd_animation_load_spot_front_texture(...) CD_APPLET_LOAD_TEXTURE_WITH_DEFAULT (myConfig.cSpotFrontImage, "")

void cd_animations_init_spot (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt);

void cd_animation_render_spot (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor);

void cd_animation_render_halo (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor, int iHaloRotationAngle);

void cd_animation_render_spot_front (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor);

gboolean cd_animations_update_spot (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bWillContinue);


#endif
