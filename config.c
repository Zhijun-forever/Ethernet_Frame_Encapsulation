/**
 * @file    config.c
 * @brief   配置解析实现
 */
#define _CRT_SECURE_NO_WARNINGS
#include "config.h"
#include "mac_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//设置默认配置
void set_default_config(program_config_t* config)
{
    strcpy(config->input_file, "");
    strcpy(config->output_file, DEFAULT_OUTPUT_FILE);
    config->eth_type = DEFAULT_ETH_TYPE;

    parse_mac_address(DEFAULT_DST_MAC, &config->dst_mac);
    parse_mac_address(DEFAULT_SRC_MAC, &config->src_mac);
}


//解析命令行参数
int parse_arguments(int argc, char* argv[], program_config_t* config)
{
    int i;

    //如果没有参数（argc=1，只有程序名），打印用法并返回 1（主函数识别为帮助，正常退出）
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;   /* 请求帮助，正常退出 */
    }

    
    for (i = 1; i < argc; i++) {
        //处理 -h 或 --help，打印帮助并返回 1
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 1;
        }
        //-o 或 --output：取下一个参数作为输出文件名。使用 strncpy 限制长度，防止溢出，并确保结尾 \0
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                strncpy(config->output_file, argv[++i], sizeof(config->output_file) - 1);
                config->output_file[sizeof(config->output_file) - 1] = '\0';
            }
            else {
                fprintf(stderr, "错误：选项 '%s' 需要指定一个文件路径。\n", argv[i - 1]);
                return -1;
            }
        }
        //--dst：解析 MAC 地址，失败返回 -1
        else if (strcmp(argv[i], "--dst") == 0) {
            if (i + 1 < argc) {
                if (parse_mac_address(argv[++i], &config->dst_mac) != 0)
                    return -1;
            }
            else {
                fprintf(stderr, "错误：选项 '--dst' 需要指定一个 MAC 地址。\n");
                return -1;
            }
        }
        //--dst：解析 MAC 地址，失败返回 -1
        else if (strcmp(argv[i], "--src") == 0) {
            if (i + 1 < argc) {
                if (parse_mac_address(argv[++i], &config->src_mac) != 0)
                    return -1;
            }
            else {
                fprintf(stderr, "错误：选项 '--src' 需要指定一个 MAC 地址。\n");
                return -1;
            }
        }
        //--type：将下一个参数作为十六进制字符串转换成整数，检查范围 0~65535，存入 eth_type
        else if (strcmp(argv[i], "--type") == 0) {
            if (i + 1 < argc) {
                char* endptr;
                long val = strtol(argv[++i], &endptr, 16);
                if (*endptr != '\0' || val < 0 || val > 0xFFFF) {
                    fprintf(stderr, "错误：'%s' 不是有效的十六进制类型字段（范围 0000~FFFF）。\n", argv[i]);
                    return -1;
                }
                config->eth_type = (unsigned short)val;
            }
            else {
                fprintf(stderr, "错误：选项 '--type' 需要指定一个十六进制值。\n");
                return -1;
            }
        }

        //如果当前参数不是以 - 开头（即不是选项），则视为输入文件名
        else if (argv[i][0] != '-') {
            strncpy(config->input_file, argv[i], sizeof(config->input_file) - 1);
            config->input_file[sizeof(config->input_file) - 1] = '\0';
        }
        //--stdin：设置标志，表示从标准输入读取数据
        else if (strcmp(argv[i], "--stdin") == 0) {
            config->use_stdin = 1;
        }
        //未知选项报错
        else {
            fprintf(stderr, "错误：未知选项 '%s'。使用 -h 查看帮助。\n", argv[i]);
            return -1;
        }
    }

    if (!config->use_stdin && strlen(config->input_file) == 0) {
        fprintf(stderr, "错误：未指定输入文件。\n");
        print_usage(argv[0]);
        return -1;
    }
    return 0;
}

//打印帮助信息
void print_usage(const char* prog_name)
{
    printf("以太网帧封装程序 (EFrame) v1.1.0\n\n");
    printf("用法: %s <输入文件> [选项]\n\n", prog_name);
    printf("选项:\n");
    printf("  -o, --output <文件>    指定输出文件路径（默认: output_frame.bin）\n");
    printf("  --dst <MAC地址>        指定目的MAC地址（默认: FF-FF-FF-FF-FF-FF）\n");
    printf("  --src <MAC地址>        指定源MAC地址（默认: 00-0C-29-00-00-01）\n");
    printf("  --type <十六进制>      指定类型字段（默认: 0800 表示 IPv4）\n");
    printf("  --stdin                从标准输入读取十六进制数据（交互模式）\n");
    printf("  -h, --help             显示此帮助信息\n");
    printf("\nMAC 地址格式: XX-XX-XX-XX-XX-XX（十六进制，大小写均可）\n\n");
    printf("示例:\n");
    printf("  %s data.bin\n", prog_name);
    printf("  %s data.bin -o frame.bin\n", prog_name);
    printf("  %s data.bin --dst AA-BB-CC-DD-EE-FF --src 00-11-22-33-44-55\n", prog_name);
    printf("  %s data.bin --type 0806 -o arp_frame.bin\n", prog_name);
    printf("\n常见类型字段:\n");
    printf("  0800 = IPv4\n");
    printf("  0806 = ARP\n");
    printf("  8035 = RARP\n");
    printf("  86DD = IPv6\n");
    printf("  8100 = VLAN 标记 (IEEE 802.1Q)\n");
}