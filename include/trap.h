#ifndef __TRAP_H
#define __TRAP_H

typedef struct sign_s {
	int no;
	char *name;
} signal_struct;

void init_trap(char *progname);

#endif

