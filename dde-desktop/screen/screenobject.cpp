#include "screenobject.h"

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
    return m_screen->availableGeometry();
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
