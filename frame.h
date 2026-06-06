/**
 * @file    frame.h
 * @brief   以太网帧封装、序列化与保存
 */

#define _CRT_SECURE_NO_WARNINGS
#ifndef FRAME_H
#define FRAME_H

#include "eframe.h"

int  build_ethernet_frame(const program_config_t* config,
    const unsigned char* data,
    int data_len, ethernet_frame_t* frame);

unsigned char* frame_to_buffer(const ethernet_frame_t* frame, int frame_len);

int  save_frame_to_file(const char* filename,
    const ethernet_frame_t* frame,
    int frame_len);

#endif /* FRAME_H */ 