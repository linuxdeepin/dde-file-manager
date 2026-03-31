# Git 约定

> DDE 项目的 Git 提交和分支管理约定。

---

## 概览

本指南定义 DDE 项目的 Git 工作流约定，参考现有 DDE 项目规范。

---

## 提交信息格式

### Conventional Commits

```
<type>(<scope>): <subject>

[body]

[footer]
```

### Type 类型

| Type | 描述 | 示例 |
|------|------|------|
| `feat` | 新功能 | feat(filemanager): 添加搜索功能 |
| `fix` | Bug 修复 | fix(dialog): 修复对话框内存泄漏 |
| `docs` | 文档更新 | docs(readme): 更新安装说明 |
| `style` | 代码格式 | style(cpplint): 修复格式问题 |
| `refactor` | 重构 | refactor(model): 简化数据模型 |
| `test` | 测试 | test(unit): 添加文件操作测试 |
| `chore` | 构建/工具 | chore(cmake): 更新 Qt6 支持 |

### Subject 规则

- 使用中文描述
- 不超过 50 字符
- 句子末尾不加句号
- 使用祈使语气开头

```
✅ 好:
feat(filemanager): 添加文件重命名功能
fix(dialog): 修复空指针崩溃
style(cmake): 格式化代码

❌ 不好:
feat(filemanager): 添加文件重命名功能。    // 有句号
feat(filemanager): 这是一个新功能          // 过于冗长
feat(filemanager): new feature added        // 应该用中文
```

### Body 规则

- 描述做了什么以及为什么
- 每行不超过 72 字符
- 可以包含多行

```
feat(filemanager): 添加文件重命名功能

- 实现右键菜单重命名选项
- 支持批量重命名
- 添加重命名验证

相关: issue#123
```

---

## 分支命名

### 功能分支

```
feature/<short-feature-name>

示例:
feature/file-search
feature/dark-theme
feature/plugin-api
```

### 修复分支

```
fix/<short-bug-description>

示例:
fix/crash-on-window-close
fix/memory-leak-dialog
fix/dbus-connection-fail
```

### 发布分支

```
release/<version>

示例:
release/1.0.0
release/1.1.0
```

---

## 工作流

### 功能开发流程

```
1. 从 main 创建功能分支
   git checkout -b feature/feature-name

2. 开发功能

3. 提交代码
   git add .
   git commit -m "feat(scope): description"

4. 推送到远程
   git push origin feature/feature-name

5. 创建 Merge Request (MR)

6. 代码审查通过后合并
```

### Bug 修复流程

```
1. 从 main 创建修复分支
   git checkout -b fix/bug-description

2. 修复 Bug

3. 提交代码
   git add .
   git commit -m "fix(scope): description"

4. 推送和创建 MR

5. 审查通过后合并
```

---

## Merge Request 模板

### 功能 MR

```markdown
## 功能描述

简要描述这个功能。

## 变更内容

- 添加了 ...
- 修改了 ...
- 删除了 ...

## 相关 Issue

关联 Issue: #123

## 测试

- [ ] 单元测试通过
- [ ] 手动测试通过
- [ ] 代码审查通过

## 截图

<如果适用，添加截图>
```

### Bug 修复 MR

```markdown
## Bug 描述

简要描述 Bug。

## 复现步骤

1. 步骤 1
2. 步骤 2
3. ...

## 修复方案

描述修复方案。

## 验证

- [ ] Bug 已修复
- [ ] 没有引入新问题
- [ ] 代码审查通过
```

---

## 版本号

### 语义化版本

```
MAJOR.MINOR.PATCH

示例:
1.0.0  - 初始版本
1.1.0  - 新功能 (minor)
1.1.1  - Bug 修复 (patch)
2.0.0  - 重大变更 (major)
```

### 版本规则

| 变更类型 | 版本变更 |
|---------|---------|
| 新功能，向后兼容 | MINOR + 1 |
| Bug 修复 | PATCH + 1 |
| 不兼容的 API 变更 | MAJOR + 1 |

---

## 忽略文件

### .gitignore

```gitignore
# 构建产物
build/
*.o
*.so
*.a

# Qt
*.autosave
*.ui.qml
*.pro.user

# IDE
.vscode/
.idea/
*.swp
*~

# 临时文件
tmp/
temp/
*.tmp
*.log
```

---

## 快速参考

| 任务 | 命令 |
|------|------|
| 创建功能分支 | `git checkout -b feature/name` |
| 修改提交 | `git commit -m "fix(scope): desc"` |
| 推送分支 | `git push origin feature/name` |
| 拉取最新 | `git pull origin main` |
| 撤销提交 | `git reset HEAD~1` |
| 修改最近提交 | `git commit --amend` |
| 查看历史 | `git log --oneline -10` |
