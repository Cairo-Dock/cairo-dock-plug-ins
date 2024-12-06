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


#ifndef __CAIRO_DOCK_GIO_VFS__
#define  __CAIRO_DOCK_GIO_VFS__

#include "cairo-dock-file-manager.h"
#include <gio/gio.h>

/** Init the GVfs backend.
 *@param  bNeedDbus check if the "org.gtk.vfs.Daemon" service is running and fail if it is not
 *@return the default GVfs instance as per g_vfs_get_default () (no need to free this)
 */
GVfs *cairo_dock_gio_vfs_init (gboolean bNeedDbus);

/** Fill the backend with gio/gvfs fonctions, if possible.
 *@param  pVFSBackend The backend structure to fill up 
 *@return TRUE if all went well, FALSE if gio/gvfs is not available
 */
gboolean cairo_dock_gio_vfs_fill_backend(CairoDockDesktopEnvBackend *pVFSBackend);

#endif
