# 验证工具使用指南

本文档详细介绍用于检测 C++ 内存问题的各种工具，包括 AddressSanitizer、Valgrind、编译器警告和静态分析工具。

---

## 1. 工具概览

### 1.1 工具对比表

| 工具 | 平台 | 检测能力 | 性能开销 | 易用性 | 安装难度 |
|------|------|----------|----------|--------|----------|
| **AddressSanitizer** | Linux/Mac/Windows | 内存泄露、越界访问、UAF | 2x | ⭐⭐⭐⭐⭐ | 简单（随编译器） |
| **Valgrind** | Linux/Mac | 详细泄露报告、未初始化内存 | 10-20x | ⭐⭐⭐⭐ | 简单（包管理器） |
| **Dr.Memory** | Windows/Linux | 类似 Valgrind | 5-10x | ⭐⭐⭐ | 中等 |
| **编译器警告** | 全平台 | 潜在问题、未使用变量 | 无 | ⭐⭐⭐⭐⭐ | 无（内置） |
| **Clang Static Analyzer** | 全平台 | 静态分析、路径敏感 | 无运行时 | ⭐⭐⭐ | 简单 |
| **Cppcheck** | 全平台 | 静态分析、代码质量 | 无运行时 | ⭐⭐⭐⭐ | 简单 |

**推荐组合：**
- **日常开发**：AddressSanitizer + 编译器警告
- **深度分析**：Valgrind（Linux/Mac）或 Dr.Memory（Windows）
- **代码审查**：静态分析工具

---

## 2. AddressSanitizer 详解

### 2.1 什么是 AddressSanitizer？

**AddressSanitizer（ASan）** 是由 Google 开发的快速内存错误检测工具，集成在 GCC 和 Clang 编译器中。

**检测能力：**
- ✅ 堆内存越界访问（heap buffer overflow）
- ✅ 栈内存越界访问（stack buffer overflow）
- ✅ 全局变量越界访问（global buffer overflow）
- ✅ Use-after-free（使用已释放的内存）
- ✅ Use-after-return（使用已销毁的栈内存）
- ✅ Use-after-scope（使用超出作用域的变量）
- ✅ Double-free（重复释放）
- ✅ 内存泄露（memory leaks）

### 2.2 工作原理简介

**影子内存（Shadow Memory）：**
- ASan 为每 8 字节的应用内存分配 1 字节的影子内存
- 影子内存记录对应地址是否可访问
- 每次内存访问前，检查影子内存

**示例：**
```
应用内存：[0x1000 - 0x1007] 已分配
影子内存：[shadow(0x1000)] = 0 (可访问)

应用内存：[0x1008 - 0x100f] 未分配
影子内存：[shadow(0x1008)] = 非 0 (不可访问)

访问 0x1005：检查 shadow(0x1000) == 0 ✅ 允许
访问 0x1009：检查 shadow(0x1008) != 0 ❌ 报错
```

### 2.3 编译选项详解

#### 基本选项

```bash
g++ -fsanitize=address -fno-omit-frame-pointer -g -O1 source.cpp -o program
```

**选项说明：**

| 选项 | 作用 | 必需？ |
|------|------|--------|
| `-fsanitize=address` | 启用 AddressSanitizer | ✅ 必需 |
| `-fno-omit-frame-pointer` | 保留栈帧指针，提供更好的栈跟踪 | 推荐 |
| `-g` | 包含调试符号，显示源文件行号 | 推荐 |
| `-O1` | 轻量优化，平衡性能和检测能力 | 可选 |

#### 高级选项

```bash
# 同时检测泄露和未定义行为
g++ -fsanitize=address,undefined -g source.cpp -o program

# 只检测泄露（无其他开销）
g++ -fsanitize=leak -g source.cpp -o program
```

### 2.4 环境变量配置

**ASAN_OPTIONS** 可以自定义 ASan 的行为：

```bash
# 检测内存泄露
export ASAN_OPTIONS=detect_leaks=1

# 立即退出（不等待程序结束）
export ASAN_OPTIONS=halt_on_error=1

# 保存日志到文件
export ASAN_OPTIONS=log_path=/tmp/asan.log

# 组合多个选项
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:log_path=/tmp/asan.log
```

**常用选项：**

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `detect_leaks` | 1（Linux）、0（Mac） | 是否检测内存泄露 |
| `halt_on_error` | 0 | 遇到错误是否立即退出 |
| `log_path` | stderr | 日志输出路径 |
| `symbolize` | 1 | 是否符号化栈跟踪 |
| `abort_on_error` | 0 | 错误时是否调用 abort() |

### 2.5 报告解读

#### 示例 1：内存泄露

**代码：**
```cpp
int main() {
    int* ptr = new int[10];
    return 0;  // 忘记 delete[]
}
```

**ASan 报告：**
```
=================================================================
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 40 byte(s) in 1 object(s) allocated from:
    #0 0x7ffff7a9a678 in operator new[](unsigned long) (/usr/lib/x86_64-linux-gnu/libasan.so.5+0xe9678)
    #1 0x555555555169 in main /path/to/source.cpp:2
    #2 0x7ffff7a0509a in __libc_start_main ../csu/libc-start.c:308

SUMMARY: AddressSanitizer: 40 byte(s) leaked in 1 allocation(s).
```

**解读：**
1. **Direct leak of 40 byte(s)**：直接泄露 40 字节（10 个 int）
2. **operator new[]**：通过 `new[]` 分配
3. **source.cpp:2**：分配发生在源文件第 2 行
4. **in 1 allocation(s)**：1 次分配未释放

---

#### 示例 2：Use-After-Free

**代码：**
```cpp
int main() {
    int* ptr = new int;
    delete ptr;
    *ptr = 10;  // 使用已释放的内存
    return 0;
}
```

**ASan 报告：**
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010 at pc 0x555555555189

WRITE of size 4 at 0x602000000010 thread T0
    #0 0x555555555188 in main /path/to/source.cpp:4

0x602000000010 is located 0 bytes inside of 4-byte region [0x602000000010,0x602000000014)
freed by thread T0 here:
    #0 0x7ffff7a9b458 in operator delete(void*) (/usr/lib/x86_64-linux-gnu/libasan.so.5+0xea458)
    #1 0x555555555179 in main /path/to/source.cpp:3

previously allocated by thread T0 here:
    #0 0x7ffff7a9a628 in operator new(unsigned long) (/usr/lib/x86_64-linux-gnu/libasan.so.5+0xe9628)
    #1 0x55555555516a in main /path/to/source.cpp:2

SUMMARY: AddressSanitizer: heap-use-after-free /path/to/source.cpp:4 in main
```

**解读：**
1. **heap-use-after-free**：使用已释放的堆内存
2. **WRITE of size 4**：写入 4 字节（int）
3. **source.cpp:4**：错误使用发生在第 4 行
4. **freed by ... source.cpp:3**：内存在第 3 行被释放
5. **previously allocated ... source.cpp:2**：内存在第 2 行分配

---

#### 示例 3：Double-Free

**代码：**
```cpp
int main() {
    int* ptr = new int;
    delete ptr;
    delete ptr;  // 重复释放
    return 0;
}
```

**ASan 报告：**
```
=================================================================
==12345==ERROR: AddressSanitizer: attempting double-free on 0x602000000010 in thread T0:
    #0 0x7ffff7a9b458 in operator delete(void*) (/usr/lib/x86_64-linux-gnu/libasan.so.5+0xea458)
    #1 0x55555555517a in main /path/to/source.cpp:4

0x602000000010 is located 0 bytes inside of 4-byte region [0x602000000010,0x602000000014)
freed by thread T0 here:
    #0 0x7ffff7a9b458 in operator delete(void*) (/usr/lib/x86_64-linux-gnu/libasan.so.5+0xea458)
    #1 0x555555555179 in main /path/to/source.cpp:3

SUMMARY: AddressSanitizer: attempting double-free /path/to/source.cpp:4 in main
```

---

### 2.6 最佳实践

#### 在 CMake 中集成

```cmake
# CMakeLists.txt
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()
```

**使用：**
```bash
cmake .. -DENABLE_ASAN=ON
make
```

#### 在 CI/CD 中使用

```yaml
# .github/workflows/ci.yml
- name: Run tests with ASan
  run: |
    mkdir build && cd build
    cmake .. -DENABLE_ASAN=ON
    make
    ctest --output-on-failure
  env:
    ASAN_OPTIONS: detect_leaks=1:halt_on_error=1
```

---

## 3. Valgrind 详解

### 3.1 什么是 Valgrind？

**Valgrind** 是一个强大的内存调试和性能分析工具套件，包含多个工具：

- **Memcheck**：内存错误检测器（最常用）
- **Cachegrind**：缓存性能分析器
- **Callgrind**：调用图生成器
- **Helgrind**：线程错误检测器
- **Massif**：堆性能分析器

### 3.2 安装方法

```bash
# Ubuntu/Debian
sudo apt-get install valgrind

# Fedora
sudo dnf install valgrind

# macOS（注意：较新版本可能不完全支持）
brew install valgrind
```

### 3.3 基本用法

```bash
# 编译程序（需要调试符号）
g++ -g source.cpp -o program

# 运行 Memcheck
valgrind --leak-check=full --show-leak-kinds=all ./program
```

**选项说明：**

| 选项 | 作用 |
|------|------|
| `--leak-check=full` | 详细的泄露报告 |
| `--show-leak-kinds=all` | 显示所有类型的泄露 |
| `--track-origins=yes` | 追踪未初始化值的来源 |
| `--verbose` | 详细输出 |
| `--log-file=<file>` | 保存日志到文件 |

### 3.4 报告解读

#### 内存泄露类型

Valgrind 将泄露分为四类：

1. **Definitely lost（明确泄露）**
   - 无任何指针指向该内存
   - 最严重的泄露

2. **Indirectly lost（间接泄露）**
   - 通过指针可达，但根指针已丢失
   - 通常由"Definitely lost"引起

3. **Possibly lost（可能泄露）**
   - 存在指向内存中间位置的指针
   - 可能是误报（如自定义内存池）

4. **Still reachable（仍可达）**
   - 程序退出时仍有指针指向
   - 不是真正的泄露（程序退出时 OS 回收）

#### 示例报告

**代码：**
```cpp
int main() {
    int* ptr = new int[10];
    ptr = new int[20];  // 丢失了第一次分配的指针
    delete[] ptr;
    return 0;
}
```

**Valgrind 报告：**
```
==12345== 40 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2E0EF: operator new[](unsigned long) (vg_replace_malloc.c:423)
==12345==    by 0x40084B: main (source.cpp:2)
==12345==
==12345== LEAK SUMMARY:
==12345==    definitely lost: 40 bytes in 1 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
==12345==         suppressed: 0 bytes in 0 blocks
```

### 3.5 常用选项

#### 追踪未初始化内存

```bash
valgrind --track-origins=yes ./program
```

**示例报告：**
```
==12345== Conditional jump or move depends on uninitialised value(s)
==12345==    at 0x400850: main (source.cpp:5)
==12345==  Uninitialised value was created by a heap allocation
==12345==    at 0x4C2E0EF: operator new[](unsigned long) (vg_replace_malloc.c:423)
==12345==    by 0x40083B: main (source.cpp:2)
```

#### 生成日志文件

```bash
valgrind --log-file=valgrind_%p.log --leak-check=full ./program
# %p 会被替换为进程 ID
```

---

## 4. 编译器警告

### 4.1 推荐的警告选项组合

```bash
# GCC/Clang 推荐选项
g++ -Wall -Wextra -Wpedantic -Werror source.cpp -o program
```

**选项说明：**

| 选项 | 作用 |
|------|------|
| `-Wall` | 启用大部分常见警告 |
| `-Wextra` | 启用额外的警告 |
| `-Wpedantic` | 严格遵循 C++ 标准 |
| `-Werror` | 将警告视为错误 |
| `-Wshadow` | 警告变量遮蔽 |
| `-Wconversion` | 警告隐式类型转换 |
| `-Wunused` | 警告未使用的变量 |

### 4.2 具体警告示例

#### 未使用的变量

**代码：**
```cpp
int main() {
    int x = 10;  // 未使用
    return 0;
}
```

**警告：**
```
source.cpp:2:9: warning: unused variable 'x' [-Wunused-variable]
    int x = 10;
        ^
```

#### 返回局部变量的引用

**代码：**
```cpp
const int& func() {
    int x = 10;
    return x;  // 危险
}
```

**警告：**
```
source.cpp:3:12: warning: reference to local variable 'x' returned [-Wreturn-local-addr]
    return x;
           ^
```

#### 内存泄露提示

某些编译器（如 Clang）可以提示潜在的泄露：

**代码：**
```cpp
void func() {
    int* ptr = new int;
}  // 未 delete
```

**Clang 静态分析器警告：**
```
source.cpp:2:16: warning: Potential leak of memory pointed to by 'ptr'
    int* ptr = new int;
               ^~~~~~~
```

---

## 5. 静态分析工具

### 5.1 Clang Static Analyzer

#### 使用方法

```bash
# 方法 1：使用 scan-build
scan-build g++ source.cpp -o program

# 方法 2：使用 clang --analyze
clang++ --analyze source.cpp
```

#### 检测能力

- 空指针解引用
- 内存泄露
- 未初始化变量
- Dead code
- 逻辑错误

---

### 5.2 Cppcheck

#### 安装

```bash
# Ubuntu/Debian
sudo apt-get install cppcheck

# macOS
brew install cppcheck
```

#### 使用方法

```bash
# 检查单个文件
cppcheck source.cpp

# 检查整个项目
cppcheck --enable=all --inconclusive ./src

# 生成 XML 报告
cppcheck --xml --xml-version=2 ./src 2> cppcheck.xml
```

#### 示例输出

```
[source.cpp:5]: (error) Memory leak: ptr
[source.cpp:10]: (warning) Possible null pointer dereference: ptr
[source.cpp:15]: (style) Variable 'x' is assigned a value that is never used
```

---

## 6. 调试技巧

### 6.1 在 GDB 中使用

```bash
# 编译时启用调试符号
g++ -g -fsanitize=address source.cpp -o program

# 使用 GDB 运行
gdb ./program

# GDB 命令
(gdb) run                       # 运行程序
(gdb) break main                # 在 main 设置断点
(gdb) print ptr                 # 查看变量值
(gdb) x/10x ptr                 # 查看内存内容（16 进制）
(gdb) backtrace                 # 查看调用栈
```

### 6.2 追踪对象生命周期

**技巧：** 在构造函数和析构函数中打印日志

```cpp
class Tracker {
    int id_;
public:
    Tracker(int id) : id_(id) {
        std::cout << "Tracker(" << id_ << ") constructed" << std::endl;
    }

    ~Tracker() {
        std::cout << "Tracker(" << id_ << ") destructed" << std::endl;
    }

    Tracker(const Tracker& other) : id_(other.id_) {
        std::cout << "Tracker(" << id_ << ") copied" << std::endl;
    }
};

int main() {
    Tracker t1(1);
    {
        Tracker t2(2);
        Tracker t3 = t1;  // 拷贝
    }  // t2 和 t3 析构
    return 0;
}  // t1 析构
```

**输出：**
```
Tracker(1) constructed
Tracker(2) constructed
Tracker(1) copied
Tracker(2) destructed
Tracker(1) destructed
Tracker(1) destructed
```

---

## 7. 工具选择指南

### 7.1 决策树

```
需要检测内存问题？
  ├─ 是 → 日常开发？
  │        ├─ 是 → 使用 AddressSanitizer（快速）
  │        └─ 否 → 深度分析？
  │                 ├─ 是 → 使用 Valgrind（详细）
  │                 └─ 否 → 使用编译器警告
  └─ 否 → 代码审查？
           ├─ 是 → 使用静态分析工具
           └─ 否 → 性能分析？
                    └─ 使用 Cachegrind/Massif
```

### 7.2 场景推荐

| 场景 | 推荐工具 | 理由 |
|------|---------|------|
| **开发阶段** | ASan + 编译器警告 | 快速、易集成 |
| **CI/CD** | ASan + 单元测试 | 自动化、低开销 |
| **发布前测试** | Valgrind | 深度检测 |
| **性能优化** | Cachegrind/Massif | 专业分析 |
| **代码审查** | Clang Static Analyzer + Cppcheck | 静态分析 |
| **多线程调试** | Helgrind/ThreadSanitizer | 线程安全 |

---

## 8. 故障排查流程

### 8.1 内存泄露诊断流程图

```
1. 编译时启用 ASan
   ↓
2. 运行程序
   ↓
3. ASan 报告泄露？
   ├─ 是 → 定位泄露位置（行号）
   │        ↓
   │      检查该行的内存分配
   │        ↓
   │      是否有对应的 delete？
   │        ├─ 无 → 添加 delete
   │        └─ 有 → 检查异常路径
   │                 ↓
   │               使用 RAII 重构
   └─ 否 → 检查是否有未捕获的异常
            ↓
          使用 Valgrind 深度分析
```

### 8.2 常见问题及解决方案

#### 问题 1：ASan 报告 false positive

**现象：** 报告泄露，但代码看起来没问题

**解决方案：**
1. 检查是否是"still reachable"（非真泄露）
2. 使用 `__lsan_ignore_object()` 忽略特定对象
3. 检查第三方库是否有已知泄露

#### 问题 2：Valgrind 运行太慢

**解决方案：**
1. 只在小数据集上测试
2. 使用 `--leak-check=summary`（简化报告）
3. 优先使用 ASan

#### 问题 3：编译器警告太多

**解决方案：**
1. 逐步修复，不要一次性启用所有警告
2. 使用 `#pragma` 忽略特定警告（谨慎使用）
3. 配置 `.clang-tidy` 或 `.cppcheck` 规则

---

## 9. 总结

### 9.1 关键要点

- ✅ **AddressSanitizer** 是日常开发的首选工具（快速、易用）
- ✅ **Valgrind** 适合深度分析和发布前测试（详细但慢）
- ✅ **编译器警告** 应始终启用（零成本）
- ✅ **静态分析** 适合代码审查阶段
- ✅ 组合使用多种工具以获得最佳效果

### 9.2 推荐阅读

- [AddressSanitizer 官方文档](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [Valgrind 用户手册](https://valgrind.org/docs/manual/manual.html)
- [GCC 警告选项](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)
- [Clang Static Analyzer](https://clang-analyzer.llvm.org/)

---

## 10. 下一步

👉 [06-copilot-prompt-library.md](./06-copilot-prompt-library.md) - Copilot 提示词库

👉 [07-best-practices.md](./07-best-practices.md) - C++ 内存管理最佳实践
