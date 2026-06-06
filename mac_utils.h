/**
 * @file    mac_utils.h
 * @brief   控制台、十六进制打印、MAC 地址处理
 */

#define _CRT_SECURE_NO_WARNINGS
#ifndef MAC_UTILS_H
#define MAC_UTILS_H

#include "eframe.h"

void console_init(void);
void print_separator(void);
void print_banner(void);
void print_hex(const char* title, const unsigned char* data, int len);
int  parse_mac_address(const char* str, mac_addr_t* mac);
void mac_to_string(const mac_addr_t* mac, char* buf);

#endif /* MAC_UTILS_H */ 