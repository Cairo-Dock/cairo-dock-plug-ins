
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

/**Le concept de ce plug-in est le suivant : 
 * - l'utilisateur l'active par la combinaison de touches pre-definie :
 *   => mode navigation : les fleches deplacent l'icone courante
 *      une lettre deplace sur la 1ere icone correspondante au mot courant
 *      TAB deplace sur l'icone suivante corespondante au mot courant
 *      la completion est automatique
 *      si aucune icone n'est trouvee, la completion elargie est utilisee.
 *   => mode recherche : les fleches deplacent parmi la liste d'icones correspondantes
 *      une lettre reduit/elargit la liste courante
 *      les icones de la liste sont affichees dans le dock principal, zoomees si necessaire
 *      completion: idem
 * - l'utilisateur appuie sur une lettre en ayant la souris dans un dock ayant le focus :
 *   => le mode navigation est active avec cette 1ere lettre
 *      seule le dock courant est pris en compte
 *      en cas de sortie du dock, la session est terminee
 *      la completion est automatique sur les icones trouvees seulement.
 * */

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gdouble fFontSizeRatio;  // 1 <=> taille du dock
	gboolean bTextOnTop;
	CairoDockLabelDescription labelDescription;
	gchar *cShortkeyNav;
	gchar *cShortkeySearch;
	gchar *cIconAnimation;
	gdouble pFrameColor[4];
	gint iAnimationDuration;
	gint iAppearanceDuration;
	gint iCloseDuration;
	gchar **pDirList;
	gint iNbResultMax;
	CairoDockLabelDescription infoDescription;
	} ;


typedef struct _CDChar {
	gchar c;
	cairo_surface_t *pSurface;
	GLuint iTexture;
	gint iWidth, iHeight;
	gint iAnimationTime;
	gint iInitialX, iInitialY;
	gint iFinalX, iFinalY;
	gint iCurrentX, iCurrentY;
	gdouble fRotationAngle;
	} CDChar;

typedef struct _CDAppli {
	gchar *cDesktopFilePath;
	gchar *cCommand;
	gchar *cIcon;
	gchar *cWorkingDirectory;
	} CDAppli;

typedef enum _CDFileType {
	DO_TYPE_NONE	=0,
	DO_TYPE_MUSIC	=1<<0,
	DO_TYPE_IMAGE	=1<<1,
	DO_TYPE_VIDEO	=1<<2,
	DO_TYPE_TEXT	=1<<3,
	DO_TYPE_HTML	=1<<4,
	DO_TYPE_SOURCE	=1<<5
	} CDFileType;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GString *sCurrentText;
	gint iStopCount;
	gint iNbValidCaracters;
	Window iPreviouslyActiveWindow;
	Icon *pCurrentIcon;
	CairoDock *pCurrentDock;
	gint iTextWidth, iTextHeight;
	gint iCloseTime;
	GList *pCharList;
	gboolean bIgnoreIconState;
	gint iAppearanceTime;
	
	gint iPrevMouseX, iPrevMouseY;
	gint iMouseX, iMouseY;
	gint iMotionCount;
	
	gboolean bNavigationMode;
	gboolean bSessionStartedAutomatically;
	gint iPromptAnimationCount;
	cairo_surface_t *pPromptSurface;
	gint iPromptWidth, iPromptHeight;
	GLuint iPromptTexture;
	cairo_surface_t *pArrowSurface;
	gint iArrowWidth, iArrowHeight;
	GLuint iArrowTexture;
	
	GList *pMatchingIcons;
	GList *pCurrentMatchingElement;
	
	GList *pApplications;
	GList *pMonitorList;
	GList *pCurrentApplicationToLoad;
	guint iSidLoadExternAppliIdle;
	
	gint iLocateAvailable;
	gint iCurrentFilter;
	gboolean bMatchCase;
	GtkWidget *pFileMenu;
	gint iNbMatchingFiles;
	CairoDialog *pFilterDialog;
	CairoDockTask *pLocateTask;
	// shared memory
	gchar **pMatchingFiles;
	gchar *cCurrentLocateText;  // lecture seule dans le thread
	gint iLocateFilter;  // lecture seule dans le thread
	gboolean bLocateMatchCase;  // lecture seule dans le thread
	// end of shared memory
	
	cairo_surface_t *pInfoSurface;
	GLuint iInfoTexture;
	gint iInfoWidth, iInfoHeight;
	
	/// not used.
	GList *pCompletionItemSurface;
	GList *pCompletionItemTexture;
	} ;


#endif
