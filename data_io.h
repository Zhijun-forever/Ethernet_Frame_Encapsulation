/**
 * @file    data_io.h
 * @brief   数据读取与填充
 */

#define _CRT_SECURE_NO_WARNINGS
#ifndef DATA_IO_H
#define DATA_IO_H

#include "eframe.h"

unsigned char* read_data_from_file(const char* filename, int* data_len);
int            pad_data(unsigned char* data, int data_len, int max_len);
unsigned char* read_data_from_stdin(int* data_len);

#endif /* DATA_IO_H */ 