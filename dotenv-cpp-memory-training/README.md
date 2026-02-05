# C++ 内存安全与 GitHub Copilot 练习材料

## 项目简介

这是一套面向C++初级到中级开发者的内存安全培训材料，旨在通过实践练习帮助开发者掌握如何使用GitHub Copilot识别和修复常见的C++内存问题。

**培训目标：**
- 通过GitHub Copilot修复4个递增难度的内存问题
- 掌握内存泄露、double-free、悬挂指针和异常安全问题的识别与修复
- 学习现代C++内存管理最佳实践（RAII、智能指针等）
- 熟练使用内存检测工具（AddressSanitizer、Valgrind等）

**培训时长：** 1-2小时单次培训

**前置知识：**
- C++指针基础
- 基本的编译和链接知识
- 了解类和对象的概念

## 快速开始

### 1. 克隆项目

```bash
# 假设本项目位于 dotenv-cpp 仓库中
cd dotenv-cpp/dotenv-cpp-memory-training
```

### 2. 环境验证（3-5分钟）

快速检查您的开发环境：

```bash
# 检查编译器
gcc --version   # 或 clang --version

# 检查CMake
cmake --version

# 检查ASan支持（可选）
echo 'int main() { int *p = new int; return 0; }' | g++ -fsanitize=address -x c++ - && ./a.out
```

详细的环境配置指南请参阅 [环境配置指南](docs/00-setup-guide.md)

### 3. 运行第一个问题

```bash
# 使用验证脚本
./scripts/check_memory.sh v1

# 预期输出：检测到内存泄露
```

## 学习路径图

```
问题1: 基本内存泄露 (15-20分钟) ⭐
   ↓
问题2: double-free陷阱 (15-20分钟) ⭐⭐
   ↓
问题3: 悬挂指针 (15-20分钟) ⭐⭐⭐
   ↓
问题4: 异常安全 (15-20分钟) ⭐⭐⭐⭐
```

**推荐学习流程：**
1. 阅读对应的文档（docs/XX-problem-XXX.md）
2. 分析问题代码（src/buggy/config_manager_vX.cpp）
3. 使用GitHub Copilot进行修复
4. 运行验证脚本检查修复效果
5. 对比参考答案（src/fixed/config_manager_vX_fixed.cpp）

## 文件导航

### 📚 文档

- [环境配置指南](docs/00-setup-guide.md) - 详细的环境安装和配置说明
- [问题1：基本内存泄露](docs/01-problem-basic-leak.md) - 识别和修复基本的内存泄露
- [问题2：double-free陷阱](docs/02-problem-double-free.md) - 理解浅拷贝和深拷贝
- [问题3：悬挂指针](docs/03-problem-dangling-pointer.md) - 避免返回局部变量的指针
- [问题4：异常安全](docs/04-problem-exception-safety.md) - 使用RAII保证异常安全
- [验证工具使用指南](docs/05-verification-tools.md) - ASan、Valgrind等工具详解
- [GitHub Copilot提示词库](docs/06-copilot-prompt-library.md) - 精选的提示词模板
- [C++内存管理最佳实践](docs/07-best-practices.md) - 最佳实践和代码审查检查清单

### 💻 代码

**问题代码（src/buggy/）：**
- `config_manager_v1.cpp` - 问题1代码（基本内存泄露）
- `config_manager_v2.cpp` - 问题2代码（double-free）
- `config_manager_v3.cpp` - 问题3代码（悬挂指针）
- `config_manager_v4.cpp` - 问题4代码（异常安全）

**参考答案（src/fixed/）：**
- `config_manager_v1_fixed.cpp` - 问题1的修复方案
- `config_manager_v2_fixed.cpp` - 问题2的修复方案
- `config_manager_v3_fixed.cpp` - 问题3的修复方案
- `config_manager_v4_fixed.cpp` - 问题4的修复方案

⚠️ **重要提示：** 在完成练习之前，请不要查看 `src/fixed/` 目录中的参考答案！

### 🧪 测试

- `tests/test_v1.cpp` - 问题1的测试用例
- `tests/test_v2.cpp` - 问题2的测试用例
- `tests/test_v3.cpp` - 问题3的测试用例
- `tests/test_v4.cpp` - 问题4的测试用例

### 🛠️ 工具

- `scripts/check_memory.sh` - Linux/Mac内存检测脚本
- `scripts/check_memory.bat` - Windows内存检测脚本
- `CMakeLists.txt` - CMake构建配置

### 📄 其他

- `TRAINING_GUIDE.md` - 单文件合并版本（便于打印）
- `env-files/` - 示例.env配置文件

## 培训师指南

### 建议的时间分配表

| 时间段 | 内容 | 活动 |
|--------|------|------|
| 0-10分钟 | 开场和环境验证 | 介绍课程、检查环境、演示工具 |
| 10-30分钟 | 问题1 | 讲解5分钟 + 演示5分钟 + 实践7分钟 + 讨论3分钟 |
| 30-50分钟 | 问题2 | 讲解5分钟 + 演示5分钟 + 实践7分钟 + 讨论3分钟 |
| 50-70分钟 | 问题3 | 讲解5分钟 + 演示5分钟 + 实践7分钟 + 讨论3分钟 |
| 70-90分钟 | 问题4 | 讲解5分钟 + 演示5分钟 + 实践7分钟 + 讨论3分钟 |
| 90-100分钟 | 总结和Q&A | 回顾关键点、推荐资源、收集反馈 |

### 常见学员问题和应对策略

**问题1："我的环境没有ASan，怎么办？"**
- 应对：引导使用Valgrind或升级编译器版本，参考环境配置指南

**问题2："Copilot生成的代码有错误"**
- 应对：这是教学机会！让学员学会验证和迭代改进提示词

**问题3："我不理解为什么需要深拷贝"**
- 应对：使用图示说明内存布局，展示多个对象指向同一内存的问题

**问题4："进度太快/太慢"**
- 应对：
  - 快：准备高级话题（移动语义、完美转发）
  - 慢：可以跳过问题4的深入讨论，留作课后作业

### 如何使用验证脚本进行现场演示

1. **演示buggy版本的问题：**
   ```bash
   ./scripts/check_memory.sh v1
   # 指出ASan报告中的关键信息：泄露大小、位置
   ```

2. **演示修复后的效果：**
   ```bash
   # 应用修复代码后
   ./scripts/check_memory.sh v1
   # 展示 "✅ No memory leaks detected"
   ```

3. **备用方案：**
   - 如果现场演示失败，使用预先录制的终端输出
   - 准备截图作为备份

## 许可和致谢

本培训材料基于 [dotenv-cpp](https://github.com/laserpants/dotenv-cpp) 项目构建。

dotenv-cpp 采用 BSD 3-Clause License 许可，详见 [dotenv-cpp LICENSE](../include/laserpants/dotenv/dotenv.h)。

本培训材料中的示例代码和文档内容为教学目的而创建，遵循相同的许可证条款。

---

## 开始学习

准备好了吗？让我们从第一个问题开始！

👉 [问题1：基本内存泄露](docs/01-problem-basic-leak.md)

---

**祝学习愉快！如有问题，请参考各文档或联系培训师。**
