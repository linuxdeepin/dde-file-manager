# 国际化规范

> DDE 应用的国际化 (i18n) 规范。

---

## 概览

本指南定义 DDE 应用的国际化标准，包括翻译字符串管理、翻译文件组织和处理流程。

---

## tr() 调用规则

### 基本使用

```cpp
// ✅ 推荐: 所有面向用户的文本使用 tr()
button->setText(tr("Save"));
dialog->setTitle(tr("Confirm Action"));
label->setText(tr("Hello World"));

// ❌ 禁止: 硬编码用户可见文本
button->setText("保存");  // 不会翻译
```

### tr() 参数

```cpp
// 无参数
tr("Save")

// 上下文参数（用于消除歧义）
tr("File", "Noun - File to open")
tr("File", "Verb - File a document")

// 数量参数
tr("%n file(s)", "", count)  // 自动处理单复数
```

### 动态文本

```cpp
// ✅ 推荐: 使用参数化字符串
label->setText(tr("%1 files selected").arg(count));

// ❌ 避免字符串拼接（可能导致顺序问题）
label->setText(tr("Files selected: ") + QString::number(count));

// ✅ 推荐: 复数形式
label->setText(tr("%n file(s)", "", count));
// 英文: "1 file" / "2 files"
// 中文: 指定复数形式
```

---

## 上下文前缀

### DTK 组件

`DWIDGET_USE_NAMESPACE` 宏已定义 `tr()` 为 DTK 命名空间，可以使用 `Dtk::Widget::tr()`。

```cpp
#include <DApplication>
DWIDGET_USE_NAMESPACE

// 等价于 Dtk::Widget::tr()
QString text = tr("Save");
```

---

## 翻译文件

### 文件命名

```
<appname>_<language>.ts

示例:
myapp_zh_CN.ts  // 简体中文
myapp_en.ts     // 英语
myapp_ja.ts     // 日语
```

### CMake 配置

```cmake
# 查找翻译工具
find_package(Qt6LinguistTools REQUIRED)

# 添加翻译文件
set(TS_FILES
    translations/myapp_zh_CN.ts
    translations/myapp_en.ts
)

# 创建翻译目标
qt_add_translations(myapp
    TS_FILES ${TS_FILES}
    SOURCE_TARGETS myapp
)

# 更新翻译命令
add_custom_target(update_ts
    COMMAND ${Qt6_LUPDATE_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Updating translation files..."
)

# 创建翻译命令
add_custom_target(l_release
    COMMAND ${Qt6_LRELEASE_EXECUTABLE}
        ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Creating translation files..."
)
```

---

## 翻译组织

### 按模块组织

```
translations/
├── core/
│   ├── core_zh_CN.ts
│   └── core_en.ts
├── widgets/
│   ├── widgets_zh_CN.ts
│   └── widgets_en.ts
└── plugin/
    ├── plugin_zh_CN.ts
    └── plugin_en.ts
```

### 加载翻译

```cpp
int main(int argc, char *argv[]) {
    DApplication a(argc, argv);

    // 加载翻译
    a.loadTranslator();

    // 或加载特定翻译
    QTranslator translator;
    if (translator.load(":/translations/myapp_zh_CN")) {
        a.installTranslator(&translator);
    }

    // ...
}
```

---

## 翻译字符串规则

### DO

- **面向用户的文本** 使用 `tr()`
- **菜单项** 使用 `tr()`
- **对话框** 使用 `tr()`
- **错误消息** 使用 `tr()`
- **按钮文本** 使用 `tr()`

### DON'T

```cpp
// ❌ 不要翻译的内容
// 1. 代码注释
// Initialize the widget
m_widget = new DWidget(this);

// 2. 日志
qDebug() << "Connection failed";  // 日志可以用英文

// 3. 技术术语（如果行业通用）
USB, RGB, API, JSON

// 4. 只在开发环境显示的内容
DEBUG: "Connection established"
```

---

## 特殊字符处理

### 格式化字符串

```cpp
// ✅ 推荐: 使用 %1 占位符
tr("File: %1").arg(filename);

// ✅ 推荐: HTML 实体
tr("Save &amp; Close");

// ✅ 推荐: 避免在文本中嵌入变量
tr("The operation failed: %1").arg(error);
// 而不是
tr("The operation failed: ") + error;
```

### 快捷键

```cpp
// ✅ 推荐: 使用 & 标记快捷键
menu->addAction(tr("&File"));
menu->addAction(tr("&Edit"));
menu->addAction(tr("&View"));

// 快捷键会显示为带下划线
```

---

## 翻译清单

### 翻译前

- [ ] 所有 `tr()` 调用已添加
- [ ] 复数形式正确处理
- [ ] 参数化字符串使用 `%1` 占位符
- [ ] 上下文需要的地方添加了注释

### 翻译后

- [ ] 翻译文本符合文化习惯
- [ ] 变量占位符完整保留
- [ ] 字符串结束符 `\n` 正确处理
- [ ] 标签 XML/HTML 正确处理

---

## 代码注释

### 用于翻译上下文

```cpp
// Translators: "File" as noun (document), not verb (action)
tr("File");

// Translators: Context for ambiguous terms
tr("Server", "Computer that provides services");
tr("Server", "To serve clients");

// Translators: Explain placeholders
// %1 is the number of files
tr("%1 file(s)", "", count);
```

---

## 品牌名称规范

> 详见 [deepin/DDE 品牌术语规范](./deepin-terminology.md)。

### deepin 品牌名称

**deepin** 品牌名称在任何文档、图片、代码注释中都应使用**全小写**，即使是段落首字母。

```cpp
// ✅ 正确 - 文档和注释
// deepin 桌面环境
// deepin 开源项目

// ✅ 正确 - 版权信息
// Copyright (c) 2021. deepin All rights reserved.

// ❌ 错误
// Deepin 桌面环境
// DEEPIN 项目
```

### DDE 品牌名称

**DDE** 是 Deepin Desktop Environment 的缩写。

- **文档中**：使用全大写 `DDE`
- **文件名中**：使用全小写 `dde`

```cpp
// ✅ 正确 - 文档中
DDE 桌面环境 (Deepin Desktop Environment)
DDE 启动器、DDE 任务栏

// ✅ 正确 - 文件名中
dde-dock
dde-launcher
org.deepin.DDE1.Accounts

// ❌ 错误
dde 桌面环境           // 文档中不应该小写
DDE-System-Daemon      // 文件名不应该大写
```

### 翻译中的品牌名称

```cpp
// ✅ 正确 - 品牌名称不应翻译
tr("deepin desktop environment")
tr("DDE Settings")
tr("deepin Music")

// ❌ 错误 - 品牌名称不应翻译
tr("深度桌面环境")     // 应该使用英文品牌名
```

---

## 快速参考

| 任务 | 方法 |
|------|------|
| 用户可见文本 | `tr("Text")` |
| 上下文 | `tr("Text", "Context")` |
| 复数形式 | `tr("%n item(s)", "", count)` |
| 参数 | `tr("Text %1").arg(value)` |
| 创建 TS 文件 | `lupdate src -ts app_zh_CN.ts` |
| 创建 QM 文件 | `lrelease app_zh_CN.ts` |
| 加载翻译 | `a.loadTranslator()` |
