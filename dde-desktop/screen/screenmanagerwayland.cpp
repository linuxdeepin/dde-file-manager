#include "screenmanagerwayland.h"
#include "screenobjectwayland.h"
#include "dbus/dbusdisplay.h"
#include "dbus/dbusdock.h"
#include "dbus/dbusmonitor.h"

#include <QGuiApplication>
#include <QScreen>

#define SCREENOBJECT(screen) dynamic_cast<ScreenObjectWayland*>(screen)
ScreenManagerWayland::ScreenManagerWayland(QObject *parent)
    : AbstractScreenManager(parent)
{
    m_display = new DBusDisplay(this);
    init();
}

ScreenManagerWayland::~ScreenManagerWayland()
{
    if (m_display){
        m_display->deleteLater();
        m_display = nullptr;
    }
}

ScreenPointer ScreenManagerWayland::primaryScreen()
{
    QString primaryName = m_display->primary();
    ScreenPointer ret;
    for ( ScreenPointer sp : m_screens.values()) {
        if (sp->name() == primaryName){
            ret = sp;
            break;
        }
    }
    return ret;
}

QVector<ScreenPointer> ScreenManagerWayland::screens() const
{
    return  m_screens.values().toVector();
}

qreal ScreenManagerWayland::devicePixelRatio() const
{
    //dbus获取的缩放不是应用值而是设置值，目前还是使用QT来获取
    return qApp->primaryScreen()->devicePixelRatio();
}

AbstractScreenManager::DisplayMode ScreenManagerWayland::displayMode() const
{
    AbstractScreenManager::DisplayMode ret = AbstractScreenManager::DisplayMode(m_display->displayMode());
    return ret;
}

void ScreenManagerWayland::onMonitorChanged()
{
    QStringList monitors;
    //检查新增的屏幕
    for (auto objectPath : m_display->monitors()){
        QString path = objectPath.path();

        //新增的
        if (!m_screens.contains(path)){
            ScreenPointer sp(new ScreenObjectWayland(new DBusMonitor(path)));
            m_screens.insert(path,sp);
            connectScreen(sp);
        }
        monitors << path;
    }

    //检查移除的屏幕
    for (const QString &path : m_screens.keys()){
        if (!monitors.contains(path)){
            ScreenPointer sp = m_screens.take(path);
            disconnectScreen(sp);
        }
    }
    emit sigScreenChanged();
}

void ScreenManagerWayland::onDockChanged()
{
    auto screen = primaryScreen();
    qDebug() << "dockkkkkkkkkkk" << screen->geometry() << screen->availableGeometry()
             << qApp->primaryScreen()->geometry() << qApp->primaryScreen()->availableGeometry();
    emit sigScreenAvailableGeometryChanged(screen, screen->availableGeometry());
}

void ScreenManagerWayland::onScreenGeometryChanged(const QRect &rect)
{
    ScreenObjectWayland *sc = SCREENOBJECT(sender());
    if (sc != nullptr) {
        ScreenPointer sp = m_screens.value(sc->path());
        if (sp.get() != nullptr) {
            emit sigScreenGeometryChanged(sp, rect);
        }
    }

    qDebug() << "ddddddddddddddd" << sc->geometry() << sc->availableGeometry()
             << qApp->primaryScreen()->geometry() << qApp->primaryScreen()->availableGeometry();
}

void ScreenManagerWayland::init()
{
    m_screens.clear();

    //先尝试使用Qt信号，若有问题再使用DBUS的信号
    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenManagerWayland::onMonitorChanged);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreenManagerWayland::onMonitorChanged);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &AbstractScreenManager::sigScreenChanged);
    connect(m_display, &DBusDisplay::DisplayModeChanged, this, &AbstractScreenManager::sigDisplayModeChanged);

    //dock区处理
    connect(DockGeoIns,&DBusDockGeometry::GeometryChanged,this, &ScreenManagerWayland::onDockChanged);
    connect(DockInfoIns,&DBusDock::HideModeChanged,this, &ScreenManagerWayland::onDockChanged);
    connect(DockInfoIns,&DBusDock::PositionChanged,this, &ScreenManagerWayland::onDockChanged);

    //初始化屏幕
    for (auto objectPath : m_display->monitors()){
        const QString path = objectPath.path();
        ScreenPointer sp(new ScreenObjectWayland(new DBusMonitor(path)));
        m_screens.insert(path,sp);
        connectScreen(sp);
    }
}

void ScreenManagerWayland::connectScreen(ScreenPointer sp)
{
    connect(sp.get(),&AbstractScreen::sigGeometryChanged,this,
            &ScreenManagerWayland::onScreenGeometryChanged);
}

void ScreenManagerWayland::disconnectScreen(ScreenPointer sp)
{
    disconnect(sp.get(),&AbstractScreen::sigGeometryChanged,this,
            &ScreenManagerWayland::onScreenGeometryChanged);
}
