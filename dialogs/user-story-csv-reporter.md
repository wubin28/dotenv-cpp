# User Story：csv-reporter 多源 CSV 数据聚合报表工具

---

## User Story

**作为**一名需要每日合并销售与库存数据的数据分析师，
**我想**通过 `.env` 文件配置数据源路径、分隔符、过滤阈值和输出格式，然后运行一条命令完成两份 CSV 的 Join、过滤与汇总，
**以便**在不修改、不重新编译程序的前提下，灵活切换不同区域、不同任务的分析参数，快速产出今日摘要报告。

---

## Acceptance Criteria（BDD 风格）

### ✅ Happy Path

---

**AC-H1：标准场景——完整配置下成功生成 JSON 报表**

```
Given 当前目录存在有效的 .env 文件，内容为：
        SALES_FILE=csv-reporter/data/sales.csv
        INVENTORY_FILE=csv-reporter/data/inventory.csv
        DELIMITER=,
        MIN_AMOUNT=1000
        OUTPUT_FORMAT=json
  And  两个 CSV 文件均存在，格式正确，均以逗号分隔
  And  sales.csv 包含字段 order_id, product_id, amount
  And  inventory.csv 包含字段 product_id, stock_qty
When  用户在终端执行 ./csv-reporter
Then  程序读取两个文件，以 product_id 进行 Join
  And 仅保留 amount >= 1000 的行
  And 在标准输出打印 JSON 格式的统计摘要，包含：
        匹配行总数、amount 合计、amount 均值
  And 程序以退出码 0 结束
```

---

**AC-H2：切换区域任务——仅替换 .env 即可复用程序**

```
Given 用户将 .env 中的 SALES_FILE 从 csv-reporter/data/north_sales.csv 改为 csv-reporter/data/south_sales.csv
  And 将 MIN_AMOUNT 从 5000 改为 2000
  And 不修改任何源代码，不重新编译
When  用户再次执行 ./csv-reporter
Then  程序读取新配置指定的文件，使用新的阈值 2000 进行过滤
  And 输出结果反映华南区数据，与上一次运行结果不同
  And 程序以退出码 0 结束
```

---

**AC-H3：管道符分隔符场景**

```
Given .env 中配置 DELIMITER=|
  And sales.csv 和 inventory.csv 均为管道符（|）分隔格式
When  用户执行 ./csv-reporter
Then  程序正确解析管道符分隔的字段，不将整行视为单个字段
  And 输出结果与使用逗号分隔的等价数据集结果一致
```

---

**AC-H4：过滤后零行匹配——程序仍正常退出**

```
Given .env 中 MIN_AMOUNT=9999999
  And sales.csv 中所有订单金额均低于 9999999
When  用户执行 ./csv-reporter
Then  程序输出空摘要（匹配行数为 0，合计为 0）
  And 不抛出异常，不崩溃
  And 程序以退出码 0 结束
```

---

**AC-H5：OUTPUT_FORMAT=csv 时输出 CSV 格式摘要**

```
Given .env 中 OUTPUT_FORMAT=csv
When  用户执行 ./csv-reporter
Then  程序以 CSV 格式（表头 + 数据行）输出统计摘要
  And 不输出 JSON 格式内容
```

---

### ❌ Sad Path

---

**AC-S1：缺少 .env 文件**

```
Given 当前目录不存在 .env 文件，也不存在任何默认配置
When  用户执行 ./csv-reporter
Then  程序在标准错误输出一条明确的错误信息，例如：
        "[ERROR] .env file not found. Please create one before running."
  And 程序以非零退出码（如 1）结束
  And 不输出任何摘要内容
```

---

**AC-S2：SALES_FILE 指向不存在的文件**

```
Given .env 中 SALES_FILE=csv-reporter/data/nonexistent.csv
  And 该路径对应的文件不存在
When  用户执行 ./csv-reporter
Then  程序在标准错误输出包含文件路径的错误信息，例如：
        "[ERROR] Cannot open SALES_FILE: csv-reporter/data/nonexistent.csv"
  And 程序以非零退出码结束
  And 不输出任何摘要内容，也不崩溃
```

---

**AC-S3：CSV 某行列数少于预期（越界防护）**

```
Given sales.csv 中存在某行只有 2 列，而程序预期读取第 3 列（如 amount）
When  用户执行 ./csv-reporter
Then  程序跳过该异常行，并在标准错误输出一条带行号的警告，例如：
        "[WARN] Skipping malformed row at line 7: insufficient columns"
  And 程序继续处理其余行，最终正常输出摘要
  And 程序以退出码 0 结束（降级处理，非致命错误）
```

---

**AC-S4：MIN_AMOUNT 配置为非数字字符串**

```
Given .env 中 MIN_AMOUNT=abc
When  用户执行 ./csv-reporter
Then  程序在标准错误输出配置校验错误，例如：
        "[ERROR] Invalid value for MIN_AMOUNT: 'abc' is not a valid number."
  And 程序以非零退出码结束
  And 不使用默认值 0 静默降级（避免产生无意义的报表）
```

---

**AC-S5：OUTPUT_FORMAT 为不支持的值**

```
Given .env 中 OUTPUT_FORMAT=xml
  And 程序当前仅支持 json 和 csv
When  用户执行 ./csv-reporter
Then  程序在标准错误输出支持格式列表，例如：
        "[ERROR] Unsupported OUTPUT_FORMAT: 'xml'. Supported values: json, csv."
  And 程序以非零退出码结束
```

---

**AC-S6：DELIMITER 缺失时使用默认值逗号**

```
Given .env 中未设置 DELIMITER 字段
  And sales.csv 和 inventory.csv 均为逗号分隔
When  用户执行 ./csv-reporter
Then  程序默认使用逗号作为分隔符，正常完成解析
  And 输出与显式配置 DELIMITER=, 时结果相同
  And 不报错，程序以退出码 0 结束
```

---

## 备注（供 AI 生成自动化测试时参考）

| 关注点 | 对应 AC |
|--------|---------|
| dotenv 核心价值（免重编译切换参数） | H2 |
| Join 与过滤正确性 | H1、H3、H4 |
| 越界访问防护 | S3 |
| 配置校验与错误处理 | S1、S2、S4、S5 |
| 默认值回退逻辑 | S6 |
| 输出格式切换 | H5 |