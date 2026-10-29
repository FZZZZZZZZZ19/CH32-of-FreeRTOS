# FreeRTOS Kernel 来源

- 上游项目：<https://github.com/FreeRTOS/FreeRTOS-Kernel>
- 标签：`V11.3.2`
- 提交：`640d0c43705e64cd53f46b31d882de6cd8433933`
- 许可证：MIT，见本目录 `LICENSE.md`
- 本工程保留官方 `portable/GCC/RISC-V` 端口，并在 `portASM.S` 显式设置 FPU 上下文选项以匹配 CH32V307VCT6 和 `FreeRTOSConfig.h`。
