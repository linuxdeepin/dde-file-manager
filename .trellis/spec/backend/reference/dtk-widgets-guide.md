# DTK Widgets 使用指南

## 核心原则

**必须使用 DTK 控件替代 Qt 原生控件**，确保应用符合 DDE 设计规范并支持主题切换。

## 控件对照表

| 用途 | DTK 控件 | Qt 控件 (禁止) | 说明 |
|------|----------|---------------|------|
| 主窗口 | `DMainWindow` | `QMainWindow` | 支持 DDE 窗口装饰、模糊标题栏 |
| 对话框 | `DDialog` | `QDialog` | 自适应主题、圆角设计 |
| 消息框 | `DMessageBox` | `QMessageBox` | DDE 风格按钮布局 |
| 基础控件 | `DWidget` | `QWidget` | 支持 DTK 样式和调色板 |
| 表格 | `DTableView` | `QTableView` | DDE 风格滚动条、选中效果 |
| 树形 | `DTreeView` | `QTreeView` | 主题适配 |
| 列表 | `DListView` | `QListView` | 主题适配 |
| 按钮 | `DPushButton` | `QPushButton` | DDE 风格按钮 |
| 输入框 | `DLineEdit` | `QLineEdit` | 圆角边框、主题色 |
| 标签 | `DLabel` | `QLabel` | 支持 DTK 调色板 |

## 主题适配

### 正确做法

```cpp
#include <DGuiApplicationHelper>
#include <DPalette>

DWIDGET_USE_NAMESPACE

// 获取当前主题调色板
DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();

// 使用调色板颜色
QColor bgColor = palette.color(DPalette::Window);
QColor textColor = palette.color(DPalette::WindowText);
QColor highlight = palette.color(DPalette::Highlight);

// 设置到控件
widget->setPalette(palette);
```

### 错误做法

```cpp
// ❌ 硬编码颜色值
setStyleSheet("background-color: #2b2b2b; color: #ffffff;");

// ❌ 手动检测主题然后切换
if (isDarkMode) {
    setStyleSheet("background-color: #2b2b2b;");
} else {
    setStyleSheet("background-color: #f0f0f0;");
}
```

## 常用控件示例

### DMainWindow

```cpp
#include <DMainWindow>
#include <DTitlebar>
#include <DWidget>

DWIDGET_USE_NAMESPACE

DMainWindow *window = new DMainWindow();
window->setWindowTitle("应用名称");
window->setMinimumSize(800, 600);

// 自定义标题栏
DTitlebar *titlebar = window->titlebar();
titlebar->setIcon(QIcon::fromTheme("app-icon"));
titlebar->setAutoFillBackground(true);

// 设置中央控件
DWidget *centralWidget = new DWidget(window);
window->setCentralWidget(centralWidget);

window->show();
```

### DDialog

```cpp
#include <DDialog>
#include <DLineEdit>

DWIDGET_USE_NAMESPACE

DDialog *dialog = new DDialog("标题", "内容描述");
dialog->setIcon(QIcon::fromTheme("dialog-icon"));

// 添加输入控件
DLineEdit *lineEdit = new DLineEdit(dialog);
lineEdit->setPlaceholderText("请输入...");
dialog->addContent(lineEdit);

// 添加按钮
dialog->addButton("取消", false, DDialog::ButtonNormal);
dialog->addButton("确定", true, DDialog::ButtonRecommend);

// 连接信号
connect(dialog, &DDialog::buttonClicked,
        dialog, [](int index, const QString &text) {
    if (text == "确定") {
        // 处理确认
    }
});

dialog->exec();
```

### DMessageBox

```cpp
#include <DMessageBox>

DWIDGET_USE_NAMESPACE

// 信息
DMessageBox::information(parent, "标题", "操作已完成");

// 确认
int ret = DMessageBox::question(
    parent,
    "确认删除",
    "确定要删除此文件吗？",
    DMessageBox::Yes | DMessageBox::No,
    DMessageBox::No
);

if (ret == DMessageBox::Yes) {
    // 删除文件
}

// 警告
DMessageBox::warning(parent, "磁盘空间不足", "剩余空间小于 1GB");

// 错误
DMessageBox::critical(parent, "保存失败", "无法写入磁盘，请检查权限");
```

## 样式定制

### 使用 DTK 调色板

```cpp
#include <DStyleOption>
#include <DStyle>

void CustomWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    
    DStyleOption opt;
    opt.initFrom(this);
    
    DStyle *style = qobject_cast<DStyle*>(DApplication::style());
    QColor bgColor = style->palette(opt, this).color(QPalette::Window);
    
    painter.fillRect(rect(), bgColor);
}
```

### 监听主题变化

```cpp
#include <DGuiApplicationHelper>

// 在构造函数中连接信号
connect(DGuiApplicationHelper::instance(), 
        &DGuiApplicationHelper::themeTypeChanged,
        this, 
        &YourClass::handleThemeChanged);

void YourClass::handleThemeChanged(DGuiApplicationHelper::ColorType theme) {
    // 更新自定义颜色
    updateColors();
    
    // 刷新样式
    style()->unpolish(this);
    style()->polish(this);
    update();
}
```

## 常见错误

| 错误 | 后果 | 修复 |
|------|------|------|
| 使用 `QMainWindow` | 主题切换失效、DDE 集成缺失 | → `DMainWindow` |
| 硬编码 `#2b2b2b` | 深色/浅色模式显示错误 | → `applicationPalette()` |
| 不使用 `DWIDGET_USE_NAMESPACE` | 编译错误 | → 添加宏 |
| 忽略 `themeTypeChanged` 信号 | 主题切换时 UI 不更新 | → 连接信号并刷新 |
| 直接设置 RGB 颜色 | 与系统主题不协调 | → 使用调色板 |

## 相关资源

- [DTK Widget API 文档](https://github.com/linuxdeepin/dtkwidget)
- [DDE 设计规范](https://github.com/linuxdeepin/deepin-design-guidelines)
