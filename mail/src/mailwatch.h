/*
 *  xfce4-mailwatch-plugin - a mail notification applet for the xfce4 panel
 *  Copyright (c) 2005 Brian Tarricone <bjt23@cornell.edu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License ONLY.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __MAILWATCH_H__
#define __MAILWATCH_H__

#include <time.h>

#include <glib.h>

#include "mailwatch-mailbox.h"

G_BEGIN_DECLS

#define XFCE_MAILWATCH_DEFAULT_TIMEOUT (10*60)  /* in seconds */
/* keep in sync with mailwatch-utils.c */
#define XFCE_MAILWATCH_ERROR           xfce_mailwatch_get_error_quark()

typedef struct _XfceMailwatch XfceMailwatch;
typedef void (*XMCallback)(XfceMailwatch *mailwatch, gpointer arg, gpointer user_data);

typedef enum
{
    XFCE_MAILWATCH_SIGNAL_TIMEOUT_CHANGED = 0,
    XFCE_MAILWATCH_SIGNAL_NEW_MESSAGE_COUNT_CHANGED,
    XFCE_MAILWATCH_SIGNAL_LOG_MESSAGE,
    XFCE_MAILWATCH_NUM_SIGNALS
} XfceMailwatchSignal;

typedef enum
{
    XFCE_MAILWATCH_LOG_INFO = 0,
    XFCE_MAILWATCH_LOG_WARNING,
    XFCE_MAILWATCH_LOG_ERROR,
    XFCE_MAILWATCH_N_LOG_LEVELS
} XfceMailwatchLogLevel;

typedef struct {
    XfceMailwatch           *mailwatch;
    XfceMailwatchLogLevel   level;
    time_t                  timestamp;
    gchar                   *mailbox_name;
    gchar                   *message;
} XfceMailwatchLogEntry;

GQuark xfce_mailwatch_get_error_quark  ();

XfceMailwatch *xfce_mailwatch_new      ();
void xfce_mailwatch_destroy            (XfceMailwatch *mailwatch);

void xfce_mailwatch_set_config_file    (XfceMailwatch *mailwatch,
                                        const gchar *filename);
G_CONST_RETURN gchar *xfce_mailwatch_get_config_file
                                       (XfceMailwatch *mailwatch);

gboolean xfce_mailwatch_load_config    (XfceMailwatch *mailwatch, GKeyFile *pKeyFile);
gboolean xfce_mailwatch_save_config    (XfceMailwatch *mailwatch, GKeyFile *pKeyFile);

guint xfce_mailwatch_get_new_messages  (XfceMailwatch *mailwatch);

void xfce_mailwatch_get_new_message_breakdown
                                       (XfceMailwatch *mailwatch,
                                        gchar ***mailbox_names,
                                        guint **new_message_counts);

void cd_mailwatch_get_mailboxes_infos( XfceMailwatch *mailwatch,
                                       GList **list_names,
                                       GList **mailboxes_data,
                                       GList **mailboxes_cmd  );

void cd_mailwatch_remove_account (XfceMailwatch *mailwatch, XfceMailwatchMailbox *mailbox);

void xfce_mailwatch_force_update       (XfceMailwatch *mailwatch);

GtkContainer *xfce_mailwatch_get_configuration_page
                                       (XfceMailwatch *mailwatch);

void xfce_mailwatch_signal_connect     (XfceMailwatch *mailwatch,
                                        XfceMailwatchSignal signal,
                                        XMCallback callback,
                                        gpointer user_data);
void xfce_mailwatch_signal_disconnect  (XfceMailwatch *mailwatch,
                                        XfceMailwatchSignal signal,
                                        XMCallback callback,
                                        gpointer user_data);

/*< only used by XfceMailwatchMailboxType implementations >*/
void xfce_mailwatch_signal_new_messages(XfceMailwatch *mailwatch,
                                        XfceMailwatchMailbox *mailbox,
                                        guint num_new_messages);
void xfce_mailwatch_log_message        (XfceMailwatch *mailwatch,
                                        XfceMailwatchMailbox *mailbox,
                                        XfceMailwatchLogLevel level,
                                        const gchar *fmt,
                                        ... );

void xfce_mailwatch_threads_enter      ();
void xfce_mailwatch_threads_leave      ();
G_END_DECLS

#endif
