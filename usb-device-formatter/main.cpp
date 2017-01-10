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
DUTIL_USE_NAMESPACE
//DWIDGET_BEGIN_NAMESPACE

int main(int argc, char *argv[])
{
    //Logger
    DLogManager::registerConsoleAppender();

    //Load DXcbPlugin
    DApplication::loadDXcbPlugin();
    DApplication a(argc, argv);

    //Load translation
    QTranslator *translator = new QTranslator(QCoreApplication::instance());

    translator->load("/usr/share/usb-device-formatter/translations/usb-device-formatter_"
                     +QLocale::system().name()+".qm");
    a.installTranslator(translator);

    a.setTheme("light");
    a.setFont(QFont("",10));
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
        QString message = QObject::tr("Device does not exit");
        MessageDialog d(message, 0);
        d.exec();
        return 0;
    }

    //Check if is a removable device
    PartMan::Partition p = PartMan::Partition::getPartitionByDevicePath(path);
    if(!p.getIsRemovable()){
        QString message = QObject::tr("Cannot format native device");
        MessageDialog d(message, 0);
        d.exec();
        return 0;
    }

    MainWindow* w = new MainWindow(path);
    w->show();
    QRect rect = w->geometry();
    rect.moveCenter(qApp->desktop()->geometry().center());
    w->move(rect.x(), rect.y());

    int code = a.exec();
    quick_exit(code);
}
