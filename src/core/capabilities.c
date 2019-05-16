#include <stdlib.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <linux/capability.h>

#include "../include/capabilities.h"

int capabilities()
{
	int drop_caps[] = {
		CAP_AUDIT_CONTROL, CAP_AUDIT_WRITE,
		CAP_BLOCK_SUSPEND, CAP_DAC_READ_SEARCH, CAP_FSETID,
		CAP_IPC_LOCK, CAP_MAC_ADMIN, CAP_MAC_OVERRIDE,
		CAP_MKNOD, CAP_SETFCAP, CAP_SYSLOG,
		CAP_SYS_ADMIN, CAP_SYS_BOOT, CAP_SYS_MODULE,
		CAP_SYS_NICE, CAP_SYS_RAWIO, CAP_SYS_RESOURCE,
		CAP_SYS_TIME, CAP_WAKE_ALARM, CAP_NET_ADMIN
	};
	size_t num_caps = sizeof(drop_caps) / sizeof(*drop_caps);
	cap_t caps = NULL;
	size_t i;
	
	//setting bounding capabilities
	for (i = 0; i < num_caps; i++) {
		if (prctl(PR_CAPBSET_DROP, drop_caps[i], 0, 0, 0)) {
			fprintf(stderr, "prctl failed: %m\n");
			return 1;
		}
	}
	//setting inheritable capabilities
	if (!(caps = cap_get_proc())
	    || cap_set_flag(caps, CAP_INHERITABLE, num_caps, drop_caps, CAP_CLEAR)
	    || cap_set_proc(caps)) {
		fprintf(stderr, "failed: %m\n");
		if (caps) cap_free(caps);
		return 1;
	}
	cap_free(caps);
	
	return 0;
}
