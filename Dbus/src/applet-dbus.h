
#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>
#include "applet-struct.h"

void cd_dbus_launch_service (void);
void cd_dbus_stop_service (void);


gboolean cd_dbus_callback_show_desklet(dbusCallback *pDbusCallback, gboolean *widgetLayer, GError **error);

gboolean cd_dbus_callback_reboot(dbusCallback *pDbusCallback, GError **error);

gboolean cd_dbus_callback_quit (dbusCallback *pDbusCallback, GError **error);

gboolean cd_dbus_callback_show_dock (dbusCallback *pDbusCallback, gboolean bShow, GError **error);

gboolean cd_dbus_callback_reload_module (dbusCallback *pDbusCallback, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_load_launcher_from_file (dbusCallback *pDbusCallback, const gchar *cDesktopFile, GError **error);

gboolean cd_dbus_callback_create_launcher_from_scratch (dbusCallback *pDbusCallback, const gchar *cIconFile, const gchar *cLabel, const gchar *cCommand, const gchar *cParentDockName, GError **error);

gboolean cd_dbus_callback_set_quick_info (dbusCallback *pDbusCallback, const gchar *cQuickInfo, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_set_label (dbusCallback *pDbusCallback, const gchar *cLabel, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_set_icon (dbusCallback *pDbusCallback, const gchar *cImage, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_animate (dbusCallback *pDbusCallback, const gchar *cAnimation, gint iNbRounds, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_show_dialog (dbusCallback *pDbusCallback, const gchar *message, gint iDuration, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);


gboolean cd_dbus_callback_register_new_module (dbusCallback *pDbusCallback, const gchar *cModuleName, gint iCategory, const gchar *cDescription, const gchar *cShareDataDir, GError **error);

gboolean cd_dbus_callback_populate_menu (dbusCallback *pDbusCallback, const gchar *cModuleName, const gchar **pLabels, GError **error);


gboolean cd_dbus_emit_on_click_icon (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer, guint iButtonState);

gboolean cd_dbus_emit_on_middle_click_icon (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer);

gboolean cd_dbus_emit_on_scroll_icon (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer, int iDirection);

gboolean cd_dbus_emit_on_build_menu (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer, GtkWidget *pAppletMenu);

gboolean cd_dbus_emit_on_drop_data (CairoDockModuleInstance *myApplet, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, CairoContainer *pClickedContainer);


#endif
