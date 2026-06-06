/**
 * @file    crc32.h
 * @brief   CRC-32 计算（标准以太网 FCS）
 */

#define _CRT_SECURE_NO_WARNINGS
#ifndef CRC32_H
#define CRC32_H

void crc32_init_table(void);
unsigned int crc32_calculate(const unsigned char* data, int len);

#endif /* CRC32_H */ 