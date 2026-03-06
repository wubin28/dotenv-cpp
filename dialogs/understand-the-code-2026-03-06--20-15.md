# dotenv-cpp 项目理解指南

> 生成时间：2026-03-06--20-15  
> 实测环境：Windows 11 + Visual Studio 2026 Developer PowerShell v18.2.1 (Community) + PowerShell

---

## Q1：本项目的主要功能、特点、优势、劣势和适用场景

### 主要功能

`dotenv-cpp` 是 [dotenv](https://github.com/motdotla/dotenv)（Node.js 生态）的 C++ 移植版。它的核心功能是：**从 `.env` 文件中读取键值对，并将其设置为进程的环境变量**，从而可以用 `std::getenv()` 在代码中读取它们。

具体支持的能力：

| 功能 | 说明 |
|------|------|
| 读取 `.env` 文件 | `dotenv::init()` 默认读取当前目录下的 `.env` 文件，也可指定文件路径 |
| 带默认值的读取 | `dotenv::getenv("KEY", "default")` — 当变量未定义时返回默认值 |
| 变量引用展开 | 支持 `$VAR` 和 `${VAR}` 语法，在 `.env` 的值中引用其他变量 |
| Preserve 模式 | `dotenv::init(dotenv::Preserve)` — 当环境变量已存在时，**不**覆盖它 |
| 引号处理 | 值中的双引号会被自动去除（`"antipasto"` → `antipasto`） |

### 特点

- **Header-Only**：整个库只有一个头文件 `include/laserpants/dotenv/dotenv.h`，无需编译、无需链接静态库，直接 `#include` 即可使用
- **MSVC 兼容**：从 0.9.1 版本起已添加 MSVC 下 `setenv` 的包装（`_putenv_s`），可在 Windows/Visual Studio 环境下正常工作
- **零依赖**：只依赖 C++ 标准库（`<string>`、`<fstream>`、`<cstdlib>` 等）

### 优势

1. **集成极简**：只需将 `include/` 目录复制到项目中，加一行 `#include <dotenv.h>` 即可，无额外构建步骤
2. **配置与代码分离**：遵循 [12-Factor App](https://12factor.net/config) 原则，敏感配置（密码、API Key）不写入代码，通过 `.env` 文件管理
3. **开发环境友好**：`.env` 文件可加入 `.gitignore`，每个开发者/服务器维护自己的配置，互不干扰
4. **跨平台**：Linux/macOS/Windows（MSVC）均支持
5. **代码量极小**：源码约 300 行，易于审计和理解

### 劣势

1. **无加密**：`.env` 文件是明文，不适合直接在生产服务器上存储高机密信息（应结合密钥管理服务）
2. **进程级作用域**：设置的环境变量仅对当前进程及其子进程有效，不会写入系统环境变量
3. **格式简单**：不支持多行值、不支持注释嵌套、不支持 JSON/YAML 等结构化格式
4. **无类型转换**：所有值均为字符串，需要开发者自行转换为 `int`、`bool` 等类型
5. **MSVC 警告**：使用 `std::getenv` 在 MSVC 下会产生 C4996 弃用警告，需要加 `/D_CRT_SECURE_NO_WARNINGS` 消除

### 适用场景

- ✅ 本地开发环境配置（数据库地址、API 密钥、调试开关等）
- ✅ CI/CD 流水线中注入构建参数
- ✅ 小型工具/命令行程序的配置管理
- ✅ 希望遵循 12-Factor App 规范的 C++ 项目
- ❌ 不适合需要加密配置的生产环境（应使用密钥管理服务）
- ❌ 不适合需要复杂配置结构（嵌套、数组）的场景（应考虑 `nlohmann/json` 或 TOML 库）

---

## Q2：在 Windows + PowerShell 中编译并运行项目

### 前提条件

- Visual Studio 2022 或更高版本，已安装 **"使用 C++ 的桌面开发"** 工作负载
- CMake（VS 安装时自带，或从 [cmake.org](https://cmake.org) 单独安装）
- 网络连接（CMake 配置时会自动从 GitHub 下载 GoogleTest）

### 步骤一：打开 VS 开发者 PowerShell

在普通 PowerShell 中执行以下命令，导入 cl.exe 等编译工具的环境变量：

```powershell
# 如果安装的是 VS 2022 (版本号 17)：
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64

# 如果安装的是 VS 2026 (版本号 18，本机实测)：
& "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64
```

> **验证**：执行后终端提示符会变化，并显示 "Visual Studio Developer PowerShell"。  
> 可运行 `cl.exe` 确认编译器可用，应看到版本信息输出。

### 步骤二：切换到项目根目录

```powershell
Set-Location "C:\Users\wubin\OOR\katas\dotenv-cpp"
# 或者你的实际项目路径
```

### 步骤三：创建必要的示例文件

运行项目需要 `.env` 文件和 `example.cpp`。

**创建 `.env` 文件**（项目根目录下）：

```
DATABASE_HOST=localhost
DATABASE_USERNAME=user
DATABASE_PASSWORD="antipasto"
```

**创建 `example.cpp`**（项目根目录下）：

```cpp
#include <iostream>
#include <dotenv.h>

int main()
{
    dotenv::init();

    std::cout << std::getenv("DATABASE_USERNAME") << std::endl;
    std::cout << std::getenv("DATABASE_PASSWORD") << std::endl;

    return 0;
}
```

> 这两个文件在当前项目中已创建，如果你是全新克隆的项目，需要手动创建。

### 步骤四：用 cl.exe 直接编译（展示 header-only 特性）

因为 dotenv-cpp 是 header-only 库，**不需要 CMake 就能编译**，只需一条命令：

```powershell
# 在项目根目录执行
cl.exe /EHsc /W3 /D_CRT_SECURE_NO_WARNINGS /I include\laserpants\dotenv example.cpp /Fe:example.exe
```

参数说明：
| 参数 | 含义 |
|------|------|
| `/EHsc` | 启用 C++ 标准异常处理 |
| `/W3` | 启用 3 级警告 |
| `/D_CRT_SECURE_NO_WARNINGS` | 消除 `getenv` 的 MSVC C4996 弃用警告 |
| `/I include\laserpants\dotenv` | 指定头文件搜索路径 |
| `/Fe:example.exe` | 指定输出可执行文件名 |

### 步骤五：运行示例程序

```powershell
.\example.exe
```

**实测输出**（与 README 完全一致）：

```
user
antipasto
```

**解读**：程序读取了 `.env` 文件中的 `DATABASE_USERNAME=user` 和 `DATABASE_PASSWORD="antipasto"`（双引号在解析时被自动去除），并输出到终端。

---

## Q3：如何运行自动化测试

### 前提：创建 .env.example 文件

测试代码（`tests/base_test.cc`）使用 `.env.example` 作为测试数据源，**该文件不在仓库中，必须手动创建**，否则部分测试会失败。

在项目根目录创建 `.env.example`，内容如下：

```
DEFINED_VAR=OLHE
BASE=hello
EXPANDED=hello world
```

> 上述内容是根据 `base_test.cc` 中的断言反推得出的：  
> - `DEFINED_VAR` 的值应为 `OLHE`（测试 `ReadDefinedVariableWithDefaultValue`）  
> - `BASE=hello`、`EXPANDED=hello world`（测试 `VariableReferenceExpansion`）

### 步骤一：CMake 配置（开启测试）

```powershell
# 在项目根目录执行（确保已进入 VS 开发者 PowerShell）
# VS 2022 环境使用：
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON -DBUILD_DOCS=OFF

# VS 2026 环境使用（本机实测）：
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DBUILD_TESTS=ON -DBUILD_DOCS=OFF
```

> **注意**：`-DBUILD_DOCS=OFF` 关闭文档生成，避免因未安装 Doxygen 而报错。  
> CMake 会自动从 GitHub 下载 GoogleTest（需要网络）。若 `build/` 目录已存在且之前未开启测试，需重新配置。

### 步骤二：编译测试

```powershell
cmake --build build --config Debug --target tests
```

编译成功后，测试可执行文件位于：`build\Debug\tests.exe`

### 步骤三：运行测试

```powershell
Set-Location build
ctest -C Debug --output-on-failure
```

**实测输出**：

```
Test project C:/Users/wubin/OOR/katas/dotenv-cpp/build
    Start 1: BaseTestFixture.ReadUndefinedVariableWithDefaultValue
1/6 Test #1: BaseTestFixture.ReadUndefinedVariableWithDefaultValue ...   Passed    0.01 sec
    Start 2: BaseTestFixture.ReadDefinedVariableWithDefaultValue
2/6 Test #2: BaseTestFixture.ReadDefinedVariableWithDefaultValue .....   Passed    0.01 sec
    Start 3: BaseTestFixture.VariableReferenceExpansion
3/6 Test #3: BaseTestFixture.VariableReferenceExpansion ..............   Passed    0.01 sec
    Start 4: DotenvPreserveTest.PreserveExistingVariable
4/6 Test #4: DotenvPreserveTest.PreserveExistingVariable .............   Passed    0.01 sec
    Start 5: DotenvErrorTest.InvalidFileDoesNotCrash
5/6 Test #5: DotenvErrorTest.InvalidFileDoesNotCrash .................   Passed    0.01 sec
    Start 6: DotenvErrorTest.MalformedLineIgnored
6/6 Test #6: DotenvErrorTest.MalformedLineIgnored ....................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 6

Total Test time (real) =   0.08 sec
```

**6/6 测试全部通过**。

### 测试覆盖说明

| 测试名称 | 验证内容 |
|----------|----------|
| `ReadUndefinedVariableWithDefaultValue` | 当变量不存在时，`dotenv::getenv()` 返回默认值 |
| `ReadDefinedVariableWithDefaultValue` | 当变量存在时，`dotenv::getenv()` 返回实际值而非默认值 |
| `VariableReferenceExpansion` | `$VAR` 变量引用语法能正确展开 |
| `PreserveExistingVariable` | `Preserve` 模式不覆盖已有环境变量 |
| `InvalidFileDoesNotCrash` | 加载不存在的 `.env` 文件不会崩溃 |
| `MalformedLineIgnored` | 格式不正确的行被忽略，不影响其他变量加载 |

---

## 附：编译环境信息（本机实测）

| 项目 | 版本 |
|------|------|
| 操作系统 | Windows 11 |
| Visual Studio | Visual Studio 2026 Developer PowerShell v18.2.1 (Community) |
| cl.exe | Version 19.50.35723 for x64 |
| CMake | 4.1（VS 内置） |
| CMake 生成器 | Visual Studio 18 2026, x64 |
| dotenv-cpp 版本 | 0.9.3 |

> **注意**：如果你安装的是 VS 2022（版本 17），请将上表中的 `18` 替换为 `17`，路径中的 `2026` 替换为 `2022`，CMake 生成器使用 `"Visual Studio 17 2022"`。
