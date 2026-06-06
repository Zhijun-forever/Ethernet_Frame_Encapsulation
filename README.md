
# 以太网帧封装程序 (EFrame) v1.1.0

模拟以太网 V2 MAC 帧的封装过程，支持从文件或标准输入读取数据，自动填充最小帧长，计算标准 CRC‑32 FCS，并输出完整的以太网帧文件。

## ✨ 功能特性

- 🖥️ **命令行自定义**：目的/源 MAC 地址、以太类型字段
- 📂 **文件输入**：读取二进制文件作为网络层数据（最大 1500 字节）
- ⌨️ **交互式输入**：通过 `--stdin` 选项直接输入十六进制数据（如 `48 65 6C 6C 6F`）
- 🔧 **自动填充**：数据不足 46 字节时自动补 `0x00` 至最小帧长
- ✅ **标准 CRC‑32**：采用反射多项式 `0xEDB88320`，生成的 FCS 与 Wireshark 等工具完全一致
- 🛡️ **健壮性**：完整错误检查，内存泄漏防护，缓冲区溢出保护
- 🌍 **跨平台**：Windows、Linux、macOS 均可用 C99 编译器构建

## 📁 项目结构

```
eframe/
├── eframe.h          # 公共类型、常量定义
├── config.h / .c     # 命令行参数解析、帮助信息
├── mac_utils.h / .c  # 控制台初始化、十六进制打印、MAC 地址处理
├── crc32.h / .c      # CRC‑32 查表与计算（标准以太网算法）
├── data_io.h / .c    # 文件读取、标准输入交互、数据填充
├── frame.h / .c      # 以太网帧封装、序列化、保存
├── main.c            # 主流程控制
└── README.md
```

## ⚙️ 编译

### 前置要求

- C99 兼容编译器（GCC、Clang、MSVC）
- CMake（可选，仅用于 IDE 集成；命令行可直接编译）

### 命令行编译

将项目目录中所有 `.c` 文件一起编译：

```bash
# Windows (MinGW / MSYS2)
gcc -Wall -Wextra -std=c99 -O2 -o eframe.exe main.c config.c mac_utils.c crc32.c data_io.c frame.c

# Linux / macOS
gcc -Wall -Wextra -std=c99 -O2 -o eframe main.c config.c mac_utils.c crc32.c data_io.c frame.c

# Visual Studio (Developer Command Prompt)
cl /W4 /std:c11 /Fe:eframe.exe main.c config.c mac_utils.c crc32.c data_io.c frame.c
```

> **注意**：在 Visual Studio 中请确保所有源文件保存为 **UTF‑8 with BOM**，否则中文提示可能乱码。

## 🚀 使用方法

程序需要一个**输入源**（文件或标准输入），并可附带多个选项。

```bash
eframe <输入文件> [选项]
eframe --stdin [选项]
```

### 1. 从文件读取（默认模式）

```bash
eframe data.bin
eframe data.bin -o frame.bin
eframe data.bin --dst AA-BB-CC-DD-EE-FF --src 00-11-22-33-44-55 --type 0806
```

### 2. 交互式输入（`--stdin`）

```bash
eframe --stdin
```

运行后会提示输入十六进制数据，按回车结束：

```
请输入十六进制数据（例如：48 65 6C 6C 6F），按回车结束：
> 48 65 6C 6C 6F
```

然后生成包含 `Hello` 的以太网帧。也可与其它选项组合：

```bash
eframe --stdin -o hello_frame.bin --type 0800
```

### 3. 查看帮助

```bash
eframe -h
```

## 🧾 命令行选项

| 选项                       | 描述                                         | 默认值                         |
|----------------------------|----------------------------------------------|--------------------------------|
| `-o`, `--output <文件>`    | 指定输出帧文件路径                           | `output_frame.bin`             |
| `--dst <MAC地址>`          | 目的 MAC 地址（格式：XX-XX-XX-XX-XX-XX）     | `FF-FF-FF-FF-FF-FF`            |
| `--src <MAC地址>`          | 源 MAC 地址（格式同上）                      | `00-0C-29-00-00-01`            |
| `--type <十六进制>`        | 以太类型字段（如 0800 表示 IPv4）            | `0800`                         |
| `--stdin`                  | 从标准输入读取十六进制数据（交互模式）       | 关闭                           |
| `-h`, `--help`             | 显示帮助信息                                 |                                |

> **MAC 地址格式**：`XX-XX-XX-XX-XX-XX`（十六进制，大小写均可），每字节用 `-` 分隔，长度必须 17 个字符。  
> **类型字段**：十六进制，范围 `0000` ~ `FFFF`。

## 📖 示例

### 基本文件封装

```bash
# 将 data.bin 封装成以太网帧，输出到 output_frame.bin
eframe data.bin
```

### 指定 MAC 和类型

```bash
eframe data.bin --dst 01-02-03-04-05-06 --src 0A-0B-0C-0D-0E-0F --type 0806 -o arp_frame.bin
```

### 交互式输入一个 Ping 请求

```bash
eframe --stdin --type 0800
> 08 00 00 00 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ...
```

### 生成最小帧（空数据）

```bash
# 输入空数据（直接回车），程序将生成一个只有填充的 46 字节数据帧
eframe --stdin
>
```

## 🧪 测试数据创建

你可以用以下命令快速创建一个测试用的二进制文件：

```bash
# Linux / macOS
echo -n "Hello, World!" > data.bin

# Windows PowerShell
[System.IO.File]::WriteAllText("data.bin", "Hello, World!")
```

然后运行 `eframe data.bin`，生成的帧将包含这段文本。

## 🔧 技术细节

### 以太网帧格式

```
+--------+--------+--------+----------+------+
| 目的MAC (6) | 源MAC (6) | 类型 (2) | 数据 (46-1500) | FCS (4) |
+--------+--------+--------+----------+------+
```

- 数据长度不足 46 字节时，自动用 `0x00` 填充至 46 字节。
- FCS 覆盖范围：目的 MAC、源 MAC、类型、数据（不含 FCS 自身）。

### CRC‑32 算法

- 多项式：`0xEDB88320`（反射形式，与 IEEE 802.3 一致）
- 初始值：`0xFFFFFFFF`，输出异或 `0xFFFFFFFF`
- 与 Wireshark、tcpdump 等工具的 FCS 校验结果兼容。

### 编码与跨平台

- 程序内部字符串使用 UTF‑8 编码，Windows 下通过 `SetConsoleOutputCP(65001)` 支持中文提示。
- 建议将所有源文件保存为 **UTF‑8 with BOM**（Visual Studio）或 UTF‑8（其他编译器），避免编译警告。

## ⚠️ 注意事项

1. **输入数据限制**：最大 1500 字节（标准以太网 MTU），超出会报错。
2. **十六进制输入格式**：使用 `--stdin` 时，每两个十六进制字符之间**必须用空格分隔**（例如 `48 65` 正确，`4865` 错误）。
3. **文件路径**：若程序运行路径中包含非 ASCII 字符，帮助信息中显示的路径可能乱码（不影响功能）；可通过将项目放在全英文路径下避免。
4. **空数据输入**：`--stdin` 下直接回车将生成空数据帧（全部填充为 0x00 至 46 字节）。
5. **调试输出**：程序会显示 CRC 计算值、帧长度等信息到标准输出，不影响生成的二进制文件。
