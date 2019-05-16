#define _GNU_SOURCE
#include <sys/mount.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/mount.h"

//wrapper for pivot root syscall
int pivot_root(char *new_root, char *old_root) 
{
	return syscall(SYS_pivot_root, new_root, old_root);
}

int config_mount(char* config_mount_dir)
{
	char mount_dir[] = "/tmp/tmp.XXXXXX";
	char inner_mount_dir[] = "/tmp/tmp.XXXXXX/oldroot.XXXXXX";
	char old_root[sizeof(basename(inner_mount_dir) +1)] = {"/"};
	
	//make host filesystem private
	if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
		fprintf(stderr, "failed! %m\n");
		return -1;
	}
	//create temporary directory for new root
	if (!mkdtemp(mount_dir)) {
		fprintf(stderr, "failed making a directory!\n");
		return -1;
	}
	//bind config mount dir to created temp dir
	if (mount(config_mount_dir, mount_dir, NULL, MS_BIND | MS_PRIVATE, NULL)) {
		fprintf(stderr, "bind mount failed!\n");
		return -1;
	}
	//create temportaty directory for old root
	memcpy(inner_mount_dir, mount_dir, sizeof(mount_dir) - 1);
	if (!mkdtemp(inner_mount_dir)) {
		fprintf(stderr, "failed making the inner directory!\n");
		return -1;
	}
	//pivot root
	if (pivot_root(mount_dir, inner_mount_dir)) {
		fprintf(stderr, "pivot_root failed!\n");
		return -1;
	}
	//remove old root
	strcat(old_root, basename(inner_mount_dir));
	if (chdir("/")) {
		fprintf(stderr, "chdir failed! %m\n");
		return -1;
	}
	if (umount2(old_root, MNT_DETACH)) {
		fprintf(stderr, "umount failed! %m\n");
		return -1;
	}
	if (rmdir(old_root)) {
		fprintf(stderr, "rmdir failed! %m\n");
		return -1;
	}
	//mount new proc
	if (mount("proc", "/proc", "proc",0, NULL)) {
		fprintf(stderr, "mounting proc failed! %m\n");
		return -1;
	}
	if (mount("sysfs", "sys", "sysfs", 0, NULL)) {
		fprintf(stderr, "mounting sys failed! %m\n");
		return -1;
	}
	
	return 0;
}
