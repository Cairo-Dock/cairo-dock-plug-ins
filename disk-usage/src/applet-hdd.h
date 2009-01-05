
#ifndef __HDD_FS__
#define  __HDD_FS__

#include <mntent.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <cairo-dock.h>

void cd_hdd_read_data(CairoDockModuleInstance *myApplet);

gboolean cd_hdd_update_from_data (CairoDockModuleInstance *myApplet);

gchar *cd_human_readable(long long num);

void cd_mount_unmount_device (CairoDockModuleInstance *myApplet);

#endif __HDD_FS__
