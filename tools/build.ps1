[CmdletBinding()]
param(
    [string]$Compiler = $env:MOUNRIVER_GCC
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$buildDir = Join-Path $root 'build'
$objDir = Join-Path $buildDir 'obj'

if ([string]::IsNullOrWhiteSpace($Compiler)) {
    $candidate = Get-Command 'riscv-wch-elf-gcc.exe' -ErrorAction SilentlyContinue
    if ($candidate) { $Compiler = $candidate.Source }
}
if ([string]::IsNullOrWhiteSpace($Compiler)) {
    $candidate = Get-Command 'riscv-none-embed-gcc.exe' -ErrorAction SilentlyContinue
    if ($candidate) { $Compiler = $candidate.Source }
}
if ([string]::IsNullOrWhiteSpace($Compiler)) {
    $known = 'E:\RISC-V\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin\riscv-wch-elf-gcc.exe'
    if (Test-Path -LiteralPath $known) { $Compiler = $known }
}
if ([string]::IsNullOrWhiteSpace($Compiler) -or -not (Test-Path -LiteralPath $Compiler)) {
    throw '找不到 RISC-V GCC。请设置 MOUNRIVER_GCC 为编译器完整路径。'
}

$Compiler = (Resolve-Path -LiteralPath $Compiler).Path
$toolBin = Split-Path -Parent $Compiler
$toolName = [IO.Path]::GetFileNameWithoutExtension($Compiler)
if (-not $toolName.EndsWith('gcc')) { throw "无法从编译器名称识别工具链前缀：$toolName" }
$prefix = $toolName.Substring(0, $toolName.Length - 3)
$objcopy = Join-Path $toolBin ($prefix + 'objcopy.exe')
$sizeTool = Join-Path $toolBin ($prefix + 'size.exe')
if (-not (Test-Path -LiteralPath $objcopy)) { throw "找不到 objcopy：$objcopy" }
if (-not (Test-Path -LiteralPath $sizeTool)) { throw "找不到 size 工具：$sizeTool" }

$env:PATH = "$toolBin;$env:PATH"
New-Item -ItemType Directory -Force -Path $objDir | Out-Null

$includeDirs = @(
    'Startup', 'Debug', 'Core', 'User', 'Peripheral/inc',
    'zf_common', 'zf_driver', 'zf_device', 'zf_components',
    'FreeRTOS/Config', 'FreeRTOS/Source/include',
    'FreeRTOS/Source/portable/GCC/RISC-V',
    'FreeRTOS/Source/portable/GCC/RISC-V/chip_specific_extensions/CH32V307'
)
$includeArgs = foreach ($dir in $includeDirs) { '-I' + (Join-Path $root $dir) }
$cpuFlags = @('-march=rv32imafcxw', '-mabi=ilp32f', '-msmall-data-limit=8', '-msave-restore')
$commonFlags = @('-Os', '-g', '-fmessage-length=0', '-fsigned-char', '-ffunction-sections', '-fdata-sections', '-fno-common')
$cFlags = @($cpuFlags + $commonFlags + @('-std=gnu11', '-Wall', '-Wunused', '-Wuninitialized') + $includeArgs)
$asmFlags = @($cpuFlags + $commonFlags + @('-x', 'assembler-with-cpp', '-DconfigENABLE_FPU=1', '-DconfigENABLE_VPU=0') + $includeArgs)

$sourceDirs = @('Core', 'Debug', 'User', 'Peripheral/src', 'zf_common', 'zf_driver', 'zf_device', 'zf_components', 'FreeRTOS/Source')
$sources = foreach ($dir in $sourceDirs) {
    $fullDir = Join-Path $root $dir
    if (Test-Path -LiteralPath $fullDir) {
        Get-ChildItem -LiteralPath $fullDir -Filter '*.c' -File -Recurse | ForEach-Object { $_.FullName }
    }
}
$sources = @($sources | Sort-Object -Unique)
$assemblySources = @(
    (Join-Path $root 'Startup/startup_ch32v30x_D8C.S'),
    (Join-Path $root 'FreeRTOS/Source/portable/GCC/RISC-V/portASM.S')
)
$objects = [System.Collections.Generic.List[string]]::new()

foreach ($source in $sources) {
    $relative = $source.Substring($root.Length + 1).Replace('\', '_')
    $object = Join-Path $objDir ([IO.Path]::ChangeExtension($relative, '.o'))
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $object) | Out-Null
    Write-Host "CC  $relative"
    & $Compiler @cFlags '-c' $source '-o' $object
    if ($LASTEXITCODE -ne 0) { throw "C 编译失败：$relative (exit $LASTEXITCODE)" }
    $objects.Add($object)
}

foreach ($source in $assemblySources) {
    if (-not (Test-Path -LiteralPath $source)) { throw "缺少汇编源文件：$source" }
    $relative = $source.Substring($root.Length + 1).Replace('\', '_')
    $object = Join-Path $objDir ([IO.Path]::ChangeExtension($relative, '.o'))
    Write-Host "AS  $relative"
    & $Compiler @asmFlags '-c' $source '-o' $object
    if ($LASTEXITCODE -ne 0) { throw "汇编失败：$relative (exit $LASTEXITCODE)" }
    $objects.Add($object)
}

$elf = Join-Path $buildDir 'CH32-of-FreeRTOS.elf'
$hex = Join-Path $buildDir 'CH32-of-FreeRTOS.hex'
$map = Join-Path $buildDir 'CH32-of-FreeRTOS.map'
$linkArgs = @(
    $cpuFlags + $commonFlags + @('-nostartfiles', '--specs=nano.specs', '--specs=nosys.specs',
        '-Wl,--gc-sections', "-Wl,-Map=$map", '-T', (Join-Path $root 'Ld/Link.ld'),
        "-L$(Join-Path $root 'zf_device')") + $objects.ToArray() + @('-Wl,--start-group', '-lm', '-lzf_device_config', '-Wl,--end-group', '-o', $elf)
)
Write-Host 'LD  build/CH32-of-FreeRTOS.elf'
& $Compiler @linkArgs
if ($LASTEXITCODE -ne 0) { throw "链接失败 (exit $LASTEXITCODE)" }

& $objcopy '-O' 'ihex' $elf $hex
if ($LASTEXITCODE -ne 0) { throw "HEX 转换失败 (exit $LASTEXITCODE)" }
& $sizeTool $elf
if ($LASTEXITCODE -ne 0) { throw "size 工具执行失败 (exit $LASTEXITCODE)" }
Write-Host "构建成功：$elf"
Write-Host "HEX 文件：$hex"
