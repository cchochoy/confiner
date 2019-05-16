#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <wait.h>

#include "./include/config.h"
#include "./include/mount.h"
#include "./include/capabilities.h"
#include "./include/seccomp.h"

int child_exec(void* arg)
{
	struct confiner_conf* config = arg;
	if ((config->clone_flags & CLONE_NEWUTS)  && (sethostname(config->hostname, strlen(config->hostname)))) {
		fprintf(stderr, "hostname failed: %m\n");
		close(config->fd);
		return -1;
	}
	if ((config->clone_flags & CLONE_NEWNS) && (config_mount(config->mount_dir))) {
		close(config->fd);
		return -1;
	}
	if (capabilities()) {
		fprintf(stderr, "capabilities failed: %m\n");
		close(config->fd);
		return -1;
	}
	if (seccomp()) {
		fprintf(stderr, "seccomp failed: %m\n");
		close(config->fd);
		return -1;
	}
	if (close(config->fd)) {
		fprintf(stderr, "close failed: %m\n");
		return -1;
	}
	if (execvp(config->argv[0], config->argv)) {
		fprintf(stderr, "execvp failed! %m\n");
		return -1;
	}
	 
	return 0;
}

int main(int argc, char *argv[])  
{
	int option = 0;
	int err = 0;
	struct confiner_conf config = {0};
	int sockets[2] = {0};
	char *child_stack;
	pid_t pid = 0;
	
	//----------------------------- parse commandline args ---------------//
	if (argc < 2) goto usage;
	if (strcmp(argv[1], "run") == 0) {
		config.mode = RUN_MODE;
		config.mount_dir = argv[2];
	} 
	else if (strcmp(argv[1], "sandbox") == 0) {
		config.mode = SANDBOX_MODE;
	}
	else {
		fprintf(stderr, "Mode is either run or sandbox\n");
		goto usage;
	}
	
	while ((option = getopt(argc, argv, "mnuhipc:")) != -1) {
        switch (option) {
        	case 'c':
				config.argc = argc - optind + 1;
				config.argv = &argv[optind - 1];
				goto finish_options;
		    case 'm': config.clone_flags |= CLONE_NEWNS;   break;
		    case 'n': config.clone_flags |= CLONE_NEWNET;  break;
		    case 'u': fprintf(stderr, "user namespace not supported yet\n"); goto cleanup;
		    case 'h': config.clone_flags |= CLONE_NEWUTS;  break;
		    case 'i': config.clone_flags |= CLONE_NEWIPC;  break;
		    case 'p': config.clone_flags |= CLONE_NEWPID;  break;
		    default: goto usage;
        }
    }

finish_options:
	if (config.clone_flags == 0) {
		config.clone_flags = CLONE_NEWNS | CLONE_NEWNET |
			CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID;
	}
	if (config.argc == 0) {
		config.argc = 1;
		config.argv = &argv[argc - config.argc];
		config.argv[0] = "/bin/sh";
	}
	if (!(config.clone_flags & CLONE_NEWNS) && (config.mode == RUN_MODE)) {
		fprintf(stderr, "Can't launch run mode without -m\n");
		goto usage;
	}
	if (!config.mount_dir && (config.mode == RUN_MODE)) {
		fprintf(stderr, "Container mountpath must be provided in run mode\n");
		goto usage;
	}
	
	//-------------------------- setup container ------------------------------//
	config.hostname = "container";
	//create socket pair for container-host communication
	if (socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, sockets)) {
		fprintf(stderr, "socketpair failed: %m\n");
		goto error;
	}
	//close sockets[0] after the execvp
	if (fcntl(sockets[0], F_SETFD, FD_CLOEXEC)) {
		fprintf(stderr, "fcntl failed: %m\n");
		goto error;
	}
	config.fd = sockets[1];
	if (!(child_stack = malloc(STACK_SIZE))) {
		fprintf(stderr, "=> malloc failed, out of memory?\n");
		goto error;
	}
	/*if (resources(&config)) {
		err = 1;
		goto clear_resources;
	}*/
	
	//-------------------- launch container ----------------------------//
	pid = clone(child_exec, child_stack + STACK_SIZE, config.clone_flags | SIGCHLD, &config);
	if (pid < 0) {
		fprintf(stderr, "clone failed! %m\n");
		err = 1;
		goto clear_resources;
	}
	close(sockets[1]);
	sockets[1] = 0;
	waitpid(pid, NULL, 0);
	
	goto cleanup;

usage:
	fprintf(stderr, "Usage: %s [run MOUNTPATH | sandbox] [-mnuhipc] [-c CMD]\n", argv[0]);
error:
	err = 1;
clear_resources:
cleanup:
	if (sockets[0]) close(sockets[0]);
	if (sockets[1]) close(sockets[1]);
	
	return err;
}
