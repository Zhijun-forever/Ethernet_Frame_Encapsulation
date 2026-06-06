/**
 * @file    crc32.c
 * @brief   CRC-32 查表与计算实现（反射算法，多项式 0xEDB88320）
 */

#define _CRT_SECURE_NO_WARNINGS

#include "crc32.h"
#define CRC32_POLYNOMIAL 0xEDB88320  /* 标准以太网反射多项式 */

static unsigned int crc32_table[256];

void crc32_init_table(void)
{
    unsigned int remainder;
    int i, j;

    for (i = 0; i < 256; i++) {
        remainder = (unsigned int)i;
        for (j = 0; j < 8; j++) {
            if (remainder & 1)
                remainder = (remainder >> 1) ^ CRC32_POLYNOMIAL;
            else
                remainder >>= 1;
        }
        crc32_table[i] = remainder;
    }
}

unsigned int crc32_calculate(const unsigned char* data, int len)
{
    unsigned int reg = 0xFFFFFFFF;
    int i;

    for (i = 0; i < len; i++) {
        unsigned char index = (reg ^ data[i]) & 0xFF;
        reg = (reg >> 8) ^ crc32_table[index];
    }
    return reg ^ 0xFFFFFFFF;
}