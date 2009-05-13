
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bEnableDialogs;
	gdouble dTimeDialogs;
	gboolean bEnableHistory;
	gboolean bEnableHistoryLimit;
	gint iNbItems;
	gint iUrlPicturesType;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gchar *cWorkingDirPath;
		
	gint iNumberOfStoredPic;
		
	gint iCurrentPictureNumber;
	gchar *cCurrentPicturePath;
	gchar *cCurrentConfigFile;
	gchar *cCurrentLogFile;
		
	gchar *cDisplayImage;
	gchar *cDirectLink;
	gchar *cBBCodeFullPic;
	gchar *cBBCode150px;
	gchar *cBBCode600px;
	
	gint iUrlTypeToCopy;
	} ;


#endif
