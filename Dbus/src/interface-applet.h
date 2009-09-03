
#ifndef __INTERFACE_APPLET__
#define  __INTERFACE_APPLET__

#include <cairo-dock.h>
#include "applet-struct.h"


void cd_dbus_create_remote_applet_object (CairoDockModuleInstance *pModuleInstance);

void cd_dbus_delete_remote_applet_object (CairoDockModuleInstance *pModuleInstance);

void cd_dbus_unregister_notifications (void);


void cd_dbus_emit_on_init_module (CairoDockModuleInstance *pModuleInstance, GKeyFile *pKeyFile);

void cd_dbus_emit_on_stop_module (CairoDockModuleInstance *pModuleInstance);

gboolean cd_dbus_emit_on_reload_module (CairoDockModuleInstance *pModuleInstance, CairoContainer *pOldContainer, GKeyFile *pKeyFile);


gboolean cd_dbus_applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, GError **error);

gboolean cd_dbus_applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, GError **error);

gboolean cd_dbus_applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, GError **error);

gboolean cd_dbus_applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, GError **error);

gboolean cd_dbus_applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, GError **error);

gboolean cd_dbus_applet_populate_menu (dbusApplet *pDbusApplet, const gchar **pLabels, GError **error);

gboolean cd_dbus_applet_add_sub_icons (dbusApplet *pDbusApplet, const gchar **pIconFields, GError **error);


gboolean cd_dbus_applet_emit_on_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, guint iButtonState);

gboolean cd_dbus_applet_emit_on_middle_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer);

gboolean cd_dbus_applet_emit_on_scroll_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, int iDirection);

gboolean cd_dbus_applet_emit_on_build_menu (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, GtkWidget *pAppletMenu);

gboolean cd_dbus_applet_emit_on_drop_data (gpointer data, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, CairoContainer *pClickedContainer);


#endif
