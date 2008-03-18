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

#ifndef __MAILWATCH_UTILS_H__
#define __MAILWATCH_UTILS_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <gtk/gtk.h>

#ifdef HAVE_SSL_SUPPORT
#include <gnutls/gnutls.h>
#endif

#define TRACE cd_message
#define DBG   cd_debug

G_BEGIN_DECLS

typedef enum
{
    AUTH_NONE = 0,
    AUTH_SSL_PORT,
    AUTH_STARTTLS
} XfceMailwatchAuthType;

typedef struct
{
    gboolean using_tls;
    gboolean gnutls_inited;
#ifdef HAVE_SSL_SUPPORT
    gnutls_session_t gt_session;
    gnutls_certificate_credentials_t gt_creds;
#endif
} XfceMailwatchSecurityInfo;

gboolean xfce_mailwatch_net_get_sockaddr(const gchar *host, const gchar *service, struct addrinfo *hints, struct sockaddr_in *addr, GError **error);
gboolean xfce_mailwatch_net_negotiate_tls(gint sockfd, XfceMailwatchSecurityInfo *security_info, const gchar *host, GError **error);
gssize xfce_mailwatch_net_send(gint sockfd, XfceMailwatchSecurityInfo *security_info, const gchar *buf, GError **error);
gssize xfce_mailwatch_net_recv(gint sockfd, XfceMailwatchSecurityInfo *security_info, gchar *buf, gsize len, GError **error);
void xfce_mailwatch_net_tls_teardown(XfceMailwatchSecurityInfo *security_info);

GtkWidget *xfce_mailwatch_custom_button_new(const gchar *text, const gchar *icon);
GtkWidget *xfce_mailwatch_create_framebox(const gchar *title, GtkWidget **frame_bin);

void xfce_textdomain (const gchar *package, const gchar *localedir, const gchar *encoding);

gint xfce_mailwatch_base64_encode(const guint8 *data, gint size, gchar **str);

G_END_DECLS

#endif
