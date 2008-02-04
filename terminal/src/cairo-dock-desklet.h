/*
** Login : <ctaf42@gmail.com>
** Started on  Sun Jan 27 18:35:38 2008 Cedric GESTES
** $Id$
**
** Copyright (C) 2008 Cedric GESTES
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

#ifndef __CAIRO_DOCK_DESKLET_H__
#define  __CAIRO_DOCK_DESKLET_H__

#include <glib.h>

#include <cairo-dock.h>



typedef struct _CairoDockDesklet
{
  /// la fenetre GTK du dialogue.
  GtkWidget *pWidget;
  /// icone sur laquelle pointe le dialogue.
  Icon *pIcon;
  /// le widget d'interaction utilisateur (GtkEntry, GtkHScale, etc).
  GtkWidget *pInteractiveWidget;

  ///the menu
  GtkWidget *pMenu;

  //window position
  gint x;
  gint y;

  //move
  gboolean moving;
  gint diff_x;
  gint diff_y;


  /// donnees transmises a la fonction.
  gpointer pUserData;
  /// fonction appelee pour liberer les donnees.
  GFreeFunc pFreeUserDataFunc;
} CairoDockDesklet;


CairoDockDesklet *cd_desklet_new(Icon *pIcon,
                                 GtkWidget *pInteractiveWidget,
                                 gpointer data,
                                 GFreeFunc pFreeUserDataFunc);

void cd_desklet_free(CairoDockDesklet *pDialog);


void cd_desklet_hide(CairoDockDesklet *pDialog);
void cd_desklet_show(CairoDockDesklet *pDialog);



#endif
