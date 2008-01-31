/*
** Login : <ctaf42@gmail.com>
** Started on  Fri Nov 30 05:31:31 2007 GESTES Cedric
** $Id$
**
** Copyright (C) 2007 GESTES Cedric
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "stdlib.h"

#include "terminal-config.h"
#include "terminal-menu-functions.h"
#include "terminal-init.h"
#include "cairo-dock-desklet.h"

t_terminal term = {
	35555.,
	FALSE,
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	NULL,
	NULL,
	25,
	80,
        NULL,
        NULL
};


CD_APPLET_DEFINITION ("terminal", 1, 4, 7)


CD_APPLET_INIT_BEGIN (erreur)
{
  CD_APPLET_REGISTER_FOR_CLICK_EVENT;
  CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
  CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
  term_tab_apply_settings();
}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
{
  cd_desklet_free(term.dialog);
  term.dialog = NULL;
  CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
  CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
  CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
{
  if (CD_APPLET_MY_CONFIG_CHANGED)
    term_tab_apply_settings();
}
CD_APPLET_RELOAD_END
