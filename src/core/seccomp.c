#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <seccomp.h>
#include <errno.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "../include/seccomp.h"

#define SCMP_FAIL SCMP_ACT_ERRNO(EPERM)

int seccomp()
{
	scmp_filter_ctx ctx = NULL;
	
	if (!(ctx = seccomp_init(SCMP_ACT_ALLOW))
		|| seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(chmod), 1,
				SCMP_A1(SCMP_CMP_MASKED_EQ, S_ISUID, S_ISUID))
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(chmod), 1,
				SCMP_A1(SCMP_CMP_MASKED_EQ, S_ISGID, S_ISGID))
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(fchmod), 1,
				SCMP_A1(SCMP_CMP_MASKED_EQ, S_ISUID, S_ISUID))
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(fchmod), 1,
				SCMP_A1(SCMP_CMP_MASKED_EQ, S_ISGID, S_ISGID))
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(fchmodat), 1,
				SCMP_A2(SCMP_CMP_MASKED_EQ, S_ISUID, S_ISUID))
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(fchmodat), 1,
				SCMP_A2(SCMP_CMP_MASKED_EQ, S_ISGID, S_ISGID))
		|| seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(unshare), 1,
				SCMP_A0(SCMP_CMP_MASKED_EQ, CLONE_NEWUSER, CLONE_NEWUSER))
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(clone), 1,
				SCMP_A0(SCMP_CMP_MASKED_EQ, CLONE_NEWUSER, CLONE_NEWUSER))
		|| seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(keyctl), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(add_key), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(request_key), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(ptrace), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(mbind), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(migrate_pages), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(move_pages), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(set_mempolicy), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(userfaultfd), 0)
	    || seccomp_rule_add(ctx, SCMP_FAIL, SCMP_SYS(perf_event_open), 0)
	    || seccomp_attr_set(ctx, SCMP_FLTATR_CTL_NNP, 0)
	    || seccomp_load(ctx)) {
	    	if (ctx) seccomp_release(ctx);
			return 1;
	}
	seccomp_release(ctx);
	
	return 0;
}
