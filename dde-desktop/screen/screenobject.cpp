#include "screenobject.h"

ScreenObject::ScreenObject(QScreen *sc, QObject *parent)
  : AbstractScreen(parent)
  ,m_screen(sc)
{

}

ScreenObject::~ScreenObject()
{

}

QString ScreenObject::name() const
{
    m_screen->name();
}

QRect ScreenObject::geometry() const
{
    m_screen->geometry();
}

QScreen *ScreenObject::screen() const
{
    return  m_screen;
}

void ScreenObject::init()
{
    connect(m_screen,SIGNAL(geometryChanged(const QRect &geometry)),this,SIGNAL(sigGeometryChanged()));
}
