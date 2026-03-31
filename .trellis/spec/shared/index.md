# 共享规范层

> 跨层共享的开发规范和约定。

---

## 规范索引

| 文档 | 描述 | 何时阅读 |
|------|------|----------|
| [cpp-conventions.md](cpp-conventions.md) | C++17 编码约定、RAII、智能指针、D-Pointer 命名 | 编写 C++ 代码 |
| [git-conventions.md](git-conventions.md) | Git 提交和分支管理约定 | 提交代码 |
| [internationalization.md](internationalization.md) | 国际化规范、tr() 使用、品牌名称规范 | 添加用户文本 |
| [deepin-terminology.md](deepin-terminology.md) | deepin/DDE 品牌术语规范 | 编写文档/代码 |
| [dbus-conventions.md](dbus-conventions.md) | DBus 接口命名规范 | 使用 DBus |

---

## 开发前检查清单

在编写共享代码前：

- [x] 阅读 [cpp-conventions.md](cpp-conventions.md) 了解 C++17 特性和 RAII
- [ ] 项目是否需要 Qt6/Qt5 双版本支持
- [ ] 是否涉及国际化
- [ ] 检查提交信息格式约定
- [ ] 如果使用 DBus，阅读 [dbus-conventions.md](dbus-conventions.md)
- [ ] 查阅品牌名称规范 [deepin-terminology.md](deepin-terminology.md)

---

## 质量检查

共享代码完成后：

- [ ] C++17 特性正确使用
- [ ] RAII 资源管理
- [ ] 智能指针正确使用
- [ ] D-Pointer 模式命名正确（Private 类不加 m_ 前缀）
- [ ] 文件命名使用小写+下划线
- [ ] Git 提交信息符合规范
- [ ] tr() 用于所有面向用户的文本
- [ ] 代码注释使用英文
- [ ] 品牌名称使用正确（deepin 小写、DDE 文档大写）

---

## 核心规则摘要

| 规则 | 要求 |
|------|------|
| C++ 标准 | C++17 |
| 资源管理 | RAII、smart pointer |
| 文件命名 | 小写+下划线（`my_widget.h`） |
| D-Pointer 命名 | Private 类无前缀、普通类 `m_` 前缀 |
| Git 提交 | conventional commits |
| 国际化 | tr() 用于用户文本 |
| 品牌名称 | deepin 小写、DDE 文档大写 |
| 代码注释 | 英文 |
