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

#ifndef __MAILWATCH_MAILBOX_H__
#define __MAILWATCH_MAILBOX_H__

#include <glib.h>
#include <gtk/gtkcontainer.h>

G_BEGIN_DECLS

#define XFCE_MAILWATCH_MAILBOX(ptr) ((XfceMailwatchMailbox *)ptr)

/* this is used for shared objects only; unsupported as of yet; may not be
 * useful/necessary */
#define MAILWATCH_MAILBOX_TYPE(mmt_struct) \
G_MODULE_EXPORT XfceMailwatchkMailboxType *\
xfce_mailwatch_mailbox_get_type()\
{\
	return mmt_struct;\
}

struct _XfceMailwatch;
typedef struct _XfceMailwatchMailboxType XfceMailwatchMailboxType;

/**
 * XfceMailwatchParam
 * @key: A key with which to identify the configuration data.  All keys
 *       beginning with the string "mailwatch-" are reserved for internal use.
 * @value: A string representation of the configuration data.
 **/
typedef struct
{
    gchar *key;
    gchar *value;
} XfceMailwatchParam;

/**
 * XfceMailwatchMailbox:
 * @type: A pointer to a #XfceMailwatchMailboxType struct providing functions
 *        to manipulate this particular type of mailbox.
 *
 * Implementations of various mailboxes/mail protocols should "subclass" this
 * struct by creating a private struct and including this struct as the first
 * member.  Instances of this private struct should be cast to/from
 * #XfceMailwatchMailbox.
 **/
typedef struct
{
    XfceMailwatchMailboxType *type;
} XfceMailwatchMailbox;

#define XFCE_MAILWATCH_MAILBOX(ptr) ((XfceMailwatchMailbox *)ptr)

/**
 * NewMailboxFunc:
 * @mailwatch: The #XfceMailwatch instance.
 * @type: The #XfceMailwatchMailboxType being created.
 *
 * Should return a new unconfigured #XfceMailwatchMailbox instance, which should
 * internally keep track of the #mailwatch passed to it.
 *
 * Returns: A #XfceMailwatchMailbox instance.
 **/
typedef XfceMailwatchMailbox *(*NewMailboxFunc)(struct _XfceMailwatch *mailwatch, XfceMailwatchMailboxType *type);

/**
 * SetActivatedFunc:
 * @mailbox: The #XfceMailwatchMailbox instance.
 * @activated: Whether or not @mailbox should be actively checking for new mail.
 *
 * Temporarily disables or reenables a particular mailbox, without freeing any
 * memory or clearing any settings.
 **/
typedef void (*SetActivatedFunc)(XfceMailwatchMailbox *mailbox, gboolean activated);

/**
 * ForceUpdateCallback:
 * @mailbox: The #XfceMailwatchMailbox instance.
 *
 * A callback that the #XfceMailwatch instance can call to request that @mailbox
 * update its count of new messages immediately.  Since this call occurs in the
 * main (UI) thread, @mailbox must NOT immediately check for new mail, but must
 * signal its worker thread to do so.
 **/
typedef void (*ForceUpdateCallback)(XfceMailwatchMailbox *mailbox);

/**
 * GetSetupPageFunc:
 * @mailbox: The #XfceMailwatchMailbox instance.
 *
 * Should return a #GtkContainer, preferably a #GtkVBox or #GtkHBox, and
 * definitely not a #GtkWindow.  This container should have the UI for
 * configuring this instance of #XfceMailwatchMailbox.
 *
 * Returns: A #GtkContainer.
 **/
typedef GtkContainer *(*GetSetupPageFunc)(XfceMailwatchMailbox *mailbox);

/**
 * RestoreParamListFunc:
 * @mailbox: The #XfceMailwatchMailbox instance to which the configuration data
 *           belongs.
 * @params: A #GList of #XfceMailwatchParam<!-- -->s.
 *
 * The #XfceMailwatchMailbox instance should take @params and use them to
 * configure @mailbox to be able to check mail.  The caller will take care of
 * freeing the list and parameters.
 **/
typedef void (*RestoreParamListFunc)(XfceMailwatchMailbox *mailbox, GList *params);

/**
 * SaveParamListFunc:
 * @mailbox: The #XfceMailwatchMailbox instance.
 *
 * Should return a #GList of #XfceMailwatchParam<!-- -->s describing the
 * configuration of @mailbox.  The list itself, and each parameter and their
 * @key<!-- -->s and @value<!-- -->s will be freed by the caller.
 *
 * Returns: A #Glist of #XfceMailwatchParam<!-- -->s.
 **/
typedef GList *(*SaveParamListFunc)(XfceMailwatchMailbox *mailbox);

/**
 * FreeMailboxFunc:
 * @mailbox: The #XfceMailwatchMailbox instance being freed.
 *
 * Should release all memory associated with the specified @mailbox.
 **/
typedef void (*FreeMailboxFunc)(XfceMailwatchMailbox *mailbox);

/**
 * XfceMailwatchMailboxType:
 * @id: A short string ID to identify the mailbox type in config files.
 * @name: A short name for the mailbox type, e.g., "IMAP".
 * @description: A longer description of the mailbox type.
 * @new_mailbox_func: A pointer to a function of type #NewMailboxFunc.
 * @get_setup_page_func: A pointer to a function of type #GetSetupPageFunc.
 * @restore_param_list_func: A pointer to a function of type
 *                           #RestoreParamListFunc.
 * @save_param_list_func: A pointer to a function of type #SaveParamListFunc.
 * @free_mailbox_func: A pointer to a function of type #FreeMailboxFunc.
 **/
struct _XfceMailwatchMailboxType
{
    gchar *id;
    gchar *name;
    gchar *description;

    NewMailboxFunc new_mailbox_func;
    SetActivatedFunc set_activated_func;
    ForceUpdateCallback force_update_callback;
    GetSetupPageFunc get_setup_page_func;
    RestoreParamListFunc restore_param_list_func;
    SaveParamListFunc save_param_list_func;
    FreeMailboxFunc free_mailbox_func;
};

G_END_DECLS

#endif
