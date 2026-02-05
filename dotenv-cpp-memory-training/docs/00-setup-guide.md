# 环境配置指南

## 1. 系统要求

### 操作系统
- Linux（推荐 Ubuntu 18.04+, Fedora 30+, Debian 10+）
- macOS（推荐 10.14+）
- Windows（推荐 Windows 10/11）

### 编译器
- **GCC**: 7.0 或更高版本
- **Clang**: 5.0 或更高版本
- **MSVC**: Visual Studio 2017 或更高版本

### 必需工具
- **CMake**: 3.10 或更高版本
- **Git**: 任意最新版本
- **Make**: Linux/Mac 默认自带，Windows 需要安装

---

## 2. Linux 安装步骤

### 2.1 安装编译器和 CMake

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git
sudo apt-get install -y g++ clang
```

**Fedora:**
```bash
sudo dnf install -y gcc gcc-c++ cmake git
sudo dnf install -y clang
```

### 2.2 验证编译器支持 AddressSanitizer

AddressSanitizer（ASan）通常随现代编译器自动安装：

```bash
# 验证 GCC
gcc --version
# 应该显示 7.0 或更高版本

# 验证 ASan 支持
echo 'int main() { int *p = new int; return 0; }' | g++ -fsanitize=address -x c++ - -o test_asan && ./test_asan
echo $?
# 如果返回 0 或 1（ASan 检测到泄露），说明 ASan 工作正常
rm -f test_asan
```

### 2.3 可选：安装 Valgrind

Valgrind 提供更详细的内存分析报告（性能开销较大）：

```bash
# Ubuntu/Debian
sudo apt-get install -y valgrind

# Fedora
sudo dnf install -y valgrind

# 验证安装
valgrind --version
```

---

## 3. macOS 安装步骤

### 3.1 安装 Xcode Command Line Tools

```bash
xcode-select --install
```

按照提示完成安装。

### 3.2 使用 Homebrew 安装工具

如果尚未安装 Homebrew，请访问 [https://brew.sh](https://brew.sh) 安装。

```bash
# 安装 CMake
brew install cmake

# 安装 GCC（可选，Clang 已随 Xcode 安装）
brew install gcc

# 验证安装
clang --version
cmake --version
```

### 3.3 可选：安装 Valgrind

```bash
brew install valgrind
```

**注意：** Valgrind 在较新版本的 macOS 上可能不完全支持，建议优先使用 AddressSanitizer。

---

## 4. Windows 安装步骤

### 4.1 选项 A：Visual Studio（推荐）

1. 下载并安装 [Visual Studio 2019 或 2022 Community Edition](https://visualstudio.microsoft.com/)
2. 在安装过程中，选择"使用 C++ 的桌面开发"工作负载
3. 确保勾选以下组件：
   - MSVC v142 或更高版本的编译器
   - Windows 10 SDK
   - CMake 工具

### 4.2 选项 B：MinGW-w64 + CMake

1. 下载并安装 [MinGW-w64](https://www.mingw-w64.org/)
2. 下载并安装 [CMake for Windows](https://cmake.org/download/)
3. 将 MinGW-w64 和 CMake 添加到系统 PATH

**验证安装：**
```cmd
g++ --version
cmake --version
```

### 4.3 可选：安装 Dr.Memory

Dr.Memory 是 Windows 上的 Valgrind 替代品：

1. 下载 [Dr.Memory](https://drmemory.org/page_download.html)
2. 解压并添加到系统 PATH
3. 验证安装：
```cmd
drmemory -version
```

---

## 5. 验证安装

### 5.1 快速验证脚本

**Linux/Mac:**
```bash
#!/bin/bash

echo "=== 验证环境配置 ==="

# 检查编译器
if command -v g++ &> /dev/null; then
    echo "✅ GCC 已安装: $(g++ --version | head -n1)"
else
    echo "❌ GCC 未找到"
fi

if command -v clang++ &> /dev/null; then
    echo "✅ Clang 已安装: $(clang++ --version | head -n1)"
else
    echo "⚠️  Clang 未找到（可选）"
fi

# 检查 CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "✅ CMake 已安装: $CMAKE_VERSION"
else
    echo "❌ CMake 未找到"
fi

# 检查 ASan 支持
echo "正在测试 AddressSanitizer 支持..."
echo 'int main() { int *p = new int; return 0; }' | g++ -fsanitize=address -x c++ - -o /tmp/test_asan 2>&1
if [ $? -eq 0 ]; then
    echo "✅ AddressSanitizer 支持正常"
    rm -f /tmp/test_asan
else
    echo "❌ AddressSanitizer 不可用"
fi

# 检查 Valgrind（可选）
if command -v valgrind &> /dev/null; then
    echo "✅ Valgrind 已安装: $(valgrind --version)"
else
    echo "⚠️  Valgrind 未找到（可选工具）"
fi

echo "=== 验证完成 ==="
```

将上述脚本保存为 `verify_env.sh`，运行：
```bash
chmod +x verify_env.sh
./verify_env.sh
```

**Windows (PowerShell):**
```powershell
Write-Host "=== 验证环境配置 ===" -ForegroundColor Cyan

# 检查编译器
try {
    $gccVersion = (g++ --version 2>&1 | Select-Object -First 1)
    Write-Host "✅ GCC 已安装: $gccVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ GCC 未找到" -ForegroundColor Red
}

# 检查 CMake
try {
    $cmakeVersion = (cmake --version 2>&1 | Select-Object -First 1)
    Write-Host "✅ CMake 已安装: $cmakeVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ CMake 未找到" -ForegroundColor Red
}

Write-Host "=== 验证完成 ===" -ForegroundColor Cyan
```

---

## 6. 克隆 dotenv-cpp 项目

### 6.1 克隆代码库

```bash
# 克隆项目
cd ~/projects  # 或您的工作目录
git clone https://github.com/laserpants/dotenv-cpp.git
cd dotenv-cpp

# 查看目录结构
ls -la
```

预期输出：
```
.
├── include/
│   └── laserpants/
│       └── dotenv/
│           └── dotenv.h
├── tests/
├── CMakeLists.txt
├── dotenv-cpp-memory-training/  # 我们的培训材料
├── LICENSE
└── README.md
```

### 6.2 进入培训材料目录

```bash
cd dotenv-cpp-memory-training
ls -la
```

预期目录结构：
```
.
├── docs/               # 培训文档
├── src/                # 源代码
│   ├── buggy/         # 包含问题的代码
│   ├── fixed/         # 修复后的参考答案
│   └── common/        # 共享头文件
├── tests/             # 测试代码
├── scripts/           # 验证脚本
├── env-files/         # 示例配置文件
├── CMakeLists.txt     # 构建配置
└── README.md          # 项目总览
```

---

## 7. 首次构建

### 7.1 构建所有问题代码

**Linux/Mac:**
```bash
cd dotenv-cpp-memory-training

# 创建构建目录
mkdir build
cd build

# 配置项目（启用 AddressSanitizer）
cmake .. -DENABLE_ASAN=ON

# 编译所有代码
make

# 查看生成的可执行文件
ls -la
```

预期输出：
```
config_v1_buggy        # 问题 1：基本内存泄露
config_v2_buggy        # 问题 2：double-free
config_v3_buggy        # 问题 3：悬挂指针
config_v4_buggy        # 问题 4：异常安全
config_v1_fixed        # 问题 1 的修复版本
config_v2_fixed        # 问题 2 的修复版本
config_v3_fixed        # 问题 3 的修复版本
config_v4_fixed        # 问题 4 的修复版本
test_v1                # 测试程序
test_v2
test_v3
test_v4
```

**Windows (CMD):**
```cmd
cd dotenv-cpp-memory-training
mkdir build
cd build

# 使用 Visual Studio 生成器
cmake .. -G "Visual Studio 16 2019" -DENABLE_ASAN=ON

# 编译
cmake --build . --config Debug

# 查看生成的可执行文件
dir Debug\*.exe
```

### 7.2 测试第一个问题

运行验证脚本检测问题 1：

**Linux/Mac:**
```bash
cd ..  # 返回到 dotenv-cpp-memory-training 目录
./scripts/check_memory.sh v1
```

**Windows:**
```cmd
cd ..
scripts\check_memory.bat v1
```

**预期输出（检测到内存泄露）：**
```
=================================================================
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 256 byte(s) in 1 object(s) allocated from:
    #0 0x7f... in operator new[](unsigned long)
    #1 0x7f... in ConfigManager::ConfigManager(char const*)
    #2 0x7f... in main

SUMMARY: AddressSanitizer: 256 byte(s) leaked in 1 allocation(s).
```

如果看到类似的错误报告，说明环境配置成功！🎉

---

## 8. 常见问题排查

### 问题 1：找不到 dotenv.h

**错误信息：**
```
fatal error: laserpants/dotenv/dotenv.h: No such file or directory
```

**解决方法：**
1. 确认您在正确的目录结构中（`dotenv-cpp-memory-training` 应该在 `dotenv-cpp` 项目内）
2. 检查 `CMakeLists.txt` 中的 `include_directories` 配置
3. 确认路径：
```bash
ls ../include/laserpants/dotenv/dotenv.h
# 应该输出：../include/laserpants/dotenv/dotenv.h
```

### 问题 2：AddressSanitizer 不可用

**错误信息：**
```
unrecognized argument: -fsanitize=address
```

**解决方法：**
1. 检查编译器版本（需要 GCC 7+ 或 Clang 5+）
```bash
g++ --version
clang++ --version
```

2. 如果版本过低，升级编译器：
```bash
# Ubuntu
sudo apt-get install -y gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
```

3. 如果仍然不可用，可以禁用 ASan 编译（不推荐）：
```bash
cmake .. -DENABLE_ASAN=OFF
make
```

### 问题 3：权限不足

**错误信息：**
```
Permission denied: ./scripts/check_memory.sh
```

**解决方法：**
```bash
chmod +x scripts/check_memory.sh
chmod +x scripts/check_memory.bat
```

### 问题 4：Windows 上找不到 setenv

**错误信息：**
```
'setenv' was not declared in this scope
```

**解决方法：**
- 这是预期行为，`dotenv.h` 在 Windows 上使用 `_putenv_s` 而非 `setenv`
- 确保您使用的是最新版本的 `dotenv.h`（已包含 Windows 兼容处理）

### 问题 5：CMake 版本过低

**错误信息：**
```
CMake 3.10 or higher is required. You are running version 3.5
```

**解决方法：**
1. 升级 CMake：
```bash
# Ubuntu（使用 pip）
pip install --upgrade cmake

# macOS
brew upgrade cmake

# 或从官网下载最新版本：https://cmake.org/download/
```

---

## 9. 环境验证检查清单

在开始培训前，请确认以下所有项目：

- [ ] 编译器已安装（GCC 7+ 或 Clang 5+）
- [ ] CMake 版本 >= 3.10
- [ ] 能够成功编译 `config_v1_buggy`
- [ ] 运行 `./scripts/check_memory.sh v1` 检测到内存泄露
- [ ] 如果在 Windows 上，确认使用 MSVC 或 MinGW-w64
- [ ] 可选：Valgrind 或 Dr.Memory 已安装并可用
- [ ] GitHub Copilot 已在您的 IDE 中启用（VS Code, Visual Studio, 或其他）

---

## 10. IDE 配置建议

### Visual Studio Code

1. 安装推荐扩展：
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft)
   - GitHub Copilot

2. 配置 `settings.json`：
```json
{
  "cmake.configureOnOpen": true,
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
  "github.copilot.enable": {
    "*": true,
    "cpp": true
  }
}
```

### Visual Studio 2019/2022

1. 打开 `dotenv-cpp-memory-training` 文件夹（文件 → 打开 → 文件夹）
2. 确认 GitHub Copilot 扩展已安装并激活
3. CMake 集成会自动识别 `CMakeLists.txt`

### CLion

1. 打开 `dotenv-cpp-memory-training` 目录
2. 启用 GitHub Copilot 插件（Settings → Plugins → 搜索 "GitHub Copilot"）
3. CMake 配置会自动加载

---

## 11. 下一步

环境配置完成后，请进入第一个问题：

👉 [01-problem-basic-leak.md](./01-problem-basic-leak.md)

---

**提示：** 如果在配置过程中遇到任何问题，请参考 [05-verification-tools.md](./05-verification-tools.md) 获取更详细的工具使用指南。
