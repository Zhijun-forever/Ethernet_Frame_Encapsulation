/**
 * @file    data_io.c
 * @brief   数据读取与填充实现
 */

#define _CRT_SECURE_NO_WARNINGS
#include "data_io.h"

unsigned char* read_data_from_file(const char* filename, int* data_len)
{
    FILE* fp;
    long  file_size;
    unsigned char* buffer;

    *data_len = 0;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "错误：输入文件 '%s' 不存在，请检查文件路径。\n", filename);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fprintf(stderr, "错误：无法读取输入文件 '%s'，请检查文件权限。\n", filename);
        fclose(fp);
        return NULL;
    }

    file_size = ftell(fp);
    if (file_size < 0) {
        fprintf(stderr, "错误：无法读取输入文件 '%s'，请检查文件权限。\n", filename);
        fclose(fp);
        return NULL;
    }
    rewind(fp);

    /* 提前检查文件是否超过最大数据长度 */
    if (file_size > DATA_MAX_LEN) {
        fprintf(stderr, "错误：输入文件 %ld 字节，超过 MTU 上限 %d 字节。\n",
            file_size, DATA_MAX_LEN);
        fclose(fp);
        return NULL;
    }

    buffer = (unsigned char*)malloc((size_t)(file_size + DATA_MIN_LEN + 1));
    if (buffer == NULL) {
        fprintf(stderr, "错误：内存分配失败。\n");
        fclose(fp);
        return NULL;
    }

    if (file_size == 0) {
        printf("提示：输入文件为空（0 字节），将全部填充 0x00。\n");
        fclose(fp);
        *data_len = 0;
        return buffer;
    }

    *data_len = (int)fread(buffer, 1, (size_t)file_size, fp);
    if (*data_len != file_size) {
        fprintf(stderr, "错误：无法读取输入文件 '%s'，请检查文件权限。\n", filename);
        free(buffer);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return buffer;
}

unsigned char* read_data_from_stdin(int* data_len)
{
    char line[8192];   /* 足够容纳最长输入 */
    unsigned char* buffer = NULL;
    int i, buf_index = 0;
    int nibble_count = 0;
    unsigned char current_byte = 0;

    *data_len = 0;

    printf("请输入十六进制数据（例如：48 65 6C 6C 6F），按回车结束：\n");
    printf("> ");

    if (fgets(line, sizeof(line), stdin) == NULL) {
        fprintf(stderr, "错误：无法读取标准输入。\n");
        return NULL;
    }

    /* 分配最大可能的内存（每两个十六进制字符生成一个字节） */
    buffer = (unsigned char*)malloc((DATA_MAX_LEN + DATA_MIN_LEN + 1) * sizeof(unsigned char));
    if (buffer == NULL) {
        fprintf(stderr, "错误：内存分配失败。\n");
        return NULL;
    }

    for (i = 0; line[i] != '\0' && line[i] != '\n'; i++) {
        char ch = line[i];

        /* 跳过空格 */
        if (ch == ' ' || ch == '\t') {
            /* 如果当前半个字节未完成，则视为错误（例如 "4 "） */
            if (nibble_count == 1) {
                fprintf(stderr, "错误：十六进制输入格式不正确（单个字符后遇空格）。\n");
                free(buffer);
                return NULL;
            }
            continue;
        }

        /* 将字符转换为 0-15 的值 */
        int nibble = -1;
        if (ch >= '0' && ch <= '9')
            nibble = ch - '0';
        else if (ch >= 'A' && ch <= 'F')
            nibble = ch - 'A' + 10;
        else if (ch >= 'a' && ch <= 'f')
            nibble = ch - 'a' + 10;
        else {
            fprintf(stderr, "错误：输入包含非法字符 '%c'。\n", ch);
            free(buffer);
            return NULL;
        }

        if (nibble_count == 0) {
            current_byte = (unsigned char)(nibble << 4);
            nibble_count = 1;
        }
        else {
            current_byte |= (unsigned char)nibble;
            /* 检查是否超过最大数据长度 */
            if (buf_index >= DATA_MAX_LEN) {
                fprintf(stderr, "错误：输入数据超过 %d 字节。\n", DATA_MAX_LEN);
                free(buffer);
                return NULL;
            }
            buffer[buf_index++] = current_byte;
            nibble_count = 0;
        }
    }

    /* 输入结束，但半个字节残留（例如 "48 6"） */
    if (nibble_count == 1) {
        fprintf(stderr, "错误：十六进制输入不完整（最后一个字节缺少低位）。\n");
        free(buffer);
        return NULL;
    }

    /* 允许空输入（0 字节），后续会填充至最小帧长 */
    *data_len = buf_index;

    if (buf_index == 0) {
        printf("提示：输入为空，将生成一个仅含填充数据的帧。\n");
    }

    return buffer;
}

int pad_data(unsigned char* data, int data_len, int max_len)
{
    int pad_count;

    /* 忽略 max_len 参数，因为填充后最大长度不会超过 DATA_MIN_LEN，
     * 而缓冲区已在调用者处分配了至少 DATA_MIN_LEN 字节的额外空间。 */
    (void)max_len;

    if (data_len >= DATA_MIN_LEN)
        return data_len;

    pad_count = DATA_MIN_LEN - data_len;

    /* 填充后的长度绝不会超过 DATA_MAX_LEN（46 <= 1500），
     * 但仍做一次保守检查以防逻辑错误导致 data_len 异常。 */
    if (data_len + pad_count > DATA_MAX_LEN) {
        fprintf(stderr, "错误：数据长度异常，无法填充。\n");
        return data_len;
    }

    memset(data + data_len, 0x00, (size_t)pad_count);
    printf("提示：数据长度 %d 字节 < %d 字节，已填充 %d 个 0x00 至最小帧长。\n",
        data_len, DATA_MIN_LEN, pad_count);
    return DATA_MIN_LEN;
}   


