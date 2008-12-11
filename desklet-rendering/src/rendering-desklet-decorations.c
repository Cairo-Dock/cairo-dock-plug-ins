/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <glib.h>
#include <cairo-dock.h>

#include "rendering-desklet-decorations.h"


#define _register_desklet_decorations(cName, _cBackGroundImagePath, _cForeGroundImagePath, _iLeftMargin, _iTopMargin, _iRightMargin, _iBottomMargin) \
	pDecoration = g_new0 (CairoDeskletDecoration, 1);\
	if (_cBackGroundImagePath != NULL)\
		pDecoration->cBackGroundImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, _cBackGroundImagePath);\
	if (_cForeGroundImagePath != NULL)\
		pDecoration->cForeGroundImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, _cForeGroundImagePath);\
	pDecoration->fBackGroundAlpha = 1.;\
	pDecoration->fForeGroundAlpha = 1.;\
	pDecoration->iLeftMargin = _iLeftMargin;\
	pDecoration->iTopMargin = _iTopMargin;\
	pDecoration->iRightMargin = _iRightMargin;\
	pDecoration->iBottomMargin = _iBottomMargin;\
	cairo_dock_register_desklet_decoration (cName, pDecoration)
	
void cd_rendering_register_desklet_decorations (void)
{
	CairoDeskletDecoration *pDecoration;
	_register_desklet_decorations ("frame&reflects",
		"frame.svg",
		"reflect.svg",
		5,
		5,
		5,
		5);  // 200x200
	
	_register_desklet_decorations ("scotch",
		NULL,
		"scotch.svg",
		40,
		60,
		40,
		0);  // 550x500
	
	_register_desklet_decorations ("frame with scotch",
		NULL,
		"scotch+frame.svg",
		87,
		76,
		87,
		50);  // 550x500
	
	_register_desklet_decorations ("CD box",
		"cd_box.svg",
		"cd_box_cover.svg",
		93,
		86,
		72,
		79);  // 750x700
	_register_desklet_decorations ("dark",
		"dark-bg.png",
		NULL,
		0,
		0,
		0,
		0);  // ...
	_register_desklet_decorations ("clear",
		"clear-bg.svg",
		NULL,
		0,
		0,
		0,
		0);  // ...
	_register_desklet_decorations ("Starcraft2",
		"starcraft2.png",
		NULL,
		5,
		60,
		72,
		5);  // 265x253
	_register_desklet_decorations ("none",
		NULL,
		NULL,
		0,
		0,
		0,
		0);
	
}

