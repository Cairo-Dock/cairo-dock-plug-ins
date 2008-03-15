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

#define XFCE_MAILWATCH_MH_MAILBOX( p )      ( (XfceMailwatchMHMailbox *) p )
#define BORDER          ( 8 )

#define MH_PLUGIN_NAME  "mh-maildir-plugin"
/* For now these ain't user configurable (from gui) - Should they be? */
#define MH_PROFILE      ( ".mh_profile" )
#define MH_SEQUENCES    ( ".mh_sequences" )
#define MH_INBOX        ( "inbox" )
#define MH_UNSEEN_SEQ   ( "unseen" )

typedef enum {
    MH_MSG_START = 1,
    MH_MSG_PAUSE,
    MH_MSG_FORCE_UPDATE,
    MH_MSG_TIMEOUT_CHANGED,
    MH_MSG_QUIT
} XfceMailwatchMHMessage;

typedef struct {
    XfceMailwatchMailbox    *xfce_mailwatch_mailbox;
    XfceMailwatch           *mailwatch;

    gchar                   *mh_profile_fn;
    time_t                  mh_profile_ctime;
    gchar                   *mh_sequences_fn;
    time_t                  mh_sequences_ctime;
    gchar                   *unseen_sequence; /* This should be a list as there can be multiple? */
    gboolean                active;

    guint                   timeout;
    guint                   last_update;

    GThread                 *thread;
    GAsyncQueue             *aqueue;
} XfceMailwatchMHMailbox;

typedef struct {
    gchar                   *component;
    gchar                   *value;
} MHProfileEntry;

static void
mh_profile_free( GList *list )
{
    GList       *li;

    DBG( " " );

    for ( li = g_list_first( list ); li != NULL; li = g_list_next( li ) ) {
        MHProfileEntry     *entry = li->data;

        g_free( entry->component );
        g_free( entry->value );
    }
    g_list_free( list );
}

#ifdef DEBUG
static void
mh_profile_print( GList *profile )
{
    GList       *li;

    for ( li = g_list_first( profile ); li != NULL; li = g_list_next( li ) ) {
        MHProfileEntry     *e = li->data;

        DBG( "%s: %s", e->component, e->value );
    }
}
#endif /* DEBUG */

static MHProfileEntry *
mh_profile_entry_create_new( const char *line )
{
    MHProfileEntry *entry = NULL;
    gchar                       **v;

    v = g_strsplit( line, ":", 2 );

    if ( v && v[0] && v[1] ) {
        entry = g_new0( MHProfileEntry, 1 );

        entry->component    = g_strstrip( v[0] );
        entry->value        = g_strstrip( v[1] );

        g_free( v );
    }
    else {
        g_strfreev( v );
    }
    return ( entry );
}

static gchar *
mh_profile_readline( XfceMailwatchMHMailbox *mh, const gchar *mh_profile, GIOChannel *ioc )
{
    gchar           *line = NULL, *curline;
    gsize           nread, newline;
    GIOStatus       status;
    GError          *error = NULL;

    g_return_val_if_fail( ioc != NULL, NULL );

    status = g_io_channel_read_line( ioc, &curline,
                                     &nread, &newline,
                                     &error );
    while ( status == G_IO_STATUS_NORMAL ) {
        gchar       c;

        curline[newline] = 0;

        if ( !*curline ) {
            /* An mh profile shouldn't contain blank lines. Ignore 'em */
            g_free( curline );
        }
        else {
            if ( !line ) {
                if ( g_ascii_isspace( *curline ) ) {
                    /* The profile isn't right, ignore */
                    curline = g_strstrip( curline );
                }

                line = curline;
            }
            else {
                gchar       *p;

                curline = g_strstrip( curline );

                p = g_strconcat( line, curline, NULL );

                g_free( line );
                g_free( curline );

                line = p;
            }

            if ( g_io_channel_read_chars( ioc, &c, 1, &nread, NULL ) == G_IO_STATUS_NORMAL ) {
                if ( !g_ascii_isspace( c ) || g_ascii_iscntrl( c ) ) {
                    /* g_ascii_iscntrl() is supposed to catch newlines */
                    g_io_channel_seek_position( ioc, -1, G_SEEK_CUR, NULL );
                    break;
                }
            }
        }

        status = g_io_channel_read_line( ioc, &curline,
                                         &nread, &newline,
                                         &error );
    }

    if ( status == G_IO_STATUS_ERROR ) {
        xfce_mailwatch_log_message( mh->mailwatch, XFCE_MAILWATCH_MAILBOX( mh ),
                                    XFCE_MAILWATCH_LOG_WARNING,
                                    "Error reading file %s: %s",
                                    mh_profile, error->message );

        g_error_free( error );
    }

    return ( line );
}

static GList *
mh_profile_read( XfceMailwatchMHMailbox *mh, const gchar *mh_profile )
{
    GIOChannel      *ioc;
    GError          *error = NULL;
    gchar           *line = NULL;
    GList           *li = NULL;

    DBG( "-->>" );

    ioc = g_io_channel_new_file( mh_profile, "r", &error );
    if ( !ioc ) {
        xfce_mailwatch_log_message( mh->mailwatch, XFCE_MAILWATCH_MAILBOX( mh ),
                                    XFCE_MAILWATCH_LOG_ERROR,
                                    "Failed to open file %s: %s",
                                    mh_profile, error->message );
        g_error_free( error );
        return ( NULL );
    }
    g_io_channel_set_encoding( ioc, NULL, NULL );

    while ( ( line = mh_profile_readline( mh, mh_profile, ioc ) ) ) {
        MHProfileEntry          *entry;

        entry = mh_profile_entry_create_new( line );
        if ( entry ) {
            li = g_list_prepend( li, entry );
        }
        else {
            xfce_mailwatch_log_message( mh->mailwatch, XFCE_MAILWATCH_MAILBOX( mh ),
                    XFCE_MAILWATCH_LOG_WARNING,
                    _( "Malformed line %s in %s ignored." ), line, mh_profile );
        }
        g_free( line );
    }

    g_io_channel_shutdown( ioc, FALSE, NULL );
    g_io_channel_unref( ioc );

    DBG( "<<--" );
    return ( li );
}

static gint
mh_profile_entry_compare( gconstpointer a, gconstpointer b )
{
    const MHProfileEntry    *e = a;

    DBG( "%s <-> %s", e->component, (const gchar *) b );

    return ( g_ascii_strcasecmp( e->component, b ) );
}

static gchar *
mh_profile_entry_get_value( GList *profile, const gchar *component )
{
    MHProfileEntry          *entry;
    GList                   *li;

    DBG( "--> %s", component );

    li = g_list_find_custom( profile, component, mh_profile_entry_compare );
    if ( !li ) {
        DBG( "NULL" );
        return ( NULL );
    }
    entry = li->data;

    g_assert( entry != NULL );

    return ( g_strdup( entry->value ) );
}

static gchar *
mh_get_profile_filename( void )
{
    const gchar *env;
    gchar       *mh_profile;

    env = g_getenv( "MH" );
    if ( env ) {
        if ( !g_path_is_absolute( env ) ) {
            gchar       *curdir = g_get_current_dir();

            mh_profile = g_build_filename( curdir, env, NULL );

            g_free( curdir );
        }
        else {
            mh_profile = g_strdup( env );
        }
    }
    else {
        mh_profile = g_build_filename( g_get_home_dir(), MH_PROFILE, NULL );
    }
    return ( mh_profile );
}

static void
mh_read_config( XfceMailwatchMHMailbox *mh )
{
    gchar       *mh_path = NULL, *mh_inbox = NULL;
    gchar       *mh_sequences = NULL, *tmpptr = NULL;
    GList       *profile;

    DBG( "-->>" );

    if ( mh->mh_sequences_fn ) {
        g_free( mh->mh_sequences_fn );
        mh->mh_sequences_fn = NULL;
    }
    if ( mh->unseen_sequence ) {
        g_free( mh->unseen_sequence );
        mh->unseen_sequence = NULL;
    }

    if ( !mh->mh_profile_fn ) {
        mh->mh_profile_fn = mh_get_profile_filename();
    }

    profile = mh_profile_read( mh, mh->mh_profile_fn );
    if ( !profile ) {
        DBG( "Profile == NULL" );
        return;
    }

#ifdef DEBUG
    mh_profile_print( profile );
#endif

    mh_path = mh_profile_entry_get_value( profile, "Path" );
    if ( !mh_path ) {
        mh_profile_free( profile );
        return;
    }

    if ( !g_path_is_absolute( mh_path ) ) {
        tmpptr = g_build_filename( g_get_home_dir(), mh_path, NULL );
        g_free( mh_path );
        mh_path = tmpptr;
    }

    mh_inbox = mh_profile_entry_get_value( profile, "Inbox" );
    mh_sequences = mh_profile_entry_get_value( profile, "mh-sequences" );

    mh->unseen_sequence = mh_profile_entry_get_value( profile, "Unseen-Sequence" );

    mh->mh_sequences_fn = g_build_filename( mh_path,
            mh_inbox ? mh_inbox : MH_INBOX,
            mh_sequences ? mh_sequences : MH_SEQUENCES,
            NULL );

    g_free( mh_path );
    if ( mh_inbox ) {
        g_free( mh_inbox );
    }
    if ( mh_sequences ) {
        g_free( mh_sequences );
    }
    mh_profile_free( profile );
}

static void
mh_check_mail( XfceMailwatchMHMailbox *mh )
{
    struct stat     st;

    DBG( "-->>" );

    if ( !mh->mh_profile_fn ) {
        mh->mh_profile_fn = mh_get_profile_filename();
    }

    if ( stat( mh->mh_profile_fn, &st ) == 0 ) {
        if ( st.st_ctime != mh->mh_profile_ctime ) {
            mh_read_config( mh );
            mh->mh_profile_ctime = st.st_ctime;
        }
    }
    else {
        xfce_mailwatch_log_message( mh->mailwatch, XFCE_MAILWATCH_MAILBOX( mh ),
                                    XFCE_MAILWATCH_LOG_WARNING,
                                    _( "Failed to get status of file %s: %s" ),
                                    mh->mh_profile_fn, strerror( errno ) );
    }

    if ( !mh->mh_sequences_fn ) {
        return;
    }

    if ( stat( mh->mh_sequences_fn, &st ) < 0 ) {
        xfce_mailwatch_log_message( mh->mailwatch, XFCE_MAILWATCH_MAILBOX( mh ),
                                    XFCE_MAILWATCH_LOG_ERROR,
                                    _( "Failed to get status of file %s: %s" ),
                                    mh->mh_sequences_fn, strerror( errno ) );
    }
    else {
        if ( st.st_ctime != mh->mh_sequences_ctime ) {
            GList           *seqlist;
            gchar           *unseen;
            gulong          num_new = 0;

            mh->mh_sequences_ctime = st.st_ctime;

            seqlist = mh_profile_read( mh, mh->mh_sequences_fn );

#ifdef DEBUG
            mh_profile_print( seqlist );
#endif

            unseen = mh_profile_entry_get_value( seqlist,
                    mh->unseen_sequence ? mh->unseen_sequence : MH_UNSEEN_SEQ  );
            mh_profile_free( seqlist );
            if ( unseen ) {
                gchar       **v;
                guint       i;

                v = g_strsplit_set( unseen, " \t\r\n", 0 );
                g_free( unseen );

                for ( i = 0; v[i] != NULL; i++ ) {
                    gchar       *q = NULL;
                    gulong      l1, l2;

                    l1 = strtoul( v[i], &q, 10 );
                    if ( q && *q ) {
                        q++;

                        l2 = strtoul( q, NULL, 10 );
                        if ( l2 ) {
                            l1 = 1 + l2 - l1;
                        }
                        else {
                            /* In this case some sort of error occured */
                            l1 = 1;
                        }
                    }
                    else {
                        l1 = 1;
                    }

                    num_new += l1;
                }
                g_strfreev( v );
            }
            xfce_mailwatch_signal_new_messages( mh->mailwatch, XFCE_MAILWATCH_MAILBOX( mh ), num_new );
        }
    }

    DBG( "<<--" );
}

static gpointer
mh_main_thread( gpointer data )
{
    XfceMailwatchMHMailbox  *mh = XFCE_MAILWATCH_MH_MAILBOX( data );
    GTimeVal                tv;

    g_async_queue_ref( mh->aqueue );

    for ( ;; ) {
        XfceMailwatchMHMessage  msg;

        g_get_current_time( &tv );
        g_time_val_add( &tv, G_USEC_PER_SEC * 5 );

        msg = GPOINTER_TO_INT( g_async_queue_timed_pop( mh->aqueue, &tv ) );

        if ( msg ) {
            switch ( msg ) {
                case MH_MSG_START:
                    mh->active = TRUE;
                    break;

                case MH_MSG_PAUSE:
                    mh->active = FALSE;
                    break;

                case MH_MSG_TIMEOUT_CHANGED:
                    mh->timeout = GPOINTER_TO_INT( g_async_queue_pop( mh->aqueue ) );
                    break;

                case MH_MSG_FORCE_UPDATE:
                    mh->last_update = 0;
                    break;

                case MH_MSG_QUIT:
                    g_async_queue_unref( mh->aqueue );
                    g_thread_exit( NULL );
                    break;
            }
        }

        if ( mh->active &&
             tv.tv_sec - mh->last_update > mh->timeout ) {
            mh_check_mail( mh );
            mh->last_update = tv.tv_sec;
        }
    }

    return ( NULL );
}

static XfceMailwatchMailbox *
mh_new( XfceMailwatch *mailwatch, XfceMailwatchMailboxType *type )
{
    XfceMailwatchMHMailbox  *mh;

    DBG( "entering" );

    mh = g_new0( XfceMailwatchMHMailbox, 1 );

    mh->mailwatch       = mailwatch;
    mh->active          = FALSE;
    mh->timeout         = XFCE_MAILWATCH_DEFAULT_TIMEOUT;
    mh->aqueue          = g_async_queue_new();
    mh->thread          = g_thread_create( mh_main_thread, mh, TRUE, NULL );

    return ( XFCE_MAILWATCH_MAILBOX( mh ) );
}

static void
mh_free( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMHMailbox  *mh = XFCE_MAILWATCH_MH_MAILBOX( mailbox );

    DBG( "-->>" );

    g_async_queue_push( mh->aqueue, GINT_TO_POINTER( MH_MSG_QUIT ) );
    g_thread_join( mh->thread );
    g_async_queue_unref( mh->aqueue );

    if ( mh->mh_profile_fn ) {
        g_free( mh->mh_profile_fn );
    }
    if ( mh->mh_sequences_fn ) {
        g_free( mh->mh_sequences_fn );
    }
    if ( mh->unseen_sequence ) {
        g_free( mh->unseen_sequence );
    }

    g_free( mh );
}

static void
mh_restore_param_list( XfceMailwatchMailbox *mailbox, GList *params )
{
    XfceMailwatchMHMailbox  *mh = XFCE_MAILWATCH_MH_MAILBOX( mailbox );
    GList                   *li;

    DBG( "-->>" );

    for ( li = g_list_first( params ); li != NULL; li = g_list_next( li ) ) {
        XfceMailwatchParam  *param = li->data;

        if ( !strcmp( param->key, "timeout" ) ) {
            mh->timeout = (guint) atol( param->value );
        }
    }
}

static GList *
mh_save_param_list( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMHMailbox  *mh = XFCE_MAILWATCH_MH_MAILBOX( mailbox );
    XfceMailwatchParam      *param;
    GList                   *params = NULL;

    DBG( "-->>" );

    param           = g_new( XfceMailwatchParam, 1 );
    param->key      = g_strdup( "timeout" );
    param->value    = g_strdup_printf( "%u", mh->timeout );
    params = g_list_prepend( params, param );

    return ( params );
}

static void
mh_timeout_changed_cb( GtkWidget *spinner, XfceMailwatchMHMailbox *mh )
{
    DBG( "-->>" );
    g_async_queue_push( mh->aqueue, GINT_TO_POINTER( MH_MSG_TIMEOUT_CHANGED ) );
    g_async_queue_push( mh->aqueue,
            GINT_TO_POINTER( gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spinner ) ) * 60 ) );
}

static GtkContainer *
mh_get_setup_page( XfceMailwatchMailbox *mailbox )
{
    XfceMailwatchMHMailbox  *mh = XFCE_MAILWATCH_MH_MAILBOX( mailbox );
    GtkWidget               *vbox, *hbox;
    GtkWidget               *label, *spinner;

    vbox = gtk_vbox_new( FALSE, BORDER );
    gtk_widget_show( vbox );

    hbox = gtk_hbox_new( FALSE, BORDER );
    gtk_widget_show( hbox );
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new( _( "The configuration of this plugin is read from\n"
                              "the default mh maildir profile file ~/.mh_profile" ) );
    gtk_widget_show( label );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

    hbox = gtk_hbox_new( FALSE, BORDER );
    gtk_widget_show( hbox );
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new_with_mnemonic( _( "_Interval:" ) );
    gtk_widget_show( label );
    gtk_misc_set_alignment( GTK_MISC( label ), 1, 0.5 );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

    spinner = gtk_spin_button_new_with_range( 1.0, 1440.0, 1.0 );
    gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spinner ), TRUE );
    gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spinner ), FALSE );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON( spinner ), mh->timeout / 60 );
    gtk_widget_show( spinner );
    gtk_box_pack_start( GTK_BOX( hbox ), spinner, FALSE, FALSE, 0 );
    g_signal_connect( G_OBJECT( spinner ), "value-changed",
            G_CALLBACK( mh_timeout_changed_cb ), mh );
    gtk_label_set_mnemonic_widget( GTK_LABEL( label ), spinner );

    label = gtk_label_new( _( "minute(s)." ) );
    gtk_widget_show( label );
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

    return ( GTK_CONTAINER( vbox ) );
}

static void
mh_force_update_cb( XfceMailwatchMailbox *mailbox )
{
    DBG( " " );

    g_async_queue_push( XFCE_MAILWATCH_MH_MAILBOX( mailbox )->aqueue, GINT_TO_POINTER( MH_MSG_FORCE_UPDATE ) );
}

static void
mh_set_activated_cb( XfceMailwatchMailbox *mailbox, gboolean activate )
{
    DBG( " " );

    g_async_queue_push( XFCE_MAILWATCH_MH_MAILBOX( mailbox )->aqueue,
            GINT_TO_POINTER( activate ? MH_MSG_START : MH_MSG_PAUSE ) );
}

XfceMailwatchMailboxType builtin_mailbox_type_mh = {
    "mh-maildir",
    N_( "Local MH mail folder" ),
    N_( "MH plugin watches local MH folders for new mail" ),
    mh_new,
    mh_set_activated_cb,
    mh_force_update_cb,
    mh_get_setup_page,
    mh_restore_param_list,
    mh_save_param_list,
    mh_free
};

