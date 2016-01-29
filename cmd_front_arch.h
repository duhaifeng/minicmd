#ifndef __CMD_FRONT_ARCH__
#define __CMD_FRONT_ARCH__

#define TRUE 1
#define FALSE 0
#define MAX_PARAM_NUM 10

#define CMD_PARAM_DEFINE(p_name, p_short_name, p_is_must, p_is_mark, p_type, p_handler) \
{											\
	.param_name = p_name,					\
	.param_short_name = p_short_name,		\
	.is_must = p_is_must,					\
	.is_mark_param = p_is_mark,				\
	.param_type = p_type,					\
	.param_handler = p_handler,				\
}

enum cmd_param_type
{
	string, number, ipv4, ipv6
};

struct cmd_param_define;
struct parsed_params;
typedef int (*cmd_process_handler)(struct parsed_params* params);

struct cmd_param_define
{
	char *param_name;
	char *param_short_name;
	int is_must;
	int is_mark_param;
	enum cmd_param_type param_type;
	cmd_process_handler param_handler;
};

struct cmd_define
{
	char *cmd_name;
	char *help_str;
	cmd_process_handler cmd_handler;
	struct cmd_param_define *param_defines[];
};

struct parsed_params
{
	struct cmd_param_define *param_cfgs[MAX_PARAM_NUM];
	char *param_values[MAX_PARAM_NUM];
};

struct parsed_cmd
{
	struct cmd_define *cmd_def;
	struct parsed_params *cmd_params;
};

void print_cmd_config();

void print_cmd_params_cfg(const struct cmd_define *cmd_def);

void run_cmdline(const char *prompt, struct cmd_define *cmd_defines[]);

const char *get_param_val(const char *param_name, struct parsed_params* params);

int get_param_int_val(const char *param_name, struct parsed_params* params);

#endif
