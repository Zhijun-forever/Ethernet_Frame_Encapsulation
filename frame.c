/**
 * @file    frame.c
 * @brief   帧封装实现
 */



#define _CRT_SECURE_NO_WARNINGS
#include "frame.h"
#include "crc32.h"
#include <string.h>

//构建以太网帧
int build_ethernet_frame(const program_config_t* config,
    const unsigned char* data,
    int data_len,
    ethernet_frame_t* frame)
{

    /*
        config：用户配置（MAC地址、类型等）

        data：已经填充好的网络层数据

        data_len：填充后的数据长度

        frame：输出参数，存放构建好的帧

        crc_input：临时缓冲区，用于计算 CRC（包含帧头 + 数据）

        crc_value：计算出的 CRC-32 值
    */
    unsigned char crc_input[FRAME_HEADER_LEN + DATA_MAX_LEN];
    unsigned int  crc_value;
    int           crc_input_len;

    /* 组装帧头:将配置中的目的MAC、源MAC、类型字段拷贝到帧头 */
    memcpy(frame->header.dst_mac.byte, config->dst_mac.byte, MAC_ADDR_LEN);
    memcpy(frame->header.src_mac.byte, config->src_mac.byte, MAC_ADDR_LEN);
    frame->header.type = config->eth_type;

    /* 拷贝数据到帧的数据区，记录数据长度 */
    frame->data_len = data_len;
    memcpy(frame->data, data, (size_t)data_len);

    /* 准备 CRC 输入缓冲区 */
    memcpy(crc_input, frame->header.dst_mac.byte, MAC_ADDR_LEN);
    memcpy(crc_input + MAC_ADDR_LEN, frame->header.src_mac.byte, MAC_ADDR_LEN);

    // 调用 CRC32 模块计算校验值
    crc_input[MAC_ADDR_LEN * 2] = (unsigned char)((config->eth_type >> 8) & 0xFF);
    crc_input[MAC_ADDR_LEN * 2 + 1] = (unsigned char)(config->eth_type & 0xFF);
    memcpy(crc_input + FRAME_HEADER_LEN, data, (size_t)data_len);

    crc_input_len = FRAME_HEADER_LEN + data_len;
    crc_value = crc32_calculate(crc_input, crc_input_len);

    // 将 32 位 CRC 值拆成 4 个字节存入 fcs
    frame->fcs[0] = (unsigned char)((crc_value >> 24) & 0xFF);
    frame->fcs[1] = (unsigned char)((crc_value >> 16) & 0xFF);
    frame->fcs[2] = (unsigned char)((crc_value >> 8) & 0xFF);
    frame->fcs[3] = (unsigned char)(crc_value & 0xFF);

    printf("提示：CRC-32 计算完成，FCS = 0x%08X（网络字节序: %02X %02X %02X %02X）\n",
        crc_value, frame->fcs[0], frame->fcs[1], frame->fcs[2], frame->fcs[3]);

    return FRAME_HEADER_LEN + data_len + FCS_LEN;
}


//将帧结构体序列化为字节数组
unsigned char* frame_to_buffer(const ethernet_frame_t* frame, int frame_len)
{
    /*
        frame：要序列化的帧

        frame_len：预期总长度（由 build_ethernet_frame 返回，用于一致性检查）

        expected_len：根据帧头+数据+FCS 重新计算的长度，应该与 frame_len 一致
    */
    unsigned char* buf;
    int pos = 0;
    int expected_len = FRAME_HEADER_LEN + frame->data_len + FCS_LEN;

    if (expected_len != frame_len) {
        fprintf(stderr, "错误：帧长度不一致（期望 %d，实际 %d）。\n", expected_len, frame_len);
        return NULL;
    }

    buf = (unsigned char*)malloc((size_t)frame_len);
    if (buf == NULL) {
        fprintf(stderr, "错误：内存分配失败。\n");
        return NULL;
    }

    /* 目的 MAC */
    if (pos + MAC_ADDR_LEN > frame_len) goto overflow;
    memcpy(buf + pos, frame->header.dst_mac.byte, MAC_ADDR_LEN);
    pos += MAC_ADDR_LEN;

    /* 源 MAC */
    if (pos + MAC_ADDR_LEN > frame_len) goto overflow;
    memcpy(buf + pos, frame->header.src_mac.byte, MAC_ADDR_LEN);
    pos += MAC_ADDR_LEN;

    /* 类型字段 */
    if (pos + TYPE_LEN > frame_len) goto overflow;
    buf[pos] = (unsigned char)((frame->header.type >> 8) & 0xFF);
    buf[pos + 1] = (unsigned char)(frame->header.type & 0xFF);
    pos += TYPE_LEN;

    /* 数据 */
    if (pos + frame->data_len > frame_len) goto overflow;
    memcpy(buf + pos, frame->data, (size_t)frame->data_len);
    pos += frame->data_len;

    /* FCS */
    if (pos + FCS_LEN > frame_len) goto overflow;
    memcpy(buf + pos, frame->fcs, FCS_LEN);
    pos += FCS_LEN;

    if (pos != frame_len) {
        fprintf(stderr, "错误：序列化后长度不一致（写入 %d / 期望 %d）。\n", pos, frame_len);
        free(buf);
        return NULL;
    }
    return buf;

//溢出处理
overflow:
    fprintf(stderr, "错误：序列化帧时发生缓冲区溢出（pos=%d, frame_len=%d）。\n", pos, frame_len);
    free(buf);
    return NULL;
}


//保存帧到文件
int save_frame_to_file(const char* filename,
    const ethernet_frame_t* frame,
    int frame_len)
{
    FILE* fp;
    unsigned char* buffer;
    size_t written;

    buffer = frame_to_buffer(frame, frame_len);
    if (buffer == NULL)
        return -1;

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "错误：无法写入输出文件 '%s'，请检查磁盘空间和文件权限。\n", filename);
        free(buffer);
        return -1;
    }

    written = fwrite(buffer, 1, (size_t)frame_len, fp);
    fclose(fp);
    free(buffer);

    if ((int)written != frame_len) {
        fprintf(stderr, "错误：写入输出文件时发生错误，期望写入 %d 字节，实际写入 %zu 字节。\n",
            frame_len, written);
        return -1;
    }

    printf("提示：帧数据已保存至 '%s'（%d 字节）。\n", filename, frame_len);
    return 0;
}