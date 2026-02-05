@echo off
REM
REM 内存检测脚本 - Windows版本
REM 用于编译和检测 dotenv-cpp 内存练习项目的内存问题
REM
REM 使用方法:
REM   check_memory.bat v1    检测问题1
REM   check_memory.bat v2    检测问题2
REM   check_memory.bat v3    检测问题3
REM   check_memory.bat v4    检测问题4
REM

setlocal enabledelayedexpansion

REM ============================================
REM 检查参数
REM ============================================
if "%1"=="" (
    echo [错误] 使用方法: %0 ^<version^>
    echo.
    echo 示例:
    echo   %0 v1    检测问题1
    echo   %0 v2    检测问题2
    echo   %0 v3    检测问题3
    echo   %0 v4    检测问题4
    exit /b 1
)

set VERSION=%1

REM 验证版本号
if not "%VERSION%"=="v1" if not "%VERSION%"=="v2" if not "%VERSION%"=="v3" if not "%VERSION%"=="v4" (
    echo [错误] 无效的版本号: %VERSION%
    echo 有效的版本号: v1, v2, v3, v4
    exit /b 1
)

echo.
echo =========================================
echo   内存检测工具 - dotenv-cpp 练习
echo =========================================
echo.

REM ============================================
REM 检查环境
REM ============================================
echo [信息] 检查开发环境...

REM 检查CMake
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [错误] CMake未安装，请先安装CMake (^>= 3.10^)
    exit /b 1
)
for /f "tokens=3" %%i in ('cmake --version ^| findstr /C:"cmake version"') do set CMAKE_VERSION=%%i
echo [成功] CMake版本: %CMAKE_VERSION%

REM 检查编译器（MSVC或MinGW）
where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [成功] 编译器: MSVC
    set COMPILER=MSVC
    goto :compiler_found
)

where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /f "delims=" %%i in ('g++ --version ^| findstr /C:"g++"') do set GCC_VERSION=%%i
    echo [成功] 编译器: !GCC_VERSION!
    set COMPILER=MinGW
    goto :compiler_found
)

echo [错误] 未找到C++编译器（MSVC或MinGW）
exit /b 1

:compiler_found

REM 检查Dr.Memory（可选）
where drmemory >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [成功] Dr.Memory已安装（可选工具）
    set HAS_DRMEMORY=1
) else (
    echo [警告] Dr.Memory未安装（可选，建议安装以获得更详细的报告）
    set HAS_DRMEMORY=0
)

echo.

REM ============================================
REM 编译代码
REM ============================================
echo [信息] 编译代码...

REM 创建build目录
if not exist "build" (
    mkdir build
    echo [信息] 创建build目录
)

cd build

REM 运行CMake配置
echo [信息] 运行CMake配置（启用AddressSanitizer）...
if "%COMPILER%"=="MSVC" (
    cmake .. -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug -G "NMake Makefiles" >nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo [错误] CMake配置失败
        cd ..
        exit /b 1
    )
) else (
    cmake .. -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" >nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo [错误] CMake配置失败
        cd ..
        exit /b 1
    )
)

REM 编译指定版本
echo [信息] 编译 config_%VERSION%_buggy...
if "%COMPILER%"=="MSVC" (
    nmake config_%VERSION%_buggy >nul 2>&1
) else (
    mingw32-make config_%VERSION%_buggy >nul 2>&1
)

if exist "config_%VERSION%_buggy.exe" (
    echo [成功] 编译成功
) else (
    echo [错误] 编译失败
    cd ..
    exit /b 1
)

cd ..
echo.

REM ============================================
REM 运行AddressSanitizer测试
REM ============================================
echo [信息] 运行AddressSanitizer检测...

REM 设置ASan环境变量
set ASAN_OPTIONS=detect_leaks=1:symbolize=1:abort_on_error=0

REM 运行程序并捕获输出
cd build
config_%VERSION%_buggy.exe > output.txt 2>&1
set TEST_RESULT=%ERRORLEVEL%
cd ..

REM 检查输出
findstr /C:"ERROR: AddressSanitizer" build\output.txt >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [错误] 检测到内存问题！
    echo.
    echo === AddressSanitizer 报告 ===
    type build\output.txt | findstr /C:"ERROR:" /C:"SUMMARY:"
    echo.
    set ASAN_RESULT=1
) else (
    findstr /C:"ERROR: LeakSanitizer" build\output.txt >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        echo [错误] 检测到内存泄露！
        echo.
        echo === LeakSanitizer 报告 ===
        type build\output.txt | findstr /C:"ERROR:" /C:"LeakSanitizer"
        echo.
        set ASAN_RESULT=1
    ) else (
        echo [成功] AddressSanitizer未检测到问题
        set ASAN_RESULT=0
    )
)

REM ============================================
REM 运行Dr.Memory测试（可选）
REM ============================================
set DRMEMORY_RESULT=0
if %HAS_DRMEMORY% EQU 1 (
    echo [信息] 运行Dr.Memory检测（可能需要较长时间）...
    cd build
    drmemory -- config_%VERSION%_buggy.exe > drmemory_output.txt 2>&1
    cd ..

    findstr /C:"ERRORS FOUND" build\drmemory_output.txt >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        echo [错误] Dr.Memory检测到问题！
        echo.
        echo === Dr.Memory 摘要 ===
        type build\drmemory_output.txt | findstr /C:"ERRORS"
        echo.
        set DRMEMORY_RESULT=1
    ) else (
        echo [成功] Dr.Memory检测完成
    )
) else (
    echo [警告] 跳过Dr.Memory测试（未安装）
)

echo.

REM ============================================
REM 生成报告
REM ============================================
echo.
echo =========================================
echo            检测报告 - 版本 %VERSION%
echo =========================================

if %ASAN_RESULT% EQU 0 (
    echo [成功] AddressSanitizer: 通过
) else (
    echo [错误] AddressSanitizer: 发现问题
)

if %HAS_DRMEMORY% EQU 1 (
    if %DRMEMORY_RESULT% EQU 0 (
        echo [成功] Dr.Memory: 通过
    ) else (
        echo [错误] Dr.Memory: 发现问题
    )
)

echo.
if %ASAN_RESULT% NEQ 0 (
    echo [建议] 修复步骤：
    echo   1. 查看上方的详细报告，定位问题代码行号
    echo   2. 参考文档: docs\0%VERSION:~1%-problem-*.md
    echo   3. 使用GitHub Copilot分析和修复问题
    echo   4. 重新运行此脚本验证修复效果
) else (
    echo [成功] 恭喜！未检测到内存问题
    echo.
    echo   如果这是问题代码（buggy版本），请确认：
    echo   - 是否使用了正确的可执行文件？
    echo   - 是否已经修复了代码？
)

echo =========================================
echo.

REM 清理临时文件
if exist build\output.txt del build\output.txt
if exist build\drmemory_output.txt del build\drmemory_output.txt

REM 返回状态码
if %ASAN_RESULT% NEQ 0 (
    exit /b 1
) else (
    exit /b 0
)
