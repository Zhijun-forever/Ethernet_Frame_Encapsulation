/**
 * @file    eframe.h
 * @brief   以太网帧封装程序公共定义
 */

#define _CRT_SECURE_NO_WARNINGS

//防止重复包含头文件的宏定义
#ifndef EFRAME_H
#define EFRAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

 /* ================================================================
  * 常量定义
  * ================================================================ */
#define MAC_ADDR_LEN         6              //与目的/源 MAC 长度对应
#define TYPE_LEN             2              //与类型字段长度对应
#define FCS_LEN              4              //FCS长度(使用CRC32)
#define FRAME_HEADER_LEN     14             //以太网帧头，目的 MAC(6) + 源 MAC(6) + 类型(2)
#define DATA_MIN_LEN         46             //最小数据段长度，若不足需要填充
#define DATA_MAX_LEN         1500           //最大数据段长度，即 MTU

#define DEFAULT_OUTPUT_FILE  "output_frame.bin"         


#define DEFAULT_DST_MAC      "FF-FF-FF-FF-FF-FF"    //默认 MAC 地址字符串，广播地址
#define DEFAULT_SRC_MAC      "00-0C-29-00-00-01"    // 示例源MAC
#define DEFAULT_ETH_TYPE     0x0800                 //默认配置中的类型值为IPV4

  /* ================================================================
   * 数据结构定义
   * ================================================================ */

//以太网MAC地址结构体定义
typedef struct {
    unsigned char byte[MAC_ADDR_LEN];
} mac_addr_t;


//以太网帧头，包含：目的 MAC、源 MAC、类型
typedef struct {
    mac_addr_t     dst_mac;
    mac_addr_t     src_mac;
    unsigned short type;    //标识上层协议（如 IPv4=0x0800, ARP=0x0806）
                            //当该字段 ≤ 1500 时，表示“长度”，那是 IEEE 802.3 原始帧格式，本程序不处理
} ethernet_header_t;



//完整的以太网帧：包含帧头、数据和FCS
typedef struct {
    ethernet_header_t header;
    unsigned char     data[DATA_MAX_LEN];
	int               data_len;     //定义实际数据长度，便于处理
    unsigned char     fcs[FCS_LEN];
} ethernet_frame_t;


//程序配置结构体，它的作用是存储用户通过命令行指定的所有参数，方便在各个函数之间传递
typedef struct {
    char           input_file[256];     //输入文件路径（当不使用stdin时）
    char           output_file[256];    //输出文件路径（默认 output_frame.bin）
    mac_addr_t     dst_mac;
    mac_addr_t     src_mac;
    unsigned short eth_type;
    int            use_stdin;       /* 1 = 从标准输入读取数据 */
} program_config_t;

#endif /* EFRAME_H */