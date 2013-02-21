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

#include <glib.h>
#include <cairo-dock.h>

#include "rendering-desklet-decorations.h"


static inline void _register_desklet_decorations (const gchar *cName, const gchar *_cDisplayedName, const gchar *_cBackGroundImagePath, const gchar *_cForeGroundImagePath, int _iLeftMargin, int _iTopMargin, int _iRightMargin, int _iBottomMargin)
{
	CairoDeskletDecoration *pDecoration = g_new0 (CairoDeskletDecoration, 1);
	pDecoration->cDisplayedName = _cDisplayedName;
	if (_cBackGroundImagePath != NULL)
		pDecoration->cBackGroundImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, _cBackGroundImagePath);
	if (_cForeGroundImagePath != NULL)
		pDecoration->cForeGroundImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, _cForeGroundImagePath);
	pDecoration->fBackGroundAlpha = 1.;
	pDecoration->fForeGroundAlpha = 1.;
	pDecoration->iLeftMargin = _iLeftMargin;
	pDecoration->iTopMargin = _iTopMargin;
	pDecoration->iRightMargin = _iRightMargin;
	pDecoration->iBottomMargin = _iBottomMargin;
	cairo_dock_register_desklet_decoration (cName, pDecoration);
}
	
void cd_rendering_register_desklet_decorations (void)
{
	_register_desklet_decorations ("frame&reflects",
		D_("frame&reflects"),
		"frame.svg",
		"reflect.svg",
		5,
		5,
		5,
		5);  // 200x200
	
	_register_desklet_decorations ("scotch",
		D_("scotch sellotape"),
		NULL,
		"scotch.svg",
		40,
		60,
		40,
		0);  // 550x500
	
	_register_desklet_decorations ("frame with scotch",
		D_("frame with sellotape"),
		NULL,
		"scotch+frame.svg",
		87,
		76,
		87,
		50);  // 550x500
	
	_register_desklet_decorations ("CD box",
		D_("CD box"),
		"cd_box.svg",
		"cd_box_cover.svg",
		93,
		86,
		72,
		79);  // 750x700
	_register_desklet_decorations ("dark",
		D_("dark"),
		"dark-bg.svg",
		NULL,
		0,
		0,
		0,
		0);  // ...
	_register_desklet_decorations ("clear",
		D_("clear"),
		"clear-bg.svg",
		NULL,
		0,
		0,
		0,
		0);  // ...
	_register_desklet_decorations ("light",
		D_("light"),
		"light-bg.svg",
		NULL,
		0,
		0,
		0,
		0);  // ...
	_register_desklet_decorations ("water-drop",
		D_("water drop"),
		"water-drop-bg.png",
		NULL,
		0,
		0,
		0,
		0);  // ...
	_register_desklet_decorations ("water",
		D_("water"),
		"water-dark-bg.png",
		NULL,
		0,
		0,
		0,
		0);  // ...
	_register_desklet_decorations ("futuristic",
		D_("futuristic"),
		"starcraft2.png",
		NULL,
		5,
		60,
		72,
		5);  // 265x253
	_register_desklet_decorations ("none",
		D_("none"),
		NULL,
		NULL,
		0,
		0,
		0,
		0);
	_register_desklet_decorations ("board",
		D_("board"),
		"board.png",
		NULL,
		0,
		0,
		0,
		0);  // ...
}

