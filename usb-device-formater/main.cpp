#include "mainwindow.h"
#include <DApplication>
#include <QDesktopWidget>
#include "dplatformwindowhandle.h"

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();
    DApplication a(argc, argv);
    a.setTheme("light");
    a.setFont(QFont("",10));

    MainWindow* w = new MainWindow(QObject::tr("Format"));
    w->show();
    QRect rect = w->geometry();
    rect.moveCenter(qApp->desktop()->geometry().center());
    w->move(rect.x(), rect.y());

    return a.exec();
}
