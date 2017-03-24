#include <DApplication>
#include <QDesktopWidget>
#include <DLog>
#include <QDebug>
#include <QFile>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include "app/cmdmanager.h"
#include "view/mainwindow.h"
#include "dplatformwindowhandle.h"
#include "dialogs/messagedialog.h"
#include "../partman/partition.h"
#include "app/singletonapp.h"
#include <QProcessEnvironment>
#include <QX11Info>
#include <X11/Xlib.h>
DUTIL_USE_NAMESPACE
//DWIDGET_BEGIN_NAMESPACE

int main(int argc, char *argv[])
{
    //Logger
    DLogManager::registerConsoleAppender();

    //Load DXcbPlugin
    DApplication::loadDXcbPlugin();
    DApplication a(argc, argv);

    qputenv("QT_IM_MODULE", "xim");
    qputenv("GTK_IM_MODULE", "xim");
    qputenv("QT4_IM_MODULE", "xim");
    qputenv("IM_CONFIG_PHASE", "1");
    qputenv("XMODIFIERS", "@im");
    qputenv("IBUS_DISABLE_SNOOPER", "1");
    qputenv("QT_LINUX_ACCESSIBILITY_ALWAYS_ON", "1");
    qputenv("WINDOWPATH","2");
    qputenv("XDG_SESSION_TYPE","x11");
    qputenv("PAGER", "less");
    qputenv("PAGER", "-R");
    qputenv("SHLVL", "1");
    qputenv("DBUS_STARTER_BUS_TYPE", "session");
    qputenv("GDMSESSION", "deepin");
    qputenv("DESKTOP_SESSION", "deepin");

    //Singleton app handle
    bool isSingletonApp = SingletonApp::instance()->setSingletonApplication("usb-device-formatter");
    if(!isSingletonApp)
        return 0;

    //Load translation
    QTranslator *translator = new QTranslator(QCoreApplication::instance());

    translator->load("/usr/share/usb-device-formatter/translations/usb-device-formatter_"
                     +QLocale::system().name()+".qm");
    a.installTranslator(translator);

    a.setTheme("light");
    a.setOrganizationName("deepin");
    a.setApplicationName("Deepin usb device formatter");
    a.setApplicationVersion("1.0");
    a.setWindowIcon(QIcon(":/app/usb-device-formatter.png"));
    a.setQuitOnLastWindowClosed(true);

    //Command line
    CMDManager::instance()->process(a);

    //Check if exists path
    const QString path = CMDManager::instance()->getPath();
    if(path.isEmpty() || !QFile::exists(path)){
        QString message = QObject::tr("Device does not exist");
        MessageDialog d(message, 0);
        d.exec();
        return 0;
    }

    //Check if is a removable device
    PartMan::Partition p = PartMan::Partition::getPartitionByDevicePath(path);
    if(!p.getIsRemovable()){
        QString message = QObject::tr("Cannot format local device");
        MessageDialog d(message, 0);
        d.exec();
        return 0;
    }

    MainWindow* w = new MainWindow(path);
    w->show();
    QRect rect = w->geometry();
    rect.moveCenter(qApp->desktop()->geometry().center());
    w->move(rect.x(), rect.y());

    if(CMDManager::instance()->isSet("m")){
        int parentWinId = CMDManager::instance()->getWinId();
        int winId = w->winId();

        if(parentWinId != -1)
            qDebug() << XSetTransientForHint(QX11Info::display(), (Window)winId, (Window)parentWinId);
    }

    int code = a.exec();
    quick_exit(code);
}
