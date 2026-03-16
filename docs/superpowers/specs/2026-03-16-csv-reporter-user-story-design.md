# csv-reporter — User Story & BDD Acceptance Criteria

**Date:** 2026-03-16  
**Status:** Draft  
**Scope:** End-to-end CLI behavior (纯端到端用户视角)

---

## 用户故事

**作为**数据分析师，  
**我想**在 `.env` 配置文件中指定销售数据文件路径（`SALES_FILE`）、库存数据文件路径（`INVENTORY_FILE`）、字段分隔符（`DELIMITER`）、金额过滤阈值（`MIN_AMOUNT`）和输出格式（`OUTPUT_FORMAT`），然后运行 `csv-reporter`，  
**以便**在不修改、不重新编译程序的情况下，灵活切换分析任务（如换地区、调阈值、适配不同导出系统），得到两张 CSV 内连接（inner join）并过滤后的聚合统计摘要（订单数 / 总金额 / 均值）。

---

## 术语约定

| 术语 | 含义 |
|------|------|
| 有效配置 | `.env` 文件存在且 `SALES_FILE`、`INVENTORY_FILE` 均指向可读文件，`MIN_AMOUNT` 为合法数字，`OUTPUT_FORMAT` 为 `json` 或 `csv` |
| 聚合摘要 | 程序输出的 `count`（订单数）、`total`（金额合计）、`average`（均值）三个字段 |
| inner join | 仅保留 `sales.product_id` 在 `inventory` 中存在的销售记录 |

---

## Happy Path Acceptance Criteria

### AC-H1：标准聚合统计 — JSON 输出（北区场景）

> **Given** `.env` 中 `SALES_FILE` 和 `INVENTORY_FILE` 均指向合法的逗号分隔 CSV；`DELIMITER=,`，`MIN_AMOUNT=1000`，`OUTPUT_FORMAT=json`  
> **When** 运行 `csv-reporter`  
> **Then** stdout 输出一行 JSON，包含 `"count": 3`、`"total": 4700.00`、`"average": 1566.67`；退出码为 0

---

### AC-H2：不同阈值产生不同统计结果（南区场景）

> **Given** 同上的 CSV 文件；`.env` 中 `MIN_AMOUNT=2000`，`OUTPUT_FORMAT=json`  
> **When** 运行 `csv-reporter`  
> **Then** JSON 输出中 `"count": 1`，仅保留金额 ≥ 2000 的唯一订单；退出码为 0

---

### AC-H3：管道符分隔符（周报场景）

> **Given** `.env` 中 `DELIMITER=|`；`SALES_FILE` 和 `INVENTORY_FILE` 均为管道符分隔的 CSV 文件；`MIN_AMOUNT=1000`，`OUTPUT_FORMAT=json`  
> **When** 运行 `csv-reporter`  
> **Then** 程序正确解析并输出有效聚合摘要（含 `"count"` 键的 JSON）；退出码为 0

---

### AC-H4：过滤阈值高于所有金额时输出空摘要

> **Given** `.env` 中 `MIN_AMOUNT=9999999`；其余配置有效  
> **When** 运行 `csv-reporter`  
> **Then** 输出包含 `count=0`（或 `"count": 0`）、`total=0.00`、`average=0.00`；退出码为 0；stderr 无 `[ERROR]`

---

### AC-H5：CSV 格式输出

> **Given** `.env` 中 `OUTPUT_FORMAT=csv`；其余配置有效  
> **When** 运行 `csv-reporter`  
> **Then** stdout 第一行为 `count,total,average`，第二行为对应数据值；输出中不含 `{` 或 `}`；退出码为 0

---

### AC-H6：未设置 DELIMITER 时默认使用逗号

> **Given** `.env` 中 `DELIMITER` 键不存在或值为空；`SALES_FILE` 和 `INVENTORY_FILE` 为逗号分隔的 CSV  
> **When** 运行 `csv-reporter`  
> **Then** 程序正常解析两个文件并输出有效聚合摘要；退出码为 0

---

### AC-H7：Join 无匹配记录时输出空摘要

> **Given** sales CSV 中所有 `product_id` 均不在 inventory CSV 中；`MIN_AMOUNT=0`；其余配置有效  
> **When** 运行 `csv-reporter`  
> **Then** 输出 `count=0` 的摘要；退出码为 0；stderr 无 `[ERROR]`

---

## Sad Path Acceptance Criteria

### AC-S1：`.env` 文件不存在

> **Given** 当前目录下不存在 `.env` 文件  
> **When** 运行 `csv-reporter`  
> **Then** stderr 输出包含 `[ERROR]`；stdout 无输出；退出码非 0

---

### AC-S2：CSV 文件路径不存在

> **Given** `.env` 中 `SALES_FILE`（或 `INVENTORY_FILE`）的值为一个不存在的文件路径；其余配置有效  
> **When** 运行 `csv-reporter`  
> **Then** stderr 输出包含 `[ERROR]`、对应的变量名（如 `SALES_FILE`）及该文件路径；退出码非 0

---

### AC-S3：数据行列数不足 — 跳过并警告，程序继续

> **Given** sales CSV 中某数据行仅有 2 列（缺少金额字段）；其余行格式正常；其余配置有效  
> **When** 运行 `csv-reporter`  
> **Then** 该行被跳过；stderr 输出包含 `[WARN]`、该行的行号及 `insufficient columns`；其余有效行正常处理，stdout 输出摘要；退出码为 0

---

### AC-S4：`MIN_AMOUNT` 为非数字值

> **Given** `.env` 中 `MIN_AMOUNT=abc`（无法解析为数字）；其余配置有效  
> **When** 运行 `csv-reporter`  
> **Then** stderr 输出包含 `[ERROR]`；退出码非 0

---

### AC-S5：`OUTPUT_FORMAT` 为不支持的值

> **Given** `.env` 中 `OUTPUT_FORMAT=xml`（既非 `json` 也非 `csv`）；其余配置有效  
> **When** 运行 `csv-reporter`  
> **Then** stderr 输出包含 `[ERROR]`；退出码非 0

---

## 设计说明

- **AC 范围**：所有 AC 均为纯端到端 CLI 视角，描述用户从命令行观察到的行为（stdout、stderr、退出码），不涉及组件内部实现。
- **自动化测试映射**：每条 AC 可直接映射为一个集成测试用例（准备 `.env` 和 CSV fixture → 运行进程 → 断言输出和退出码）；AC-S3 的 `[WARN]` 行为亦可在单元测试中针对 `parse_sales_rows` 单独验证。
- **取代旧编号**：本文档取代原代码注释中的 `AC-H1`～`AC-H5`、`AC-S1`～`AC-S6` 标签体系；代码注释可更新为引用本文档对应条目。
