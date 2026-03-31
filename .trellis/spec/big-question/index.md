# 常见问题和陷阱

> 快速查阅常见问题和解决方案。

---

## 概览

本层收录 DDE 开发中的常见问题和陷阱，提供快速解答和解决方案。

---

## 问题索引

| 问题 | 严重性 | 描述 |
|------|--------|------|
| [qt-memory-management-pitfalls.md](qt-memory-management-pitfalls.md) | P1 | Qt 对象生命周期和内存管理 |
| [gthread-ui-thread-safety.md](gthread-ui-thread-safety.md) | P0 | 跨线程更新 UI |
| [gvfs-mount-path-issues.md](gvfs-mount-path-issues.md) | P1 | GVfs 挂载路径会话敏感 |
| [dbus-async-vs-sync.md](dbus-async-vs-sync.md) | P1 | DBus 调用阻塞 UI |

---

## 严重性说明

| 严重性 | 说明 |
|--------|------|
| P0 | 关键问题，必须立即解决 |
| P1 | 重要问题，应该尽快解决 |
| P2 | 一般问题，按优先级解决 |

---

## 使用方式

### 查找问题

1. 描述问题症状
2. 在本文档中查找匹配的问题
3. 按照解决方案修复
4. 验证修复效果

### 贡献新问题

发现文档中没有收录的新问题时：
1. 记录问题描述和根因
2. 记录错误示例和正确做法
3. 记录检测方法和预防措施
4. 添加到本文档
