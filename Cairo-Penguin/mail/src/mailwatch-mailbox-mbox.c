/*
 *  xfce4-mailwatch-plugin - a mail notification applet for the xfce4 panel
 *  Copyright (c) 2005 Pasi Orovuo <pasi.ov@gmail.com>
 *                     Brian Tarricone <bjt23@cornell.edu>
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

#define XFCE_MAILWATCH_MBOX_MAILBOX( p )    ( (XfceMailwatchMboxMailbox *) p )
#define BORDER                              ( 8 )

#define MBOX_MSG_START                      GINT_TO_POINTER( 1 )
#define MBOX_MSG_PAUSE                      GINT_TO_POINTER( 2 )
#define MBOX_MSG_QUIT                       GINT_TO_POINTER( 3 )
#define MBOX_MSG_UPDATE                     GINT_TO_POINTER( 4 )

typedef struct {
    XfceMailwatchMailbox    xfce_mailwatch_mailbox;
    XfceMailwatch           *mailwatch;

    gchar                   *fn;
    time_t                  ctime;
    size_t                  size;
    guint                   new_messages;
    guint                   interval;

    GThread                 *thread;
    GAsyncQueue             *queue;
    GMutex                  *settings_mutex;
} XfceMailwatchMboxMailbox;

static void
mbox_check_mail( XfceMailwatchMboxMailbox *mbox )
{
    gchar           *mailbox;
    struct stat     st;
    guint           num_new = 0;

    g_mutex_lock( mbox->settings_mutex );
    if ( !mbox->fn ) {
        g_mutex_unlock( mbox->settings_mutex );
        return;
    }
    mailbox = g_strdup( mbox->fn );
    g_mutex_unlock( mbox->settings_mutex );

    /* For some reason g_stat() doesn't update
     * ctime */
    if ( stat( mailbox, &st ) < 0 ) {
        xfce_mailwatch_log_message( mbox->mailwatch,
                                    XFCE_MAILWATCH_MAILBOX( mbox ),
                                    XFCE_MAILWATCH_LOG_ERROR,
                                    _( "Failed to get status of file %s: %s" ),
                                    mailbox, g_strerror( errno ) );
        g_free( mailbox );
        return;
    }

    if ( st.st_ctime > mbox->ctime ) {
        gboolean        in_header = FALSE;
        gboolean        cur_new = FALSE;
        gchar           *p;
        GIOChannel      *ioc;
        gsize           nl;
        GError          *error = NULL;

        num_new = 0;

        ioc = g_io_channel_new_file( mailbox, "r", &error );
        if ( !ioc ) {
            xfce_mailwatch_log_message( mbox->mailwatch,
                                        XFCE_MAILWATCH_MAILBOX( mbox ),
                                        XFCE_MAILWATCH_LOG_ERROR,
                                        error->message );
            g_free( mailbox );
            g_error_free( error );
            return;
        }
        if ( g_io_channel_set_encoding( ioc, NULL, &error ) != G_IO_STATUS_NORMAL ) {
            xfce_mailwatch_log_message( mbox->mailwatch,
                                        XFCE_MAILWATCH_MAILBOX( mbox ),
                                        XFCE_MAILWATCH_LOG_WARNING,
                                        error->message );
            g_error_free( error );
            error = NULL;
        }

        if ( mbox->size && st.st_size > mbox->size ) {
            /* G_SEEK_CUR is same as G_SEEK_SET in this context. */
            if ( g_io_channel_seek_position( ioc, mbox->size, G_SEEK_CUR, &error ) !=  G_IO_STATUS_NORMAL ) {
                xfce_mailwatch_log_message( mbox->mailwatch,
                                            XFCE_MAILWATCH_MAILBOX( mbox ),
                                            XFCE_MAILWATCH_LOG_ERROR,
                                            error->message );
                g_io_channel_unref( ioc );
                g_free( mailbox );
                g_error_free( error );
                return;
            }
            num_new += mbox->new_messages;
        }

        while ( g_io_channel_read_line( ioc, &p, NULL, &nl, NULL ) == G_IO_STATUS_NORMAL ) {
            p[nl] = 0;

            if ( !in_header ) {
                if ( !strncmp( p, "From ", 5 ) ) {
                    in_header = TRUE;
                    cur_new = TRUE;
                }
            }
            else {
                if ( *p == 0 ) {
                    in_header = FALSE;

                    if ( cur_new ) {
                        num_new++;
                    }
                }
                else if ( !strncmp( p, "Status: ", 8 ) ) {
                    gchar       *q = p + 8;
                    if ( strchr( q, 'R' ) || strchr( q, 'O' ) ) {
                        cur_new = FALSE;
                    }
                }
                else if ( !strncmp( p, "X-Mozilla-Status: ", 18 ) ) {
                    if ( strncmp( p + 18, "0000", 4 ) ) {
                        cur_new = FALSE;
                    }
                }
            }
            g_free( p );
        }
        g_io_channel_unref( ioc );

        if ( st.st_size > mbox->size && num_new <= mbox->new_messages ) {
            /* Assume mailbox opened and some headers added by client */
            num_new = mbox->new_messages = 0;
        }
        else {
            mbox->new_messages = num_new;
        }

        xfce_mailwatch_signal_new_messages( mbox->mailwatch, (XfceMailwatchMailbox *) mbox, num_new );

        mbox->ctime = st.st_ctime;
        mbox->size = st.st_size;
    }
    g_free( mailbox );
}

gpointer
mbox_check_mail_thread( gpointer data )
{
    XfceMailwatchMboxMailbox    *mbox = XFCE_MAILWATCH_MBOX_MAILBOX( data );
    gboolean                    active = FALSE;
    GTimeVal                    tv;
    glong                       last_update = 0;

    g_async_queue_ref( mbox->queue );

    while ( TRUE ) {
        gpointer        msg;

        g_get_current_time( &tv );
        g_time_val_add( &tv, G_USEC_PER_SEC * 5 );

        msg = g_async_queue_timed_pop( mbox->queue, &tv );

        if ( msg ) {
            if ( msg == MBOX_MSG_START ) {
                active = TRUE;
            }
            else if ( msg == MBOX_MSG_PAUSE ) {
                active = FALSE;
            }
            else if ( msg == MBOX_MSG_QUIT ) {
                g_async_queue_unref( mbox->queue );
                g_thread_exit( NULL );
            }
        }

        if ( active && ( msg == MBOX_MSG_UPDATE
                || tv.tv_sec - last_update >= mbox->interval ) )
        {
            mbox_check_mail( mbox );
            last_update = tv.tv_sec;
        }
    }
}

static XfceMailwatchMailbox *
mbox_new( XfceMailwatch *mailwatch, XfceMailwatchMailboxType *type )
{
    XfceMailwatchMboxMailbox    *mbox = NULL;

    mbox = g_new0( XfceMailwatchMboxMailbox, 1 );

    mbox->mailwatch     = mailwatch;

    mbox->settings_mutex = g_mutex_new();
    mbox->queue = g_async_queue_new();
    mbox->interval = XFCE_MAILWATCH_DEFAULT_TIMEOUT;
    mbox->thread = g_thread_create( mbox_check_mail_thread, mbox, TRUE, NULL );

    return ( (XfceMailwatchMailbox *) mbox );
}

static void
mbox_free( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMboxMailbox    *mbox = XFCE_MAILWATCH_MBOX_MAILBOX( mailbox );

    g_async_queue_push( mbox->queue, MBOX_MSG_QUIT );
    g_thread_join( mbox->thread );

    g_mutex_free( mbox->settings_mutex );
    g_async_queue_unref( mbox->queue );

    if ( mbox->fn ) {
        g_free( mbox->fn );
    }
    g_free( mbox );
}

static GList *
mbox_save_settings( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMboxMailbox    *mbox = XFCE_MAILWATCH_MBOX_MAILBOX( mailbox );
    XfceMailwatchParam          *param;
    GList                       *settings = NULL;

    g_mutex_lock( mbox->settings_mutex );

    param = g_new( XfceMailwatchParam, 1 );
    param->key      = g_strdup( "filename" );
    param->value    = g_strdup( ( mbox->fn ) ? mbox->fn : "" );
    settings = g_list_append( settings, param );

    param = g_new( XfceMailwatchParam, 1 );
    param->key      = g_strdup( "ctime" );
    param->value    = g_strdup_printf( "%li", mbox->ctime );
    settings = g_list_append( settings, param );

    param = g_new( XfceMailwatchParam, 1 );
    param->key      = g_strdup( "size" );
    param->value    = g_strdup_printf( "%u", mbox->size );
    settings = g_list_append( settings, param );

    param = g_new( XfceMailwatchParam, 1 );
    param->key      = g_strdup( "interval" );
    param->value    = g_strdup_printf( "%u", mbox->interval );
    settings = g_list_append( settings, param );

    g_mutex_unlock( mbox->settings_mutex );

    return ( settings );
}

static void
mbox_restore_settings( XfceMailwatchMailbox *mailbox, GList *settings )
{
    XfceMailwatchMboxMailbox    *mbox = XFCE_MAILWATCH_MBOX_MAILBOX( mailbox );
    GList                       *li;

    g_mutex_lock( mbox->settings_mutex );

    for ( li = g_list_first( settings ); li != NULL; li = g_list_next( li ) ) {
        XfceMailwatchParam      *p = (XfceMailwatchParam *) li->data;

        if ( !strcmp( p->key, "filename" ) ) {
            if ( mbox->fn ) {
                g_free( mbox->fn );
            }
            mbox->fn = g_strdup( p->value );
        }
        else if ( !strcmp( p->key, "ctime" ) ) {
            mbox->ctime = atol( p->value );
        }
        else if ( !strcmp( p->key, "size" ) ) {
            mbox->size = (size_t) atol( p->value );
        }
        else if ( !strcmp( p->key, "interval" ) ) {
            mbox->interval = (guint) atol( p->value );
        }
    }

    g_mutex_unlock( mbox->settings_mutex );
}

static gboolean
mbox_path_entry_changed_cb( GtkWidget *widget, XfceMailwatchMboxMailbox *mbox )
{
    const gchar     *text;

    g_mutex_lock( mbox->settings_mutex );
    if ( mbox->fn ) {
        g_free( mbox->fn );
    }

    text = gtk_entry_get_text( GTK_ENTRY( widget ) );
    if ( text ) {
        mbox->fn = g_strdup( text );
    }
    else {
        mbox->fn = g_strdup( "" );
    }
    g_mutex_unlock( mbox->settings_mutex );

    return ( FALSE );
}

static void
mbox_browse_button_clicked_cb( GtkWidget *button,
        XfceMailwatchMboxMailbox *mbox )
{
    GtkWidget       *chooser;
    gint            result;
    GtkWidget       *top;

    /* _TOFE
    xfce_textdomain( GETTEXT_PACKAGE, LOCALEDIR, "UTF-8" );
    */

    top = gtk_widget_get_toplevel( button );

    chooser = gtk_file_chooser_dialog_new( _( "Select mbox file" ),
            GTK_WINDOW( top ),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OK, GTK_RESPONSE_OK,
            NULL );
    if ( mbox->fn ) {
        gtk_file_chooser_set_filename( GTK_FILE_CHOOSER( chooser ), mbox->fn );
    }

    result = gtk_dialog_run( GTK_DIALOG( chooser ) );
    if ( result == GTK_RESPONSE_OK ) {
        gchar       *fn =
            gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( chooser ) );
        GtkWidget   *entry = g_object_get_data( G_OBJECT( button ), "mbox_entry" );

        gtk_entry_set_text( GTK_ENTRY( entry ), ( fn ) ? fn : "" );
        g_free( fn );
    }
    gtk_widget_destroy( chooser );
}

static void
mbox_interval_changed_cb( GtkWidget *spinner, XfceMailwatchMboxMailbox *mbox ) {
    mbox->interval = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spinner ) ) * 60;
}

static GtkContainer *
mbox_get_setup_page( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMboxMailbox    *mbox = XFCE_MAILWATCH_MBOX_MAILBOX( mailbox );
    GtkWidget                   *vbox, *hbox;
    GtkWidget                   *label, *entry;
    GtkWidget                   *button, *image, *spinner;
    GtkSizeGroup                *sg;

    /* _TOFE_
    xfce_textdomain( GETTEXT_PACKAGE, LOCALEDIR, "UTF-8" );
    */

    vbox = gtk_vbox_new( FALSE, BORDER / 2 );
    gtk_widget_show( vbox );

    hbox = gtk_hbox_new( FALSE, BORDER );
    gtk_widget_show( hbox );
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

    sg = gtk_size_group_new( GTK_SIZE_GROUP_HORIZONTAL );

    label = gtk_label_new_with_mnemonic( _( "Mbox _Filename:" ) );
    gtk_widget_show( label );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

    gtk_size_group_add_widget( GTK_SIZE_GROUP( sg ), label );

    entry = gtk_entry_new();
    g_mutex_lock( mbox->settings_mutex );
    if ( mbox->fn ) {
        gtk_entry_set_text( GTK_ENTRY( entry ), mbox->fn );
    }
    g_mutex_unlock( mbox->settings_mutex );
    gtk_widget_show( entry );
    gtk_box_pack_start( GTK_BOX( hbox ), entry, FALSE, FALSE, 0 );
    gtk_label_set_mnemonic_widget( GTK_LABEL( label ), entry );

    g_signal_connect( G_OBJECT( entry ), "changed",
            G_CALLBACK( mbox_path_entry_changed_cb ), mbox );

    button = gtk_button_new();
    gtk_widget_show( button );

    image = gtk_image_new_from_stock( GTK_STOCK_OPEN, GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_widget_show( image );

    gtk_container_add( GTK_CONTAINER( button ), image );
    gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );

    g_object_set_data( G_OBJECT( button ), "mbox_entry", entry );
    g_signal_connect( G_OBJECT( button ), "clicked",
            G_CALLBACK( mbox_browse_button_clicked_cb ), mbox );

    hbox = gtk_hbox_new( FALSE, BORDER );
    gtk_widget_show( hbox );
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new_with_mnemonic( _( "_Interval:" ) );
    gtk_widget_show( label );
    gtk_misc_set_alignment( GTK_MISC( label ), 1, 0.5 );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

    gtk_size_group_add_widget( GTK_SIZE_GROUP( sg ), label );

    spinner = gtk_spin_button_new_with_range( 1.0, 1440.0, 1.0 );
    gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spinner ), TRUE );
    gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spinner ), FALSE );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON( spinner ), mbox->interval / 60 );
    gtk_widget_show( spinner );
    gtk_box_pack_start( GTK_BOX( hbox ), spinner, FALSE, FALSE, 0 );
    g_signal_connect( G_OBJECT( spinner ), "value-changed",
            G_CALLBACK( mbox_interval_changed_cb ), mbox );
    gtk_label_set_mnemonic_widget( GTK_LABEL( label ), spinner );

    label = gtk_label_new( _( "minute(s)." ) );
    gtk_widget_show( label );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

    return ( GTK_CONTAINER( vbox ) );
}

static void
mbox_activate( XfceMailwatchMailbox *mailbox, gboolean activated )
{
    XfceMailwatchMboxMailbox    *mbox = XFCE_MAILWATCH_MBOX_MAILBOX( mailbox );

    g_async_queue_push( mbox->queue, ( activated ) ? MBOX_MSG_START : MBOX_MSG_PAUSE );
}

static void
mbox_force_update( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMboxMailbox    *mbox = XFCE_MAILWATCH_MBOX_MAILBOX( mailbox );

    g_async_queue_push( mbox->queue, MBOX_MSG_UPDATE );
}

XfceMailwatchMailboxType    builtin_mailbox_type_mbox = {
    "mbox",
    N_( "Local Mbox spool" ),
    N_( "Mbox plugin watches a local mbox-type mail spool for new messages." ),
    mbox_new,
    mbox_activate,
    mbox_force_update,
    mbox_get_setup_page,
    mbox_restore_settings,
    mbox_save_settings,
    mbox_free
};
