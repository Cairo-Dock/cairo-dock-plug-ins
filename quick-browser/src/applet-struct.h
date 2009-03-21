
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *cPath;
	GtkWidget *pSubMenu;
	CairoDockModuleInstance *pApplet;
	gboolean bFilled;
	const gchar *cTmpFileName;
	} CDQuickBrowserItem;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bHasIcons;
	gboolean bFoldersFirst;
	gboolean bCaseUnsensitive;
	gboolean bShowHiddenFiles;
	gchar *cMenuShortkey;
	gchar *cDirPath;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gboolean bFoldersFirst;
	GtkWidget *pMenu;
	gint iSidFillDirIdle;
	} ;


#endif
