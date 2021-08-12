#include "dbusfiledialogmanager.h"
#include "filedialogmanager_adaptor.h"
#include "log/dfmLogManager.h"

#include <DApplication>

#include <QDBusConnection>

DFM_USE_NAMESPACE
using namespace Dtk::Core;
using namespace Dtk::Widget;

static bool registerDialogDBus()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return false;
    }

    // add our D-Bus interface and connect to D-Bus
    if (!QDBusConnection::sessionBus().registerService("com.deepin.filemanager.filedialog_wayland")) {
        qWarning("Cannot register the \"com.deepin.filemanager.filedialog_wayland\" service.\n");
        return false;
    }

    DBusFileDialogManager *manager = new DBusFileDialogManager();
    Q_UNUSED(new FiledialogmanagerAdaptor(manager));

    if (!QDBusConnection::sessionBus().registerObject("/com/deepin/filemanager/filedialogmanager_wayland", manager)) {
        qWarning("Cannot register to the D-Bus object: \"/com/deepin/filemanager/filedialogmanager_wayland\"\n");
        manager->deleteLater();
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    // 设置环境变量xcb
    qputenv("QT_QPA_PLATFORM", "wayland");

    DApplication a(argc, argv);
    a.setOrganizationName("deepin");
    // 加载翻译文件
    DFMGlobal::installTranslator();
    // 设置该进程不要因为没有窗口了就退出
    a.setQuitOnLastWindowClosed(false);
    // 初始化日志系统
    DFMLogManager::registerFileAppender();
    // 打印平台信息
    qInfo() << "平台信息：" << QApplication::platformName();
    // ability to show file selection dialog
    if (!registerDialogDBus()) {
        qWarning() << "Register dialog dbus failed.";
        return -1;
    }

    return a.exec();
}
