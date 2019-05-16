#include <unistd.h>

#define STACK_SIZE (1024 * 1024)

struct confiner_conf {
	enum { RUN_MODE, SANDBOX_MODE } mode;
	int clone_flags;
	char *mount_dir;
	int argc;
	char **argv;
	uid_t uid;
	int fd;
	char *hostname;
};
