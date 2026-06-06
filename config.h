/**
 * @file    config.h
 * @brief   配置解析与默认值
 */

#define _CRT_SECURE_NO_WARNINGS
#ifndef CONFIG_H
#define CONFIG_H

#include "eframe.h"

void set_default_config(program_config_t* config);
int  parse_arguments(int argc, char* argv[], program_config_t* config);
void print_usage(const char* prog_name);

#endif /* CONFIG_H */