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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
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

#include <glib.h>
#include <gtk/gtk.h>

#include "gnome-keyring.h"

/* compatibilite ascendante avec libgnome-keyring <= 0.20 */
#ifndef GNOME_KEYRING_DEFAULT
#define GNOME_KEYRING_DEFAULT NULL
#endif

/* libC crypt usage in case we are not on a gnome desktop */
#include "crypt.h"

#include "mailwatch.h"
#include "mailwatch-utils.h"
#include "cairo-dock.h"

#define BORDER          8

#if !GTK_CHECK_VERSION(2, 6, 0)
#define GTK_STOCK_EDIT GTK_STOCK_PROPERTIES
#endif

static char DES_crypt_key[64] =
{
    1,0,0,1,1,1,0,0, 1,0,1,1,1,0,1,1, 1,1,0,1,0,1,0,1, 1,1,0,0,0,0,0,1,
    0,0,0,1,0,1,1,0, 1,1,1,0,1,1,1,0, 1,1,1,0,0,1,0,0, 1,0,1,0,1,0,1,1
}; 

void encrypt_string_DES( char *input, char **output )
{
  char *last_char_in_input = input + strlen(input);
  char *current_output = NULL;
  if( output )
  {
    *output = g_malloc( strlen(input)*3+1 );
    current_output = *output;
  }

//  g_print( "Password (before encrypt): %s\n", input );

  for( ; input < last_char_in_input; input += 8, current_output += 16+8 )
  {
    char txt[64];
    unsigned int i = 0, j = 0;
    unsigned char current_letter = 0;
    
    memset( txt, 0, 64 );
    
    // process the eight first characters of "input"
    for( i = 0; i < strlen(input) && i < 8 ; i++ )
      for ( j = 0; j < 8; j++ )
        txt[i*8+j] = input[i] >> j & 1;
    
    setkey( DES_crypt_key );
    encrypt( txt, 0 );  // encrypt

    for ( i = 0; i < 8; i++ )
    {
      current_letter = 0;
      for ( j = 0; j < 8; j++ )
      {
        current_letter |= txt[i*8+j] << j;
      }
      snprintf( current_output + i*3, 4, "%02X-", (unsigned char)current_letter );
    }
  }

  *(current_output-1) = 0;

//  g_print( "Password (after encrypt): %s\n", *output );
}

void decrypt_string_DES( char *input, char **output )
{
  char *last_char_in_input = input + strlen(input);
  char *current_output = NULL;
  if( output )
  {
    *output = g_malloc( (strlen(input)+1)/3 );
    current_output = *output;
  }

//  g_print( "Password (before decrypt): %s\n", input );

  for( ; input < last_char_in_input; input += 16+8, current_output += 8 )
  {
    unsigned int block[8];
    char txt[64];
    int i = 0, j = 0;
    unsigned char current_letter = 0;
    
    memset( txt, 0, 64 );

    input[16+8-1] = 0; // cut the string

    sscanf( input, "%X-%X-%X-%X-%X-%X-%X-%X",
    &block[0], &block[1], &block[2], &block[3], &block[4], &block[5], &block[6], &block[7] );

    // process the eight first characters of "input"
    for( i = 0; i < 8 ; i++ )
      for ( j = 0; j < 8; j++ )
        txt[i*8+j] = block[i] >> j & 1;
    
    setkey( DES_crypt_key );
    encrypt( txt, 1 );  // decrypt

    for ( i = 0; i < 8; i++ )
    {
      current_output[i] = 0;
      for ( j = 0; j < 8; j++ )
      {
        current_output[i] |= txt[i*8+j] << j;
      }
    }
  }

  *current_output = 0;

//  g_print( "Password (after decrypt): %s\n", *output );
}

typedef struct
{
    XfceMailwatchMailbox *mailbox;
    gchar *mailbox_name;
    gchar *associated_cmd;
    guint num_new_messages;
} XfceMailwatchMailboxData;

struct _XfceMailwatch
{
    gchar *config_file;

    GList *mailbox_types;  /* XfceMailwatchMailboxType * */
    GList *mailboxes;      /* XfceMailwatchMailboxData * */

    GMutex *mailboxes_mx;

    GList *xm_callbacks[XFCE_MAILWATCH_NUM_SIGNALS];
    GList *xm_data[XFCE_MAILWATCH_NUM_SIGNALS];

    /* config GUI */
    GtkWidget *config_treeview;
    GtkWidget *mbox_types_lbl;
};

/* fwd decl from other modules... */
extern XfceMailwatchMailboxType builtin_mailbox_type_imap;
extern XfceMailwatchMailboxType builtin_mailbox_type_pop3;
extern XfceMailwatchMailboxType builtin_mailbox_type_maildir;
extern XfceMailwatchMailboxType builtin_mailbox_type_mbox;
extern XfceMailwatchMailboxType builtin_mailbox_type_mh;
#ifdef HAVE_SSL_SUPPORT
extern XfceMailwatchMailboxType builtin_mailbox_type_gmail;
#endif

XfceMailwatchMailboxType *builtin_mailbox_types[] = {
    &builtin_mailbox_type_imap,
    &builtin_mailbox_type_pop3,
#ifdef HAVE_SSL_SUPPORT
    &builtin_mailbox_type_gmail,
#endif
    &builtin_mailbox_type_maildir,
    &builtin_mailbox_type_mbox,
    &builtin_mailbox_type_mh,
    NULL
};
#define N_BUILTIN_MAILBOX_TYPES (sizeof(builtin_mailbox_types)/sizeof(builtin_mailbox_types[0]))

static GMutex *big_happy_mailwatch_mx = NULL;
static void xfce_mailwatch_threads_init();

static GList *
mailwatch_load_mailbox_types()
{
    GList *mailbox_types = NULL;
    gint i;

    for(i = 0; builtin_mailbox_types[i]; i++)
        mailbox_types = g_list_prepend(mailbox_types, builtin_mailbox_types[i]);
    mailbox_types = g_list_reverse(mailbox_types);

    return mailbox_types;
}

GQuark
xfce_mailwatch_get_error_quark()
{
    static GQuark q = 0;

    if(!q)
        q = g_quark_from_string("xfce-mailwatch-error");

    return q;
}

XfceMailwatch *
xfce_mailwatch_new()
{
    XfceMailwatch *mailwatch;

    // deja initialise par le dock.
    /**if(!g_thread_supported())
        g_thread_init(NULL);
    if(!g_thread_supported()) {
        // _TOFE_
        //xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
       // g_critical(_("xfce4-mailwatch-plugin: Unable to initialise GThread support.  This is likely a problem with your GLib install."));
        return NULL;
    }*/

    xfce_mailwatch_threads_init();

    mailwatch = g_new0(XfceMailwatch, 1);
    mailwatch->mailbox_types = mailwatch_load_mailbox_types();
    mailwatch->mailboxes_mx = g_mutex_new();

    return mailwatch;
}

void
xfce_mailwatch_destroy(XfceMailwatch *mailwatch)
{
    GList *stuff_to_free, *l;

    g_return_if_fail(mailwatch);

    /* lock it, bitch! */
    g_mutex_lock(mailwatch->mailboxes_mx);

    /* just clear out the mailbox list.  we have to call free_mailbox_func for
     * each mailbox outside the mailboxes_mx lock so we don't cause deadlocks */
    stuff_to_free = mailwatch->mailboxes;
    mailwatch->mailboxes = NULL;

    /* we are SO done. */
    g_mutex_unlock(mailwatch->mailboxes_mx);

    for(l = stuff_to_free; l; l = l->next) {
        XfceMailwatchMailboxData *mdata = l->data;

        mdata->mailbox->type->free_mailbox_func(mdata->mailbox);
        g_free(mdata->mailbox_name);
        g_free(mdata);
    }
    if(stuff_to_free)
        g_list_free(stuff_to_free);

    /* really.  SO SO done. */
    g_mutex_free(mailwatch->mailboxes_mx);

    g_list_free(mailwatch->mailbox_types);
    g_free(mailwatch->config_file);

    g_free(mailwatch);
}

void
xfce_mailwatch_set_config_file(XfceMailwatch *mailwatch, const gchar *filename)
{
    g_return_if_fail(mailwatch && filename);

    g_free(mailwatch->config_file);
    mailwatch->config_file = g_strdup(filename);
}

G_CONST_RETURN gchar *
xfce_mailwatch_get_config_file(XfceMailwatch *mailwatch)
{
    g_return_val_if_fail(mailwatch, NULL);

    return mailwatch->config_file;
}

gboolean
xfce_mailwatch_load_config(XfceMailwatch *mailwatch, GKeyFile *pKeyFile)
{
    gchar buf[32];
    GList *l;
    gint i, j, nmailboxes;
    gboolean bFlushConfFileNeeded;

    g_return_val_if_fail(mailwatch, FALSE);

    DBG("Reading configuration");

    nmailboxes = CD_CONFIG_GET_INTEGER_WITH_DEFAULT("Configuration", "nmailboxes", 0);

    /* lock mutex - doesn't matter yet, but once we start creating mailboxes,
     * it will. */
    g_mutex_lock(mailwatch->mailboxes_mx);

    for(i = 0; i < nmailboxes; i++) {
        const gchar *mailbox_id, *mailbox_name;
        XfceMailwatchMailbox *mailbox = NULL;
        XfceMailwatchMailboxData *mdata;
        gchar **cfg_entries;
        gsize nb_cfg_entries = 0;
        GList *config_params = NULL;
        gchar *mailbox_password = NULL;

        g_snprintf(buf, 32, "mailbox %d name", i);
        mailbox_name = CD_CONFIG_GET_STRING("Configuration", buf);
        if(!mailbox_name)
            continue;

        mailbox_id = CD_CONFIG_GET_STRING(mailbox_name, "type");
        if(!mailbox_id)
            continue;

        for(l = mailwatch->mailbox_types; l; l = l->next) {
            XfceMailwatchMailboxType *mtype = l->data;
            if(!strcmp(mtype->id, mailbox_id)) {
                mailbox = mtype->new_mailbox_func(mailwatch, mtype);
                if(!mailbox->type)
                    mailbox->type = mtype;
                mailbox->type->set_activated_func(mailbox, FALSE);
                break;
            }
        }
        if(!mailbox)
            continue;

        mdata = g_new0(XfceMailwatchMailboxData, 1);
        mdata->mailbox = mailbox;
        mdata->mailbox_name = g_strdup(mailbox_name);
        mdata->associated_cmd = CD_CONFIG_GET_STRING(mailbox_name, "command");
        mailwatch->mailboxes = g_list_append(mailwatch->mailboxes, mdata);

        cfg_entries = g_key_file_get_keys(pKeyFile, mailbox_name, &nb_cfg_entries, NULL);
        if(!cfg_entries)
            continue;

        for(j = 0; cfg_entries[j] && j < nb_cfg_entries; j++) {
            XfceMailwatchParam *param = NULL;
            const gchar *value;

            if(g_iDesktopEnv == CAIRO_DOCK_GNOME && g_strcasecmp( param->key, cfg_entries[j] ) == 0)
            {
              continue;
            }

            value = CD_CONFIG_GET_STRING(mailbox_name, cfg_entries[j]);

            param = g_new(XfceMailwatchParam, 1);
            param->key = cfg_entries[j];

            /* if we are not using gnome keyring, then use the glibc two-way DES encryption ! */
            if( g_strcasecmp( param->key, "password" ) == 0 )
            {
              decrypt_string_DES( value, &(param->value) );
            }
            else
            {
              param->value = g_strdup(value);
            }

            config_params = g_list_append(config_params, param);
        }
        g_free(cfg_entries);  /* yes, not using g_strfreev() is correct */
        
        if(g_iDesktopEnv == CAIRO_DOCK_GNOME)
        {
          // gnome-keyring related stuff
          GnomeKeyringResult res;
          GnomeKeyringAttributeList *attributes = NULL;
          GnomeKeyringAttribute attribute;
          GnomeKeyringFound *f = NULL;
          GList* found = NULL;
          
          attributes = gnome_keyring_attribute_list_new();
          gnome_keyring_attribute_list_append_string( attributes, "mailbox_name", mdata->mailbox_name );

          found = g_list_alloc();

          res = gnome_keyring_find_items_sync(GNOME_KEYRING_ITEM_GENERIC_SECRET, attributes, &found);

          gnome_keyring_attribute_list_free(attributes);

          if (res == GNOME_KEYRING_RESULT_OK) {
            mailbox_password = NULL;
            if (g_list_length (found) > 0) {
              f = (GnomeKeyringFound*)(found->data);
              mailbox_password = f->secret;
              f->secret = NULL;
            }
          }

          gnome_keyring_found_list_free (found);

          if( GNOME_KEYRING_RESULT_OK == res ) 
          {
              XfceMailwatchParam *param = NULL;
              
              param = g_new(XfceMailwatchParam, 1);
              param->key = g_strdup("password");
              param->value = g_strdup(mailbox_password);

              config_params = g_list_append(config_params, param);
              
              gnome_keyring_free_password(mailbox_password);
              mailbox_password = NULL;
          }
        }

        mailbox->type->restore_param_list_func(mailbox, config_params);
        mailbox->type->set_activated_func(mailbox, TRUE);
        for(l = config_params; l; l = l->next) {
            XfceMailwatchParam *param = l->data;
            g_free(param->key);
            g_free(param->value);
            g_free(param);
        }
        if(config_params)
            g_list_free(config_params);
    }

    /* we're done, unlock mutex */
    g_mutex_unlock(mailwatch->mailboxes_mx);

    return TRUE;
}

gboolean
xfce_mailwatch_save_config(XfceMailwatch *mailwatch, GKeyFile *pKeyFile)
{
    gchar buf[32];
    gchar buf_comment[80];
    GList *l;
    gint i;

    g_return_val_if_fail(mailwatch, FALSE);

    /* remove the old configuration in order to handle mailbox renaming */
    guint old_nmailboxes;
    gchar *old_mailbox_name;
    old_nmailboxes = g_key_file_get_integer(pKeyFile, "Configuration", "nmailboxes", NULL);
    for(i = 0; i < old_nmailboxes; i++) {
        g_snprintf(buf, 32, "mailbox %d name", i);

        old_mailbox_name = g_key_file_get_value(pKeyFile, "Configuration", buf, NULL);
        g_key_file_remove_key(pKeyFile,"Configuration",buf, NULL);
        g_key_file_remove_group(pKeyFile, old_mailbox_name, NULL);
        g_free( old_mailbox_name );
    }

    /* write out global config and index */
    g_key_file_set_integer(pKeyFile, "Configuration", "nmailboxes", g_list_length(mailwatch->mailboxes));

    for(l = mailwatch->mailboxes, i = 0; l; l = l->next, i++) {
        XfceMailwatchMailboxData *mdata = l->data;

        g_snprintf(buf, 32, "mailbox %d name", i);

        /* write out global config and index */
        g_key_file_set_value(pKeyFile, "Configuration", buf, mdata->mailbox_name);
        g_snprintf(buf_comment, 80, "S Name of mailbox %d:",i);
        g_key_file_set_comment(pKeyFile,"Configuration",buf,buf_comment, NULL);
    }

    /* write out config data for each mailbox */
    for(l = mailwatch->mailboxes, i = 0; l; l = l->next, i++) {
        XfceMailwatchMailboxData *mdata = l->data;
        GList *config_data, *m;

        /* here the name of the group is mdata->mailbox_name */

        g_snprintf(buf, 32, "type", i);
        g_key_file_set_value(pKeyFile, mdata->mailbox_name, buf, mdata->mailbox->type->id);
        g_key_file_set_comment(pKeyFile,mdata->mailbox_name,buf,"s type of mailbox:", NULL);

        config_data = mdata->mailbox->type->save_param_list_func(mdata->mailbox);
        for(m = config_data; m; m = m->next) {
            XfceMailwatchParam *param = m->data;

            if(param->key && strncmp( param->key, "type", 4 ) != 0 )
            {
                if( strcmp( param->key, "password" ) == 0 )
                {
                  if(g_iDesktopEnv != CAIRO_DOCK_GNOME)
                  {
                    char *txt = NULL;
                    encrypt_string_DES( param->value, &txt );
                    g_key_file_set_value(pKeyFile, mdata->mailbox_name, param->key, txt);
                    g_free( txt );
                  }
                  else
                  {
                    /* store the password in the gnome keyring */
                    GnomeKeyringAttributeList *attributes = NULL;
                    GnomeKeyringAttribute attribute;
                    GnomeKeyringResult res;
                    guint32 item_id;

                    attributes = gnome_keyring_attribute_list_new();
                    gnome_keyring_attribute_list_append_string( attributes, "mailbox_name", mdata->mailbox_name );

                    res = gnome_keyring_item_create_sync (GNOME_KEYRING_DEFAULT, GNOME_KEYRING_ITEM_GENERIC_SECRET, "Cairo-dock Mail password", 
                                                          attributes, param->value, TRUE, &item_id);

                    gnome_keyring_attribute_list_free(attributes);
                  }
                }
                else
                {
                  g_key_file_set_value(pKeyFile, mdata->mailbox_name, param->key, param->value);
                }
                if( strcmp( param->key, "timeout" ) == 0 ||
                    strcmp( param->key, "port" ) == 0 ||
                    strcmp( param->key, "timeout" ) == 0 )
                {
                    g_snprintf(buf_comment, 80, "I %s:",param->key);
                }
                else
                {
                    g_snprintf(buf_comment, 80, "s %s:",param->key);
                }
                g_key_file_set_comment(pKeyFile,mdata->mailbox_name,param->key,buf_comment, NULL);
            }
            g_free(param->key);
            g_free(param->value);
            g_free(param);
        }
        if(config_data)
            g_list_free(config_data);
    }

    return TRUE;
}

void cd_mailwatch_get_mailboxes_infos( XfceMailwatch *mailwatch, GList **list_names, GList **mailboxes_data, GList **mailboxes_cmd  )
{
    GList *l;

    if( !list_names ) return;
    if( !mailboxes_data ) return;

    /* we don't want to be trying to access the mailbox list while they might
     * be in the process of being destroyed. */
    g_mutex_lock(mailwatch->mailboxes_mx);

    for(l = mailwatch->mailboxes; l; l = l->next) {
        XfceMailwatchMailboxData *mdata = l->data;

        if( mdata )
        {
            *list_names = g_list_append( *list_names, mdata->mailbox_name );
            *mailboxes_data = g_list_append( *mailboxes_data, mdata->mailbox );
            if( mailboxes_cmd )
            {
                *mailboxes_cmd = g_list_append( *list_names, mdata->associated_cmd );
            }
        }
    }

    /* and we're done, unlock */
    g_mutex_unlock(mailwatch->mailboxes_mx);
}

guint
xfce_mailwatch_get_new_messages(XfceMailwatch *mailwatch)
{
    GList *l;
    guint num_new_messages = 0;

    g_return_val_if_fail(mailwatch, 0);

    /* we don't want to be trying to access the mailbox list while they might
     * be in the process of being destroyed. */
    g_mutex_lock(mailwatch->mailboxes_mx);

    for(l = mailwatch->mailboxes; l; l = l->next) {
        XfceMailwatchMailboxData *mdata = l->data;
        num_new_messages += mdata->num_new_messages;
    }

    /* and we're done, unlock */
    g_mutex_unlock(mailwatch->mailboxes_mx);

    return num_new_messages;
}

/**
 * The caller should free @mailbox_names with g_strfreev(), and
 * @new_message_counts with g_free().
 **/
void
xfce_mailwatch_get_new_message_breakdown(XfceMailwatch *mailwatch,
        gchar ***mailbox_names, guint **new_message_counts)
{
    GList *l;
    gint i;

    g_return_if_fail(mailbox_names && new_message_counts);

    /* fire! */
    g_mutex_lock(mailwatch->mailboxes_mx);

    *mailbox_names = g_new0(gchar *, g_list_length(mailwatch->mailboxes)+1);
    *new_message_counts = g_new0(guint, g_list_length(mailwatch->mailboxes)+1);

    for(l = mailwatch->mailboxes, i = 0; l; l = l->next, i++) {
        XfceMailwatchMailboxData *mdata = l->data;

        (*mailbox_names)[i] = g_strdup(mdata->mailbox_name);
        (*new_message_counts)[i] = mdata->num_new_messages;
    }

    /* direct hit, captain */
    g_mutex_unlock(mailwatch->mailboxes_mx);
}

static gboolean
mailwatch_signal_new_messages_idled(gpointer data)
{
    XfceMailwatch *mailwatch = data;
    GList *cl, *dl;
    guint new_messages = xfce_mailwatch_get_new_messages(mailwatch);

    for(cl = mailwatch->xm_callbacks[XFCE_MAILWATCH_SIGNAL_NEW_MESSAGE_COUNT_CHANGED],
                dl = mailwatch->xm_data[XFCE_MAILWATCH_SIGNAL_NEW_MESSAGE_COUNT_CHANGED];
        cl && dl;
        cl = cl->next, dl = dl->next)
    {
        XMCallback callback = cl->data;
        gpointer user_data = dl->data;

        if(callback)
            callback(mailwatch, GUINT_TO_POINTER( new_messages ), user_data);
    }

    return FALSE;
}

void
xfce_mailwatch_signal_new_messages(XfceMailwatch *mailwatch,
        XfceMailwatchMailbox *mailbox, guint num_new_messages)
{
    GList *l;
    gboolean do_signal = FALSE;

    g_return_if_fail(mailwatch && mailbox);

    /* we don't want to be trying to access the mailbox list while they might
     * be in the process of being destroyed. */
    g_mutex_lock(mailwatch->mailboxes_mx);

    for(l = mailwatch->mailboxes; l; l = l->next) {
        XfceMailwatchMailboxData *mdata = l->data;

        if(mdata->mailbox == mailbox) {
            if(mdata->num_new_messages != num_new_messages) {
                mdata->num_new_messages = num_new_messages;
                do_signal = TRUE;
            }
            break;
        }
    }

    /* and we're done, unlock */
    g_mutex_unlock(mailwatch->mailboxes_mx);

    if(do_signal)
        g_idle_add(mailwatch_signal_new_messages_idled, mailwatch);  /// bof les g_idle_add ...
}

void
xfce_mailwatch_force_update(XfceMailwatch *mailwatch)
{
    GList *l;

    /* CLEAR! */
    g_mutex_lock(mailwatch->mailboxes_mx);

    for(l = mailwatch->mailboxes; l; l = l->next) {
        XfceMailwatchMailboxData *mdata = l->data;
        mdata->mailbox->type->force_update_callback(mdata->mailbox);
    }

    /* mmm, ten thousand volts */
    g_mutex_unlock(mailwatch->mailboxes_mx);

    g_idle_add(mailwatch_signal_new_messages_idled, mailwatch);
}

void cd_mailwatch_remove_account (XfceMailwatch *mailwatch, XfceMailwatchMailbox *mailbox)
{
    GList *l;
    XfceMailwatchMailboxData *mdata = NULL;

    /* add a "remove account" item for each mailbox */
    for(l = mailwatch->mailboxes; l; l = l->next) {
        mdata = l->data;
        if( mdata->mailbox == mailbox )
            break;
    }

    if( l == NULL || mdata == NULL )
    {
        TRACE( "ATTENTION: cd_mailwatch_remove_account: account not found !" );
    }
    else
    {
        TRACE( "Removing account %s ...", mdata->mailbox_name );
    }

    /* batter up! */
    g_mutex_lock(mailwatch->mailboxes_mx);

    mailwatch->mailboxes = g_list_remove(mailwatch->mailboxes, mdata);
    g_free(mdata->mailbox_name);
    g_free(mdata);

    /* you're out! */
    g_mutex_unlock(mailwatch->mailboxes_mx);

    mailbox->type->free_mailbox_func(mailbox);
}

static gboolean
xfce_mailwatch_signal_log_message( gpointer data )
{
    XfceMailwatchLogEntry       *entry = data;
    XfceMailwatch               *mailwatch = entry->mailwatch;
    GList                       *cbl, *udl;

    for ( cbl = mailwatch->xm_callbacks[XFCE_MAILWATCH_SIGNAL_LOG_MESSAGE],
          udl = mailwatch->xm_data[XFCE_MAILWATCH_SIGNAL_LOG_MESSAGE];
          cbl && udl;
          cbl = cbl->next, udl = udl->next )
    {
        XMCallback      cb = cbl->data;
        gpointer        user_data = udl->data;

        if ( cb ) {
            cb( mailwatch, entry, user_data );
        }
    }
    g_free( entry->message );
    g_free(entry->mailbox_name);
    g_free( entry );

    return ( FALSE );
}

void
xfce_mailwatch_log_message(XfceMailwatch *mailwatch,
                           XfceMailwatchMailbox *mailbox,
                           XfceMailwatchLogLevel level,
                           const gchar *fmt,
                           ...)
{
    XfceMailwatchLogEntry   *entry = NULL;
    va_list                 args;
    GList *l;
    GTimeVal                gt;

    g_return_if_fail( mailwatch &&
            level < XFCE_MAILWATCH_N_LOG_LEVELS && fmt );

    entry = g_new0( XfceMailwatchLogEntry, 1 );
    entry->mailwatch        = mailwatch;
    entry->level            = level;
    g_get_current_time(&gt);
    entry->timestamp        = (time_t)gt.tv_sec;

    va_start( args, fmt );
    entry->message          = g_strdup_vprintf( fmt, args );
    va_end( args );

    if(mailbox) {
        /* locked on, captain */
        g_mutex_lock(mailwatch->mailboxes_mx);

        for(l = mailwatch->mailboxes; l; l = l->next) {
            XfceMailwatchMailboxData *mdata = l->data;

            if(mdata->mailbox == mailbox) {
                entry->mailbox_name = g_strdup(mdata->mailbox_name);
                break;
            }
        }

        /* and we're done, unlock */
        g_mutex_unlock(mailwatch->mailboxes_mx);
    }

    g_idle_add( xfce_mailwatch_signal_log_message, entry );
}

static gboolean
config_run_addedit_window(const gchar *title, GtkWindow *parent,
        const gchar *mailbox_name, XfceMailwatchMailbox *mailbox,
        gchar **new_mailbox_name)
{
    GtkContainer *cfg_box;
    GtkWidget *dlg, *topvbox, *hbox, *lbl, *entry;
    gboolean ret = FALSE;

    g_return_val_if_fail(title && mailbox && new_mailbox_name, FALSE);

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    cfg_box = mailbox->type->get_setup_page_func(mailbox);
    if(!cfg_box) {
        /* Even the mailboxes that don't have configurable settings need a name */
        cfg_box = GTK_CONTAINER(gtk_hbox_new(FALSE, BORDER/2));
        gtk_widget_show(GTK_WIDGET(cfg_box));

        lbl = gtk_label_new(_("This mailbox type does not require any configuration settings."));
        gtk_widget_show(lbl);
        gtk_box_pack_start(GTK_BOX(cfg_box), lbl, TRUE, TRUE, 0);
    }

    if(!mailbox_name) {
        /* add window */
        dlg = gtk_dialog_new_with_buttons(title, parent,
                GTK_DIALOG_NO_SEPARATOR, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
    } else {
        /* edit window */
        dlg = gtk_dialog_new_with_buttons(title, parent,
                GTK_DIALOG_NO_SEPARATOR, GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
                NULL);
    }

    topvbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_container_set_border_width(GTK_CONTAINER(topvbox), BORDER);
    gtk_widget_show(topvbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), topvbox, TRUE, TRUE, 0);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(topvbox), hbox, FALSE, FALSE, 0);

    lbl = gtk_label_new_with_mnemonic(_("Mailbox _Name:"));
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);

    entry = gtk_entry_new();
    if(mailbox_name)
        gtk_entry_set_text(GTK_ENTRY(entry), mailbox_name);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    gtk_label_set_mnemonic_widget( GTK_LABEL( lbl ), entry );

    gtk_box_pack_start(GTK_BOX(topvbox), GTK_WIDGET(cfg_box), TRUE, TRUE, 0);

    for(;;) {
        if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
            *new_mailbox_name = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
            if(!*new_mailbox_name || !**new_mailbox_name) {
                /* _TOFE
                xfce_message_dialog(GTK_WINDOW(dlg), _("Mailwatch"),
                        GTK_STOCK_DIALOG_ERROR, _("Mailbox name required."),
                        _("Please enter a name for the mailbox."),
                        GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
                */
                if(*new_mailbox_name) {
                    g_free(*new_mailbox_name);
                    *new_mailbox_name = NULL;
                }
            } else {
                if(mailbox_name && !g_utf8_collate(mailbox_name, *new_mailbox_name)) {
                    g_free(*new_mailbox_name);
                    *new_mailbox_name = NULL;
                }
                ret = TRUE;
                break;
            }
        } else
            break;
    }
    gtk_widget_destroy(dlg);

    return ret;
}

static gboolean
config_do_edit_window(GtkTreeSelection *sel, GtkWindow *parent)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter itr;
    gboolean ret = FALSE;

    /* _TOFE
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    if(gtk_tree_selection_get_selected(sel, &model, &itr)) {
        gchar *mailbox_name = NULL, *win_title = NULL, *new_mailbox_name = NULL;
        XfceMailwatchMailboxData *mdata = NULL;

        gtk_tree_model_get(model, &itr,
                0, &mailbox_name,
                1, &mdata,
                -1);

        /* pause the mailbox */
        mdata->mailbox->type->set_activated_func(mdata->mailbox, FALSE);

        win_title = g_strdup_printf(_("Edit Mailbox: %s"), mailbox_name);
        if(config_run_addedit_window(win_title, parent,
                mailbox_name, mdata->mailbox, &new_mailbox_name))
        {
            if(new_mailbox_name) {
                gtk_list_store_set(GTK_LIST_STORE(model), &itr,
                        0, new_mailbox_name, -1);
                g_free(mdata->mailbox_name);
                mdata->mailbox_name = new_mailbox_name;
            }

            ret = TRUE;
        }
        g_free(win_title);
        g_free(mailbox_name);

        /* and unpause */
        mdata->mailbox->type->set_activated_func(mdata->mailbox, TRUE);
    }

    return ret;
}

static gint
config_compare_mailbox_data(gconstpointer a, gconstpointer b)
{
    XfceMailwatchMailboxData *xa = (XfceMailwatchMailboxData *)a;
    XfceMailwatchMailboxData *xb = (XfceMailwatchMailboxData *)b;

    return g_utf8_collate(xa->mailbox_name, xb->mailbox_name);
}

static void
config_ask_combo_changed_cb(GtkComboBox *cb, gpointer user_data)
{
    XfceMailwatch *mailwatch = user_data;
    gint active_index = gtk_combo_box_get_active(cb);
    XfceMailwatchMailboxType *mbox_type;
    GtkRequisition req;

    if(active_index >= g_list_length(mailwatch->mailbox_types))
        return;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    mbox_type = g_list_nth_data(mailwatch->mailbox_types, active_index);

    gtk_label_set_text(GTK_LABEL(mailwatch->mbox_types_lbl),
            _(mbox_type->description));
    gtk_widget_set_size_request(mailwatch->mbox_types_lbl, -1, -1);
    gtk_widget_size_request(mailwatch->mbox_types_lbl, &req);
}

static XfceMailwatchMailboxType *
config_ask_new_mailbox_type(XfceMailwatch *mailwatch, GtkWindow *parent)
{
    XfceMailwatchMailboxType *new_mtype = NULL;
    GtkWidget *dlg, *topvbox, *lbl, *combo;
    GList *l;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    dlg = gtk_dialog_new_with_buttons(_("Select Mailbox Type"), parent,
            GTK_DIALOG_NO_SEPARATOR, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);

    topvbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_container_set_border_width(GTK_CONTAINER(topvbox), BORDER);
    gtk_widget_show(topvbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), topvbox, TRUE, TRUE, 0);

    lbl = gtk_label_new(_("Select a mailbox type.  A description of the type will appear below."));
    gtk_label_set_line_wrap(GTK_LABEL(lbl), TRUE);
    gtk_misc_set_alignment(GTK_MISC(lbl), 0.0, 0.5);
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(topvbox), lbl, FALSE, FALSE, 0);

    combo = gtk_combo_box_new_text();
    for(l = mailwatch->mailbox_types; l; l = l->next) {
        XfceMailwatchMailboxType *mtype = l->data;
        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _(mtype->name));
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_widget_show(combo);
    gtk_box_pack_start(GTK_BOX(topvbox), combo, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(combo), "changed",
            G_CALLBACK(config_ask_combo_changed_cb), mailwatch);

    if(mailwatch->mailbox_types) {
        XfceMailwatchMailboxType *mtype = mailwatch->mailbox_types->data;
        mailwatch->mbox_types_lbl = lbl = gtk_label_new(_(mtype->description));
    } else
        mailwatch->mbox_types_lbl = lbl = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(lbl), TRUE);
    gtk_misc_set_alignment(GTK_MISC(lbl), 0.5, 0.0);
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(topvbox), lbl, TRUE, TRUE, 0);

    if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));

        if(active >= 0 && active < g_list_length(mailwatch->mailbox_types))
            new_mtype = g_list_nth_data(mailwatch->mailbox_types, active);
    }
    gtk_widget_destroy(dlg);

    return new_mtype;
}

void
config_add_btn_clicked_cb(GtkWidget *w, XfceMailwatch *mailwatch)
{
    XfceMailwatchMailboxType *mailbox_type = NULL;
    XfceMailwatchMailbox *new_mailbox;
    gchar *new_mailbox_name = NULL;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(w));

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    mailbox_type = config_ask_new_mailbox_type(mailwatch, parent);
    if(!mailbox_type)
        return;

    new_mailbox = mailbox_type->new_mailbox_func(mailwatch, mailbox_type);
    if(!new_mailbox->type)
        new_mailbox->type = mailbox_type;
    mailbox_type->set_activated_func(new_mailbox, FALSE);
    if(config_run_addedit_window(_("Add New Mailbox"), parent, NULL,
                new_mailbox, &new_mailbox_name))
    {
        XfceMailwatchMailboxData *mdata = g_new(XfceMailwatchMailboxData, 1);
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(mailwatch->config_treeview));
        GtkTreeIter itr;

        /* to serve and protect */
        g_mutex_lock(mailwatch->mailboxes_mx);

        mdata->mailbox = new_mailbox;
        mdata->mailbox_name = new_mailbox_name;
        mdata->num_new_messages = 0;

        mailwatch->mailboxes = g_list_insert_sorted(mailwatch->mailboxes,
                mdata, (GCompareFunc)config_compare_mailbox_data);

        /* tcetorp dna evres ot */
        g_mutex_unlock(mailwatch->mailboxes_mx);

        mailbox_type->set_activated_func(new_mailbox, TRUE);

        gtk_list_store_append(GTK_LIST_STORE(model), &itr);
        gtk_list_store_set(GTK_LIST_STORE(model), &itr,
                0, new_mailbox_name,
                1, mdata,
                -1);
    } else
        mailbox_type->free_mailbox_func(new_mailbox);
}

static void
config_edit_btn_clicked_cb(GtkWidget *w, XfceMailwatch *mailwatch)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(mailwatch->config_treeview));

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */
    config_do_edit_window(sel, GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(w))));
}

gboolean config_do_edit_window_2(GtkWidget *w, XfceMailwatch *mailwatch, XfceMailwatchMailbox *mailbox)
{
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(w)));
    gboolean ret = FALSE;

    /* _TOFE
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */
    GList *l;
    XfceMailwatchMailboxData *mdata = NULL;

    /* add a "remove account" item for each mailbox */
    for(l = mailwatch->mailboxes; l; l = l->next) {
        mdata = l->data;
        if( mdata->mailbox == mailbox )
            break;
    }

    gchar *mailbox_name = g_strdup(mdata->mailbox_name), *win_title = NULL, *new_mailbox_name = NULL;

    /* pause the mailbox */
    mailbox->type->set_activated_func(mailbox, FALSE);

    win_title = g_strdup_printf(_("Edit Mailbox: %s"), mailbox_name);
    if(config_run_addedit_window(win_title, parent,
            mailbox_name, mailbox, &new_mailbox_name))
    {
        if(new_mailbox_name) {
            g_free(mdata->mailbox_name);
            mdata->mailbox_name = new_mailbox_name;
        }

        ret = TRUE;
    }
    g_free(win_title);
    g_free(mailbox_name);

    /* and unpause */
    mailbox->type->set_activated_func(mailbox, TRUE);

    return ret;
}

static void
config_remove_btn_clicked_cb(GtkWidget *w, XfceMailwatch *mailwatch)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(mailwatch->config_treeview));
    GtkTreeModel *model = NULL;
    GtkTreeIter itr;
    XfceMailwatchMailboxData *mailbox_data = NULL;
    XfceMailwatchMailbox *mailbox = NULL;
    GtkWindow *parent;
    gint resp;
    GList *l;

    if(!gtk_tree_selection_get_selected(sel, &model, &itr))
        return;

    gtk_tree_model_get(model, &itr, 1, &mailbox_data, -1);
    if(!mailbox_data)
        return;
    mailbox = mailbox_data->mailbox;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

    parent = GTK_WINDOW(gtk_widget_get_toplevel(mailwatch->config_treeview));
    resp = xfce_message_dialog(parent, _("Remove Mailbox"),
            GTK_STOCK_DIALOG_QUESTION, _("Are you sure?"),
            _("Removing a mailbox will discard all settings, and cannot be undone."),
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE,
            GTK_RESPONSE_ACCEPT, NULL);
    if(resp != GTK_RESPONSE_ACCEPT)
        return;
    */

    gtk_list_store_remove(GTK_LIST_STORE(model), &itr);

    /* batter up! */
    g_mutex_lock(mailwatch->mailboxes_mx);

    for(l = mailwatch->mailboxes; l; l = l->next) {
        XfceMailwatchMailboxData *mdata = l->data;

        if(mdata->mailbox == mailbox) {
            mailwatch->mailboxes = g_list_remove(mailwatch->mailboxes, mdata);
            g_free(mdata->mailbox_name);
            g_free(mdata);
            break;
        }
    }

    /* you're out! */
    g_mutex_unlock(mailwatch->mailboxes_mx);

    mailbox->type->free_mailbox_func(mailbox);

    mailwatch_signal_new_messages_idled(mailwatch);
}

static gboolean
config_treeview_button_press_cb(GtkTreeView *treeview, GdkEventButton *evt,
        XfceMailwatch *mailwatch)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection(treeview);

    if(evt->type == GDK_2BUTTON_PRESS && evt->button == 1) {
        config_do_edit_window(sel,
                GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeview))));
    }

    return FALSE;
}

static gboolean
config_set_button_sensitive(GtkTreeView *treeview, GdkEventButton *evt,
        GtkWidget *w)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection(treeview);

    if(gtk_tree_selection_get_selected(sel, NULL, NULL))
        gtk_widget_set_sensitive(w, TRUE);
    else
        gtk_widget_set_sensitive(w, FALSE);

    return FALSE;
}

#if 0
/* _TOFE_ */
GtkContainer *
xfce_mailwatch_get_configuration_page(XfceMailwatch *mailwatch)
{
    GtkWidget *frame, *frame_bin, *vbox, *hbox, *sw, *treeview, *btn;
    GtkListStore *ls;
    GList *l;
    GtkTreeIter itr;
    GtkTreeViewColumn *col;
    GtkCellRenderer *render;
    GtkTreeSelection *sel;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    frame = xfce_mailwatch_create_framebox(_("Mailboxes"), &frame_bin);
    gtk_widget_show(frame);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame_bin), hbox);

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
            GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER,
            GTK_POLICY_AUTOMATIC);
    gtk_widget_show(sw);
    gtk_box_pack_start(GTK_BOX(hbox), sw, TRUE, TRUE, 0);

    /* time to make the doughnuts */
    g_mutex_lock(mailwatch->mailboxes_mx);

    ls = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
    for(l = mailwatch->mailboxes; l; l = l->next) {
        XfceMailwatchMailboxData *mdata = l->data;

        gtk_list_store_append(ls, &itr);
        gtk_list_store_set(ls, &itr,
                0, mdata->mailbox_name,
                1, mdata,
                -1);
    }

    /* yum.  they're done. */
    g_mutex_unlock(mailwatch->mailboxes_mx);

    mailwatch->config_treeview = treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ls));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
    gtk_widget_add_events(treeview, GDK_BUTTON_PRESS|GDK_BUTTON_RELEASE);

    render = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes("mailbox-name", render,
            "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

    gtk_widget_show(treeview);
    gtk_container_add(GTK_CONTAINER(sw), treeview);
    g_signal_connect(G_OBJECT(treeview), "button-press-event",
            G_CALLBACK(config_treeview_button_press_cb), mailwatch);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    gtk_tree_selection_unselect_all(sel);

    vbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

    btn = gtk_button_new_from_stock(GTK_STOCK_ADD);
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(btn), "clicked",
            G_CALLBACK(config_add_btn_clicked_cb), mailwatch);

    btn = gtk_button_new_from_stock(GTK_STOCK_EDIT);
    gtk_widget_set_sensitive(btn, FALSE);
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);
    g_signal_connect_after(G_OBJECT(treeview), "button-release-event",
            G_CALLBACK(config_set_button_sensitive), btn);
    g_signal_connect(G_OBJECT(btn), "clicked",
            G_CALLBACK(config_edit_btn_clicked_cb), mailwatch);

    btn = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
    gtk_widget_set_sensitive(btn, FALSE);
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);
    g_signal_connect_after(G_OBJECT(treeview), "button-release-event",
            G_CALLBACK(config_set_button_sensitive), btn);
    g_signal_connect(G_OBJECT(btn), "clicked",
            G_CALLBACK(config_remove_btn_clicked_cb), mailwatch);

    return GTK_CONTAINER(frame);
}
#endif

void
xfce_mailwatch_signal_connect(XfceMailwatch *mailwatch,
        XfceMailwatchSignal signal, XMCallback callback, gpointer user_data)
{
    g_return_if_fail(mailwatch && callback
            && signal < XFCE_MAILWATCH_NUM_SIGNALS);

    mailwatch->xm_callbacks[signal] =
            g_list_append(mailwatch->xm_callbacks[signal], callback);
    mailwatch->xm_data[signal] = g_list_append(mailwatch->xm_data[signal],
            user_data);
}

void
xfce_mailwatch_signal_disconnect(XfceMailwatch *mailwatch,
        XfceMailwatchSignal signal, XMCallback callback, gpointer user_data)
{
    GList *cl, *dl;
    g_return_if_fail(mailwatch && callback
            && signal < XFCE_MAILWATCH_NUM_SIGNALS);

    for(cl = mailwatch->xm_callbacks[signal], dl = mailwatch->xm_data[signal];
        cl && dl;
        cl = cl->next, dl = dl->next)
    {
        XMCallback a_callback = cl->data;

        if(callback == a_callback) {
            mailwatch->xm_callbacks[signal] =
                    g_list_delete_link(mailwatch->xm_callbacks[signal], cl);
            mailwatch->xm_data[signal] =
                    g_list_delete_link(mailwatch->xm_data[signal], dl);
            break;
        }
    }
}

static void
xfce_mailwatch_threads_init()
{
    if(!big_happy_mailwatch_mx)
        big_happy_mailwatch_mx = g_mutex_new();
}

void
xfce_mailwatch_threads_enter()
{
    g_return_if_fail(big_happy_mailwatch_mx);

    g_mutex_lock(big_happy_mailwatch_mx);
}

void xfce_mailwatch_threads_leave()
{
    g_return_if_fail(big_happy_mailwatch_mx);

    g_mutex_unlock(big_happy_mailwatch_mx);
}

/* this might not be a good idea, but memleaks are bad */
G_MODULE_EXPORT void
g_module_unload(GModule *module)
{
    if(big_happy_mailwatch_mx) {
        g_mutex_free(big_happy_mailwatch_mx);
        big_happy_mailwatch_mx = NULL;
    }
}
