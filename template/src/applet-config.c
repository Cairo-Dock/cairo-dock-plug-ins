
#include <cairo-dock.h>

#include "applet-config.h"


CD_APPLET_CONFIG_BEGIN (CD_APPLET_LABEL, CD_APPLET_ICON)
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	
CD_APPLET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	//\_________________ On libere toutes les ressources allouees lors de l'init et qui sont liees a notre config ou a celle du dock.
	
CD_APPLET_RESET_DATA_END
