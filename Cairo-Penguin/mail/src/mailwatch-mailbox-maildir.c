/*
 *  xfce4-mailwatch-plugin - a mail notification applet for the xfce4 panel
 *  Copyright (c) 2005 Pasi Orovuo <pasi.ov@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include "mailwatch.h"
#include "mailwatch-utils.h"
#include "cairo-dock.h"

#define XFCE_MAILWATCH_MAILDIR_MAILBOX( p ) ( (XfceMailwatchMaildirMailbox *) p )
#define BORDER                              ( 8 )

typedef enum {
    MAILDIR_MSG_START = 1,
    MAILDIR_MSG_PAUSE,
    MAILDIR_MSG_FORCE_UPDATE,
    MAILDIR_MSG_INTERVAL_CHANGED,
    MAILDIR_MSG_QUIT
} XfceMailwatchMaildirMessage;

typedef struct {
    XfceMailwatchMailbox    xfce_mailwatch_mailbox;

    XfceMailwatch           *mailwatch;

    gchar                   *path;
    gboolean                active;
    time_t                  mtime;

    guint                   interval;
    guint                   last_update;

    GMutex                  *mutex;
    GThread                 *thread;
    GAsyncQueue             *aqueue;
} XfceMailwatchMaildirMailbox;

static void
maildir_check_mail( XfceMailwatchMaildirMailbox *maildir )
{
    gchar           *path = NULL;
    struct stat     st;

    DBG( "-->>" );

    g_mutex_lock( maildir->mutex );
    if ( !maildir->path || !*(maildir->path) ) {
        goto out;
    }

    path = g_build_filename( maildir->path, "new", NULL );
    if ( stat( path, &st ) < 0 ) {
        xfce_mailwatch_log_message( maildir->mailwatch,
                                    XFCE_MAILWATCH_MAILBOX( maildir ),
                                    XFCE_MAILWATCH_LOG_ERROR,
                                    _( "Failed to get status of file %s: %s" ),
                                    path, g_strerror( errno ) );
        goto out;
    }

    if ( !S_ISDIR( st.st_mode ) ) {
        xfce_mailwatch_log_message( maildir->mailwatch,
                                    XFCE_MAILWATCH_MAILBOX( maildir ),
                                    XFCE_MAILWATCH_LOG_ERROR,
                                    _( "%s is not a directory. Is %s really a valid maildir?" ),
                                    path, maildir->path );
        goto out;
    }

    if ( st.st_mtime > maildir->mtime ) {
        GDir        *dir;
        GError      *error = NULL;

        dir = g_dir_open( path, 0, &error );

        if ( dir ) {
            int             count_new = 0;
            const gchar     *entry;

            while ( ( entry = g_dir_read_name( dir ) ) ) {
                count_new++;
            }
            g_dir_close( dir );

            xfce_mailwatch_signal_new_messages( maildir->mailwatch,
                    (XfceMailwatchMailbox *) maildir, count_new );
        }
        else {
            xfce_mailwatch_log_message( maildir->mailwatch,
                                        XFCE_MAILWATCH_MAILBOX( maildir ),
                                        XFCE_MAILWATCH_LOG_ERROR,
                                        "%s", error->message );
            g_error_free( error );
        }
        maildir->mtime = st.st_mtime;
    }

out:
    g_mutex_unlock( maildir->mutex );
    if ( path ) {
        g_free( path );
    }

    DBG( "<<--" );
}

static gpointer
maildir_main_thread( gpointer data ) {
    XfceMailwatchMaildirMailbox     *maildir = data;
    GTimeVal                        tv;

    DBG( "-->>" );

    g_async_queue_ref( maildir->aqueue );

    for ( ;; ) {
        XfceMailwatchMaildirMessage msg;

        g_get_current_time( &tv );
        g_time_val_add( &tv, G_USEC_PER_SEC * 5 );

        msg = GPOINTER_TO_INT( g_async_queue_timed_pop( maildir->aqueue, &tv ) );

        if ( msg ) {
            switch ( msg ) {
                case MAILDIR_MSG_START:
                    maildir->active = TRUE;
                    break;

                case MAILDIR_MSG_PAUSE:
                    maildir->active = FALSE;
                    break;

                case MAILDIR_MSG_FORCE_UPDATE:
                    maildir->last_update = 0;
                    break;

                case MAILDIR_MSG_INTERVAL_CHANGED:
                    maildir->interval = GPOINTER_TO_INT( g_async_queue_pop( maildir->aqueue ) );
                    break;

                case MAILDIR_MSG_QUIT:
                    g_async_queue_unref( maildir->aqueue );
                    g_thread_exit( NULL );
                    break;

                default:
                    DBG( "msg: %d", msg );
                    break;
            }
        }

        if ( ( maildir->active )
                && tv.tv_sec - maildir->last_update > maildir->interval ) {
            maildir_check_mail( maildir );
            maildir->last_update = tv.tv_sec;
        }
    }
    return ( NULL );
}

static XfceMailwatchMailbox *
maildir_new( XfceMailwatch *mailwatch, XfceMailwatchMailboxType *type )
{
    XfceMailwatchMaildirMailbox *maildir = NULL;

    DBG( "entering" );

    maildir = g_new0( XfceMailwatchMaildirMailbox, 1 );

    maildir->mailwatch      = mailwatch;
    maildir->path           = NULL;
    maildir->active         = FALSE;
    maildir->interval       = XFCE_MAILWATCH_DEFAULT_TIMEOUT;
    maildir->mutex          = g_mutex_new();
    maildir->aqueue         = g_async_queue_new();
    maildir->thread         = g_thread_create( maildir_main_thread, maildir, TRUE, NULL );

    return ( (XfceMailwatchMailbox *) maildir );
}

static void
maildir_free( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMaildirMailbox *maildir = XFCE_MAILWATCH_MAILDIR_MAILBOX( mailbox );

    DBG( "-->>" );

    g_async_queue_push( maildir->aqueue, GINT_TO_POINTER( MAILDIR_MSG_QUIT ) );
    g_thread_join( maildir->thread );
    g_async_queue_unref( maildir->aqueue );

    if ( maildir->path ) {
        g_free( maildir->path );
    }
    g_free( maildir );

    DBG( "<<--" );
}

static GList *
maildir_save_param_list( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMaildirMailbox *maildir = XFCE_MAILWATCH_MAILDIR_MAILBOX( mailbox );
    XfceMailwatchParam          *param;
    GList                       *settings = NULL;

    DBG( "-->>" );

    g_mutex_lock( maildir->mutex );

    param           = g_new( XfceMailwatchParam, 1 );
    param->key      = g_strdup( "path" );
    param->value    = g_strdup( ( maildir->path ) ? maildir->path : "" );
    settings        = g_list_append( settings, param );

    param           = g_new( XfceMailwatchParam, 1 );
    param->key      = g_strdup( "mtime" );
    param->value    = g_strdup_printf( "%li", maildir->mtime );
    settings        = g_list_append( settings, param );

    param           = g_new( XfceMailwatchParam, 1 );
    param->key      = g_strdup( "interval" );
    param->value    = g_strdup_printf( "%u", maildir->interval );
    settings        = g_list_append( settings, param );

    g_mutex_unlock( maildir->mutex );

    DBG( "<<--" );

    return ( settings );
}

static void
maildir_restore_param_list( XfceMailwatchMailbox *mailbox, GList *params )
{
    XfceMailwatchMaildirMailbox *maildir = XFCE_MAILWATCH_MAILDIR_MAILBOX( mailbox );
    GList                       *li;

    DBG( "-->>" );

    g_mutex_lock( maildir->mutex );

    for ( li = g_list_first( params ); li != NULL; li = g_list_next( li ) ) {
        XfceMailwatchParam  *param = (XfceMailwatchParam *) li->data;

        if ( !strcmp( param->key, "path" ) ) {
            if ( maildir->path ) {
                g_free( maildir->path );
            }
            maildir->path = g_strdup( param->value );
        }
        else if ( !strcmp( param->key, "mtime" ) ) {
            maildir->mtime = atol( param->value );
        }
        else if ( !strcmp( param->key, "interval" ) ) {
            maildir->interval = (guint) atol( param->value );
        }
    }

    g_mutex_unlock( maildir->mutex );

    DBG( "<<--" );
}

static gboolean
maildir_path_entry_changed_cb( GtkWidget *widget, XfceMailwatchMaildirMailbox *maildir )
{
    const gchar                 *text;

    DBG( "-->>" );

    g_mutex_lock( maildir->mutex );
    if ( maildir->path ) {
        g_free( maildir->path );
    }

    text = gtk_entry_get_text( GTK_ENTRY( widget ) );
    maildir->path = g_strdup( text ? text : "" );

    g_mutex_unlock( maildir->mutex );

    DBG( "<<--" );

    return ( FALSE );
}

static void
maildir_browse_button_clicked_cb( GtkWidget *button,
        XfceMailwatchMaildirMailbox *maildir )
{
    GtkWidget       *chooser;
    gint            result;
    GtkWidget       *parent;

    DBG( "-->>" );

    /* _TOFE_
    xfce_textdomain( GETTEXT_PACKAGE, LOCALEDIR, "UTF-8" );
    */

    parent = gtk_widget_get_toplevel( button );
    chooser = gtk_file_chooser_dialog_new( _( "Select Maildir Folder" ),
            GTK_WINDOW( parent ),
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OPEN, GTK_RESPONSE_OK,
            NULL );
    if ( maildir->path ) {
        gtk_file_chooser_set_filename( GTK_FILE_CHOOSER( chooser ), maildir->path );
    }

    result = gtk_dialog_run( GTK_DIALOG( chooser ) );
    if ( result == GTK_RESPONSE_OK ) {
        gchar       *path =
            gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( chooser ) );
        GtkWidget   *entry = g_object_get_data( G_OBJECT( button ), "maildir_entry" );

        gtk_entry_set_text( GTK_ENTRY( entry ), ( path ) ? path : "" );
        g_free( path );
    }

    gtk_widget_destroy( chooser );

    DBG( "<<--" );
}

static void
maildir_interval_changed_cb( GtkWidget *spinner, XfceMailwatchMaildirMailbox *maildir ) {

    DBG( "-->>" );

    g_async_queue_push( maildir->aqueue,
                        GINT_TO_POINTER( MAILDIR_MSG_INTERVAL_CHANGED ) );
    g_async_queue_push( maildir->aqueue,
        GINT_TO_POINTER( gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spinner ) ) * 60 ) );

    DBG( "<<--" );
}

static GtkContainer *
maildir_get_setup_page( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMaildirMailbox *maildir = XFCE_MAILWATCH_MAILDIR_MAILBOX( mailbox );
    GtkWidget                   *vbox, *hbox;
    GtkWidget                   *label, *entry;
    GtkWidget                   *button, *image;
    GtkWidget                   *spin;
    GtkSizeGroup                *sg;

    DBG( "-->>" );

    /* _TOFE_
    xfce_textdomain( GETTEXT_PACKAGE, LOCALEDIR, "UTF-8" );
    */

    vbox = gtk_vbox_new( FALSE, BORDER / 2 );
    gtk_widget_show( vbox );

    hbox = gtk_hbox_new( FALSE, BORDER );
    gtk_widget_show( hbox );
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

    sg = gtk_size_group_new( GTK_SIZE_GROUP_HORIZONTAL );

    label = gtk_label_new_with_mnemonic( _( "Maildir _Path:" ) );
    gtk_widget_show( label );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );
    gtk_size_group_add_widget( sg, label );

    entry = gtk_entry_new();
    g_mutex_lock( maildir->mutex );
    if ( maildir->path ) {
        gtk_entry_set_text( GTK_ENTRY( entry ), maildir->path );
    }
    g_mutex_unlock( maildir->mutex );

    gtk_widget_show( entry );
    gtk_box_pack_start( GTK_BOX( hbox ), entry, FALSE, FALSE, 0 );
    gtk_label_set_mnemonic_widget( GTK_LABEL( label ), entry );

    g_signal_connect( G_OBJECT( entry ), "changed",
            G_CALLBACK( maildir_path_entry_changed_cb ), maildir );

    button = gtk_button_new();
    gtk_widget_show( button );

    image = gtk_image_new_from_stock( GTK_STOCK_OPEN, GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_widget_show( image );

    gtk_container_add( GTK_CONTAINER( button ), image );
    gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );

    g_object_set_data( G_OBJECT( button ), "maildir_entry", entry );
    g_signal_connect( G_OBJECT( button ), "clicked",
            G_CALLBACK( maildir_browse_button_clicked_cb ), maildir );

    hbox = gtk_hbox_new( FALSE, BORDER );
    gtk_widget_show( hbox );
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new_with_mnemonic( _( "_Interval:" ) );
    gtk_widget_show( label );
    gtk_misc_set_alignment( GTK_MISC( label ), 1, 0.5 );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );
    gtk_size_group_add_widget( sg, label );

    spin = gtk_spin_button_new_with_range( 1.0, 1440.0, 1.0 );
    gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
    gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), maildir->interval / 60 );
    gtk_widget_show( spin );
    gtk_box_pack_start( GTK_BOX( hbox ), spin, FALSE, FALSE, 0 );
    g_signal_connect( G_OBJECT( spin ), "value-changed",
            G_CALLBACK( maildir_interval_changed_cb ), maildir );
    gtk_label_set_mnemonic_widget( GTK_LABEL( label ), spin );

    label = gtk_label_new( _( "minute(s)." ) );
    gtk_widget_show( label );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

    DBG( "<<--" );

    return ( GTK_CONTAINER( vbox ) );
}

static void
maildir_force_update_cb( XfceMailwatchMailbox *mailbox ) {
    XfceMailwatchMaildirMailbox     *maildir = XFCE_MAILWATCH_MAILDIR_MAILBOX( mailbox );
    DBG( "-->>" );

    g_async_queue_push( maildir->aqueue, GINT_TO_POINTER( MAILDIR_MSG_FORCE_UPDATE ) );

    DBG( "<<--" );
}

static void
maildir_set_activated( XfceMailwatchMailbox *mailbox, gboolean activated )
{
    XfceMailwatchMaildirMailbox     *maildir = XFCE_MAILWATCH_MAILDIR_MAILBOX( mailbox );

    DBG( "-->>" );

    g_async_queue_push( maildir->aqueue,
                        GINT_TO_POINTER( activated ? MAILDIR_MSG_START : MAILDIR_MSG_PAUSE ) );

    DBG( "<<--" );
}

XfceMailwatchMailboxType    builtin_mailbox_type_maildir = {
    "maildir",
    N_( "Local Maildir Spool" ),
    N_( "The Maildir plugin can watch a local maildir-style mail spool for new messages." ),
    maildir_new,
    maildir_set_activated,
    maildir_force_update_cb,
    maildir_get_setup_page,
    maildir_restore_param_list,
    maildir_save_param_list,
    maildir_free
};
