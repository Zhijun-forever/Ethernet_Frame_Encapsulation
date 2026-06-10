/**
 * @file    main.c
 * @brief   以太网帧封装主流程
 */

#define _CRT_SECURE_NO_WARNINGS

#include "eframe.h"
#include "config.h"
#include "mac_utils.h"
#include "crc32.h"
#include "data_io.h"
#include "frame.h"

/**
  * @brief 程序主入口
  *
  * 执行流程：
  *   1. 初始化 CRC 查找表
  *   2. 设置默认配置
  *   3. 解析命令行参数
  *   4. 读取输入文件
  *   5. 显示原始数据
  *   6. 数据长度检查与填充
  *   7. 构建以太网帧
  *   8. 显示完整帧
  *   9. 保存到输出文件
  *
  * @param argc  命令行参数个数
  * @param argv  命令行参数数组
  * @return      0: 成功, 负数: 错误码
  */
int main(int argc, char* argv[])
{
    program_config_t config;
    unsigned char* raw_data = NULL;
    unsigned char* frame_buffer = NULL;
    ethernet_frame_t frame;
    int data_len, frame_len, ret = 0;
    int parse_result;
    char mac_str[18];

	//初始化控制台编码和显示程序横幅
    console_init();
	//打印程序横幅
    print_banner();

    /* 步骤 1: 初始化 CRC-32 查找表 */
    crc32_init_table();
    printf("[初始化] CRC-32 查找表已就绪。\n");

    /* 步骤 2: 默认配置 */
    set_default_config(&config);

    /* 步骤 3: 解析命令行 */
    parse_result = parse_arguments(argc, argv, &config);
    if (parse_result == 1)       /* 打印帮助后正常退出 */
        return 0;
    else if (parse_result != 0)  /* 错误 */
        return -4;

    /* 显示配置 */
    mac_to_string(&config.dst_mac, mac_str);
    printf("[配置] 目的 MAC 地址: %s\n", mac_str);
    mac_to_string(&config.src_mac, mac_str);
    printf("[配置] 源   MAC 地址: %s\n", mac_str);
    printf("[配置] 类型字段:      0x%04X\n", config.eth_type);
    printf("[配置] 输入文件:      %s\n", config.input_file);
    printf("[配置] 输出文件:      %s\n", config.output_file);
    printf("\n");

    /* 步骤 4: 读取数据 */
    printf("[读取] 正在读取网络层数据...\n");
    if (config.use_stdin) {
        raw_data = read_data_from_stdin(&data_len);
    }
    else {
        raw_data = read_data_from_file(config.input_file, &data_len);
    }
    if (raw_data == NULL)
        return -1;
    printf("[读取] 读取完成：%d 字节。\n", data_len);

    /* 步骤 5: 显示原始数据 */
    print_hex("原始网络层数据（来自文件）", raw_data, data_len);

    /* 步骤 6: 数据填充 (如果需要) */
    if (data_len < DATA_MIN_LEN) {
        /* 缓冲区大小足够 (文件读取时已分配 file_size+DATA_MIN_LEN+1) */
        data_len = pad_data(raw_data, data_len, DATA_MAX_LEN + DATA_MIN_LEN);
        print_hex("填充后的网络层数据", raw_data, data_len);
    }
    else {
        printf("提示：数据长度 %d 字节，在 [%d, %d] 范围内，无需填充。\n",
            data_len, DATA_MIN_LEN, DATA_MAX_LEN);
    }

    /* 步骤 7: 构建以太网帧 */
    printf("\n[封装] 正在构建以太网 MAC 帧...\n");
    memset(&frame, 0, sizeof(frame));
    frame_len = build_ethernet_frame(&config, raw_data, data_len, &frame);
    printf("[封装] 帧组装完成，总长度 = %d 字节（首部 %d + 数据 %d + FCS %d）。\n",
        frame_len, FRAME_HEADER_LEN, data_len, FCS_LEN);

    /* 步骤 8: 序列化并显示完整帧 */
    frame_buffer = frame_to_buffer(&frame, frame_len);
    if (frame_buffer == NULL) {
        ret = -1;
        goto cleanup;
    }
    print_hex("完整以太网 MAC 帧（发送帧）", frame_buffer, frame_len);

    /* 步骤 9: 保存到文件 */
    if (save_frame_to_file(config.output_file, &frame, frame_len) != 0) {
        ret = -3;
        goto cleanup;
    }

    printf("\n");
    print_separator();
    printf("|          帧封装完成！                    |\n");
    print_separator();
    printf("\n");

cleanup:
    free(raw_data);
    free(frame_buffer);
    return ret;
}