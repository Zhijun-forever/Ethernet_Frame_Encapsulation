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

    因为很多硬件（如网卡）在传输时是 先发最低位（LSB） 的，用反射多项式可以直接配合这种位序，算法也更简单
*/


static unsigned int crc32_table[256];


//初始化表
void crc32_init_table(void)
{
    unsigned int remainder;
    int i, j;


    for (i = 0; i < 256; i++) {         // 枚举每一个可能的字节值
        remainder = (unsigned int)i;    // 初始余数就是这个字节本身
        for (j = 0; j < 8; j++) {       // 处理这个字节的 8 个位
            //每次移出最低位，如果移出的那一位是 1，就需要异或多项式
            if (remainder & 1)
                remainder = (remainder >> 1) ^ CRC32_POLYNOMIAL;
            else
                remainder >>= 1;
        }
        crc32_table[i] = remainder;      // 存表，这就是字节 i 对 CRC 的“贡献值”
    }
}

unsigned int crc32_calculate(const unsigned char* data, int len)
{
    unsigned int reg = 0xFFFFFFFF;      //初始值全1,这样能防止“消息前面有一串 0”导致的 CRC 值相同（比如空消息和 0000 算出的 CRC 就是全0的补码变化）
                                        //寄存器现在是 11111111 11111111 11111111 11111111（二进制全1）
    int i;

    for (i = 0; i < len; i++) {
        unsigned char index = (reg ^ data[i]) & 0xFF;       //混合当前数据字节，查表索引
        reg = (reg >> 8) ^ crc32_table[index];              //查表更新
    }
    return reg ^ 0xFFFFFFFF;                                //最终异或全 1
}