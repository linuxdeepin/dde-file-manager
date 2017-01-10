#include "mainwindow.h"
#include <DApplication>
#include <QDesktopWidget>
#include "dplatformwindowhandle.h"
#include <DLog>
#include "app/cmdmanager.h"
#include <QDebug>
#include <QFile>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
DUTIL_USE_NAMESPACE
//DWIDGET_BEGIN_NAMESPACE

int main(int argc, char *argv[])
{
    //logger
    DLogManager::registerConsoleAppender();

    DApplication::loadDXcbPlugin();
    DApplication a(argc, argv);

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

            //command line
    CMDManager::instance()->process(a);
    const bool isOrderFormat = CMDManager::instance()->isSet("f");

    if(!isOrderFormat)
        return 1;

    const QString path = CMDManager::instance()->getPath();
    if(path.isEmpty() || !QFile::exists(path))
        return 1;

    MainWindow* w = new MainWindow(path);
    w->show();
    QRect rect = w->geometry();
    rect.moveCenter(qApp->desktop()->geometry().center());
    w->move(rect.x(), rect.y());

    int code = a.exec();
    quick_exit(code);
}
