/**
 * @file    mac_utils.c
 * @brief   工具函数模块，包含控制台、十六进制打印、MAC 地址处理实现
 */

#define _CRT_SECURE_NO_WARNINGS
#include "mac_utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

 /**
  * @brief 初始化控制台编码
  */
void console_init(void)
{
    
//SetConsoleOutputCP 是Windows API函数，在Linux下不存在，所以需要平台判断
#ifdef _WIN32
    SetConsoleOutputCP(65001);  //设置控制台输出编码为UTF - 8，在Windows系统下，将控制台的输入 / 输出代码页设置为 65001（UTF - 8），避免中文乱码
    SetConsoleCP(65001);        //设置控制台输入编码为UTF-8（比如从键盘读入中文时）
#endif
}


//打印分隔线
void print_separator(void)
{
    printf("+==========================================+\n");
}

//打印程序标题横幅
void print_banner(void)
{
    printf("\n");
    print_separator();
    printf("|   以太网帧封装程序 (EFrame) v1.0.0      |\n");
    printf("|   模拟以太网 V2 MAC 帧的封装过程        |\n");
    print_separator();
    printf("\n");
}


//十六进制+ASCII打印  把任意二进制数据同时用十六进制和可读字符（ASCII）打印出来
void print_hex(const char* title, const unsigned char* data, int len)
{
    int i, j;
    printf("\n----- %s -----\n", title);
    printf("数据长度: %d 字节\n\n", len);

    for (i = 0; i < len; i += 16) {
        printf("%08X  ", i);            //%08X 表示用 8 位十六进制，不足补零,表示当前行第一个字节在整个数据中的位置编号（偏移量）
        for (j = 0; j < 16; j++) {
            if (i + j < len)
                printf("%02X ", data[i + j]);   //打印一个字节的十六进制（两位，不足补0）
            else
                printf("   ");          //如果字节不存在就打印空格 " " 占位
            if (j == 7) printf(" ");    //当 j == 7（即第 8 个字节之后）多打印一个空格，把 16 字节分成两组，每组 8 字节，视觉效果更清晰
        }
        printf(" |");
        for (j = 0; j < 16 && (i + j) < len; j++) {
            unsigned char c = data[i + j];
            /* 显式检测 ASCII 可打印字符 */
            printf("%c", (c >= 32 && c <= 126) ? c : '.'); //取出每个字节 c，判断它是不是可打印的ASCII字符（从空格 ' '=32 到 '~'=126 之间）
        }
        printf("|\n");
    }
    printf("\n");
}

int parse_mac_address(const char* str, mac_addr_t* mac)
{
    unsigned int bytes[MAC_ADDR_LEN];
    int i;

    if (str == NULL || mac == NULL) return -1;
    if (strlen(str) != 17) {
        fprintf(stderr, "错误：MAC 地址长度不正确，应为 17 个字符（XX-XX-XX-XX-XX-XX），当前为 %zu 个字符。\n", strlen(str));
        return -1;
    }
    for (i = 2; i < 17; i += 3) {
        if (str[i] != '-') {
            fprintf(stderr, "错误：MAC 地址中第 %d 个字符应为 '-'，但实际为 '%c'。\n", i + 1, str[i]);
            return -1;
        }
    }
    for (i = 0; i < MAC_ADDR_LEN; i++) {
        char byte_str[3] = { str[i * 3], str[i * 3 + 1], '\0' };
        char* endptr;
        long val = strtol(byte_str, &endptr, 16);
        if (*endptr != '\0' || val < 0 || val > 255) {
            fprintf(stderr, "错误：'%s' 不是有效的十六进制字节。\n", byte_str);
            return -1;
        }
        bytes[i] = (unsigned int)val;
    }
    for (i = 0; i < MAC_ADDR_LEN; i++)
        mac->byte[i] = (unsigned char)bytes[i];

    return 0;
}

void mac_to_string(const mac_addr_t* mac, char* buf)
{
    sprintf(buf, "%02X-%02X-%02X-%02X-%02X-%02X",
        mac->byte[0], mac->byte[1], mac->byte[2],
        mac->byte[3], mac->byte[4], mac->byte[5]);
}