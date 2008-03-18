/*
** cd-tray.h
** Login : <ctaf@CTAF-FIX.CTAFLAND>
** Started on  Tue Dec  4 08:44:31 2007 GESTES Cedric
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

#ifndef   	CD_TRAY_H_
# define   	CD_TRAY_H_

# include "na-tray-manager.h"

typedef struct {
  NaTrayManager  *manager;
  GtkWidget      *box;
  GtkWidget      *widget;
  GdkScreen      *screen;

  GList          *icons;
  guint          idle_redraw_id;
} TrayApplet;


TrayApplet* tray_init (GtkWidget *applet);


#endif 	    /* !CD_TRAY_H_ */
