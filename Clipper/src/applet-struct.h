
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>
#include <cairo-dock-applet-single-instance.h>

typedef enum {
	CD_CLIPPER_NONE=0,
	CD_CLIPPER_CLIPBOARD,
	CD_CLIPPER_PRIMARY,
	CD_CLIPPER_BOTH
	} CDClipperItemType;

typedef struct _CDClipperCommand {
	gchar *cDescription;
	gchar *cFormat;
	gchar *cIconFileName;
	} CDClipperCommand;

typedef struct _CDClipperAction {
	gchar *cDescription;
	GRegex *pRegex;
	GList *pCommands;
	} CDClipperAction;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	CDClipperItemType iItemType;
	gint iNbItems;
	gboolean bPasteInClipboard;
	gboolean bPasteInPrimary;
	gboolean bEnableActions;
	gboolean bMenuOnMouse;
	gboolean bReplayAction;
	gint iActionMenuDuration;
	gchar *cShortCut;
	gchar **pPersistentItems;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gint iNbItems;
	GList *pItems;
	guint iSidClipboardOwnerChange;
	guint iSidPrimaryOwnerChange;
	GList *pActions;
	gboolean bActionsLoaded;
	gboolean bActionBlocked;
	GtkWidget *pActionMenu;
	} ;

#endif
