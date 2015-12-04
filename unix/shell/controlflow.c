#include <stdio.h>
#include "smsh.h"

enum status {NEUTRAL, WANT_THEN, THEN_BLOCK};
enum results {SUCCESS, FAIL};

static int if_state = NEUTRAL;
static int if_result = SUCCESS;
static int last_stat = 0;

int syn_err(char *);

int ok_to_execute() {
	int rv = 1;
	
	if (if_state == WANT_THEN) {
		syn_err("then expected");
		rv = 0;
	} else if (if_state == THEN_BLOCK && if_result == SUCCESS) {
		rv = 1;
	} else if (if_state == THEN_BLOCK && if_result == FAIL) {
		rv = 0;
	}
	return rv;
}

int is_control_command(char *s) {
	return (strcmp(s, "if") == 0 || strcmp(s, "then") == 0 || strcmp(s, "fi") == 0);
}

int do_control_command(char **args) {
	char *cmd = args[0];
	int rv = -1;

	if (strcmp(cmd, "if") == 0) {
		if (if_state != NEUTRAL)
			rv = syn_err("if unexpected");
		else {
			last_stat = process(args + 1);
			if_result = (last_stat == 0 ? SUCCESS : FAIL);
			if_state = WANT_THEN;
			rv = 0;
		}
	} else if (strcmp(cmd, "then") == 0) {
		if (if_state != WANT_THEN)
			rv = syn_err("then unexpected");
		else {
			if_state = THEN_BLOCK;
			rv = 0;
		}
	} else if (strcmp(cmd, "fi") == 0) {
		if (if_state != THEN_BLOCK)
			rv = syn_err("fi unexpected");
		else {
			if_state = NEUTRAL;
			rv = 0;
		}
	} else
		fatal("internal error processing:", cmd, 2);
	return rv;
}

int syn_err(char *msg) {
	if_state = NEUTRAL;
	fprintf(stderr, "syntax error: %s\n", msg);
	return -1;
}
