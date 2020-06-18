#include "screenobject.h"
#include "dbus/dbusdock.h"

//#include "util/xcb/xcb.h"

#include <qpa/qplatformscreen.h>
#include <QGuiApplication>
#include <qdebug.h>

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

ScreenObject::ScreenObject(QScreen *sc, QObject *parent)
  : AbstractScreen(parent)
  ,m_screen(sc)
{
    init();
}

ScreenObject::~ScreenObject()
{

}

QString ScreenObject::name() const
{
    return m_screen->name();
}

QRect ScreenObject::geometry() const
{
    return m_screen->geometry();
}

QRect ScreenObject::availableGeometry() const
{
    //!QScreen::availableGeometry在刚启动时返回的值是错的，需要拖到下dock区才能正确显示
    //return m_screen->availableGeometry();
    //end

    //使用xcb获取dock区 任务栏切换位置时显示异常
#if 0
    qreal dxratio = m_screen->devicePixelRatio();
    xcb_ewmh_wm_strut_partial_t dock_xcb_ewmh_wm_strut_partial_t;
    memset(&dock_xcb_ewmh_wm_strut_partial_t, 0, sizeof(xcb_ewmh_wm_strut_partial_t));
    auto structParialInfoList = Xcb::XcbMisc::instance().find_dock_window();
    QSize dockSize;
    for (auto info : structParialInfoList) {
        if (info.rc.isValid()) {
            dock_xcb_ewmh_wm_strut_partial_t = Xcb::XcbMisc::instance().get_strut_partial(info.winId);
            dockSize.setWidth(info.rc.width() / dxratio);   //缩放
            dockSize.setHeight(info.rc.height() / dxratio); //缩放
            break;
        }
    }

    QRect availableRect = geometry();
    if (dock_xcb_ewmh_wm_strut_partial_t.top > 0) {
        availableRect.setY(dockSize.height());
    } else if (dock_xcb_ewmh_wm_strut_partial_t.right > 0) {
        availableRect.setWidth(availableRect.width() - dockSize.width());
    } else if (dock_xcb_ewmh_wm_strut_partial_t.bottom > 0) {
        availableRect.setHeight(availableRect.height() - dockSize.height());
    } else if (dock_xcb_ewmh_wm_strut_partial_t.left > 0) {
        availableRect.setX(dockSize.width());
    }
    return availableRect;
#else //用dbus
    QRect ret = geometry(); //已经缩放过

    int dockHideMode = DockInfoIns->hideMode();
    if ( 1 == dockHideMode) {//隐藏
        qDebug() << "dock is Hidden";
        return ret;
    }

    DockRect dockrectI = DockInfoIns->frontendWindowRect(); //原始dock大小
    QRect dockrect = dealRectRatio(dockrectI.operator QRect());  //缩放处理
     qDebug() << "frontendWindowRect: dockrectI " << QRect(dockrectI);
     qDebug() << "dealRectRatio dockrect " << dockrect;
     qDebug() << "ScreenObject ret " << ret;
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
#endif
}

QRect ScreenObject::handleGeometry() const
{
    return m_screen->handle()->geometry();
}

QScreen *ScreenObject::screen() const
{
    return m_screen;
}

void ScreenObject::init()
{
    connect(m_screen,SIGNAL(geometryChanged(const QRect &)),this,SIGNAL(sigGeometryChanged(const QRect &)));
    connect(m_screen,SIGNAL(availableGeometryChanged(const QRect &)),this,SIGNAL(sigAvailableGeometryChanged(const QRect &)));
}
