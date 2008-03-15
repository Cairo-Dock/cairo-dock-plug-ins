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

#define BORDER                   8

#define POP3_PORT_S              "110"
#define POP3S_PORT_S             "995"

#define XFCE_MAILWATCH_POP3_MAILBOX(ptr) ((XfceMailwatchPOP3Mailbox *)ptr)

#define POP3_CMD_START           GINT_TO_POINTER(1)
#define POP3_CMD_PAUSE           GINT_TO_POINTER(2)
#define POP3_CMD_TIMEOUT         GINT_TO_POINTER(3)
#define POP3_CMD_QUIT            GINT_TO_POINTER(4)
#define POP3_CMD_UPDATE          GINT_TO_POINTER(5)

typedef struct
{
    XfceMailwatchMailbox mailbox;

    GMutex *config_mx;

    guint timeout;
    gchar *host;
    gchar *username;
    gchar *password;

    gboolean use_standard_port;
    gint nonstandard_port;
    XfceMailwatchAuthType auth_type;

    GThread *th;
    GAsyncQueue *aqueue;

    XfceMailwatch *mailwatch;

    /* state related to the current connection (if any) */
    gint sockfd;
    /* secure this, dude */
    XfceMailwatchSecurityInfo security_info;
} XfceMailwatchPOP3Mailbox;

static gssize
pop3_send(XfceMailwatchPOP3Mailbox *pmailbox, const gchar *buf)
{
    GError *error = NULL;
    gssize sent;

    sent = xfce_mailwatch_net_send(pmailbox->sockfd,
                                   &pmailbox->security_info,
                                   buf,
                                   &error);
    if(sent < 0) {
        xfce_mailwatch_log_message(pmailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(pmailbox),
                                   XFCE_MAILWATCH_LOG_ERROR,
                                   error->message);
        g_error_free(error);
    }

    return sent;
}

static gssize
pop3_recv(XfceMailwatchPOP3Mailbox *pmailbox, gchar *buf, gsize len)
{
    GError *error = NULL;
    gssize recvd;

    recvd = xfce_mailwatch_net_recv(pmailbox->sockfd,
                                    &pmailbox->security_info,
                                    buf,
                                    len,
                                    &error);

    if(recvd < 0) {
        xfce_mailwatch_log_message(pmailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(pmailbox),
                                   XFCE_MAILWATCH_LOG_ERROR,
                                   error->message);
        g_error_free(error);
    }

    return recvd;
}

static void
pop3_do_logout(XfceMailwatchPOP3Mailbox *pmailbox)
{
    pop3_send(pmailbox, "QUIT\r\n");

    shutdown(pmailbox->sockfd, SHUT_RDWR);
    close(pmailbox->sockfd);
    pmailbox->sockfd = -1;
}

static gboolean
pop3_get_sockaddr(XfceMailwatchPOP3Mailbox *pmailbox, const gchar *host,
                  const gchar *service, struct sockaddr_in *addr)
{
    struct addrinfo hints = { 0, PF_INET, SOCK_STREAM, IPPROTO_TCP,
            sizeof(struct sockaddr_in), NULL, NULL, NULL };
    GError *error = NULL;

    TRACE("entering (%s, %s, %p)", host, service, addr);

    g_return_val_if_fail(host && service && addr, FALSE);

    if(!xfce_mailwatch_net_get_sockaddr(host, service, &hints, addr, &error)) {
        xfce_mailwatch_log_message(pmailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(pmailbox),
                                   XFCE_MAILWATCH_LOG_ERROR,
                                   error->message);
        g_error_free(error);
        return FALSE;
    }

    return TRUE;
}

static gboolean
pop3_send_login_info(XfceMailwatchPOP3Mailbox *pmailbox, const gchar *username,
        const gchar *password)
{
#define BUFSIZE 8191
    gint bin, bout;
    gchar buf[BUFSIZE+1];

    TRACE("entering");

    /* send the username */
    g_snprintf(buf, BUFSIZE, "USER %s\r\n", username);
    bout = pop3_send(pmailbox, buf);
    DBG("sent user (%d)", bout);
    if(bout != strlen(buf))
        goto cleanuperr;

    /* check for OK response */
    bin = pop3_recv(pmailbox, buf, BUFSIZE);
    DBG("response from USER (%d): %s", bin, bin>0?buf:"(nada)");
    if(bin <= 0)
        goto cleanuperr;
    DBG("strstr() returns %p", strstr(buf, "+OK"));
    if(g_ascii_strncasecmp(buf, "+OK", 3))
        goto cleanuperr;

    /* send the password */
    g_snprintf(buf, BUFSIZE, "PASS %s\r\n", password);
    bout = pop3_send(pmailbox, buf);
    DBG("sent password (%d)", bout);
    if(bout != strlen(buf))
        goto cleanuperr;

    /* check for OK response */
    bin = pop3_recv(pmailbox, buf, BUFSIZE);
    DBG("response from USER (%d): %s", bin, bin>0?buf:"(nada)");
    if(bin <= 0)
        goto cleanuperr;
    DBG("strstr() returns %p", strstr(buf, "OK"));
    if(g_ascii_strncasecmp(buf, "+OK", 3))
        goto cleanuperr;

    TRACE("leaving (success)");

    return TRUE;

    cleanuperr:

    shutdown(pmailbox->sockfd, SHUT_RDWR);
    close(pmailbox->sockfd);
    pmailbox->sockfd = -1;

    return FALSE;
#undef BUFSIZE
}

static gboolean
pop3_negotiate_ssl(XfceMailwatchPOP3Mailbox *pmailbox, const gchar *host)
{
    gboolean ret;
    GError *error = NULL;

    ret = xfce_mailwatch_net_negotiate_tls(pmailbox->sockfd,
            &pmailbox->security_info, host, &error);

    if(!ret) {
        xfce_mailwatch_log_message(pmailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(pmailbox),
                                   XFCE_MAILWATCH_LOG_ERROR,
                                   _("TLS handshake failed: %s"),
                                   error->message);
        g_error_free(error);
        shutdown(pmailbox->sockfd, SHUT_RDWR);
        close(pmailbox->sockfd);
        pmailbox->sockfd = -1;
    }

    return ret;
}

static gboolean
pop3_do_stls(XfceMailwatchPOP3Mailbox *pmailbox, const gchar *host,
        const gchar *username, const gchar *password)
{
#define BUFSIZE 8191
    gint bin;
    gchar buf[BUFSIZE+1];

    TRACE("entering");

    if(pop3_send(pmailbox, "CAPA\r\n") != 6)
        goto cleanuperr;

    bin = pop3_recv(pmailbox, buf, BUFSIZE);
    DBG("checking for STLS caps (%d): %s", bin, bin>0?buf:"(nada)");
    if(bin <= 0)
        goto cleanuperr;
    if(!strstr(buf, "\r\nSTLS\r\n"))
        goto cleanuperr;

    if(pop3_send(pmailbox, "STLS\r\n") != 6)
        goto cleanuperr;

    if(pop3_recv(pmailbox, buf, BUFSIZE) < 0)
        goto cleanuperr;
    if(g_ascii_strncasecmp(buf, "+OK", 3))
        goto cleanuperr;

    /* now that we've negotiated SSL, reenable using_tls */
    pmailbox->security_info.using_tls = TRUE;

    return TRUE;

    cleanuperr:

    shutdown(pmailbox->sockfd, SHUT_RDWR);
    close(pmailbox->sockfd);
    pmailbox->sockfd = -1;

    return FALSE;
#undef BUFSIZE
}

static gboolean
pop3_connect(XfceMailwatchPOP3Mailbox *pmailbox, const gchar *host,
        const gchar *service, gint nonstandard_port)
{
    struct sockaddr_in addr;

    TRACE("entering (%s)", service);

    if(!pop3_get_sockaddr(pmailbox, host, service, &addr)) {
        DBG("failed to get sockaddr");
        return FALSE;
    }

    if(nonstandard_port > 0)
        addr.sin_port = htons(nonstandard_port);

    pmailbox->sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(pmailbox->sockfd < 0) {
        xfce_mailwatch_log_message(pmailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(pmailbox),
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

    if(fcntl(pmailbox->sockfd, F_SETFL, fcntl(pmailbox->sockfd, F_GETFL) | O_NONBLOCK)) {
        xfce_mailwatch_log_message(pmailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(pmailbox),
                                   XFCE_MAILWATCH_LOG_WARNING,
                                   _("Unable to set socket to non-blocking mode.  If the connect attempt hangs, the panel may hang on close."));
    }

    if(connect(pmailbox->sockfd, (struct sockaddr *)&addr,
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
                FD_SET(pmailbox->sockfd, &wfd);

                DBG("checking for a connection...");

                /* wait until the connect attempt finishes */
                if(select(FD_SETSIZE, NULL, &wfd, NULL, &tv) < 0)
                    break;

                /* check to see if it finished, and, if so, if there was an
                 * error, or if it completed successfully */
                if(FD_ISSET(pmailbox->sockfd, &wfd)) {
                    if(!getsockopt(pmailbox->sockfd, SOL_SOCKET, SO_ERROR,
                                &sock_err, &sock_err_len)
                            && !sock_err)
                    {
                        DBG("    connection succeeded");
                        failed = FALSE;
                    } else {
                        xfce_mailwatch_log_message(pmailbox->mailwatch,
                                                   XFCE_MAILWATCH_MAILBOX(pmailbox),
                                                   XFCE_MAILWATCH_LOG_ERROR,
                                                   _("Failed to connect to server: %s"),
                                                   strerror(sock_err));
                        DBG("    connection failed: sock_err is %d", sock_err);
                    }
                    break;
                }

                /* check the main thread to see if we're supposed to quit */
                msg = g_async_queue_try_pop(pmailbox->aqueue);
                if(msg) {
                    /* put it back so pop3_check_mail_th() can read it */
                    g_async_queue_push(pmailbox->aqueue, msg);
                    if(msg == POP3_CMD_QUIT) {
                        failed = TRUE;
                        break;
                    }
                }
            }
        }

        if(failed) {
            DBG("failed to connect");
            close(pmailbox->sockfd);
            pmailbox->sockfd = -1;
            return FALSE;
        }
    }

    if(fcntl(pmailbox->sockfd, F_SETFL, fcntl(pmailbox->sockfd, F_GETFL) & ~(O_NONBLOCK))) {
        xfce_mailwatch_log_message(pmailbox->mailwatch,
                                   XFCE_MAILWATCH_MAILBOX(pmailbox),
                                   XFCE_MAILWATCH_LOG_WARNING,
                                   _("Unable to return socket to blocking mode.  Data may not be retreived correctly."));
    }

    return TRUE;
}

static gboolean
pop3_authenticate(XfceMailwatchPOP3Mailbox *pmailbox, const gchar *host,
        const gchar *username, const gchar *password,
        XfceMailwatchAuthType auth_type, gint nonstandard_port)
{
    gboolean ret = FALSE;
    gchar buf[1024];

    TRACE("entering, auth_type is %d", auth_type);

    switch(auth_type) {
        case AUTH_NONE:
            pmailbox->security_info.using_tls = FALSE;
            ret = pop3_connect(pmailbox, host, "pop3", nonstandard_port);

            /* discard opening banner */
            if(ret && pop3_recv(pmailbox, buf, 1023) < 0) {
                DBG("failed to get banner");
                shutdown(pmailbox->sockfd, SHUT_RDWR);
                close(pmailbox->sockfd);
                pmailbox->sockfd = -1;
            }
            break;

        case AUTH_STARTTLS:
            pmailbox->security_info.using_tls = FALSE;
            ret = pop3_connect(pmailbox, host, "pop3", nonstandard_port);

            if(ret) {
                /* discard opening banner */
                if(pop3_recv(pmailbox, buf, 1023) < 0) {
                    DBG("failed to get banner");
                    shutdown(pmailbox->sockfd, SHUT_RDWR);
                    close(pmailbox->sockfd);
                    pmailbox->sockfd = -1;
                }

                ret = pop3_do_stls(pmailbox, host, username, password);
                if(ret)
                    ret = pop3_negotiate_ssl(pmailbox, host);
                pmailbox->security_info.using_tls = TRUE;
            }
            break;

        case AUTH_SSL_PORT:
            pmailbox->security_info.using_tls = TRUE;
            ret = pop3_connect(pmailbox, host, "pop3s", nonstandard_port);
            if(ret)
                ret = pop3_negotiate_ssl(pmailbox, host);

            /* discard opening banner */
            if(ret && pop3_recv(pmailbox, buf, 1023) < 0) {
                DBG("failed to get banner");
                shutdown(pmailbox->sockfd, SHUT_RDWR);
                close(pmailbox->sockfd);
                pmailbox->sockfd = -1;
            }
            break;

        default:
            g_critical("XfceMailwatchPOP3Mailbox: Unknown auth type (%d)", auth_type);
            return FALSE;
    }

    DBG("using_tls is %s", pmailbox->security_info.using_tls?"TRUE":"FALSE");

    if(ret && !pop3_send_login_info(pmailbox, username, password))
        return FALSE;

    return ret;
}

static guint
pop3_check_inbox(XfceMailwatchPOP3Mailbox *pmailbox)
{
    gchar buf[1024], *p;
    gint bin;
    gint new_messages;

    if(pop3_send(pmailbox, "STAT\r\n") != 6)
        return 0;

    bin = pop3_recv(pmailbox, buf, 1023);
    if(bin <= 0)
        return 0;
    DBG("got response from STAT (%d): %s", bin, buf);
    if(g_ascii_strncasecmp(buf, "+OK", 3))
        return 0;

    p = strstr(buf, "\r\n");
    if(!p)
        return 0;
    *p = 0;

    new_messages = atoi(buf+4);
    if(new_messages < 0)
       return 0;

    return new_messages;
}

static void
pop3_check_mail(XfceMailwatchPOP3Mailbox *pmailbox)
{
#define BUFSIZE 1024
    gchar host[BUFSIZE], username[BUFSIZE], password[BUFSIZE];
    guint new_messages = 0;
    XfceMailwatchAuthType auth_type;
    gint nonstandard_port = -1;

    g_mutex_lock(pmailbox->config_mx);

    if(!pmailbox->host || !pmailbox->username || !pmailbox->password) {
        g_mutex_unlock(pmailbox->config_mx);
        return;
    }

    g_strlcpy(host, pmailbox->host, BUFSIZE);
    g_strlcpy(username, pmailbox->username, BUFSIZE);
    g_strlcpy(password, pmailbox->password, BUFSIZE);
    auth_type = pmailbox->auth_type;
    if(!pmailbox->use_standard_port)
        nonstandard_port = pmailbox->nonstandard_port;

    g_mutex_unlock(pmailbox->config_mx);

    if(!pop3_authenticate(pmailbox, host, username, password, auth_type,
            nonstandard_port))
    {
        DBG("failed to connect to pop3 server");
        goto cleanup;
    }

    new_messages = pop3_check_inbox(pmailbox);
    DBG("checked inbox, %d new messages", new_messages);

    xfce_mailwatch_signal_new_messages(pmailbox->mailwatch,
            XFCE_MAILWATCH_MAILBOX(pmailbox), new_messages);

    cleanup:

    pop3_do_logout(pmailbox);

    xfce_mailwatch_net_tls_teardown(&pmailbox->security_info);

#undef BUFSIZE
}

static gpointer
pop3_check_mail_th(gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;
    gboolean running = FALSE;
    GTimeVal start, now;
    guint timeout = 0, delta = 0;

    g_async_queue_ref(pmailbox->aqueue);

    g_get_current_time(&start);

    for(;;) {
        gpointer msg = g_async_queue_try_pop(pmailbox->aqueue);

        if(msg) {
            if(msg == POP3_CMD_START) {
                g_get_current_time(&start);;
                running = TRUE;
            } else if(msg == POP3_CMD_PAUSE)
                running = FALSE;
            else if(msg == POP3_CMD_TIMEOUT)
                timeout = GPOINTER_TO_UINT(g_async_queue_pop(pmailbox->aqueue));
            else if(msg == POP3_CMD_QUIT) {
                g_async_queue_unref(pmailbox->aqueue);
                g_thread_exit(NULL);
            }
        }

        g_get_current_time(&now);

        if(running && (msg == POP3_CMD_UPDATE
                || now.tv_sec - start.tv_sec >= timeout - delta))
        {
            pop3_check_mail(pmailbox);
            g_get_current_time(&start);
            delta = (gint)start.tv_sec - now.tv_sec;
        } else
            g_usleep(250000);
    }

    /* NOTREACHED */
    g_async_queue_unref(pmailbox->aqueue);
    return NULL;
}

static XfceMailwatchMailbox *
pop3_mailbox_new(XfceMailwatch *mailwatch, XfceMailwatchMailboxType *type)
{
    XfceMailwatchPOP3Mailbox *pmailbox = g_new0(XfceMailwatchPOP3Mailbox, 1);
    pmailbox->mailbox.type = type;
    pmailbox->mailwatch = mailwatch;
    pmailbox->timeout = XFCE_MAILWATCH_DEFAULT_TIMEOUT;
    pmailbox->use_standard_port = TRUE;
    pmailbox->config_mx = g_mutex_new();
    /* init the queue */
    pmailbox->aqueue = g_async_queue_new();
    /* and init the timeout */
    g_async_queue_push(pmailbox->aqueue, POP3_CMD_TIMEOUT);
    g_async_queue_push(pmailbox->aqueue,
            GUINT_TO_POINTER(XFCE_MAILWATCH_DEFAULT_TIMEOUT));
    /* create checker thread */
    pmailbox->th = g_thread_create(pop3_check_mail_th, pmailbox, TRUE, NULL);

    return (XfceMailwatchMailbox *)pmailbox;
}

static void
pop3_set_activated(XfceMailwatchMailbox *mailbox, gboolean activated)
{
    XfceMailwatchPOP3Mailbox *pmailbox = XFCE_MAILWATCH_POP3_MAILBOX(mailbox);

    g_async_queue_push(pmailbox->aqueue, activated ? POP3_CMD_START : POP3_CMD_PAUSE);
}

static void
pop3_force_update_cb(XfceMailwatchMailbox *mailbox)
{
    XfceMailwatchPOP3Mailbox *pmailbox = XFCE_MAILWATCH_POP3_MAILBOX(mailbox);

    g_async_queue_push(pmailbox->aqueue, POP3_CMD_UPDATE);
}

static gboolean
pop3_host_entry_focus_out_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;
    gchar *str;

    str = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);

    g_mutex_lock(pmailbox->config_mx);

    g_free(pmailbox->host);
    if(!str || !*str) {
        pmailbox->host = NULL;
        g_free(str);
    } else
        pmailbox->host = str;

    g_mutex_unlock(pmailbox->config_mx);

    return FALSE;
}

static gboolean
pop3_username_entry_focus_out_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;
    gchar *str;

    str = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);

    g_mutex_lock(pmailbox->config_mx);

    g_free(pmailbox->username);
    if(!str || !*str) {
        pmailbox->username = NULL;
        g_free(str);
    } else
        pmailbox->username = str;

    g_mutex_unlock(pmailbox->config_mx);

    return FALSE;
}

static gboolean
pop3_password_entry_focus_out_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;
    gchar *str;

    str = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);

    g_mutex_lock(pmailbox->config_mx);

    g_free(pmailbox->password);
    if(!str || !*str) {
        pmailbox->password = NULL;
        g_free(str);
    } else
        pmailbox->password = str;

    g_mutex_unlock(pmailbox->config_mx);

    return FALSE;
}

static gboolean
pop3_config_timeout_spinbutton_changed_cb(GtkSpinButton *sb, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;
    gint value = gtk_spin_button_get_value_as_int(sb) * 60;

    pmailbox->timeout = value;
    g_async_queue_push(pmailbox->aqueue, POP3_CMD_TIMEOUT);
    g_async_queue_push(pmailbox->aqueue, GUINT_TO_POINTER(value));

    return FALSE;
}

static void
pop3_config_nonstandard_chk_cb(GtkToggleButton *tb, gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;
    GtkWidget *entry = g_object_get_data(G_OBJECT(tb), "xfmw-entry");

    g_mutex_lock(pmailbox->config_mx);

    pmailbox->use_standard_port = !gtk_toggle_button_get_active(tb);
    gtk_widget_set_sensitive(entry, !pmailbox->use_standard_port);

    g_mutex_unlock(pmailbox->config_mx);
}

static gboolean
pop3_config_nonstandard_focusout_cb(GtkWidget *w, GdkEventFocus *evt,
        gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;

    g_mutex_lock(pmailbox->config_mx);

    pmailbox->nonstandard_port = atoi(gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1));

    g_mutex_unlock(pmailbox->config_mx);

    return FALSE;
}

static void
pop3_config_security_combo_changed_cb(GtkWidget *w, gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;
    GtkWidget *entry = g_object_get_data(G_OBJECT(w), "xfmw-entry");

    g_mutex_lock(pmailbox->config_mx);

    pmailbox->auth_type = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

    if(pmailbox->use_standard_port) {
        if(pmailbox->auth_type == AUTH_SSL_PORT)
            gtk_entry_set_text(GTK_ENTRY(entry), POP3S_PORT_S);
        else
            gtk_entry_set_text(GTK_ENTRY(entry), POP3_PORT_S);
    }

    g_mutex_unlock(pmailbox->config_mx);
}

static void
pop3_config_advanced_btn_clicked_cb(GtkWidget *w, gpointer user_data)
{
    XfceMailwatchPOP3Mailbox *pmailbox = user_data;
    GtkWidget *dlg, *topvbox, *vbox, *hbox, *entry, *frame, *frame_bin, *chk,
              *combo;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    dlg = gtk_dialog_new_with_buttons(_("Advanced POP3 Options"),
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
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _("Use SSL/TLS via STLS"));
#ifdef HAVE_SSL_SUPPORT
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), pmailbox->auth_type);
#else
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_widget_set_sensitive(combo, FALSE);
#endif
    gtk_widget_show(combo);
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(combo), "changed",
            G_CALLBACK(pop3_config_security_combo_changed_cb), pmailbox);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    chk = gtk_check_button_new_with_mnemonic(_("Use non-standard POP3 _port:"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk),
            !pmailbox->use_standard_port);
    gtk_widget_show(chk);
    gtk_box_pack_start(GTK_BOX(hbox), chk, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(chk), "toggled",
            G_CALLBACK(pop3_config_nonstandard_chk_cb), pmailbox);

    entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 5);
    if(!pmailbox->use_standard_port) {
        gchar portstr[16];
        g_snprintf(portstr, 16, "%d", pmailbox->nonstandard_port);
        gtk_entry_set_text(GTK_ENTRY(entry), portstr);
    } else {
        gtk_widget_set_sensitive(entry, FALSE);
        if(pmailbox->auth_type == AUTH_SSL_PORT)
            gtk_entry_set_text(GTK_ENTRY(entry), POP3S_PORT_S);
        else
            gtk_entry_set_text(GTK_ENTRY(entry), POP3_PORT_S);
    }
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(pop3_config_nonstandard_focusout_cb), pmailbox);

    g_object_set_data(G_OBJECT(chk), "xfmw-entry", entry);
    g_object_set_data(G_OBJECT(combo), "xfmw-entry", entry);

    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

static GtkContainer *
pop3_get_setup_page(XfceMailwatchMailbox *mailbox)
{
    XfceMailwatchPOP3Mailbox *pmailbox = XFCE_MAILWATCH_POP3_MAILBOX(mailbox);
    GtkWidget *topvbox, *vbox, *hbox, *frame, *frame_bin, *lbl, *entry, *btn,
              *sbtn;
    GtkSizeGroup *sg;

    /* _TOFE_
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    */

    topvbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_widget_show(topvbox);

    frame = xfce_mailwatch_create_framebox(_("POP3 Server"), &frame_bin);
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
    if(pmailbox->host)
        gtk_entry_set_text(GTK_ENTRY(entry), pmailbox->host);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(pop3_host_entry_focus_out_cb), pmailbox);
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
    if(pmailbox->username)
        gtk_entry_set_text(GTK_ENTRY(entry), pmailbox->username);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(pop3_username_entry_focus_out_cb), pmailbox);
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
    if(pmailbox->password)
        gtk_entry_set_text(GTK_ENTRY(entry), pmailbox->password);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
            G_CALLBACK(pop3_password_entry_focus_out_cb), pmailbox);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), entry);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(topvbox), hbox, FALSE, FALSE, 0);

    btn = xfce_mailwatch_custom_button_new(_("_Advanced..."),
            GTK_STOCK_PREFERENCES);
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(btn), "clicked",
            G_CALLBACK(pop3_config_advanced_btn_clicked_cb), pmailbox);

    hbox = gtk_hbox_new(FALSE, BORDER/2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(topvbox), hbox, FALSE, FALSE, 0);

    lbl = gtk_label_new_with_mnemonic(_("Check for _new messages every"));
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);

    sbtn = gtk_spin_button_new_with_range(1.0, 1440.0, 1.0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(sbtn), TRUE);
    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(sbtn), FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sbtn), pmailbox->timeout/60);
    gtk_widget_show(sbtn);
    gtk_box_pack_start(GTK_BOX(hbox), sbtn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(sbtn), "focus-out-event",
            G_CALLBACK(pop3_config_timeout_spinbutton_changed_cb), pmailbox);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), sbtn);

    lbl = gtk_label_new(_("minute(s)."));
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);

    return GTK_CONTAINER(topvbox);
}

static void
pop3_restore_param_list(XfceMailwatchMailbox *mailbox, GList *params)
{
    XfceMailwatchPOP3Mailbox *pmailbox = XFCE_MAILWATCH_POP3_MAILBOX(mailbox);
    GList *l;

    g_mutex_lock(pmailbox->config_mx);

    for(l = params; l; l = l->next) {
        XfceMailwatchParam *param = l->data;

        if(!strcmp(param->key, "host"))
            pmailbox->host = g_strdup(param->value);
        else if(!strcmp(param->key, "username"))
            pmailbox->username = g_strdup(param->value);
        else if(!strcmp(param->key, "password"))
            pmailbox->password = g_strdup(param->value);
        else if(!strcmp(param->key, "auth_type"))
            pmailbox->auth_type = atoi(param->value);
        else if(!strcmp(param->key, "use_standard_port"))
            pmailbox->use_standard_port = *(param->value) == '0' ? FALSE : TRUE;
        else if(!strcmp(param->key, "nonstandard_port"))
            pmailbox->nonstandard_port = atoi(param->value);
        else if(!strcmp(param->key, "timeout")) {
            pmailbox->timeout = atoi(param->value);
            g_async_queue_push(pmailbox->aqueue, POP3_CMD_TIMEOUT);
            g_async_queue_push(pmailbox->aqueue,
                    GUINT_TO_POINTER(pmailbox->timeout));
        }
    }

    g_mutex_unlock(pmailbox->config_mx);
}

static GList *
pop3_save_param_list(XfceMailwatchMailbox *mailbox)
{
    XfceMailwatchPOP3Mailbox *pmailbox = XFCE_MAILWATCH_POP3_MAILBOX(mailbox);
    GList *params = NULL;
    XfceMailwatchParam *param;

    g_mutex_lock(pmailbox->config_mx);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("host");
    param->value = g_strdup(pmailbox->host);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("username");
    param->value = g_strdup(pmailbox->username);
    params = g_list_prepend(params, param);

    /* FIXME: probably would be nice to obscure this somewhat to deter casual
     * accidental exposure */
    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("password");
    param->value = g_strdup(pmailbox->password);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("auth_type");
    param->value = g_strdup_printf("%d", pmailbox->auth_type);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("use_standard_port");
    param->value = g_strdup(pmailbox->use_standard_port ? "1" : "0");
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("nonstandard_port");
    param->value = g_strdup_printf("%d", pmailbox->nonstandard_port);
    params = g_list_prepend(params, param);

    param = g_new(XfceMailwatchParam, 1);
    param->key = g_strdup("timeout");
    param->value = g_strdup_printf("%d", pmailbox->timeout);
    params = g_list_prepend(params, param);

    g_mutex_unlock(pmailbox->config_mx);

    return g_list_reverse(params);
}

static void
pop3_mailbox_free(XfceMailwatchMailbox *mailbox)
{
    XfceMailwatchPOP3Mailbox *pmailbox = XFCE_MAILWATCH_POP3_MAILBOX(mailbox);

    g_async_queue_push(pmailbox->aqueue, POP3_CMD_QUIT);
    g_thread_join(pmailbox->th);
    g_async_queue_unref(pmailbox->aqueue);

    g_mutex_free(pmailbox->config_mx);

    g_free(pmailbox->host);
    g_free(pmailbox->username);
    g_free(pmailbox->password);

    g_free(pmailbox);
}

XfceMailwatchMailboxType builtin_mailbox_type_pop3 = {
    "pop3",
    N_("Remote POP3 Mailbox"),
    N_("The POP3 plugin can connect to a remote mail server that supports the POP3 protocol, optionally using SSL for link protection."),

    pop3_mailbox_new,
    pop3_set_activated,
    pop3_force_update_cb,
    pop3_get_setup_page,
    pop3_restore_param_list,
    pop3_save_param_list,
    pop3_mailbox_free
};
