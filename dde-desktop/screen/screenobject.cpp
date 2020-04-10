#include "screenobject.h"
#include "dbus/dbusdock.h"

#include "util/xcb/xcb.h"

#include <qpa/qplatformscreen.h>
#include <qdebug.h>

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

    //使用xcb获取dock区
    xcb_ewmh_wm_strut_partial_t dock_xcb_ewmh_wm_strut_partial_t;
    memset(&dock_xcb_ewmh_wm_strut_partial_t, 0, sizeof(xcb_ewmh_wm_strut_partial_t));
    auto structParialInfoList = Xcb::XcbMisc::instance().find_dock_window();
    for (auto info : structParialInfoList) {
        if (info.rc.isValid()) {
            dock_xcb_ewmh_wm_strut_partial_t = Xcb::XcbMisc::instance().get_strut_partial(info.winId);
            break;
        }
    }

    QRect availableRect = geometry();
    if (dock_xcb_ewmh_wm_strut_partial_t.top > 0) {
        availableRect.setY(dock_xcb_ewmh_wm_strut_partial_t.top);
    } else if (dock_xcb_ewmh_wm_strut_partial_t.right > 0) {
        availableRect.setWidth(2 * availableRect.width() - dock_xcb_ewmh_wm_strut_partial_t.right);
    } else if (dock_xcb_ewmh_wm_strut_partial_t.bottom > 0) {
        availableRect.setHeight(availableRect.height() - dock_xcb_ewmh_wm_strut_partial_t.bottom);
    } else if (dock_xcb_ewmh_wm_strut_partial_t.left > 0) {
        availableRect.setX(dock_xcb_ewmh_wm_strut_partial_t.left);
    }
    return availableRect;
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
