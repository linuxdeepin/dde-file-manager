#include "diskglobal.h"
#include <QProcessEnvironment>
#include <QApplication>

DiskGlobal::DiskGlobal(QObject * parent):QObject (parent)
{

}

bool DiskGlobal::isWayLand()
{
    return QApplication::platformName() == "wayland";
}
