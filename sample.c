#include <stdio.h>
#include <stddef.h>
#include "cmd_front_arch.h"


void cmd1_handler(struct parsed_params* params)
{
	printf("cmd1_handler \n");
	int i = 0;
	for(;;) {
		struct cmd_param_define *param_name_cfg = params->param_cfgs[i];
		if (param_name_cfg == NULL) {
			break;
		}
		printf(" %s = %s \n", param_name_cfg->param_name, params->param_values[i]);
		i++;
	}

	printf("p: %s \n", get_param_val("cmd1p1", params));
	printf("p: %s \n", get_param_val("cmd1p2", params));
	printf("p: %s \n", get_param_val("cmd1p3", params));
}

void cmd1p2_handler(struct parsed_params* params)
{
	printf("cmd1p2_handler \n");

	printf("p: %s \n", get_param_val("cmd1p1", params));
	printf("p: %s \n", get_param_val("cmd1p2", params));
	printf("p: %s \n", get_param_val("cmd1p3", params));
}

struct cmd_param_define cmd1_p1 = CMD_PARAM_DEFINE("cmd1p1", "-p1", TRUE, TRUE, string, NULL);
struct cmd_param_define cmd1_p2 = CMD_PARAM_DEFINE("cmd1p2", "-p2", TRUE, TRUE, string, cmd1p2_handler);

struct cmd_define cmd1 = {
	.cmd_name = "cmd1",
	.help_str = "cmd1 help str.",
	.cmd_handler = cmd1_handler,
	.param_defines = {
		&cmd1_p1,
		&cmd1_p2,
		NULL,
	},
};

void quit_handler(struct parsed_params* params)
{
	printf("bye. \n");
	exit(0);
}

struct cmd_define cmd_quit = {
	.cmd_name = "quit",
	.help_str = "quit cmd line.",
	.cmd_handler = quit_handler
};

struct cmd_define *cmd_defines[] = {
	&cmd1,
	&cmd_quit,
	NULL,
};

int main(int argc, char **argv)
{
	run_cmdline("cmd", cmd_defines);
	return 0;
}
