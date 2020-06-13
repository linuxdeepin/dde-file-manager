#include "screenobjectwayland.h"
#include "dbus/dbusmonitor.h"
#include "dbus/dbusdock.h"

#include <QGuiApplication>
#include <QScreen>

static QRect dealRectRatio(QRect orgRect)
{
    //处理缩放，先不考虑高分屏的特殊处理
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    if (ratio != 1.0)
        orgRect = QRect(orgRect.x(), orgRect.y()
                        , static_cast<int>(orgRect.width() / ratio)
                        , static_cast<int>(orgRect.height() / ratio));
    return orgRect;
}

ScreenObjectWayland::ScreenObjectWayland(DBusMonitor *monitor,QObject *parent)
    : AbstractScreen(parent)
    , m_monitor(monitor)
{
    init();
}

ScreenObjectWayland::~ScreenObjectWayland()
{
    if (m_monitor){
        m_monitor->deleteLater();
        m_monitor = nullptr;
    }
}

QString ScreenObjectWayland::name() const
{
    return  m_monitor->name();
}

QRect ScreenObjectWayland::geometry() const
{
    QRect orgRect = m_monitor->rect();
    orgRect = dealRectRatio(orgRect);
    return orgRect;
}

QRect ScreenObjectWayland::availableGeometry() const
{
    QRect ret = geometry(); //已经缩放过

    int dockHideMode = DockInfoIns->hideMode();
    if ( 1 == dockHideMode){ //隐藏
        qDebug() << "dock is Hidden";
        return ret;
    }

    //DockGeoIns->getGeometry(); //经过缩放处理后的docks,有问题
    DockRect dockrectI = DockInfoIns->frontendWindowRect(); //原始dock大小
    QRect dockrect = dealRectRatio(dockrectI.operator QRect());  //缩放处理
    switch (DockInfoIns->position()) {
    case 0: //上
        ret.setY(ret.y() + dockrect.height());
        qDebug() << "dock on top" << dockrect;
        break;
    case 1: //右
        ret.setWidth(ret.width() - dockrect.width());
        qDebug() << "dock on right" << dockrect;
        break;
    case 2: //下
        ret.setHeight(ret.height() - dockrect.height());
        qDebug() << "dock on bottom" << dockrect;
        break;
    case 3: //左
        ret.setX(ret.x() + dockrect.width());
        qDebug() << "dock on left" << dockrect;
        break;
    default:
        qCritical() << "dock postion error!";
        break;
    }
    return ret;
}

QRect ScreenObjectWayland::handleGeometry() const
{
    return m_monitor->rect();
}

QString ScreenObjectWayland::path() const
{
    return  m_monitor->path();
}

void ScreenObjectWayland::init()
{
    connect(m_monitor,&DBusMonitor::monitorRectChanged,[this](){
        emit sigGeometryChanged(geometry());
    });
}
