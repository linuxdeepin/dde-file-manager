# DDE Development Spec

> DDE 桌面应用开发规范模板 - Qt6/DTK Widget 编程指南

**ID**: `dde-spec`

**适用场景**: 开发 Linux 桌面应用，使用 Qt6 和 Deepin ToolKit (DTK)Widget 框架

---

## 技术栈

- **Qt**: 6.x (Qt5 降级支持)
- **DTK Widget**: Deepin ToolKit Widget
- **CMake**: 3.10+
- **C++**: 17
- **DBus**: System/Session 总线通信

---

## 规范索引

| 层级 | 文档 | 描述 | 状态 |
|------|------|------|------|
| **backend** | [qt-dtk-guide.md](backend/qt-dtk-guide.md) | Qt/DTK 编程指南 | 完成 |
| **backend** | [architecture-decisions.md](backend/architecture-decisions.md) | DDE 架构决策 | 完成 |
| **backend** | [architecture-design.md](backend/architecture-design.md) | 架构设计规范 | 完成 |
| **backend** | [test-standards.md](backend/test-standards.md) | 测试规范 | 完成 |
| **backend** | [bug-fix-standards.md](backend/bug-fix-standards.md) | Bug 修复规范 | 完成 |
| **backend** | [cmake-conventions.md](backend/cmake-conventions.md) | CMake 约定 | 完成 |
| **backend** | [plugin-architecture.md](backend/plugin-architecture.md) | 插件架构 | 完成 |
| **review** | [code-review-standards.md](review/code-review-standards.md) | 代码审查标准 | 完成 |
| **shared** | [cpp-conventions.md](shared/cpp-conventions.md) | C++ 编码约定 | 完成 |
| **shared** | [git-conventions.md](shared/git-conventions.md) | Git 约定 | 完成 |
| **shared** | [internationalization.md](shared/internationalization.md) | 国际化规范 | 完成 |
| **guides** | [root-cause-analysis.md](guides/root-cause-analysis.md) | 根因分析指南 | 完成 |
| **big-question** | [qt-memory-management-pitfalls.md](big-question/qt-memory-management-pitfalls.md) | Qt 内存管理陷阱 | 完成 |
| **big-question** | [gthread-ui-thread-safety.md](big-question/gthread-ui-thread-safety.md) | UI 线程安全 | 完成 |
| **big-question** | [gvfs-mount-path-issues.md](big-question/gvfs-mount-path-issues.md) | GVfs 路径问题 | 完成 |
| **big-question** | [dbus-async-vs-sync.md](big-question/dbus-async-vs-sync.md) | DBus 异步/同步 | 完成 |
| **shared** | [deepin-terminology.md](shared/deepin-terminology.md) | deepin/DDE 品牌术语规范 | 完成 |
| **shared** | [dbus-conventions.md](shared/dbus-conventions.md) | DBus 接口命名规范 | 完成 |

---

## 快速开始

### 功能开发

1. 阅读 [架构设计规范](backend/architecture-design.md)
2. 按照 [架构决策](backend/architecture-decisions.md) 选择正确组件
3. 参考 [Qt/DTK 编程指南](backend/qt-dtk-guide.md)
4. 使用 [测试规范](backend/test-standards.md) 编写测试

### Bug 修复

1. 使用 [根因分析指南](guides/root-cause-analysis.md)
2. 遵循 [Bug 修复规范](backend/bug-fix-standards.md)
3. 查阅 [big-question](big-question/) 识别常见陷阱

### 代码审查

1. 使用 [代码审查标准](review/code-review-standards.md)
2. 检查 [review/reference/](review/reference/) 中的专门指南

---

## 参考文档

- **backend/reference/** - DTK 组件、GVfs/GIO、DBus、Polkit 技术参考
- **review/reference/** - 架构审查、安全审查、性能审查参考

---

## 核心规则

| 规则 | 说明 |
|------|------|
| DTK 优先 | 使用 DTK 控件而非 Qt 原生 |
| 主题适配 | 使用 applicationPalette，禁用硬编码颜色 |
| 国际化 | 面向用户文本使用 tr() |
| 线程安全 | UI 更新在主线程，跨线程用 QueuedConnection |
| 内存管理 | Parent-child 模式，避免内存泄漏 |
