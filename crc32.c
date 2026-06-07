/**
 * @file    crc32.c
 * @brief   CRC-32 查表与计算实现（反射算法，多项式 0xEDB88320）
 */

#define _CRT_SECURE_NO_WARNINGS

#include "crc32.h"
#define CRC32_POLYNOMIAL 0xEDB88320  /* 标准以太网反射多项式 即把通常的 0x04C11DB7 进行了位反转*/
/*
    0x04C11DB7 作为 CRC-32 的生成多项式（非反射版本），它的数学表达式（系数表示）是：
    G(x) = x³² + x²⁶ + x²³ + x²² + x¹⁶ + x¹² + x¹¹ + x¹⁰ + x⁸ + x⁷ + x⁵ + x⁴ + x² + x + 1

    二进制展开是：
    0000 0100 1100 0001 0001 1101 1011 0111

*/


static unsigned int crc32_table[256];

void crc32_init_table(void)
{
    unsigned int remainder;
    int i, j;

    for (i = 0; i < 256; i++) {
        remainder = (unsigned int)i;
        for (j = 0; j < 8; j++) {
            //每次移出最低位，如果移出的那一位是 1，就需要异或多项式
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