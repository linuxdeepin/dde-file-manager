# 代码审查层

> DDE 代码审查标准和检查清单。

---

## 概览

本层定义 DDE 应用代码审查的标准流程，包括 C/C++/Qt 特定检查、DDE 约定检查、安全审计和性能评估。

---

## 规范索引

| 文档 | 描述 | 何时阅读 |
|------|------|----------|
| [code-review-standards.md](code-review-standards.md) | 代码审查流程、C/C++/Qt 检查清单 | 所有审查 |

---

## 开发前检查清单

在审查 PR 前阅读：

- [x] 理解 PR 描述和关联 issue
- [x] 检查 CI/CD 状态（构建通过？）
- [ ] 评估 PR 大小（>400 行？要求拆分）
- [ ] 识别变更类型（新功能、Bug 修复、重构）

---

## 质量检查

代码审查时验证：

- [ ] 代码质量（可读性、可维护性）
- [ ] 内存安全（无泄漏、正确的生命周期）
- [ ] 线程安全（UI 更新在主线程）
- [ ] DTE 约定（DTK 组件、主题适配）
- [ ] 安全性（输入验证、敏感数据）
- [ ] 性能（无阻塞 UI、避免 N+1 查询）

---

## 技术参考

### 审查参考文档

- [reference/architecture-review-guide.md](reference/architecture-review-guide.md) - 架构审查
- [reference/code-review-best-practices.md](reference/code-review-best-practices.md) - 最佳实践
- [reference/common-bugs-checklist.md](reference/common-bugs-checklist.md) - 常见 Bug
- [reference/performance-review-guide.md](reference/performance-review-guide.md) - 性能审查
- [reference/security-review-guide.md](reference/security-review-guide.md) - 安全审查

### 语言特定

- [/reference/cpp.md](reference/cpp.md) - C++ 审查
- [reference/c.md](reference/c.md) - C 审查
- [reference/qt.md](reference/qt.md) - Qt 审查

---

## 核心规则摘要

| 检查 | 要求 |
|------|------|
| DTK 组件 | 禁止 QMainWindow、QDialog 等 QtWidgets |
| 主题适配 | 禁止硬编码颜色 |
| 内存安全 | Parent-child 关系、智能指针 |
| 线程安全 | 跨线程使用 QueuedConnection |
| 输入验证 | 文件路径、SQL 参数验证 |
| 敏感数据 | 不记录密码等敏感信息 |

---

## 审查流程

### 第 1 阶段：上下文收集（2-3 分钟）

1. 阅读 PR/commit 描述
2. 关联 issue
3. CI 状态

### 第 2 阶段：高级审查（5-10 分钟）

1. 架构设计
2. 性能评估
3. 文件组织
4. 测试策略

### 第 3 阶段：逐行审查（10-20 分钟）

1. 逻辑与正确性
2. 安全性
3. 性能
4. 可维护性

### 第 4 阶段：总结（2-3 分钟）

1. 关键问题
2. 决策（批准/评论/请求更改）

---

## 快速参考

| 检查 | 命令/方法 |
|------|----------|
| 内存泄漏 | grep "new" 检查父对象 |
| 硬编码颜色 | grep -E "#[0-9a-fA-F]{6}" |
| Qt 组件 | grep "QMainWindow\|QDialog" |
| 主线程更新 | 检查跨线程连接 |
