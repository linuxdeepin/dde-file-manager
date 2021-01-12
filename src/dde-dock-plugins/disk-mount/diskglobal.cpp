#include "diskglobal.h"
#include <QProcessEnvironment>

DiskGlobal::DiskGlobal(QObject * parent):QObject (parent)
{

}

bool DiskGlobal::isWayLand()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    //在wayland平台下设置固定大小，解决属性框最大化问题
    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}
