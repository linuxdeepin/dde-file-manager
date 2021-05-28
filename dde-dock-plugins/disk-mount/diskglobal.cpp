#include "diskglobal.h"
#include <QProcessEnvironment>
#include <QApplication>

DiskGlobal::DiskGlobal(QObject * parent):QObject (parent)
{

}

bool DiskGlobal::isWayLand()
{
    //! 该函数只能在QApplication之后调用才能返回有效的值，在此之前会返回空值
    Q_ASSERT(qApp);
    return QApplication::platformName() == "wayland";
}
