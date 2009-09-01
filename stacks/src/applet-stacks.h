
#ifndef __APPLET_STACKS__
#define  __APPLET_STACKS__


#include <cairo-dock.h>

void cd_stacks_check_local(void);
void cd_stacks_mklink(const gchar *cFile);
void cd_stacks_clean_local(void);
void cd_stacks_run_dir(void);
GList* cd_stacks_mime_filter(GList *pList);
void cd_stacks_remove_monitors (void);

#endif
