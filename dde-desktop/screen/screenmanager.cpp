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
    m_screens.append(psc);
    emit sigScreenChanged();
}

void ScreenManager::onScreenRemoved(QScreen *screen)
{
    for (int i = 0; i < m_screens.size(); ++i){
        if (SCREENOBJECT(m_screens.at(i).get())->screen() == screen){
            m_screens.remove(i);
            break;
        }
    }
    emit sigScreenChanged();
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
        m_screens.append(psc);
    }
}

ScreenPointer ScreenManager::primaryScreen()
{
    ScreenPointer ret;
    QScreen *primary = qApp->primaryScreen();
    for (ScreenPointer psc : m_screens){
        if (SCREENOBJECT(psc.get())->screen() == primary){
            ret = ScreenPointer(psc.get());
            break;
        }
    }
    Q_ASSERT(ret.get() != nullptr);
    return ret;
}

QVector<ScreenPointer> ScreenManager::screens() const
{
    return m_screens;
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
