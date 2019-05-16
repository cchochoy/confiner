#ifndef INCLUDE_MOUNT_H
#define INCLUDE_MOUNT_H

int pivot_root(char *new_root, char *old_root);
int config_mount(char* config_mount_dir);

#endif
