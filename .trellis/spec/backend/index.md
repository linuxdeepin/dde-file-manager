# 后端编码层

> DDE 应用后端开发规范。

---

## 技术栈

- Qt6 / Qt5 (CMake 双版本支持)
- DTK Widget 6 / DTK Widget (对应)
- C++17
- CMake 3.10+

---

## 规范索引

| 文档 | 描述 | 何时阅读 |
|------|------|----------|
| [directory-structure.md](directory-structure.md) | 后端目录结构组织规范 | 所有后端开发 |
| [database-guidelines.md](database-guidelines.md) | SQLite ORM、连接池、配置管理 | 数据库相关开发 |
| [error-handling.md](error-handling.md) | 错误处理模式、FinallyUtil、事务 | 所有后端开发 |
| [logging-guidelines.md](logging-guidelines.md) | 分类日志、日志级别、日志注册 | 所有后端开发 |
| [quality-guidelines.md](quality-guidelines.md) | 测试标准、代码风格、静态分析 | 所有后端开发 |
| [qt-dtk-guide.md](qt-dtk-guide.md) | Qt/DTK 编码模式、信号槽、DTK 组件 | 所有后端开发 |
| [architecture-decisions.md](architecture-decisions.md) | DDE 架构决策、核心规则 | 架构设计 |
| [architecture-design.md](architecture-design.md) | 架构设计规范、模块结构 | 功能开发 |
| [test-standards.md](test-standards.md) | Qt Test 测试规范 | 编写测试 |
| [bug-fix-standards.md](bug-fix-standards.md) | Bug 修复规范、最小修改原则 | 修复 Bug |
| [cmake-conventions.md](cmake-conventions.md) | CMake 约定、Qt6/Qt5 双版本 | 配置构建 |
| [plugin-architecture.md](plugin-architecture.md) | 插件架构、元数据格式 | 插件开发 |

---

## 开发前检查清单

在编写后端代码前：

- [x] 阅读 [qt-dtk-guide.md](qt-dtk-guide.md) 了解 Qt/DTK 编码模式
- [x] 阅读 [architecture-decisions.md](architecture-decisions.md) 了解 DDE 核心规则
- [ ] 确定使用的组件优先级（DTK 优先于 Qt 原生）
- [ ] 检查是否涉及数据库 → 参考 DB Schema 变更流程
- [ ] 确定是否需要插件化 → 参考 [plugin-architecture.md](plugin-architecture.md)
- [ ] 检查是否涉及系统集成 → 参考 reference/ 文档

---

## 质量检查

代码完成后，验证：

- [ ] 运行 `cpplint` 和 `clang-tidy` 检查
- [ ] 构建 CMake 无警告
- [ ] DTK 组件正确使用（非 Qt 原生）
- [ ] 内存管理正确（无泄漏）
- [ ] 线程安全（UI 更新在主线程）
- [ ] 国际化（用户文本使用 tr()）
- [ ] 单元测试覆盖率 >80%

---

## 技术参考

- [reference/dtk-widgets-guide.md](reference/dtk-widgets-guide.md) - DTK 组件使用指南
- [reference/gvfs-gio-integration.md](reference/gvfs-gio-integration.md) - GVfs/GIO 文件操作
- [reference/dbus-service-usage.md](reference/dbus-service-usage.md) - DBus 服务集成
- [reference/polkit-auth-workflow.md](reference/polkit-auth-workflow.md) - Polkit 权限控制

---

## 核心规则摘要

| 规则 | 要求 |
|------|------|
| DTK 组件 | 禁止 QMainWindow、QDialog、QMessageBox |
| 主题适配 | 禁止硬编码颜色 |
| 国际化 | 用户文本必须用 tr() |
| 线程安全 | 跨线程用 QueuedConnection |
| 内存管理 | 对象必须有父对象或所有者 |

---

## 快速导航

### 按任务

| 任务 | 文档 |
|------|------|
| 创建主窗口 | [qt-dtk-guide.md](qt-dtk-guide.md#DMainWindow) |
| 使用对话框 | [qt-dtk-guide.md](qt-dtk-guide.md#DDialog) |
| 信号槽连接 | [qt-dtk-guide.md](qt-dtk-guide.md#信号槽模式) |
| 内存管理 | [qt-dtk-guide.md](qt-dtk-guide.md#内存管理模式) |
| 测试 | [test-standards.md](test-standards.md) |
| Bug 修复 | [bug-fix-standards.md](bug-fix-standards.md) |
