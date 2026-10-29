# CH32-of-FreeRTOS

CH32V307VCT6 + FreeRTOS 的 MounRiver Studio 工程模板。工程包含可运行的双任务示例、QingKe V4 SysTick 节拍、RISC-V 上下文切换入口和独立命令行构建脚本。

## 当前工程状态

- MCU：CH32V307VCT6，HCLK 144 MHz。
- FreeRTOS Kernel：V11.3.2，使用官方 GCC RISC-V 端口和 `heap_4`。
- 系统节拍：1 kHz，由 CH32V307 的 64 位 SysTick 产生。
- 调试串口：模板现有 UART3，PB10 TX / PB11 RX，115200 baud。
- `User/app_tasks.c` 中的 worker 每 10 ms 运行，monitor 每秒打印一次计数。

## 编译

Windows PowerShell 中，在仓库根目录运行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build.ps1
```

脚本会先查找 `MOUNRIVER_GCC` 环境变量，再自动查找 MounRiver 的 `riscv-wch-elf-gcc.exe` 或 `riscv-none-embed-gcc.exe`。也可以显式指定编译器完整路径：

```powershell
$env:MOUNRIVER_GCC = 'E:\RISC-V\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin\riscv-wch-elf-gcc.exe'
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build.ps1
```

成功后生成 `build/CH32-of-FreeRTOS.elf`、`build/CH32-of-FreeRTOS.hex` 和链接映射文件。用 MounRiver Studio 2 打开仓库根目录即可编辑工程；仓库中的 `.project` / `.cproject` 保留 IDE 工程配置。命令行构建脚本是已经验证通过的构建入口。

## 开发时改哪里

| 想做什么 | 修改位置 |
| --- | --- |
| 添加业务任务、状态机或周期逻辑 | `User/app_tasks.c`；任务函数写成 `static void task(void *arg)`，用 `xTaskCreate()` 创建 |
| 增删启动任务 | `app_tasks_create()`（`User/app_tasks.c`）；检查栈大小和优先级 |
| 调整时钟、节拍、堆大小、任务优先级数 | `FreeRTOS/Config/FreeRTOSConfig.h` |
| 配置系统节拍硬件、外部中断分发 | `User/freertos_port.c` |
| 添加启动向量/异常入口 | `Startup/startup_ch32v30x_D8C.S`；需要 FreeRTOS 上下文保护的中断必须进入 `freertos_risc_v_interrupt_handler` |
| 添加 malloc 失败或栈溢出处理 | `User/freertos_hooks.c` |
| 添加 CH32 外设驱动 | `Peripheral/src`、`Peripheral/inc`，配置宏在 `User/ch32v30x_conf.h` |
| 添加 SeekFree 板级模块 | 对应 `zf_driver`、`zf_device`、`zf_components` 目录 |
| 配置串口输出 | 保留 `clock_init()` 和 `debug_init()`；现有调试串口为 UART3 PB10/PB11 |

新增 FreeRTOS 任务的最小示例：

```c
static void sensor_task(void *arg)
{
    (void)arg;
    for (;;)
    {
        /* 读取传感器并更新应用状态 */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
```

在 `app_tasks_create()` 中调用 `xTaskCreate(sensor_task, "sensor", 256, NULL, 2, NULL)` 注册任务。FreeRTOS 的栈深度单位是 `StackType_t` 字数，不是字节。任务内避免长时间阻塞其他任务；共享数据请使用队列、互斥锁或临界区。

### 中断开发

当前启动表将 M-mode `ecall` 转给 FreeRTOS 的 yield/上下文切换处理，并将 SysTick 转给 FreeRTOS 通用 RISC-V 中断入口。外部中断若要与调度器共存，也应接入能保存/恢复 FreeRTOS 上下文的通用入口，再在 `freertos_application_irq()` 按 `mcause` 分发。中断中只使用带 `FromISR` 后缀的 FreeRTOS API，并按需触发 `portYIELD_FROM_ISR()`；不要从中断直接调用 `printf` 或普通任务 API。

## 目录说明

- `User/`：应用入口、示例任务、FreeRTOS 硬件适配和 hooks。
- `FreeRTOS/Config/`：本项目配置；`FreeRTOS/Source/`：Kernel 与 RISC-V 移植层。
- `Startup/`、`Core/`、`Ld/`：启动汇编、内核头文件和链接脚本。
- `Peripheral/`：WCH 外设库；`zf_*`：模板原有 SeekFree 驱动和板级代码。
- `tools/build.ps1`：Windows 命令行完整构建。

## 下载运行

使用 WCH-Link 将 `build/CH32-of-FreeRTOS.hex` 烧录到 CH32V307VCT6，再打开 UART3（115200、8-N-1）查看每秒一次的 worker 计数。实际板上启动效果还需连接开发板烧录验证；本仓库的命令行构建仅验证编译和链接。

## 来源与许可

FreeRTOS Kernel 使用 MIT 许可证，许可文件见 `FreeRTOS/LICENSE.md`；模板和外设/SeekFree 源码许可见 `LICENSES/` 及各源文件头部。FreeRTOS 上游版本与源码提交见 `FreeRTOS/KERNEL_VERSION.md`。
