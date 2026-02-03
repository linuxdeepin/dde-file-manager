@page dde-file-dialog dde-file-dialog
@brief 文件管理器文件选择对话框

# 项目介绍

文件选择对话框的可执行程序有3个，分别为：

1. dde-file-dialog：本地通用的文件选择对话框程序，它适配了 DTK 中 QDeepinTheme 的相关接口，本地的应用程序通过调用 Qt 的 QFileDialog 接口可以直接使用到它。通过 com.deepin.filemanager.filedialog 的 DBus 启动后，作为一个独立的进程运行在后台，如果1分钟没使用它，它将自动退出；

2. dde-select-dialog-x11：在DDE基础环境中（X11 平台下），通过修改 glib 库使得 GTK 的文件选择对话框接口将重定向到 dde-select-dialog-x11 提供的服务中。这使得所有GTK程序，典型的就是浏览器（以及基于浏览器框架开发的应用程序，如 VSCode）使用的文件选择对话框接口，通过 GTK 的文件对话框接口重定向到com.deepin.filemanager.filedialog_x11的DBus 接口来实现相关业务。同样地，作为一个独立的进程运行在后台，如果 1 分钟没使用它，它将自动退出；

3. dde-select-dialog-wayland：wayland 平台下的 GTK 程序使用的文件选择对话框接口，其他与 dde-select-dialog-x11 一致。

# 使用方法

对于应用程序开发者而言，使用到文件管理器的文件选择对话框并不是通过 D-Bus 接口调用到的，而是通过通用接口（这里指的 Qt 和 GTK）来进行调用。然后在 Deepin/UOS 中，通过某些技术上的方法可以把通用接口的调用重定向到文件管理器的文件选择对话框（需要文管设置中进行启用，默认是启用的）。

可以通过编写 Qt、GTK 的文件选择对话框，并调用各种接口来测试文管的文件选择框功能是否正常，是否卡死、崩溃等，这是一个非常好的测试方式。

## Qt

以下使用 Qt 编写显示文件对话框的例子：

```cpp

#include <QApplication>
#include <QFileDialog>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 创建一个父窗口（可选）
    QWidget parent;

    // 使用QFileDialog选择文件
    QString selectedFile = QFileDialog::getOpenFileName(&parent, "选择文件", "", "所有文件 (*.*)");

    if (!selectedFile.isEmpty()) {
        qDebug() << "选择的文件：" << selectedFile;
    } else {
        qDebug() << "未选择文件";
    }

    return app.exec();
}
```

- 开发者参考文档：https://doc.qt.io/qt-6/qfiledialog.html

在使用 Qt 调用文管文件选择对话框的过程中，使用 `dbus-monitor "destination=com.deepin.filemanager.filedialog"` 可以监控到整个调用过程。

## GTK

以下使用 GTK 编写显示文件对话框的例子：

```c
#include <gtk/gtk.h>

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("选择文件", NULL, action, "_取消",
                                         GTK_RESPONSE_CANCEL, "_打开",
                                         GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        gchar *filename = gtk_file_chooser_get_filename(chooser);
        g_print("选择的文件：%s\n", filename);
        g_free(filename);
    } else {
        g_print("未选择文件\n");
    }

    gtk_widget_destroy(dialog);

    return 0;
}

```

- 开发者参考文档：https://docs.gtk.org/gtk4/class.FileDialog.html

在使用 GTK 调用文管文件选择对话框的过程中，使用 `dbus-monitor "destination=com.deepin.filemanager.filedialog_x11"` 可以监控到整个调用过程。

# D-Bus 接口

文件选择对话框进程对外提供 DBus 接口`com.deepin.filemanager.filedialog`（如果是 X11 平台则多提供 `com.deepin.filemanager.filedialog_x11`，`wayland` 平台则多提供 `com.deepin.filemanager.filedialog_wayland`）。对于 D-Bus 接口的测试，可以使用 GUI 工具 d-feet 进行。

从接口上来说，它们都是一样的，因此以下内容只关注 `com.deepin.filemanager.filedialog`。但是这并不意味着 `com.deepin.filemanager.filedialog` 正常，另外的 x11 和 wayland 接口使用时就正常。

## 关键流程

文件选择对话框的 D-Bus 接口繁多，很多接口需要排列组合形成关键词流程才能使用，单纯的一个个调用 D-Bus 接口意义不大，如果只是为了测试，建议还是直接用 Qt 的 `QFileDialog` 编写测试用例来进行覆盖更合适。这里只介绍简单的关键流程的 DBus 接口。

### 创建文件选择对话框

创建文件选择对话框只需要调用一个 DBus 接口：

| key         | value                                                        |
| ----------- | ------------------------------------------------------------ |
| Bus Name    | `com.deepin.filemanager.filedialog`                          |
| Object Path | `/com/deepin/filemanager/filedialogmanager`                  |
| Interface   | `com.deepin.filemanager.filedialog`                          |
| Method      | createDialog (String key) ↦ (Object Path path)               |
| 参数        | 参数 key，它是一个任意字符串，可以是空值，意义不大           |
| 返回值      | 新的 Object Path                                             |
| 接口说明    | 创建一个新的 Object Path，该 Object Path 下的接口可以是具体去操作文件选择对话框功能的接口 |

正常情况下，调用完接口后，你会在 ``com.deepin.filemanager.filedialog`` 下看到多了一个 `/com/deepin/filemanager/filedialog/ + 随机字符串` 的Object Path，这代表着文件选择对话框被创建。

### 显示文件选择对话框

显示文件选择对话框需要建立在完成“创建文件选择对话框”的基础上，调用`/com/deepin/filemanager/filedialog/ + 随机字符串` 这个 Path 下的 `show` 接口即可：

| key         | value                                              |
| ----------- | -------------------------------------------------- |
| Bus Name    | `com.deepin.filemanager.filedialog`                |
| Object Path | `/com/deepin/filemanager/filedialog/ + 随机字符串` |
| Interface   | `com.deepin.filemanager.filedialog`                |
| Method      | show () ↦ ()                                       |
| 参数        | /                                                  |
| 返回值      | /                                                  |
| 接口说明    | 显示一个文件选择对话框接口                         |

单独调用此接口，虽然会显示一个界面，但是布局是不完整的，因此意义不大，需要结合很多前置接口进行配置。