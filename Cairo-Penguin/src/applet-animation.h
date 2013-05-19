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


#ifndef __APPLET_ANIMATION__
#define  __APPLET_ANIMATION__

#include <cairo-dock.h>


#define penguin_get_current_animation() (myData.iCurrentAnimation >= 0 ? &myData.pAnimations[myData.iCurrentAnimation] : NULL);

#define penguin_is_resting(pAnimation) (pAnimation == NULL || ((pAnimation)->iNbFrames <= 1 && (pAnimation)->iSpeed == 0))

#define penguin_remove_notfications() do {\
	gldi_object_remove_notification (myIcon, NOTIFICATION_UPDATE_ICON_SLOW, (GldiNotificationFunc) penguin_update_icon, myApplet);\
	gldi_object_remove_notification (myDock, NOTIFICATION_UPDATE_SLOW, (GldiNotificationFunc) penguin_update_container, myApplet);\
	gldi_object_remove_notification (myDock, NOTIFICATION_RENDER, (GldiNotificationFunc) penguin_render_on_container, myApplet); } while (0)

void penguin_move_in_dock (GldiModuleInstance *myApplet);
gboolean penguin_render_on_container (GldiModuleInstance *myApplet, GldiContainer *pContainer, cairo_t *pCairoContext);
void penguin_draw_on_dock_opengl (GldiModuleInstance *myApplet, GldiContainer *pContainer);
void penguin_draw_on_dock (GldiModuleInstance *myApplet, GldiContainer *pContainer, cairo_t *pCairoContext);


void penguin_move_in_icon (GldiModuleInstance *myApplet);


gboolean penguin_update_container (GldiModuleInstance *myApplet, GldiContainer *pContainer, gboolean *bContinueAnimation);
gboolean penguin_update_icon (GldiModuleInstance *myApplet, Icon *pIcon, GldiContainer *pContainer, gboolean *bContinueAnimation);






void penguin_calculate_new_position (GldiModuleInstance *myApplet, PenguinAnimation *pAnimation, int iXMin, int iXMax, int iHeight);

void penguin_advance_to_next_frame (GldiModuleInstance *myApplet, PenguinAnimation *pAnimation);

int penguin_choose_movement_animation (GldiModuleInstance *myApplet);
int penguin_choose_go_up_animation (GldiModuleInstance *myApplet);
int penguin_choose_beginning_animation (GldiModuleInstance *myApplet);
int penguin_choose_ending_animation (GldiModuleInstance *myApplet);
int penguin_choose_resting_animation (GldiModuleInstance *myApplet);
int penguin_choose_next_animation (GldiModuleInstance *myApplet, PenguinAnimation *pAnimation);

void penguin_set_new_animation (GldiModuleInstance *myApplet, int iNewAnimation);

void penguin_start_animating (GldiModuleInstance *myApplet);
void penguin_start_animating_with_delay (GldiModuleInstance *myApplet);


#endif
