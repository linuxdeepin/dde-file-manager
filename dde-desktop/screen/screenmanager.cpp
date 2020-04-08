#include "screenmanager.h"
#include "screenobject.h"
#include "dbus/dbusdisplay.h"
#include <QGuiApplication>

#define SCREENOBJECT(screen) dynamic_cast<ScreenObject*>(screen)
ScreenManager::ScreenManager(QObject *parent)
    : AbstractScreenManager(parent)
{
    m_display = new DBusDisplay(this);
    init();
}

ScreenManager::~ScreenManager()
{

}

void ScreenManager::onScreenAdded(QScreen *screen)
{
    ScreenObjectPointer psc(new ScreenObject(screen));
    m_screens.insert(screen,psc);
    connectScreen(psc);

    emit sigScreenChanged();
}

void ScreenManager::onScreenRemoved(QScreen *screen)
{
    auto psc = m_screens.take(screen);
    if (psc.get() != nullptr){
        disconnectScreen(psc);
        emit sigScreenChanged();
    }
}

void ScreenManager::onScreenGeometryChanged(const QRect &rect)
{
    QScreen *sc = dynamic_cast<QScreen *>(sender());
    if (sc != nullptr) {
        ScreenPointer sp = m_screens.value(sc);
        if (sp.get() != nullptr) {
            emit sigScreenGeometryChanged(sp, rect);
        }
    }
}

void ScreenManager::onScreenAvailableGeometryChanged(const QRect &rect)
{
    QScreen *sc = dynamic_cast<QScreen *>(sender());
    if (sc != nullptr) {
        ScreenPointer sp = m_screens.value(sc);
        if (sp.get() != nullptr) {
            emit sigScreenAvailableGeometryChanged(sp, rect);
        }
    }
}

void ScreenManager::init()
{
    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenManager::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreenManager::onScreenRemoved);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &AbstractScreenManager::sigScreenChanged);
    connect(m_display, &DBusDisplay::DisplayModeChanged, this, &AbstractScreenManager::sigDisplayModeChanged);

    m_screens.clear();
    for (QScreen *sc : qApp->screens()){
        ScreenPointer psc(new ScreenObject(sc));
        m_screens.insert(sc,psc);
        connectScreen(psc);
    }
}

void ScreenManager::connectScreen(ScreenPointer psc)
{
    connect(psc.get(),&AbstractScreen::sigGeometryChanged,this,
            &ScreenManager::onScreenGeometryChanged);
    connect(psc.get(),&AbstractScreen::sigAvailableGeometryChanged,this,
            &ScreenManager::onScreenAvailableGeometryChanged);
}


void ScreenManager::disconnectScreen(ScreenPointer psc)
{
    disconnect(psc.get(),&AbstractScreen::sigGeometryChanged,this,
               &ScreenManager::onScreenGeometryChanged);
    disconnect(psc.get(),&AbstractScreen::sigAvailableGeometryChanged,this,
                &ScreenManager::onScreenAvailableGeometryChanged);
}

ScreenPointer ScreenManager::primaryScreen()
{
    QScreen *primary = qApp->primaryScreen();
    ScreenPointer ret = m_screens.value(primary);
    Q_ASSERT(ret.get() != nullptr);
    return ret;
}

QVector<ScreenPointer> ScreenManager::screens() const
{
    return m_screens.values().toVector();
}

qreal ScreenManager::devicePixelRatio() const
{
    return qApp->primaryScreen()->devicePixelRatio();
}

AbstractScreenManager::DisplayMode ScreenManager::displayMode() const
{
    AbstractScreenManager::DisplayMode ret = AbstractScreenManager::DisplayMode(m_display->displayMode());
    return ret;
}
