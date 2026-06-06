/**
 * @file    eframe.h
 * @brief   以太网帧封装程序公共定义
 */

#define _CRT_SECURE_NO_WARNINGS
#ifndef EFRAME_H
#define EFRAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

 /* ================================================================
  * 常量定义
  * ================================================================ */
#define MAC_ADDR_LEN         6
#define TYPE_LEN             2
#define FCS_LEN              4
#define FRAME_HEADER_LEN     14
#define DATA_MIN_LEN         46
#define DATA_MAX_LEN         1500

#define DEFAULT_OUTPUT_FILE  "output_frame.bin"

#define DEFAULT_DST_MAC      "FF-FF-FF-FF-FF-FF"
#define DEFAULT_SRC_MAC      "00-0C-29-00-00-01"
#define DEFAULT_ETH_TYPE     0x0800

  /* ================================================================
   * 数据结构定义
   * ================================================================ */
typedef struct {
    unsigned char byte[MAC_ADDR_LEN];
} mac_addr_t;

typedef struct {
    mac_addr_t     dst_mac;
    mac_addr_t     src_mac;
    unsigned short type;
} ethernet_header_t;

typedef struct {
    ethernet_header_t header;
    unsigned char     data[DATA_MAX_LEN];
    int               data_len;
    unsigned char     fcs[FCS_LEN];
} ethernet_frame_t;

typedef struct {
    char           input_file[256];
    char           output_file[256];
    mac_addr_t     dst_mac;
    mac_addr_t     src_mac;
    unsigned short eth_type;
    int            use_stdin;       /* 新增：1 = 从标准输入读取数据 */
} program_config_t;

#endif /* EFRAME_H */