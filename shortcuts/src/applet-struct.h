
#ifndef __APPLET_STRUCT__
#define  __APPLET_STRUCT__


typedef struct {
	gboolean bListDrives;
	gboolean bListNetwork;
	gboolean bListBookmarks;
	gboolean bUseSeparator;
	gchar *cRenderer;
	} AppletConfig;


typedef struct {
	gint no_data;
	} AppletData;


#endif
