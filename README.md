# xWRL6844 毫米波生命体征演示工程

本目录是面向 **xWRL6844EVM** 的毫米波演示工程，运行在 Cortex-R5F / FreeRTOS 上。工程保留常规点云处理路径，并增加生命体征路径，用雷达数据估计呼吸率和心率。生命体征模式适合目标基本静止的场景。

## 目录结构

| 路径 | 内容 |
| --- | --- |
| `source/` | 演示程序源码，包括 CLI、雷达处理、跟踪、校准及 `vital_signs/` 算法 |
| `profiles/` | 可通过 CLI 下发的雷达配置；`profile_vital_signs.cfg` 用于生命体征模式 |
| `xwrL684x-evm/r5fss0-0_freertos/` | 按 SDK 示例布局组织的工程、SysConfig 配置和 TI Arm Clang Makefile |
| `mmwave_demo_xwrL684x-evm_r5fss0-0_freertos_ti-arm-clang/` | 可导入 Code Composer Studio（CCS）的工程及其构建产物 |
| `prebuilt_binaries/xwrL684x-evm/` | 仓库附带的预编译 `.appimage` |

## 构建

需要 TI Code Composer Studio、支持 xWRL684x 的 **MMWAVE-L-SDK-6**、SysConfig 和 TI Arm Clang 工具链。工程规格文件声明的编译器版本为 `4.0.4.LTS`；实际安装路径由 CCS 中的 SDK 产品及工程变量解析。

1. 在 CCS 中导入 `mmwave_demo_xwrL684x-evm_r5fss0-0_freertos_ti-arm-clang/` 目录下的现有工程。
2. 确认 CCS 能找到 MMWAVE-L-SDK-6、SysConfig、TI Arm Clang，以及工程引用的 `MCU_PLUS_SDK_PATH` 和 `MMW_DFP_PATH`。
3. 选择 `Debug` 或 `Release` 配置并构建。工程的构建后步骤会生成相应的 `.appimage`。

SDK 示例布局还提供 `xwrL684x-evm/r5fss0-0_freertos/ti-arm-clang/makefile`。它通过 `MCU_PLUS_SDK_PATH` 引用 SDK 内的头文件、库和工具；独立使用前需按该 Makefile 的相对路径及环境变量要求布置工程。

## 运行生命体征模式

1. 将构建得到的 `.appimage` 下载或烧录到 xWRL6844EVM。目录中也有预编译镜像可供使用；如需验证本地源码改动，应使用重新构建的镜像。
2. 连接板卡的 CLI 串口和数据串口。数据串口在 `example.syscfg` 中配置为 **1,250,000 baud**；CLI 串口速率以实际生成的 SysConfig 配置为准。
3. 按顺序向 CLI 发送 [`profiles/profile_vital_signs.cfg`](profiles/profile_vital_signs.cfg) 中的命令。以 `%` 开头的行是注释，不要发送。配置末尾的 `sensorStart 0 0 0 0` 启动雷达。

生命体征配置中的关键命令：

| 命令 | 作用 |
| --- | --- |
| `dataPathMode 1` | 选择生命体征处理路径；`0` 为常规点云路径 |
| `vitalSignsCfg 0 0` | 自动选择目标距离单元；`vitalSignsCfg 1 <rangeBin>` 使用指定距离单元 |
| `trackingCfg 1 ...` | 启用目标跟踪；当前配置限制为一个跟踪目标 |

自动模式优先使用跟踪目标，没有可用跟踪目标时使用检测到的最强点。配置中的区域边界、安装高度和雷达参数应按实际场景调整。

## 数据输出

数据串口按帧发送二进制 TLV 数据。生命体征模式会加入类型为 **`0x410`** 的 TLV，其载荷对应 `source/vital_signs/vitalsign.h` 中的 `vsFeature` 结构体，包含距离单元、呼吸波动、心率、呼吸率及两组历史缓冲区。启用跟踪且有目标输出时，还可发送目标列表 TLV `1010` 和目标索引 TLV `1011`。解析程序应以当前源码中的结构体和 TLV 定义为准。

## 代码入口

- `source/mmw_cli.c`：CLI 命令解析，包括 `dataPathMode` 和 `vitalSignsCfg`。
- `source/vital_signs/vital_signs_proc.c`：生命体征处理入口及雷达数据适配。
- `source/vital_signs/vitalsign.c`：呼吸和心率估计算法。
- `source/mmwave_demo.c`：主处理流程及 UART TLV 打包发送。

源码中的 TI 文件保留各自的版权和许可声明；再分发时请遵守对应文件中的条款。
