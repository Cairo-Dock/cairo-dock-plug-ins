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

#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include "mailwatch-utils.h"
#include "mailwatch.h"
#include "cairo-dock.h"

#ifndef GTK_STOCK_DIRECTORY
#define GTK_STOCK_DIRECTORY      GTK_STOCK_OPEN
#endif

#define BORDER                   8

#define XFCE_MAILWATCH_IMAP_MAILBOX(ptr) ((XfceMailwatchIMAPMailbox *)ptr)

#define IMAP_PORT_S              "143"
#define IMAPS_PORT_S             "993"

#define IMAP_CMD_START           GINT_TO_POINTER(1)
#define IMAP_CMD_PAUSE           GINT_TO_POINTER(2)
#define IMAP_CMD_TIMEOUT         GINT_TO_POINTER(3)
#define IMAP_CMD_QUIT            GINT_TO_POINTER(4)
#define IMAP_CMD_UPDATE          GINT_TO_POINTER(5)

typedef struct
{
    XfceMailwatchMailbox mailbox;

    GMutex *config_mx;

    guint timeout;
    gchar *host;
    gchar *username;
    gchar *password;
    GList *mailboxes_to_check;
    gchar *server_directory;

    gboolean use_standard_port;
    gint nonstandard_port;
    XfceMailwatchAuthType auth_type;

    GThread *th;
    GAsyncQueue *aqueue;

    XfceMailwatch *mailwatch;

    /* state related to the current connection (if any) */
    gint sockfd;
    guint imap_tag;
    /* secure this, dude */
    XfceMailwatchSecurityInfo security_info;

    /* config dlg */
    GtkWidget *folder_tree_dialog;
    GtkTreeStore *ts;
    GtkCellRenderer *render;
    GThread *folder_tree_th;
    GAsyncQueue *folder_tree_aqueue;
    GtkWidget *refresh_btn;
    GNode *folder_tree;
} XfceMailwatchIMAPMailbox;

enum
{
    IMAP_FOLDERS_NAME = 0,
    IMAP_FOLDERS_WATCHING,
    IMAP_FOLDERS_HOLDS_MESSAGES,
    IMAP_FOLDERS_FULLPATH,
    IMAP_FOLDERS_N_COLUMNS
};

typedef struct
{
    gchar *folder_name;
    gchar *full_path;
    gboolean holds_messages;
} IMAPFolderData;

static gssize
imap_send(XfceMailwatchIMAPMailbox *imailbox, const gchar *buf)
{
    GError *error = NULL;
    gssize sent;

    sent = xfce_mailwatch_net_send(imailbox->sockfd,
                                   &imailbox->security_info,
                                   buf,
                                   &error);
    if(sent < 0) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_ERROR,
                                   error->message);
        g_error_free(error);
    }

    return sent;
}

static gssize
imap_recv(XfceMailwatchIMAPMailbox *imailbox, gchar *buf, gsize len)
{
    GError *error = NULL;
    gssize recvd;

    recvd = xfce_mailwatch_net_recv(imailbox->sockfd,
                                    &imailbox->security_info,
                                    buf,
                                    len,
                                    &error);

    if(recvd < 0) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_ERROR,
                                   error->message);
        g_error_free(error);
    }

    return recvd;
}

static void
imap_do_logout(XfceMailwatchIMAPMailbox *imailbox)
{
    imap_send(imailbox, "ABCD LOGOUT\r\n");

    shutdown(imailbox->sockfd, SHUT_RDWR);
    close(imailbox->sockfd);
    imailbox->sockfd = -1;
}

static gboolean
imap_get_sockaddr(XfceMailwatchIMAPMailbox *imailbox, const gchar *host,
                  const gchar *service, struct sockaddr_in *addr)
{
    struct addrinfo hints = { 0, PF_INET, SOCK_STREAM, IPPROTO_TCP,
            sizeof(struct sockaddr_in), NULL, NULL, NULL };
    GError *error = NULL;

    TRACE("entering (%s, %s, %p)", host, service, addr);

    g_return_val_if_fail(host && service && addr, FALSE);

    if(!xfce_mailwatch_net_get_sockaddr(host, service, &hints, addr, &error)) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_ERROR,
                                   error->message);
        g_error_free(error);
        return FALSE;
    }

    return TRUE;
}

static gboolean
imap_send_login_info(XfceMailwatchIMAPMailbox *imailbox, const gchar *username,
        const gchar *password)
{
#define BUFSIZE 8191
    gint bin, bout;
    gchar buf[BUFSIZE+1];

    TRACE("entering");

    /* check capabilities */
    g_snprintf(buf, BUFSIZE, "%05d CAPABILITY\r\n", ++imailbox->imap_tag);
    bout = imap_send(imailbox, buf);
    DBG("sent CAPABILITY (%d)", bout);
    if(bout != strlen(buf))
        goto cleanuperr;
    bin = imap_recv(imailbox, buf, BUFSIZE);
    DBG("response from CAPABILITY (%d): %s", bin, bin>0?buf:"(nada)");
    if(bin <= 0)
        goto cleanuperr;

    if(strstr(buf, " LOGINDISABLED")) {
        /* _TOFE_
        xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
        */
        g_warning(_("Secure IMAP is not available, and the IMAP server does not support plaintext logins."));
        goto cleanuperr;
    }

    /* send the creds */
    g_snprintf(buf, BUFSIZE, "%05d LOGIN \"%s\" \"%s\"\r\n",
            ++imailbox->imap_tag, username, password);
    bout = imap_send(imailbox, buf);
    DBG("sent login (%d)", bout);
    if(bout != strlen(buf))
        goto cleanuperr;

    /* and see if we actually got auth-ed */
    bin = imap_recv(imailbox, buf, BUFSIZE);
    DBG("response from login (%d): %s", bin, bin>0?buf:"(nada)");
    if(bin <= 0)
        goto cleanuperr;
    DBG("strstr() returns %p", strstr(buf, "OK"));
    if(!strstr(buf, "OK"))
        goto cleanuperr;

    TRACE("leaving (success)");

    return TRUE;

    cleanuperr:

    shutdown(imailbox->sockfd, SHUT_RDWR);
    close(imailbox->sockfd);
    imailbox->sockfd = -1;

    return FALSE;
#undef BUFSIZE
}

static gboolean
imap_negotiate_ssl(XfceMailwatchIMAPMailbox *imailbox, const gchar *host)
{
    gboolean ret;
    GError *error = NULL;

    ret = xfce_mailwatch_net_negotiate_tls(imailbox->sockfd,
            &imailbox->security_info, host, &error);

    if(!ret) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_ERROR,
                                   _("TLS handshake failed: %s"),
                                   error->message);
        g_error_free(error);
        shutdown(imailbox->sockfd, SHUT_RDWR);
        close(imailbox->sockfd);
        imailbox->sockfd = -1;
    }

    return ret;
}

static gboolean
imap_do_starttls(XfceMailwatchIMAPMailbox *imailbox, const gchar *host,
        const gchar *username, const gchar *password)
{
#define BUFSIZE 8191
    gint bin;
    gchar buf[BUFSIZE+1];

    TRACE("entering");

    g_snprintf(buf, BUFSIZE, "%05d CAPABILITY\r\n", ++imailbox->imap_tag);
    if(imap_send(imailbox, buf) != strlen(buf))
        goto cleanuperr;

    bin = imap_recv(imailbox, buf, BUFSIZE);
    DBG("checking for STARTTLS caps (%d): %s", bin, bin>0?buf:"(nada)");
    if(bin <= 0)
        goto cleanuperr;
    if(!strstr(buf, " STARTTLS")) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_WARNING,
                                   _("STARTTLS security was requested, but this server does not support it."));
        goto cleanuperr;
    }

    g_snprintf(buf, BUFSIZE, "%05d STARTTLS\r\n", ++imailbox->imap_tag);
    if(imap_send(imailbox, buf) != strlen(buf))
        goto cleanuperr;

    if(imap_recv(imailbox, buf, BUFSIZE) < 0)
        goto cleanuperr;
    if(!strstr(buf, " OK"))
        goto cleanuperr;

    /* now that we've negotiated SSL, reenable using_tls */
    imailbox->security_info.using_tls = TRUE;

    return TRUE;

    cleanuperr:

    shutdown(imailbox->sockfd, SHUT_RDWR);
    close(imailbox->sockfd);
    imailbox->sockfd = -1;

    return FALSE;
#undef BUFSIZE
}

static gboolean
imap_connect(XfceMailwatchIMAPMailbox *imailbox, const gchar *host,
        const gchar *service, gint nonstandard_port)
{
    struct sockaddr_in addr;

    TRACE("entering (%s)", service);

    if(!imap_get_sockaddr(imailbox, host, service, &addr)) {
        DBG("failed to get sockaddr");
        return FALSE;
    }

    if(nonstandard_port > 0)
        addr.sin_port = htons(nonstandard_port);

    imailbox->sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(imailbox->sockfd < 0) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_WARNING,
                                   "socket(): %s",
                                   strerror(errno));
        DBG("failed to open socket");
        return FALSE;
    }

    /* this next batch of crap is necessary because it seems like a failed
     * connection (that is, one that isn't ECONNREFUSED) takes over 3 minutes
     * to fail!  if the panel is trying to quit, that's just unacceptable.
     */

    if(fcntl(imailbox->sockfd, F_SETFL, fcntl(imailbox->sockfd, F_GETFL) | O_NONBLOCK)) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_WARNING,
                                   _("Unable to set socket to non-blocking mode.  If the connect attempt hangs, the panel may hang on close."));
    }

    if(connect(imailbox->sockfd, (struct sockaddr *)&addr,
            sizeof(struct sockaddr_in)))
    {
        gboolean failed = TRUE;

        if(errno == EINPROGRESS) {
            gint iters_left;
            for(iters_left = 25; iters_left >= 0; iters_left--) {
                fd_set wfd;
                struct timeval tv = { 2, 0 };
                int sock_err = 0;
                socklen_t sock_err_len = sizeof(int);
                gpointer msg;

                FD_ZERO(&wfd);
                FD_SET(imailbox->sockfd, &wfd);

                DBG("checking for a connection...");

                /* wait until the connect attempt finishes */
                if(select(FD_SETSIZE, NULL, &wfd, NULL, &tv) < 0)
                    break;

                /* check to see if it finished, and, if so, if there was an
                 * error, or if it completed successfully */
                if(FD_ISSET(imailbox->sockfd, &wfd)) {
                    if(!getsockopt(imailbox->sockfd, SOL_SOCKET, SO_ERROR,
                                &sock_err, &sock_err_len)
                            && !sock_err)
                    {
                        DBG("    connection succeeded");
                        failed = FALSE;
                    } else {
                        xfce_mailwatch_log_message(imailbox->mailwatch,
                                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                                   XFCE_MAILWATCH_LOG_ERROR,
                                                   _("Failed to connect to server: %s"),
                                                   strerror(sock_err));
                        DBG("    connection failed: sock_err is %d", sock_err);
                    }
                    break;
                }

                /* check the main thread to see if we're supposed to quit */
                msg = g_async_queue_try_pop(imailbox->aqueue);
                if(msg) {
                    /* put it back so imap_check_mail_th() can read it */
                    g_async_queue_push(imailbox->aqueue, msg);
                    if(msg == IMAP_CMD_QUIT) {
                        failed = TRUE;
                        break;
                    }
                }
            }
        }

        if(failed) {
            DBG("failed to connect");
            close(imailbox->sockfd);
            imailbox->sockfd = -1;
            return FALSE;
        }
    }

    if(fcntl(imailbox->sockfd, F_SETFL, fcntl(imailbox->sockfd, F_GETFL) & ~(O_NONBLOCK))) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_WARNING,
                                   _("Unable to return socket to blocking mode.  Data may not be retreived correctly."));
    }

    return TRUE;
}

static gboolean
imap_authenticate(XfceMailwatchIMAPMailbox *imailbox, const gchar *host,
        const gchar *username, const gchar *password,
        XfceMailwatchAuthType auth_type, gint nonstandard_port)
{
#define BUFSIZE 2047
    gboolean ret = FALSE;
    gchar buf[BUFSIZE+1];

    TRACE("entering, auth_type is %d", auth_type);

    switch(auth_type) {
        case AUTH_NONE:
            imailbox->security_info.using_tls = FALSE;
            ret = imap_connect(imailbox, host, "imap", nonstandard_port);

            /* discard opening banner */
            if(ret && imap_recv(imailbox, buf, BUFSIZE) < 0) {
                DBG("failed to get banner");
                shutdown(imailbox->sockfd, SHUT_RDWR);
                close(imailbox->sockfd);
                imailbox->sockfd = -1;
            }
            break;

        case AUTH_STARTTLS:
            imailbox->security_info.using_tls = FALSE;
            ret = imap_connect(imailbox, host, "imap", nonstandard_port);

            if(ret) {
                /* discard opening banner */
                if(imap_recv(imailbox, buf, BUFSIZE) < 0) {
                    DBG("failed to get banner");
                    shutdown(imailbox->sockfd, SHUT_RDWR);
                    close(imailbox->sockfd);
                    imailbox->sockfd = -1;
                }

                ret = imap_do_starttls(imailbox, host, username, password);
                if(ret)
                    ret = imap_negotiate_ssl(imailbox, host);
                imailbox->security_info.using_tls = TRUE;
            }
            break;

        case AUTH_SSL_PORT:
            imailbox->security_info.using_tls = TRUE;
            ret = imap_connect(imailbox, host, "imaps", nonstandard_port);
            if(ret)
                ret = imap_negotiate_ssl(imailbox, host);

            /* discard opening banner */
            if(ret && imap_recv(imailbox, buf, BUFSIZE) < 0) {
                DBG("failed to get banner");
                shutdown(imailbox->sockfd, SHUT_RDWR);
                close(imailbox->sockfd);
                imailbox->sockfd = -1;
            }
            break;

        default:
            g_critical("XfceMailwatchIMAPMailbox: Unknown auth type (%d)", auth_type);
            return FALSE;
    }

    DBG("using_tls is %s", imailbox->security_info.using_tls?"TRUE":"FALSE");

    if(ret && !imap_send_login_info(imailbox, username, password))
        return FALSE;

    return ret;
#undef BUFSIZE
}

static guint
imap_check_mailbox(XfceMailwatchIMAPMailbox *imailbox,
        const gchar *mailbox_name)
{
#define BUFSIZE 16383   /* yeah, this is probably overkill */
    gint new_messages = 0;
    gchar buf[BUFSIZE+1], *p, *q, tmp[64];

    TRACE("entering, folder %s", mailbox_name);

    /* ask the server to look at the mailbox */
    g_snprintf(buf, BUFSIZE, "%05d EXAMINE %s\r\n", ++imailbox->imap_tag,
            mailbox_name);
    if(imap_send(imailbox, buf) != strlen(buf))
        return 0;
    DBG("  successfully sent cmd '%s'", buf);

    /* grab the response; it should end with "##### OK " */
    if(imap_recv(imailbox, buf, BUFSIZE) < 0)
        return 0;
    g_snprintf(tmp, 64, "%05d OK ", imailbox->imap_tag);
    if(!strstr(buf, tmp))
        return 0;
    DBG("  successfully got reply '%s'", buf);

    /* send SEARCH command */
    g_snprintf(buf, BUFSIZE, "%05d SEARCH UNSEEN\r\n", ++imailbox->imap_tag);
    if(imap_send(imailbox, buf) != strlen(buf))
        return 0;
    DBG("  successfully sent cmd '%s'", buf);

    /* get response; it should have "* SEARCH" followed by a space-delimited
     * list of unseen messages.  unfortunately, this means there's no upper
     * bound on string length =p */
    if(imap_recv(imailbox, buf, BUFSIZE) < 0)
        return 0;
    g_snprintf(tmp, 64, "%05d OK", imailbox->imap_tag);
    if(!strstr(buf, tmp)) {
        xfce_mailwatch_log_message(imailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(imailbox),
                                   XFCE_MAILWATCH_LOG_WARNING,
                                   _("A buffer was too small to receive all of an IMAP response.  This is a bug!"));
        g_warning("Mailwatch: Uh-oh.  We didn't get the full response back!");
    }
    p = strstr(buf, "* SEARCH");
    if(!p)
        return 0;
    DBG("  successfully got reply '%s'", buf);

    p += 8;

    /* find the end of the line */
    q = strstr(p, "\r");
    if(!q)
        q = strstr(p, "\n");
    if(!q)
        return 0;
    *q = 0;
    DBG("  ok, we have a list of messages: '%s'", p);

    /* find each space in the string; that's a message */
    while((p = strstr(p, " "))) {
        new_messages++;
        p++;
    }

    DBG("new message count in mailbox '%s' is %d", mailbox_name, new_messages);

    return (guint)new_messages;
#undef BUFSIZE
}

static void
__backfill(gchar *str)
{
    gchar *p;

    p = str + strlen(str);
    *(p+1) = 0;

    while(p != str) {
        *p = *(p-1);
        p--;
    }
}

static void
imap_escape_string(gchar *buf, gssize buflen)
{
    gssize room_left;
    gchar *p;

    g_return_if_fail(buf);

    room_left = buflen - strlen(buf);

    for(p = (gchar *)buf; *p; p++) {
        if(!room_left)
            break;

        if(*p == '\\') {
            DBG("backfilling '%s'", p);
            __backfill(p+1);
            *(p+1) = '\\';
            p++;
            room_left--;
        }
    }
}

static void
imap_check_mail(XfceMailwatchIMAPMailbox *imailbox)
{
#define BUFSIZE 1024
    gchar host[BUFSIZE], username[BUFSIZE], password[BUFSIZE];
    guint new_messages = 0;
    GList *mailboxes_to_check = NULL, *l;
    XfceMailwatchAuthType auth_type;
    gint nonstandard_port = -1;

    g_mutex_lock(imailbox->config_mx);

    if(!imailbox->host || !imailbox->username || !imailbox->password) {
        g_mutex_unlock(imailbox->config_mx);
        return;
    }

    g_strlcpy(host, imailbox->host, BUFSIZE);
    g_strlcpy(username, imailbox->username, BUFSIZE);
    g_strlcpy(password, imailbox->password, BUFSIZE);
    auth_type = imailbox->auth_type;
    if(!imailbox->use_standard_port)
        nonstandard_port = imailbox->nonstandard_port;

    /* make a deep copy of the mailbox list */
    for(l = imailbox->mailboxes_to_check; l; l = l->next)
        mailboxes_to_check = g_list_prepend(mailboxes_to_check, g_strdup(l->data));

    g_mutex_unlock(imailbox->config_mx);

    /* escape stuff */
    imap_escape_string(username, BUFSIZE);
    imap_escape_string(password, BUFSIZE);

    if(!imap_authenticate(imailbox, host, username, password, auth_type,
            nonstandard_port))
    {
        DBG("failed to connect to imap server");
        goto cleanup;
    }

    new_messages = imap_check_mailbox(imailbox, "INBOX");
    DBG("checked inbox, %d new messages", new_messages);
    for(l = mailboxes_to_check; l; l = l->next) {
        new_messages += imap_check_mailbox(imailbox, l->data);
        DBG("checked mail folder %s, total is now %d new messages", (gchar *)l->data, new_messages);
    }

    xfce_mailwatch_signal_new_messages(imailbox->mailwatch,
            XFCE_MAILWATCH_MAILBOX(imailbox), new_messages);

    cleanup:

    imap_do_logout(imailbox);

    if(mailboxes_to_check) {
        g_list_foreach(mailboxes_to_check, (GFunc)g_free, NULL);
        g_list_free(mailboxes_to_check);
    }

    xfce_mailwatch_net_tls_teardown(&imailbox->security_info);

#undef BUFSIZE
}

static gpointer
imap_check_mail_th(gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    gboolean running = FALSE;
    GTimeVal start, now;
    guint timeout = 0, delta = 0;

    g_async_queue_ref(imailbox->aqueue);

    g_get_current_time(&start);

    for(;;) {
        gpointer msg = g_async_queue_try_pop(imailbox->aqueue);

        if(msg) {
            if(msg == IMAP_CMD_START) {
                g_get_current_time(&start);;
                running = TRUE;
            } else if(msg == IMAP_CMD_PAUSE)
                running = FALSE;
            else if(msg == IMAP_CMD_TIMEOUT)
                timeout = GPOINTER_TO_UINT(g_async_queue_pop(imailbox->aqueue));
            else if(msg == IMAP_CMD_QUIT) {
                g_async_queue_unref(imailbox->aqueue);
                g_thread_exit(NULL);
            }
        }

        g_get_current_time(&now);

        if(running && (msg == IMAP_CMD_UPDATE
                || now.tv_sec - start.tv_sec >= timeout - delta))
        {
            imap_check_mail(imailbox);
            g_get_current_time(&start);
            delta = (gint)start.tv_sec - now.tv_sec;
        } else
            g_usleep(250000);
    }

    /* NOTREACHED */
    g_async_queue_unref(imailbox->aqueue);
    return NULL;
}

static XfceMailwatchMailbox *
imap_mailbox_new(XfceMailwatch *mailwatch, XfceMailwatchMailboxType *type)
{
    XfceMailwatchIMAPMailbox *imailbox = g_new0(XfceMailwatchIMAPMailbox, 1);
    imailbox->mailbox.type = type;
    imailbox->mailwatch = mailwatch;
    imailbox->timeout = XFCE_MAILWATCH_DEFAULT_TIMEOUT;
    imailbox->use_standard_port = TRUE;
    imailbox->config_mx = g_mutex_new();
    /* init the queue */
    imailbox->aqueue = g_async_queue_new();
    /* and init the timeout */
    g_async_queue_push(imailbox->aqueue, IMAP_CMD_TIMEOUT);
    g_async_queue_push(imailbox->aqueue,
                       GUINT_TO_POINTER(XFCE_MAILWATCH_DEFAULT_TIMEOUT));
    /* create checker thread */
    imailbox->th = g_thread_create(imap_check_mail_th, imailbox, TRUE, NULL);

    return (XfceMailwatchMailbox *)imailbox;
}

static void
imap_set_activated(XfceMailwatchMailbox *mailbox, gboolean activated)
{
    XfceMailwatchIMAPMailbox *imailbox = XFCE_MAILWATCH_IMAP_MAILBOX(mailbox);

    g_async_queue_push(imailbox->aqueue, activated ? IMAP_CMD_START : IMAP_CMD_PAUSE);
}

static void
imap_force_update_cb(XfceMailwatchMailbox *mailbox)
{
    XfceMailwatchIMAPMailbox *imailbox = XFCE_MAILWATCH_IMAP_MAILBOX(mailbox);

    g_async_queue_push(imailbox->aqueue, IMAP_CMD_UPDATE);
}

static gboolean
imap_host_entry_focus_out_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    gchar *str;

    str = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);

    g_mutex_lock(imailbox->config_mx);

    g_free(imailbox->host);
    if(!str || !*str) {
        imailbox->host = NULL;
        g_free(str);
    } else
        imailbox->host = str;

    g_mutex_unlock(imailbox->config_mx);

    return FALSE;
}

static gboolean
imap_username_entry_focus_out_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    gchar *str;

    str = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);

    g_mutex_lock(imailbox->config_mx);

    g_free(imailbox->username);
    if(!str || !*str) {
        imailbox->username = NULL;
        g_free(str);
    } else
        imailbox->username = str;

    g_mutex_unlock(imailbox->config_mx);

    return FALSE;
}

static gboolean
imap_password_entry_focus_out_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    gchar *str;

    str = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);

    g_mutex_lock(imailbox->config_mx);

    g_free(imailbox->password);
    if(!str || !*str) {
        imailbox->password = NULL;
        g_free(str);
    } else
        imailbox->password = str;

    g_mutex_unlock(imailbox->config_mx);

    return FALSE;
}

static gboolean
imap_config_timeout_spinbutton_changed_cb(GtkSpinButton *sb, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    gint value = gtk_spin_button_get_value_as_int(sb) * 60;

    imailbox->timeout = value;
    g_async_queue_push(imailbox->aqueue, IMAP_CMD_TIMEOUT);
    g_async_queue_push(imailbox->aqueue, GUINT_TO_POINTER(value));

    return FALSE;
}

static GNode *
my_g_node_insert_data_sorted(GNode *parent, gpointer data)
{
    IMAPFolderData *fdata = data;
    GNode *new_node = NULL, *n;

    g_return_val_if_fail(parent && data, NULL);

    for(n = parent->children; n; n = n->next) {
        IMAPFolderData *a_fdata = n->data;
        if(g_ascii_strcasecmp(fdata->folder_name, a_fdata->folder_name) <= 0) {
            new_node = g_node_insert_data_before(parent, n, data);
            break;
        }
    }

    if(!new_node)
        new_node = g_node_append_data(parent, data);

    return new_node;
}

static gboolean
imap_populate_folder_tree(XfceMailwatchIMAPMailbox *imailbox,
        const gchar *cur_folder, GNode *parent)
{
#define BUFSIZE 16383
    gboolean ret = TRUE;
    gchar buf[BUFSIZE+1], fullpath[(BUFSIZE+1)/8] = "", separator[2] = { 0, 0 };
    gchar *p, *q, **resp_lines;
    gint bin_tot = 0, bin, i;
    gboolean holds_messages = TRUE, has_children = TRUE;
    IMAPFolderData *fdata;
    GNode *node;

    g_return_val_if_fail(cur_folder, TRUE);

    TRACE("entering (%p, %s, %p)", imailbox, cur_folder, parent);

    g_snprintf(buf, BUFSIZE, "%05d LIST \"%s\" \"%%\"\r\n",
            ++imailbox->imap_tag, cur_folder);
    if(imap_send(imailbox, buf) != strlen(buf))
        return FALSE;
    DBG("sent LIST: '%s'", buf);

    *buf = 0;
    while(!strstr(buf, " OK") && bin_tot < BUFSIZE) {
        /* this is probably a bad idea... */

        bin = imap_recv(imailbox, buf+bin_tot, BUFSIZE);
        if(bin < 0) {
            DBG("imap_recv() failed");
            return FALSE;
        }
        bin_tot += bin;

        DBG("got LIST response (%d): '%s'", bin, buf+bin_tot-bin);

        if(strstr(buf, " NO ") || strstr(buf, " BAD "))
            return FALSE;
    }

    if(g_async_queue_try_pop(imailbox->folder_tree_aqueue) == IMAP_CMD_QUIT)
        return FALSE;

    if(!strstr(buf, " OK"))
        return FALSE;

    if(strstr(buf, "\r"))
        resp_lines = g_strsplit(buf, "\r\n", -1);
    else
        resp_lines = g_strsplit(buf, "\n", -1);

    for(i = 0; resp_lines[i]; i++) {
        if(*resp_lines[i] != '*')
            continue;

        /* special case: NIL for a separator */
        p = strstr(resp_lines[i], "NIL");
        if(p) {
            p += 4;
            if(!*p)
                continue;
            else if(*p == '"') {
                p++;
                p[strlen(p)-1] = 0;
            }

            /* since the separator is NIL, it can't have subfolders.  if it
             * doesn't hold any messages, there's no point in adding it. */
            if(strstr(resp_lines[i], "\\NoSelect"))
                continue;

            fdata = g_new0(IMAPFolderData, 1);
            fdata->folder_name = g_strdup(p);
            fdata->full_path = g_strdup(p);
            fdata->holds_messages = TRUE;

            my_g_node_insert_data_sorted(parent, fdata);

            continue;
        }


        /* first quote before separator */
        p = strstr(resp_lines[i], "\"");
        if(!p)
            continue;
        *separator = *(p+1);

        /* quote after separator */
        p = strstr(p+1, "\"");
        if(!p)
            continue;

        /* space before folder name */
        p = strstr(p+1, " ");
        if(!p)
            continue;
        /* this is stupid */
        p++;
        if(*p == '"') {
            p++;
            p[strlen(p)-1] = 0;
        }

        /* sometimes the first entry is just the name of the current folder
         * itself. */
        if(!strcmp(p, cur_folder))
            continue;

        if(G_NODE_IS_ROOT(parent)) {
            /* if there's no parent, we need to be especially careful about what
             * we list here, as some IMAP servers return the entire content of
             * the home directory in the toplevel listing */

            if(imailbox->server_directory && *imailbox->server_directory
                    && strstr(p, imailbox->server_directory) != p)
            {
                continue;
            }

            if(*p == '.')
                continue;

            if((strstr(resp_lines[i], "\\NoInferiors") || strstr(resp_lines[i], "\\HasNoChildren"))
                    && strstr(resp_lines[i], "\\NoSelect"))
            {
                continue;
            }
        }

        has_children = (!strstr(resp_lines[i], "\\HasNoChildren")
                && !strstr(resp_lines[i], "\\NoInferiors"));
        holds_messages = !strstr(resp_lines[i], "\\NoSelect");

        /* we only want the folder name, not the entire hierarchy */
        q = g_strrstr(p, separator);
        if(q)
            p = q + 1;

        /* i'm not sure why this happens sometimes.  my code is probably buggy */
        if(!*p)
            continue;

        g_snprintf(fullpath, (BUFSIZE+1)/8, "%s%s", cur_folder, p);

        fdata = g_new0(IMAPFolderData, 1);
        fdata->folder_name = g_strdup(p);
        fdata->full_path = g_strdup(fullpath);
        fdata->holds_messages = holds_messages;

        node = my_g_node_insert_data_sorted(parent, fdata);

        if(has_children) {
            g_strlcat(fullpath, separator, (BUFSIZE+1)/8);
            if(!imap_populate_folder_tree(imailbox, fullpath, node))
                return FALSE;
        }

        if(g_async_queue_try_pop(imailbox->folder_tree_aqueue) == IMAP_CMD_QUIT)
            return FALSE;
    }

    g_strfreev(resp_lines);

    return ret;
#undef BUFSIZE
}

static void
imap_populate_folder_tree_nodes_rec(XfceMailwatchIMAPMailbox *imailbox,
        GHashTable *mailboxes_to_check, GNode *node, GtkTreeIter *parent)
{
    GtkTreeIter itr;
    GNode *n;
    IMAPFolderData *fdata = node->data;
    gboolean is_inbox;

    is_inbox = !g_ascii_strcasecmp(fdata->folder_name, "inbox");

    if(is_inbox)
        gtk_tree_store_prepend(imailbox->ts, &itr, parent);
    else
        gtk_tree_store_append(imailbox->ts, &itr, parent);

    gtk_tree_store_set(imailbox->ts, &itr,
            IMAP_FOLDERS_NAME, fdata->folder_name,
            IMAP_FOLDERS_WATCHING,
              (is_inbox || g_hash_table_lookup(mailboxes_to_check, fdata->full_path)),
            IMAP_FOLDERS_HOLDS_MESSAGES, fdata->holds_messages,
            IMAP_FOLDERS_FULLPATH, fdata->full_path,
            -1);

    node->data = NULL;
    g_free(fdata->folder_name);
    g_free(fdata->full_path);
    g_free(fdata);

    for(n = node->children; n; n = n->next)
        imap_populate_folder_tree_nodes_rec(imailbox, mailboxes_to_check, n, &itr);
}

static gboolean
imap_populate_folder_tree_nodes(gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    GHashTable *mailboxes_to_check;
    GList *l;
    GNode *n;

    if(imailbox->folder_tree_th) {
        g_thread_join(imailbox->folder_tree_th);
        imailbox->folder_tree_th = NULL;
        g_async_queue_unref(imailbox->folder_tree_aqueue);
        imailbox->folder_tree_aqueue = NULL;
    }

    if(!imailbox->folder_tree_dialog)
        return FALSE;

    g_mutex_lock(imailbox->config_mx);

    /* make a deep copy of the mailbox list */
    mailboxes_to_check = g_hash_table_new_full(g_str_hash, g_str_equal,
            (GDestroyNotify)g_free, NULL);
    for(l = imailbox->mailboxes_to_check; l; l = l->next)
        g_hash_table_insert(mailboxes_to_check, g_strdup(l->data), GINT_TO_POINTER(1));

    g_mutex_unlock(imailbox->config_mx);

    gtk_tree_store_clear(imailbox->ts);
    g_object_set(G_OBJECT(imailbox->render), "foreground-set", FALSE,
            "style-set", FALSE, NULL);

    for(n = imailbox->folder_tree->children; n; n = n->next)
        imap_populate_folder_tree_nodes_rec(imailbox, mailboxes_to_check, n, NULL);

    g_node_destroy(imailbox->folder_tree);
    imailbox->folder_tree = NULL;

    g_hash_table_destroy(mailboxes_to_check);

    gtk_widget_set_sensitive(imailbox->refresh_btn, TRUE);

    return FALSE;
}

static gboolean
imap_populate_folder_tree_failed(gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    GtkTreeIter itr;

    if(imailbox->folder_tree_th) {
        g_thread_join(imailbox->folder_tree_th);
        imailbox->folder_tree_th = NULL;
        g_async_queue_unref(imailbox->folder_tree_aqueue);
        imailbox->folder_tree_aqueue = NULL;
    }

    if(!imailbox->folder_tree_dialog)
        return FALSE;

    gtk_tree_store_clear(imailbox->ts);
    gtk_tree_store_append(imailbox->ts, &itr, NULL);
    gtk_tree_store_set(imailbox->ts, &itr,
                       IMAP_FOLDERS_NAME, _("Failed to get folder list"),
                       IMAP_FOLDERS_HOLDS_MESSAGES, FALSE,
                       -1);

    gtk_widget_set_sensitive(imailbox->refresh_btn, TRUE);

    return FALSE;
}

static gboolean
imap_folder_tree_th_join(gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;

    if(imailbox->folder_tree_th) {
        g_thread_join(imailbox->folder_tree_th);
        imailbox->folder_tree_th = NULL;
        g_async_queue_unref(imailbox->folder_tree_aqueue);
        imailbox->folder_tree_aqueue = NULL;
    }

    if(imailbox->folder_tree_dialog)
        gtk_widget_set_sensitive(imailbox->refresh_btn, TRUE);

    return FALSE;
}

static gboolean
imap_free_folder_data(GNode *node, gpointer data)
{
    IMAPFolderData *fdata = node->data;

    if(fdata == (gpointer)0xdeadbeef)
        return FALSE;

    g_free(fdata->folder_name);
    g_free(fdata->full_path);
    g_free(fdata);

    return FALSE;
}

static gpointer
imap_populate_folder_tree_th(gpointer data)
{
#define BUFSIZE 1024
    XfceMailwatchIMAPMailbox *imailbox = data;
    gchar host[BUFSIZE], username[BUFSIZE], password[BUFSIZE];
    XfceMailwatchAuthType auth_type;
    gint nonstandard_port = -1;

    TRACE("entering");

    g_async_queue_ref(imailbox->folder_tree_aqueue);

    g_mutex_lock(imailbox->config_mx);

    if(!imailbox->host || !imailbox->username || !imailbox->password) {
        g_mutex_unlock(imailbox->config_mx);
        g_idle_add(imap_folder_tree_th_join, imailbox);
        return NULL;
    }

    g_strlcpy(host, imailbox->host, BUFSIZE);
    g_strlcpy(username, imailbox->username, BUFSIZE);
    g_strlcpy(password, imailbox->password, BUFSIZE);
    auth_type = imailbox->auth_type;
    if(!imailbox->use_standard_port)
        nonstandard_port = imailbox->nonstandard_port;

    g_mutex_unlock(imailbox->config_mx);

    imap_escape_string(username, BUFSIZE);
    imap_escape_string(password, BUFSIZE);

    if(imap_authenticate(imailbox, host, username, password, auth_type,
            nonstandard_port))
    {
       if(g_async_queue_try_pop(imailbox->folder_tree_aqueue) != IMAP_CMD_QUIT) {
           imailbox->folder_tree = g_node_new((gpointer)0xdeadbeef);
           if(imap_populate_folder_tree(imailbox, "", imailbox->folder_tree))
               g_idle_add(imap_populate_folder_tree_nodes, imailbox);
           else {
               g_node_traverse(imailbox->folder_tree, G_IN_ORDER,
                               G_TRAVERSE_ALL, -1, imap_free_folder_data, NULL);
               g_node_destroy(imailbox->folder_tree);
               g_idle_add(imap_folder_tree_th_join, imailbox);
           }
       } else
           g_idle_add(imap_folder_tree_th_join, imailbox);
    } else {
        DBG("failed to connect to imap server to probe folders");
        g_idle_add(imap_populate_folder_tree_failed, imailbox);
    }

    g_async_queue_unref(imailbox->folder_tree_aqueue);

    return NULL;
#undef BUFSIZE
}

static void
imap_config_newmailfolders_destroy_cb(GtkWidget *w, gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;

    imailbox->folder_tree_dialog = NULL;

    if(imailbox->folder_tree_aqueue)
        g_async_queue_push(imailbox->folder_tree_aqueue, IMAP_CMD_QUIT);
}

static void
imap_config_refresh_btn_clicked_cb(GtkWidget *w, gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    GtkTreeIter itr;

    if(!imailbox->host || !imailbox->username)
        return;

    gtk_widget_set_sensitive(imailbox->refresh_btn, FALSE);

    gtk_tree_store_clear(imailbox->ts);
    gtk_tree_store_append(imailbox->ts, &itr, NULL);
    gtk_tree_store_set(imailbox->ts, &itr, IMAP_FOLDERS_NAME,
            _("Please wait..."), -1);
    g_object_set(G_OBJECT(imailbox->render),
                "foreground-set", TRUE,
                "style-set", TRUE, NULL);

    imailbox->folder_tree_aqueue = g_async_queue_new();
    imailbox->folder_tree_th = g_thread_create(imap_populate_folder_tree_th,
                                               imailbox, TRUE, NULL);
}

static gboolean
imap_config_treeview_btnpress_cb(GtkWidget *w, GdkEventButton *evt,
        gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    GtkTreeViewColumn *col = NULL;
    GtkTreePath *path = NULL;

    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(w), evt->x, evt->y, &path, &col, NULL, NULL))
        return FALSE;

    if(col == gtk_tree_view_get_column(GTK_TREE_VIEW(w), 1)) {
        GtkTreeIter itr;

        if(gtk_tree_model_get_iter(GTK_TREE_MODEL(imailbox->ts), &itr, path)) {
            gchar *folder_name = NULL, *folder_path = NULL;
            gboolean watching = FALSE, holds_messages = FALSE;

            gtk_tree_model_get(GTK_TREE_MODEL(imailbox->ts), &itr,
                    IMAP_FOLDERS_NAME, &folder_name,
                    IMAP_FOLDERS_WATCHING, &watching,
                    IMAP_FOLDERS_HOLDS_MESSAGES, &holds_messages,
                    IMAP_FOLDERS_FULLPATH, &folder_path, -1);

            if(holds_messages && g_ascii_strcasecmp(folder_name, "inbox")) {
                gtk_tree_store_set(imailbox->ts, &itr,
                        IMAP_FOLDERS_WATCHING, !watching, -1);

                g_mutex_lock(imailbox->config_mx);
                if(watching) {
                    GList *l;
                    for(l = imailbox->mailboxes_to_check; l; l = l->next) {
                        if(!strcmp(folder_path, l->data)) {
                            g_free(l->data);
                            imailbox->mailboxes_to_check =
                                    g_list_delete_link(imailbox->mailboxes_to_check, l);
                            break;
                        }
                    }
                    g_free(folder_path);
                } else {
                    imailbox->mailboxes_to_check =
                            g_list_prepend(imailbox->mailboxes_to_check,
                                    folder_path);
                }
                g_mutex_unlock(imailbox->config_mx);
            } else
                g_free(folder_path);

            g_free(folder_name);
        }
    }


    if(evt->type == GDK_2BUTTON_PRESS) {
        if(!gtk_tree_view_row_expanded(GTK_TREE_VIEW(w), path))
            gtk_tree_view_expand_row(GTK_TREE_VIEW(w), path, FALSE);
        else
            gtk_tree_view_collapse_row(GTK_TREE_VIEW(w), path);
    }

    gtk_tree_path_free(path);

    return FALSE;
}

static void
imap_config_newmailfolders_btn_clicked_cb(GtkWidget *w, gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    GtkWidget *dlg, *topvbox, *vbox, *hbox, *treeview, *frame, *frame_bin,
              *btn, *sw;
    GtkWindow *toplevel = GTK_WINDOW(gtk_widget_get_toplevel(w));
    GtkTreeStore *ts;
    GtkTreeIter itr;
    GtkCellRenderer *render;
    GtkTreeViewColumn *col;
    GtkTreeSelection *sel;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    if(!imailbox->host || !imailbox->username) {
        /* _TOFE_
        xfce_message_dialog(toplevel, _("Error"), GTK_STOCK_DIALOG_WARNING,
                _("No server or username is set."),
                _("The folder list cannot be retrieved until a server, username, and probably password are set.  Also be sure to check any security settings in the Advanced dialog."),
                GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
        */
        return;
    }

    dlg = gtk_dialog_new_with_buttons(_("Set New Mail Folders"), toplevel,
            GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_NO_SEPARATOR,
            GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
    imailbox->folder_tree_dialog = dlg;
    topvbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_container_set_border_width(GTK_CONTAINER(topvbox), BORDER/2);
    gtk_widget_show(topvbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), topvbox, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(dlg), "destroy",
            G_CALLBACK(imap_config_newmailfolders_destroy_cb), imailbox);

    frame = xfce_mailwatch_create_framebox(_("New Mail Folders"), &frame_bin);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(topvbox), frame, TRUE, TRUE, 0);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame_bin), hbox);

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER,
            GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
            GTK_SHADOW_ETCHED_IN);
    gtk_widget_show(sw);
    gtk_box_pack_start(GTK_BOX(hbox), sw, TRUE, TRUE, 0);

    imailbox->ts = ts = gtk_tree_store_new(IMAP_FOLDERS_N_COLUMNS,
            G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING);

    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ts));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
    gtk_widget_add_events(treeview, GDK_BUTTON_PRESS);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "mailbox-name");
    gtk_tree_view_column_set_expand(col, TRUE);

    render = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(col, render, FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
    g_object_set(G_OBJECT(render), "stock-id", GTK_STOCK_DIRECTORY,
            "stock-size", GTK_ICON_SIZE_MENU, NULL);
#else
    {
        gint iw, ih;
        GdkPixbuf *pix;
        GList *icons = NULL;
        GdkScreen *gscreen = gtk_widget_get_screen(treeview);
        XfceIconTheme *itheme = xfce_icon_theme_get_for_screen(gscreen);

        icons = g_list_prepend(icons, "stock_open");
        icons = g_list_prepend(icons, "stock_folder");
        icons = g_list_prepend(icons, "stock_directory");

        gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &iw, &ih);
        pix = xfce_icon_theme_load_list(itheme, icons, iw);
        if(pix) {
            g_object_set(G_OBJECT(render), "pixbuf", pix, NULL);
            g_object_unref(G_OBJECT(pix));
        }

        g_list_free(icons);
    }
#endif

    imailbox->render = render = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_set_attributes(col, render,
            "text", IMAP_FOLDERS_NAME, NULL);
    {
        GtkStyle *style;
        gtk_widget_realize(topvbox);
        style = gtk_widget_get_style(topvbox);
        g_object_set(G_OBJECT(render), "foreground-gdk",
                &style->fg[GTK_STATE_INSENSITIVE],
                "foreground-set", TRUE,
                "style", PANGO_STYLE_ITALIC,
                "style-set", TRUE, NULL);
    }

    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);
    gtk_tree_view_set_expander_column(GTK_TREE_VIEW(treeview), col);

    render = gtk_cell_renderer_toggle_new();
    col = gtk_tree_view_column_new_with_attributes("watching", render, "active",
            IMAP_FOLDERS_WATCHING, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

    gtk_widget_show(treeview);
    gtk_container_add(GTK_CONTAINER(sw), treeview);
    g_signal_connect(G_OBJECT(treeview), "button-press-event",
            G_CALLBACK(imap_config_treeview_btnpress_cb), imailbox);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
    gtk_tree_selection_unselect_all(sel);

    vbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

    imailbox->refresh_btn = btn = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(btn), "mailwatch-treeview", treeview);
    g_signal_connect(G_OBJECT(btn), "clicked",
            G_CALLBACK(imap_config_refresh_btn_clicked_cb), imailbox);

    gtk_tree_store_append(ts, &itr, NULL);
    gtk_tree_store_set(ts, &itr, IMAP_FOLDERS_NAME, _("Please wait..."), -1);
    gtk_widget_set_sensitive(btn, FALSE);
    imailbox->folder_tree_aqueue = g_async_queue_new();
    imailbox->folder_tree_th = g_thread_create(imap_populate_folder_tree_th,
                                               imailbox, TRUE, NULL);

    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

static void
imap_config_nonstandard_chk_cb(GtkToggleButton *tb, gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    GtkWidget *entry = g_object_get_data(G_OBJECT(tb), "xfmw-entry");

    g_mutex_lock(imailbox->config_mx);

    imailbox->use_standard_port = !gtk_toggle_button_get_active(tb);
    gtk_widget_set_sensitive(entry, !imailbox->use_standard_port);

    g_mutex_unlock(imailbox->config_mx);
}

static gboolean
imap_config_nonstandard_focusout_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;

    g_mutex_lock(imailbox->config_mx);

    imailbox->nonstandard_port = atoi(gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1));

    g_mutex_unlock(imailbox->config_mx);

    return FALSE;
}

static void
imap_config_security_combo_changed_cb(GtkWidget *w, gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    GtkWidget *entry = g_object_get_data(G_OBJECT(w), "xfmw-entry");

    g_mutex_lock(imailbox->config_mx);

    imailbox->auth_type = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

    if(imailbox->use_standard_port) {
        if(imailbox->auth_type == AUTH_SSL_PORT)
            gtk_entry_set_text(GTK_ENTRY(entry), IMAPS_PORT_S);
        else
            gtk_entry_set_text(GTK_ENTRY(entry), IMAP_PORT_S);
    }

    g_mutex_unlock(imailbox->config_mx);
}

static gboolean
imap_config_serverdir_focusout_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;

    g_mutex_lock(imailbox->config_mx);

    g_free(imailbox->server_directory);
    imailbox->server_directory = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);

    g_mutex_unlock(imailbox->config_mx);

    return FALSE;
}

static void
imap_config_advanced_btn_clicked_cb(GtkWidget *w, gpointer user_data)
{
    XfceMailwatchIMAPMailbox *imailbox = user_data;
    GtkWidget *dlg, *topvbox, *vbox, *hbox, *lbl, *entry, *frame, *frame_bin,
              *chk, *combo;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    dlg = gtk_dialog_new_with_buttons(_("Advanced IMAP Options"),
            GTK_WINDOW(gtk_widget_get_toplevel(w)),
            GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_NO_SEPARATOR,
            GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
    topvbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_container_set_border_width(GTK_CONTAINER(topvbox), BORDER/2);
    gtk_widget_show(topvbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), topvbox, TRUE, TRUE, 0);

    frame = xfce_mailwatch_create_framebox(_("Connection"), &frame_bin);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(topvbox), frame, FALSE, FALSE, 0);

    vbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(frame_bin), vbox);

    combo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _("Use unsecured connection"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _("Use SSL/TLS on alternate port"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _("Use SSL/TLS via STARTTLS"));
#ifdef HAVE_SSL_SUPPORT
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), imailbox->auth_type);
#else
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_widget_set_sensitive(combo, FALSE);
#endif
    gtk_widget_show(combo);
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(combo), "changed",
            G_CALLBACK(imap_config_security_combo_changed_cb), imailbox);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    chk = gtk_check_button_new_with_mnemonic(_("Use non-standard IMAP _port:"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk),
            !imailbox->use_standard_port);
    gtk_widget_show(chk);
    gtk_box_pack_start(GTK_BOX(hbox), chk, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(chk), "toggled",
            G_CALLBACK(imap_config_nonstandard_chk_cb), imailbox);

    entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 5);
    if(!imailbox->use_standard_port) {
        gchar portstr[16];
        g_snprintf(portstr, 16, "%d", imailbox->nonstandard_port);
        gtk_entry_set_text(GTK_ENTRY(entry), portstr);
    } else {
        gtk_widget_set_sensitive(entry, FALSE);
        if(imailbox->auth_type == AUTH_SSL_PORT)
            gtk_entry_set_text(GTK_ENTRY(entry), IMAPS_PORT_S);
        else
            gtk_entry_set_text(GTK_ENTRY(entry), IMAP_PORT_S);
    }
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(imap_config_nonstandard_focusout_cb), imailbox);

    g_object_set_data(G_OBJECT(chk), "xfmw-entry", entry);
    g_object_set_data(G_OBJECT(combo), "xfmw-entry", entry);

    frame = xfce_mailwatch_create_framebox(_("Folders"), &frame_bin);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(topvbox), frame, FALSE, FALSE, 0);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame_bin), hbox);

    lbl = gtk_label_new_with_mnemonic(_("IMAP server _directory:"));
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);

    entry = gtk_entry_new();
    g_mutex_lock(imailbox->config_mx);
    if(imailbox->server_directory)
        gtk_entry_set_text(GTK_ENTRY(entry), imailbox->server_directory);
    g_mutex_unlock(imailbox->config_mx);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(imap_config_serverdir_focusout_cb), imailbox);

    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

static GtkContainer *
imap_get_setup_page(XfceMailwatchMailbox *mailbox)
{
    XfceMailwatchIMAPMailbox *imailbox = XFCE_MAILWATCH_IMAP_MAILBOX(mailbox);
    GtkWidget *topvbox, *vbox, *hbox, *frame, *frame_bin, *lbl, *entry, *btn,
              *sbtn;
    GtkSizeGroup *sg;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    topvbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_widget_show(topvbox);

    frame = xfce_mailwatch_create_framebox(_("IMAP Server"), &frame_bin);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(topvbox), frame, FALSE, FALSE, 0);

    sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

    vbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(frame_bin), vbox);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    lbl = gtk_label_new_with_mnemonic(_("_Mail server:"));
    gtk_misc_set_alignment(GTK_MISC(lbl), 0.0, 0.5);
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
    gtk_size_group_add_widget(sg, lbl);

    entry = gtk_entry_new();
    if(imailbox->host)
        gtk_entry_set_text(GTK_ENTRY(entry), imailbox->host);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(imap_host_entry_focus_out_cb), imailbox);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), entry);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    lbl = gtk_label_new_with_mnemonic(_("_Username:"));
    gtk_misc_set_alignment(GTK_MISC(lbl), 0.0, 0.5);
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
    gtk_size_group_add_widget(sg, lbl);

    entry = gtk_entry_new();
    if(imailbox->username)
        gtk_entry_set_text(GTK_ENTRY(entry), imailbox->username);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(imap_username_entry_focus_out_cb), imailbox);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), entry);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    lbl = gtk_label_new_with_mnemonic(_("_Password:"));
    gtk_misc_set_alignment(GTK_MISC(lbl), 0.0, 0.5);
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
    gtk_size_group_add_widget(sg, lbl);

    entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    if(imailbox->password)
        gtk_entry_set_text(GTK_ENTRY(entry), imailbox->password);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(imap_password_entry_focus_out_cb), imailbox);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), entry);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(topvbox), hbox, FALSE, FALSE, 0);

    btn = xfce_mailwatch_custom_button_new(_("_Advanced..."),
            GTK_STOCK_PREFERENCES);
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(btn), "clicked",
            G_CALLBACK(imap_config_advanced_btn_clicked_cb), imailbox);

    btn = xfce_mailwatch_custom_button_new(_("New mail _folders..."),
            GTK_STOCK_DIRECTORY);
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(btn), "clicked",
            G_CALLBACK(imap_config_newmailfolders_btn_clicked_cb), imailbox);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(topvbox), hbox, FALSE, FALSE, 0);

    lbl = gtk_label_new_with_mnemonic(_("Check for _new messages every"));
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);

    sbtn = gtk_spin_button_new_with_range(1.0, 1440.0, 1.0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(sbtn), TRUE);
    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(sbtn), FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sbtn), imailbox->timeout/60);
    gtk_widget_show(sbtn);
    gtk_box_pack_start(GTK_BOX(hbox), sbtn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(sbtn), "focus-out-event",
            G_CALLBACK(imap_config_timeout_spinbutton_changed_cb), imailbox);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), sbtn);

    lbl = gtk_label_new(_("minute(s)."));
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);

    return GTK_CONTAINER(topvbox);
}

static void
imap_restore_param_list(XfceMailwatchMailbox *mailbox, GList *params)
{
    XfceMailwatchIMAPMailbox *imailbox = XFCE_MAILWATCH_IMAP_MAILBOX(mailbox);
    GList *l;
    gint n_newmail_boxes = -1;

    g_mutex_lock(imailbox->config_mx);

    for(l = params; l; l = l->next) {
        XfceMailwatchParam *param = l->data;

        if(!strcmp(param->key, "host"))
            imailbox->host = g_strdup(param->value);
        else if(!strcmp(param->key, "username"))
            imailbox->username = g_strdup(param->value);
        else if(!strcmp(param->key, "password"))
            imailbox->password = g_strdup(param->value);
        else if(!strcmp(param->key, "auth_type"))
            imailbox->auth_type = atoi(param->value);
        else if(!strcmp(param->key, "server_directory"))
            imailbox->server_directory = g_strdup(param->value);
        else if(!strcmp(param->key, "use_standard_port"))
            imailbox->use_standard_port = *(param->value) == '0' ? FALSE : TRUE;
        else if(!strcmp(param->key, "nonstandard_port"))
            imailbox->nonstandard_port = atoi(param->value);
        else if(!strcmp(param->key, "timeout")) {
            imailbox->timeout = atoi(param->value);
            g_async_queue_push(imailbox->aqueue, IMAP_CMD_TIMEOUT);
            g_async_queue_push(imailbox->aqueue,
                    GUINT_TO_POINTER(imailbox->timeout));
        } else if(!strcmp(param->key, "n_newmail_boxes"))
            n_newmail_boxes = atoi(param->value);
    }

    for(l = params; l; l = l->next) {
        XfceMailwatchParam *param = l->data;

        if(!strncmp(param->key, "newmail_box_", 12)) {
            imailbox->mailboxes_to_check =
                    g_list_prepend(imailbox->mailboxes_to_check,
                            g_strdup(param->value));
        }
    }
    imailbox->mailboxes_to_check = g_list_reverse(imailbox->mailboxes_to_check);

    g_mutex_unlock(imailbox->config_mx);
}

static GList *
imap_save_param_list(XfceMailwatchMailbox *mailbox)
{
    XfceMailwatchIMAPMailbox *imailbox = XFCE_MAILWATCH_IMAP_MAILBOX(mailbox);
    GList *params = NULL;
    XfceMailwatchParam *param;
    gint i;

    g_mutex_lock(imailbox->config_mx);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("host");
    param->value = g_strdup(imailbox->host);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("username");
    param->value = g_strdup(imailbox->username);
    params = g_list_prepend(params, param);

    /* FIXME: probably would be nice to obscure this somewhat to deter casual
     * accidental exposure */
    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("password");
    param->value = g_strdup(imailbox->password);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("auth_type");
    param->value = g_strdup_printf("%d", imailbox->auth_type);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("server_directory");
    param->value = g_strdup(imailbox->server_directory);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("use_standard_port");
    param->value = g_strdup(imailbox->use_standard_port ? "1" : "0");
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("nonstandard_port");
    param->value = g_strdup_printf("%d", imailbox->nonstandard_port);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("timeout");
    param->value = g_strdup_printf("%d", imailbox->timeout);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("n_newmail_boxes");
    param->value = g_strdup_printf("%d", g_list_length(imailbox->mailboxes_to_check));
    params = g_list_prepend(params, param);

    for(i = 0; i < g_list_length(imailbox->mailboxes_to_check); i++) {
        param = g_new(XfceMailwatchParam, 1);
        param->key = g_strdup_printf("newmail_box_%d", i);
        param->value = g_strdup(g_list_nth_data(imailbox->mailboxes_to_check, i));
        params = g_list_prepend(params, param);
    }

    g_mutex_unlock(imailbox->config_mx);

    return g_list_reverse(params);
}

static void
imap_mailbox_free(XfceMailwatchMailbox *mailbox)
{
    XfceMailwatchIMAPMailbox *imailbox = XFCE_MAILWATCH_IMAP_MAILBOX(mailbox);

    g_async_queue_push(imailbox->aqueue, IMAP_CMD_QUIT);
    g_thread_join(imailbox->th);
    g_async_queue_unref(imailbox->aqueue);

    g_mutex_free(imailbox->config_mx);

    g_free(imailbox->host);
    g_free(imailbox->username);
    g_free(imailbox->password);

    g_free(imailbox);
}

XfceMailwatchMailboxType builtin_mailbox_type_imap = {
    "imap",
    N_("Remote IMAP Mailbox"),
    N_("The IMAP plugin can connect to a remote mail server that supports the IMAP protocol, optionally using SSL for link protection."),

    imap_mailbox_new,
    imap_set_activated,
    imap_force_update_cb,
    imap_get_setup_page,
    imap_restore_param_list,
    imap_save_param_list,
    imap_mailbox_free
};
