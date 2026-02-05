# C++ 内存管理最佳实践

本文档总结了 C++ 内存管理的最佳实践、常见陷阱和代码审查检查清单，帮助您编写安全、高效、可维护的 C++ 代码。

---

## 1. 核心原则

### 1.1 RAII（Resource Acquisition Is Initialization）

**定义：** 资源的获取即初始化，资源的释放即析构。

**核心思想：**
- 在对象构造时获取资源（内存、文件、锁等）
- 在对象析构时释放资源
- 利用 C++ 的栈展开机制保证资源释放

**示例：**
```cpp
// ❌ 不推荐：手动管理
void process_file() {
    FILE* fp = fopen("data.txt", "r");
    if (!fp) return;

    // 处理文件...
    if (error) {
        return;  // ❌ 忘记 fclose
    }

    fclose(fp);
}

// ✅ 推荐：RAII
void process_file() {
    std::ifstream file("data.txt");
    if (!file.is_open()) return;

    // 处理文件...
    if (error) {
        return;  // ✅ file 自动关闭
    }
}  // file 析构，自动关闭
```

**优势：**
- 自动保证资源释放
- 异常安全
- 代码简洁

---

### 1.2 零法则（Rule of Zero）

**定义：** 如果可以，不要自定义任何特殊成员函数（析构函数、拷贝/移动构造函数、拷贝/移动赋值操作符）。

**理念：** 使用标准库容器和智能指针管理资源，让编译器生成正确的特殊成员函数。

**示例：**
```cpp
// ✅ 遵循零法则
class ConfigManager {
    std::string filename_;
    std::vector<std::string> data_;
    std::unique_ptr<Logger> logger_;

    // 无需自定义任何特殊成员函数
    // 编译器生成的默认实现已正确
};

// ❌ 违反零法则（不必要的手动管理）
class ConfigManager {
    char* filename_;
    char** data_;
    Logger* logger_;

    // 需要自定义：
    // - 析构函数
    // - 拷贝构造函数
    // - 拷贝赋值操作符
    // - 移动构造函数
    // - 移动赋值操作符
};
```

**何时可以遵循零法则：**
- 所有成员变量都是值类型或智能指针
- 不需要自定义资源管理逻辑

---

### 1.3 明确所有权（Clear Ownership）

**定义：** 代码中应明确谁拥有资源，谁负责释放。

**所有权类型：**

| 类型 | 表示方式 | 特点 |
|------|---------|------|
| **独占所有权** | `std::unique_ptr<T>` | 只有一个所有者，不可拷贝 |
| **共享所有权** | `std::shared_ptr<T>` | 多个所有者，引用计数 |
| **无所有权（观察）** | `T*` 或 `std::weak_ptr<T>` | 不负责释放 |
| **值语义** | `T`（对象本身） | 对象直接拥有资源 |

**示例：**
```cpp
class DataProcessor {
    // 独占所有权：DataProcessor 负责释放
    std::unique_ptr<Database> db_;

    // 共享所有权：多个对象共享（引用计数）
    std::shared_ptr<Config> config_;

    // 无所有权：只是观察，不负责释放
    Logger* logger_;  // 由其他对象管理

public:
    DataProcessor(std::shared_ptr<Config> cfg, Logger* log)
        : db_(std::make_unique<Database>()),
          config_(cfg),
          logger_(log) {
    }

    // 编译器生成的析构函数：
    // - db_ 自动释放
    // - config_ 引用计数减 1
    // - logger_ 不操作（非所有者）
};
```

---

## 2. 具体指南

### 2.1 ✅ 推荐做法

#### 使用 std::string 而非 char*

```cpp
// ❌ 不推荐
class Config {
    char* name_;
public:
    Config(const char* n) {
        name_ = new char[strlen(n) + 1];
        strcpy(name_, n);
    }
    ~Config() { delete[] name_; }
    // 还需实现拷贝构造、赋值操作符...
};

// ✅ 推荐
class Config {
    std::string name_;
public:
    Config(const std::string& n) : name_(n) {}
    // 无需自定义析构函数和拷贝函数
};
```

---

#### 使用 std::vector 而非动态数组

```cpp
// ❌ 不推荐
class DataSet {
    int* data_;
    size_t size_;
public:
    DataSet(size_t n) : size_(n) {
        data_ = new int[n];
    }
    ~DataSet() { delete[] data_; }
    // 需要实现拷贝、移动...
};

// ✅ 推荐
class DataSet {
    std::vector<int> data_;
public:
    DataSet(size_t n) : data_(n) {}
    // 无需手动管理
};
```

---

#### 使用智能指针而非裸指针

```cpp
// ❌ 不推荐
class Factory {
public:
    Widget* create() {
        return new Widget();  // 谁负责 delete？
    }
};

// ✅ 推荐
class Factory {
public:
    std::unique_ptr<Widget> create() {
        return std::make_unique<Widget>();  // 所有权清晰
    }
};
```

---

#### 使用容器管理多个对象

```cpp
// ❌ 不推荐
class Manager {
    Widget** widgets_;
    size_t count_;
public:
    Manager(size_t n) : count_(n) {
        widgets_ = new Widget*[n];
        for (size_t i = 0; i < n; ++i) {
            widgets_[i] = new Widget();
        }
    }
    ~Manager() {
        for (size_t i = 0; i < count_; ++i) {
            delete widgets_[i];
        }
        delete[] widgets_;
    }
    // 需要实现拷贝、移动...
};

// ✅ 推荐
class Manager {
    std::vector<std::unique_ptr<Widget>> widgets_;
public:
    Manager(size_t n) {
        for (size_t i = 0; i < n; ++i) {
            widgets_.push_back(std::make_unique<Widget>());
        }
    }
    // 无需手动释放
};
```

---

#### 让编译器生成特殊成员函数

```cpp
// ✅ 推荐：编译器生成默认实现
class Config {
    std::string name_;
    std::vector<int> values_;
    std::unique_ptr<Logger> logger_;

    // 编译器自动生成：
    // - 析构函数
    // - 拷贝构造函数（由于 unique_ptr，自动 = delete）
    // - 移动构造函数
    // - 移动赋值操作符
};

// 如果需要拷贝语义，使用 shared_ptr
class Config {
    std::string name_;
    std::vector<int> values_;
    std::shared_ptr<Logger> logger_;  // 可拷贝

    // 编译器生成的拷贝函数是正确的
};
```

---

### 2.2 ❌ 避免做法

#### 手动 new/delete（除非必须）

```cpp
// ❌ 避免
void process() {
    Widget* w = new Widget();
    w->process();
    delete w;  // 容易忘记
}

// ✅ 推荐
void process() {
    auto w = std::make_unique<Widget>();
    w->process();
}  // 自动释放

// ✅ 或者直接使用栈对象
void process() {
    Widget w;
    w.process();
}  // 自动析构
```

---

#### 裸指针表示所有权

```cpp
// ❌ 避免：不清楚谁负责释放
Widget* create_widget();

// ✅ 推荐：明确所有权
std::unique_ptr<Widget> create_widget();

// ✅ 或者：观察者指针（不拥有）
Widget* get_current_widget();  // 返回指针，但不转移所有权
```

---

#### 在析构函数中抛出异常

```cpp
// ❌ 严重错误
class Resource {
public:
    ~Resource() {
        if (cleanup_failed()) {
            throw std::runtime_error("Cleanup failed");  // ❌ 灾难
        }
    }
};

// ✅ 推荐：记录错误，不抛异常
class Resource {
public:
    ~Resource() noexcept {  // 显式标记 noexcept
        try {
            cleanup();
        } catch (const std::exception& e) {
            // 记录错误，但不重新抛出
            std::cerr << "Cleanup error: " << e.what() << std::endl;
        }
    }
};
```

**为什么不能在析构函数中抛异常？**
- 如果在栈展开过程中（已有异常），析构函数再抛异常，程序会调用 `std::terminate()`
- 导致程序立即终止，无法捕获

---

#### 忽略三/五法则

```cpp
// ❌ 错误：只实现析构函数，未实现拷贝函数
class Buffer {
    char* data_;
public:
    Buffer(size_t n) { data_ = new char[n]; }
    ~Buffer() { delete[] data_; }
    // ❌ 缺少拷贝构造函数和赋值操作符
    // 默认的浅拷贝会导致 double-free
};

// ✅ 方案 1：遵循三法则
class Buffer {
    char* data_;
    size_t size_;
public:
    Buffer(size_t n) : size_(n) { data_ = new char[n]; }
    ~Buffer() { delete[] data_; }

    // 拷贝构造函数
    Buffer(const Buffer& other) : size_(other.size_) {
        data_ = new char[size_];
        memcpy(data_, other.data_, size_);
    }

    // 拷贝赋值操作符
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new char[size_];
            memcpy(data_, other.data_, size_);
        }
        return *this;
    }
};

// ✅ 方案 2（更好）：遵循零法则
class Buffer {
    std::vector<char> data_;
public:
    Buffer(size_t n) : data_(n) {}
    // 编译器生成的所有特殊成员函数都正确
};
```

---

#### 返回局部变量的指针/引用

```cpp
// ❌ 错误：返回局部变量的指针
const char* get_name() {
    char name[] = "Alice";
    return name;  // ❌ 悬挂指针
}

// ❌ 错误：返回局部变量的引用
const std::string& get_name() {
    std::string name = "Alice";
    return name;  // ❌ 悬挂引用
}

// ✅ 推荐：返回值
std::string get_name() {
    return "Alice";  // ✅ RVO 优化
}

// ✅ 或者：返回成员的引用
class Person {
    std::string name_;
public:
    const std::string& get_name() const {
        return name_;  // ✅ 生命周期与对象绑定
    }
};
```

---

## 3. 智能指针使用指南

### 3.1 std::unique_ptr

**何时使用：**
- 独占所有权
- 不需要共享
- 替代裸指针的首选

**示例：**
```cpp
// 工厂函数
std::unique_ptr<Widget> create_widget(int id) {
    return std::make_unique<Widget>(id);
}

// 作为成员变量
class Manager {
    std::unique_ptr<Database> db_;
public:
    Manager() : db_(std::make_unique<Database>()) {}
};

// 容器中存储
std::vector<std::unique_ptr<Task>> tasks;
tasks.push_back(std::make_unique<Task>());
```

**关键特性：**
- 不可拷贝，只能移动
- 零开销（编译器优化后与裸指针性能相同）
- 可以自定义删除器

---

### 3.2 std::shared_ptr

**何时使用：**
- 共享所有权
- 不确定谁最后使用
- 多个对象需要访问同一资源

**示例：**
```cpp
// 共享配置对象
class Application {
    std::shared_ptr<Config> config_;
public:
    Application(std::shared_ptr<Config> cfg) : config_(cfg) {}

    void start() {
        // 创建多个模块，共享配置
        auto module1 = std::make_shared<Module>(config_);
        auto module2 = std::make_shared<Module>(config_);
    }
};
```

**关键特性：**
- 引用计数，最后一个 `shared_ptr` 销毁时释放资源
- 线程安全的引用计数（原子操作）
- 有性能开销（引用计数、控制块）

**注意：避免循环引用**
```cpp
// ❌ 循环引用导致内存泄露
struct Node {
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;  // 循环引用
};

// ✅ 使用 weak_ptr 打破循环
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;  // 不增加引用计数
};
```

---

### 3.3 std::weak_ptr

**何时使用：**
- 观察 `shared_ptr` 但不拥有
- 打破循环引用
- 缓存场景

**示例：**
```cpp
class Cache {
    std::map<int, std::weak_ptr<Widget>> cache_;

public:
    std::shared_ptr<Widget> get(int id) {
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            if (auto sp = it->second.lock()) {  // 尝试获取 shared_ptr
                return sp;  // 缓存命中
            }
        }

        // 缓存未命中，创建新对象
        auto widget = std::make_shared<Widget>(id);
        cache_[id] = widget;
        return widget;
    }
};
```

---

### 3.4 std::make_unique 和 std::make_shared 的优势

**为什么使用 make_unique/make_shared？**

```cpp
// ❌ 不推荐
std::unique_ptr<Widget> w1(new Widget());
std::shared_ptr<Widget> w2(new Widget());

// ✅ 推荐
auto w1 = std::make_unique<Widget>();
auto w2 = std::make_shared<Widget>();
```

**优势：**
1. **异常安全**
```cpp
// ❌ 潜在问题
process(std::shared_ptr<Widget>(new Widget()), compute());
// 如果 compute() 抛异常，Widget 可能泄露

// ✅ 安全
process(std::make_shared<Widget>(), compute());
```

2. **性能更好**（针对 `shared_ptr`）
```cpp
// 使用 new：两次内存分配（Widget + 控制块）
std::shared_ptr<Widget> w1(new Widget());

// 使用 make_shared：一次内存分配（Widget 和控制块连续）
auto w2 = std::make_shared<Widget>();
```

3. **代码更简洁**

---

## 4. 异常安全编程

### 4.1 异常安全的三个级别

| 级别 | 保证 | 示例 |
|------|------|------|
| **基本保证** | 不泄露资源，对象处于有效状态 | 大部分标准库函数 |
| **强保证** | 操作要么成功，要么状态不变（事务性） | `std::vector::push_back`（C++11） |
| **不抛异常** | 绝不抛出异常 | `std::vector::swap`, 移动构造函数 |

---

### 4.2 构造函数中的异常安全

```cpp
// ❌ 不安全：构造失败会泄露
class Database {
    Connection* conn_;
    Logger* logger_;
public:
    Database() {
        conn_ = new Connection();
        logger_ = new Logger();  // 如果这里抛异常，conn_ 泄露
    }
};

// ✅ 安全：使用 RAII
class Database {
    std::unique_ptr<Connection> conn_;
    std::unique_ptr<Logger> logger_;
public:
    Database()
        : conn_(std::make_unique<Connection>()),
          logger_(std::make_unique<Logger>()) {
        // 即使 logger_ 构造抛异常，conn_ 也会自动释放
    }
};
```

---

### 4.3 使用 RAII 保证资源释放

```cpp
// ❌ 不安全
void process_data() {
    Resource* res = new Resource();
    process(res);  // 可能抛异常
    delete res;    // 可能不执行
}

// ✅ 安全
void process_data() {
    auto res = std::make_unique<Resource>();
    process(res.get());
}  // 异常或正常退出，res 都会释放
```

---

### 4.4 异常安全的赋值操作符（Copy-and-Swap Idiom）

```cpp
class Buffer {
    char* data_;
    size_t size_;

public:
    // 拷贝构造函数
    Buffer(const Buffer& other)
        : size_(other.size_),
          data_(new char[size_]) {
        memcpy(data_, other.data_, size_);
    }

    // 赋值操作符：Copy-and-Swap
    Buffer& operator=(Buffer other) {  // 按值传递（拷贝）
        swap(*this, other);            // 交换
        return *this;                  // other 析构释放旧资源
    }

    friend void swap(Buffer& first, Buffer& second) noexcept {
        using std::swap;
        swap(first.data_, second.data_);
        swap(first.size_, second.size_);
    }

    ~Buffer() { delete[] data_; }
};
```

**优点：**
- 自动处理自赋值
- 提供强异常安全保证
- 代码简洁

---

## 5. 代码审查检查清单

### 5.1 内存管理检查清单

- [ ] **所有 new 都有对应的 delete**
  - 检查所有 `new` 调用
  - 确认析构函数或作用域结束时有 `delete`

- [ ] **new[] 使用 delete[] 释放**
  - 数组使用 `delete[]`，单对象使用 `delete`

- [ ] **裸指针不表示所有权**
  - 所有权使用 `unique_ptr` 或 `shared_ptr`
  - 裸指针只用于观察（不负责释放）

- [ ] **遵循三/五/零法则**
  - 如果自定义析构函数，检查是否需要拷贝/移动函数
  - 或者使用零法则（标准库容器）

- [ ] **析构函数不抛出异常**
  - 析构函数标记 `noexcept`
  - 捕获所有异常，不重新抛出

- [ ] **没有返回局部变量的指针/引用**
  - 检查所有返回指针/引用的函数
  - 确认返回的是堆对象、成员变量或全局变量

- [ ] **异常路径下资源正确释放**
  - 使用 RAII，不依赖手动清理
  - 检查构造函数的异常安全性

---

### 5.2 智能指针使用检查清单

- [ ] **优先使用 std::unique_ptr**
  - 独占所有权场景

- [ ] **必要时使用 std::shared_ptr**
  - 共享所有权场景
  - 检查是否有循环引用风险

- [ ] **使用 std::weak_ptr 打破循环引用**
  - 双向链接、观察者模式等场景

- [ ] **使用 make_unique/make_shared**
  - 避免直接 `new`
  - 异常安全、性能更好

- [ ] **不要从 shared_ptr 管理的对象创建 unique_ptr**
  - 避免所有权混乱

---

### 5.3 异常安全检查清单

- [ ] **构造函数异常安全**
  - 使用初始化列表
  - 成员变量使用 RAII 类型

- [ ] **赋值操作符异常安全**
  - 检查自赋值
  - 使用 copy-and-swap 或确保强异常安全

- [ ] **不在析构函数中抛异常**

- [ ] **资源管理使用 RAII**
  - 文件、锁、内存等

---

## 6. 现代 C++ 特性

### 6.1 移动语义的使用场景

**何时使用移动语义：**
- 返回大对象（如容器）
- 将资源所有权转移
- 实现高效的 swap

**示例：**
```cpp
class Buffer {
    std::vector<char> data_;

public:
    // 移动构造函数
    Buffer(Buffer&& other) noexcept
        : data_(std::move(other.data_)) {
        // other.data_ 现在为空
    }

    // 移动赋值操作符
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
        }
        return *this;
    }
};

// 使用
Buffer create_buffer() {
    Buffer buf(1024);
    return buf;  // 移动而非拷贝（RVO 或移动）
}

Buffer b = create_buffer();  // 移动构造
```

---

### 6.2 右值引用

```cpp
// 函数重载：支持拷贝和移动
class Container {
public:
    // 拷贝版本
    void add(const Widget& w) {
        items_.push_back(w);  // 拷贝
    }

    // 移动版本
    void add(Widget&& w) {
        items_.push_back(std::move(w));  // 移动
    }

private:
    std::vector<Widget> items_;
};

// 使用
Widget w;
container.add(w);          // 调用拷贝版本
container.add(Widget());   // 调用移动版本
```

---

### 6.3 std::move 的正确用法

```cpp
// ✅ 正确：转移所有权
std::unique_ptr<Widget> w1 = std::make_unique<Widget>();
std::unique_ptr<Widget> w2 = std::move(w1);  // w1 现在为空

// ✅ 正确：移动大对象
std::vector<int> v1 = {1, 2, 3, ...};
std::vector<int> v2 = std::move(v1);  // v1 现在为空

// ❌ 错误：移动后继续使用
std::string s1 = "Hello";
std::string s2 = std::move(s1);
std::cout << s1 << std::endl;  // ❌ s1 处于未指定状态

// ⚠️ 注意：const 对象无法移动
const std::vector<int> v1 = {1, 2, 3};
std::vector<int> v2 = std::move(v1);  // ⚠️ 调用拷贝构造函数
```

---

## 7. 推荐资源

### 7.1 书籍

- **Effective Modern C++** by Scott Meyers
  - 现代 C++ 的最佳实践

- **C++ Primer (5th Edition)** by Stanley B. Lippman
  - 全面的 C++ 教程

- **A Tour of C++ (2nd Edition)** by Bjarne Stroustrup
  - C++ 创始人的简明指南

---

### 7.2 在线资源

- **C++ Core Guidelines**
  - [https://isocpp.github.io/CppCoreGuidelines/](https://isocpp.github.io/CppCoreGuidelines/)
  - 官方推荐的编码规范

- **CppReference**
  - [https://en.cppreference.com/](https://en.cppreference.com/)
  - 权威的 C++ 参考文档

- **Compiler Explorer**
  - [https://godbolt.org/](https://godbolt.org/)
  - 在线查看编译器生成的汇编代码

---

### 7.3 工具

- **Clang-Tidy**
  - 静态分析工具，检测常见错误

- **Clang-Format**
  - 代码格式化工具

- **Compiler Explorer (Godbolt)**
  - 在线编译器，查看优化效果

- **Quick Bench**
  - [https://quick-bench.com/](https://quick-bench.com/)
  - 在线性能测试

---

## 8. 总结

### 8.1 关键要点回顾

1. **优先使用 RAII**
   - 让对象管理资源
   - 利用析构函数自动释放

2. **遵循零法则**
   - 使用标准库容器
   - 避免手动内存管理

3. **明确所有权**
   - `unique_ptr` 表示独占
   - `shared_ptr` 表示共享
   - 裸指针只用于观察

4. **异常安全**
   - 构造函数中使用 RAII
   - 析构函数不抛异常

5. **现代 C++ 特性**
   - 使用移动语义提高性能
   - 使用 `auto` 简化代码
   - 使用范围 for 循环

---

### 8.2 最重要的原则

**如果你只记住一条规则，那应该是：**

> **优先使用标准库容器和智能指针，遵循零法则。最好的内存管理代码是不需要手动管理内存的代码。**

---

### 8.3 持续学习

- 定期阅读 C++ Core Guidelines
- 关注 C++ 标准委员会的提案
- 参与代码审查，分享最佳实践
- 使用静态分析工具（Clang-Tidy）
- 编写单元测试，使用 AddressSanitizer

---

## 9. 快速参考卡

### 内存管理速查表

| 场景 | 推荐方案 | 避免 |
|------|---------|------|
| **字符串** | `std::string` | `char*` + `new/delete` |
| **动态数组** | `std::vector<T>` | `T*` + `new[]/delete[]` |
| **独占所有权** | `std::unique_ptr<T>` | `T*` + `new/delete` |
| **共享所有权** | `std::shared_ptr<T>` | 手动引用计数 |
| **观察指针** | `T*` 或 `std::weak_ptr<T>` | 不清楚的所有权 |
| **返回值** | 按值返回（RVO） | 返回局部变量指针 |
| **异常安全** | RAII（智能指针、容器） | 手动 `try-catch` 清理 |

---

## 10. 下一步

完成本培训后，建议您：

1. **应用到实际项目**
   - 审查现有代码，识别改进机会
   - 重构手动内存管理为 RAII

2. **深入学习**
   - 阅读《Effective Modern C++》
   - 研究 C++ Core Guidelines

3. **持续实践**
   - 参与代码审查
   - 使用静态分析工具
   - 编写单元测试

---

👉 返回 [README.md](../README.md) - 培训总览

👉 [05-verification-tools.md](./05-verification-tools.md) - 验证工具使用指南

---

**祝贺您完成培训！** 现在您已经掌握了 C++ 内存管理的核心知识和最佳实践。记住：安全的 C++ 代码不是靠小心翼翼避免错误，而是靠正确的抽象和工具让错误变得不可能发生。
