#include "cmd_front_arch.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 缓存命令行定义的全局数组，注意此处不能声明为数组，只能使用双重指针，如果声明为数组，编译器无法确定数组长度。
struct cmd_define **_cmd_defines;

void print_cmd_config(struct cmd_define *cmd_defines[])
{
	int i=0;
	for(;;) {
		const struct cmd_define *cmd_def = cmd_defines[i++];
		if (cmd_def == NULL) {
			break;
		}
		printf("cmd_name: %s %s\n", cmd_def->cmd_name, cmd_def->help_str);
		print_cmd_params_cfg(cmd_def);
	}
}

void print_cmd_params_cfg(const struct cmd_define *cmd_def)
{
	int j = 0;
	for(;;) {
		const struct cmd_param_define *param_def = cmd_def->param_defines[j++];
		if (param_def == NULL) {
			break;
		}
		printf("param_def: %s %s\n", param_def->param_name, param_def->param_short_name);
	}
}

/**
 * 将输入命令行按照空格拆分
 */
static int split_cmd_items(char *cmd_items[], char* cmd_line)
{
	int cmd_item_num = 0;
	const char *space = " ";
	char *cmd_item = strtok(cmd_line, space);
	while (cmd_item != NULL) {
		cmd_items[cmd_item_num++] = cmd_item;
		cmd_item = strtok(NULL, space);
	}
	// 添加哨兵，防止后续处理越界
	cmd_items[cmd_item_num] = NULL;
	return cmd_item_num;
}

/**
 * 查找输入命令行对应的命令配置
 */
static const struct cmd_define * find_cmd_define(const char *cmd_name)
{
	int i = 0;
	const struct cmd_define *cmd_def;
	for (;;) {
		cmd_def = _cmd_defines[i++];
		if (cmd_def == NULL) {
			break;
		}
		if (strcmp(cmd_def->cmd_name, cmd_name) == 0) {
			return cmd_def;
		}
	}
	return NULL;
}

/**
 * 查找当前命令指定参数对应的配置
 */
static struct cmd_param_define * find_param_define(char *param_name, const struct cmd_define *cmd_def)
{
	int i = 0;
	struct cmd_param_define *param_def;
	for (;;) {
		param_def = cmd_def->param_defines[i++];
		if (param_def == NULL || i > 10) {
			break;
		}
		if (strcmp(param_def->param_name, param_name) == 0) {
			return param_def;
		}
	}
	return NULL;
}

/**
 * 解析命令行参数
 */
static struct parsed_params * parse_cmd_params(char *cmd_items[], const struct cmd_define *cmd_def)
{
	int i = 0, param_num = 0;
	struct parsed_params *cmd_params = malloc(sizeof(struct parsed_params));
	char *cmd_item;
	for (;;) {
		cmd_item = cmd_items[i++];
		if (cmd_item == NULL) {
			break;
		}
		struct cmd_param_define *param_def = find_param_define(cmd_item, cmd_def);
		if (param_def == NULL) {
			continue;
		}
		cmd_params->param_cfgs[param_num] = param_def;
		// 查找解析出来的参数的对应值，默认为紧跟的下一个字符(注意i已经在上面被++过)
		if (i <= MAX_PARAM_NUM-1) {
			cmd_params->param_values[param_num] = cmd_items[i];
		} else {
			cmd_params->param_values[param_num] = NULL;
		}
		param_num++;
	}
	// 加入哨兵，防止后续处理越界
	cmd_params->param_cfgs[param_num] = NULL;
	return cmd_params;
}

static cmd_process_handler find_cmd_handler(const struct cmd_define *cmd_def, struct parsed_params *cmd_params)
{
	cmd_process_handler param_spec_handler = (cmd_process_handler)NULL;
	int i = 0;
	for(;;) {
		struct cmd_param_define *param_name_cfg = cmd_params->param_cfgs[i++];
		if (param_name_cfg == NULL) {
			break;
		}
		if (param_spec_handler == NULL && param_name_cfg->param_handler != NULL) {
			param_spec_handler = param_name_cfg->param_handler;
		}
	}
	// 如果为某一参数指定了特定的处理函数，则优先使用，否则使用当前命令默认处理函数
	if (param_spec_handler != NULL) {
		printf("use param handler \n");
		return param_spec_handler;
	} else {
		printf("use cmd handler \n");
		return cmd_def->cmd_handler;
	}
}

static enum cmd_exec_result {
	effective, not_effective, empty_cmd
};

/**
 * 解析命令行
 */
static enum cmd_exec_result parse_run_cmdline(const char *cmd_line)
{
	// 由于strtok函数会改变原字符串，通过一个副本来避免cmd_line的内容被改变
	char cmd_line_cp[strlen(cmd_line)];
	strcpy(cmd_line_cp, cmd_line);
	char *cmd_items[MAX_PARAM_NUM];
	int cmd_item_num = split_cmd_items(cmd_items, cmd_line_cp);
	if (cmd_item_num == 0) {
		return empty_cmd;
	}
	int i = 0;
	const struct cmd_define *cmd_def = find_cmd_define(cmd_items[0]);
	if (cmd_def == NULL) {
		printf("error: [%s] is not a effective command. \n", cmd_items[0]);
		return not_effective;
	}
	// TODO：判断值是否是参数关键字，是否必须，支持短名称
	struct parsed_params *cmd_params = parse_cmd_params(cmd_items, cmd_def);
	cmd_process_handler cmd_handler = find_cmd_handler(cmd_def, cmd_params);
	cmd_handler(cmd_params);
	//释放解析后参数所占用的内存
	free(cmd_params);
	return effective;
}

//static char * command_generator(char *text, int state)
//{
//	printf("command_generator: %s %d \n", text, state);
//
//	static int list_index, len;
//	char *name;
//	/* If this is a new word to complete, initialize now.  This includes
//	 saving the length of TEXT for efficiency, and initializing the index
//	 variable to 0. */
//	if (!state) {
//		list_index = 0;
//		len = strlen(text);
//	}
//
//	list_index++;
//	if (list_index<5) {
//		return "aaaaa";
//	} else {
//		list_index = 0;
//		return ((char *) NULL);
//	}
//
////	/* Return the next name which partially matches from the command list. */
////	while (name = commands[list_index].name) {
////		list_index++;
////		if (strncmp(name, text, len) == 0)
////			return (dupstr(name));
////	}
////
////	/* If no names matched, then return NULL. */
////	return ((char *) NULL);
//}
//
//static char ** fileman_completion(const char* text, int start, int end) {
//    char **matches = (char **) NULL;
//    printf("fileman_completion: %s %d ~ %d \n", text, start, end);
//    /* If this word is at the start of the line, then it is a command
//     to complete.  Otherwise it is the name of a file in the current
//     directory. */
//    if (start == 0)
//        matches = completion_matches(text, command_generator);
//    /* matches = rl_completion_matches (text, command_generator); */
//
//    return (matches);
//}

/**
 * 启动命令行界面
 */
void run_cmdline(const char *prompt, struct cmd_define *cmd_defines[])
{
	_cmd_defines = cmd_defines;
	print_cmd_config(_cmd_defines);

	int i = 0;
	char prompt_fmt[32];
	strcpy(prompt_fmt, prompt);
	strcat(prompt_fmt, "-%d> ");
	// 设定Tab键命令提示
	// rl_attempted_completion_function = fileman_completion;
	// 设定历史记录保留条数
	stifle_history(50);
	char cmd_prompt[32];
	char *cmd_line;
	for (;;) {
		sprintf(cmd_prompt, prompt_fmt, ++i);
		cmd_line = readline(cmd_prompt);
		if (!cmd_line) {
			continue;
		}
		enum cmd_exec_result res = parse_run_cmdline(cmd_line);
		if (res != empty_cmd) {
			// 保留命令履历
			add_history(cmd_line);
		}
	}
}

const char *get_param_val(const char *param_name, struct parsed_params* params)
{
	int i = 0;
	for(;;) {
		struct cmd_param_define *param_name_cfg = params->param_cfgs[i];
		if (param_name_cfg == NULL) {
			break;
		}
		if (strcmp(param_name_cfg->param_name, param_name) == 0) {
			return params->param_values[i];
		}
		i++;
	}
	return NULL;
}

int get_param_int_val(const char *param_name, struct parsed_params* params)
{
	return 100;
}
