#include "screenmanager.h"
#include "screenobject.h"
#include "dbus/dbusdisplay.h"
#include "dbus/dbusdock.h"

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
    if (screen == nullptr || m_screens.contains(screen))
        return;

    ScreenObjectPointer psc(new ScreenObject(screen));
    m_screens.insert(screen, psc);
    connectScreen(psc);

    //emit sigScreenChanged();
    appendEvent(Screen);
}

void ScreenManager::onScreenRemoved(QScreen *screen)
{
    auto psc = m_screens.take(screen);
    if (psc.get() != nullptr) {
        disconnectScreen(psc);
        //emit sigScreenChanged();
        appendEvent(Screen);
    }
}

void ScreenManager::onScreenGeometryChanged(const QRect &rect)
{
    Q_UNUSED(rect)
//    ScreenObject *sc = SCREENOBJECT(sender());
//    if (sc != nullptr && m_screens.contains(sc->screen())) {
//        emit sigScreenGeometryChanged(m_screens.value(sc->screen()), rect);
//    }
    appendEvent(AbstractScreenManager::Geometry);
}

void ScreenManager::onScreenAvailableGeometryChanged(const QRect &rect)
{
    Q_UNUSED(rect)
//    ScreenObject *sc = SCREENOBJECT(sender());
//    if (sc != nullptr && m_screens.contains(sc->screen())) {
//        emit sigScreenAvailableGeometryChanged(m_screens.value(sc->screen()), rect);
//    }
    appendEvent(AbstractScreenManager::AvailableGeometry);
}

void ScreenManager::onDockChanged()
{
//    int dockHideMode = DockInfoIns->hideMode();
//    if (3 != dockHideMode) //只处理智能隐藏
//        return ;
#ifdef UNUSED_SMARTDOCK
    auto primary = primaryScreen();
    if (primary == nullptr) {
        qCritical() << "get primary screen failed";
        return;
    } else {
        emit sigScreenAvailableGeometryChanged(primary, primary->availableGeometry());
    }
#else
    //新增动态dock区功能，dock区不再只是在主屏幕,随鼠标移动
    //emit sigScreenAvailableGeometryChanged(nullptr, QRect());
    appendEvent(AbstractScreenManager::AvailableGeometry);
#endif
}

void ScreenManager::init()
{
    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenManager::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreenManager::onScreenRemoved);
    //connect(qApp, &QGuiApplication::primaryScreenChanged, this, &AbstractScreenManager::sigScreenChanged);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [this]() {
        this->appendEvent(Screen);
    });
#ifdef UNUSE_TEMP
    connect(m_display, &DBusDisplay::DisplayModeChanged, this, &AbstractScreenManager::sigDisplayModeChanged);
#else
    //临时方案，
    connect(m_display, &DBusDisplay::DisplayModeChanged, this, [this]() {
        //emit sigDisplayModeChanged();
        m_lastMode = m_display->GetRealDisplayMode();
        this->appendEvent(Mode);
    });

    //临时方案，使用PrimaryRectChanged信号作为拆分/合并信号
    connect(m_display, &DBusDisplay::PrimaryRectChanged, this, [this]() {
        int mode = m_display->GetRealDisplayMode();
        qDebug() << "deal merge and split" << mode << m_lastMode;
        if (m_lastMode == mode)
            return;
        m_lastMode = mode;
        //emit sigDisplayModeChanged();
        this->appendEvent(Mode);
    });

    m_lastMode = m_display->GetRealDisplayMode();
#endif

    //dock区处理
    connect(DockInfoIns, &DBusDock::FrontendWindowRectChanged, this, &ScreenManager::onDockChanged);
    connect(DockInfoIns, &DBusDock::HideModeChanged, this, &ScreenManager::onDockChanged);
    //connect(DockInfoIns,&DBusDock::PositionChanged,this, &ScreenManager::onDockChanged); 不关心位子改变，有bug#25148，全部由区域改变触发

    m_screens.clear();
    for (QScreen *sc : qApp->screens()) {
        ScreenPointer psc(new ScreenObject(sc));
        m_screens.insert(sc, psc);
        connectScreen(psc);
    }
}

void ScreenManager::connectScreen(ScreenPointer psc)
{
    connect(psc.get(), &AbstractScreen::sigGeometryChanged, this,
            &ScreenManager::onScreenGeometryChanged);
//为了解决bug33117，
//在dock时尚模式下，去切换dock位置后，会先触发QScreen的sigAvailableGeometryChanged，此时去获取
//dock的区域不正确，继而导致桌面栅格计算不正确从而导致自定义下记录的桌面图标无法恢复而异常位置。先暂时屏蔽之。
//至于此处信号会影响什么地方暂时不可追溯（目前在x86单双屏切换dock均未发现有何影响）。

//    connect(psc.get(),&AbstractScreen::sigAvailableGeometryChanged,this,
//            &ScreenManager::onScreenAvailableGeometryChanged);
}


void ScreenManager::disconnectScreen(ScreenPointer psc)
{
    disconnect(psc.get(), &AbstractScreen::sigGeometryChanged, this,
               &ScreenManager::onScreenGeometryChanged);
    disconnect(psc.get(), &AbstractScreen::sigAvailableGeometryChanged, this,
               &ScreenManager::onScreenAvailableGeometryChanged);
}

ScreenPointer ScreenManager::primaryScreen()
{
    QScreen *primary = qApp->primaryScreen();
    ScreenPointer ret = m_screens.value(primary);
    //Q_ASSERT(ret.get() != nullptr);
    return ret;
}

QVector<ScreenPointer> ScreenManager::screens() const
{
    QVector<ScreenPointer> order;
    for (QScreen *sc : qApp->screens()) {
        if (m_screens.contains(sc)) {
            if (sc->geometry().size() == QSize(0, 0))
                qCritical() << "screen error. does it is closed?";
            order.append(m_screens.value(sc));
        }
    }
    return order;
}

QVector<ScreenPointer> ScreenManager::logicScreens() const
{
    QVector<ScreenPointer> order;
    auto screens = qApp->screens();

    //调整主屏幕到第一
    QScreen *primary = qApp->primaryScreen();
    screens.removeOne(primary);
    screens.push_front(primary);

    for (QScreen *sc : screens) {
        if (m_screens.contains(sc))
            order.append(m_screens.value(sc));
    }
    return order;
}

ScreenPointer ScreenManager::screen(const QString &name) const
{
    ScreenPointer ret;
    auto screens = m_screens.values();
    auto iter = std::find_if(screens.begin(), screens.end(), [name](const ScreenPointer & sp) {
        return sp->name() == name;
    });

    if (iter != screens.end()) {
        ret = *iter;
    }

    return ret;
}

qreal ScreenManager::devicePixelRatio() const
{
    return qApp->primaryScreen()->devicePixelRatio();
}

AbstractScreenManager::DisplayMode ScreenManager::displayMode() const
{
    auto pending = m_display->GetRealDisplayMode();
    pending.waitForFinished();
    if (pending.isError()) {
        qWarning() << "Display GetRealDisplayMode Error:" << pending.error().name() << pending.error().message();
        AbstractScreenManager::DisplayMode ret = AbstractScreenManager::DisplayMode(m_display->displayMode());
        return ret;
    } else {
        /*
        DisplayModeMirror: 1
        DisplayModeExtend: 2
        DisplayModeOnlyOne: 3
        DisplayModeUnknow: 4
        */
        int mode = pending.argumentAt(0).toInt();
        qDebug() << "GetRealDisplayMode resulet" << mode;
        if (mode > 0 && mode < 4)
            return static_cast<AbstractScreenManager::DisplayMode>(mode);
        else
            return AbstractScreenManager::Custom;
    }
}

AbstractScreenManager::DisplayMode ScreenManager::lastChangedMode() const
{
    return static_cast<AbstractScreenManager::DisplayMode>(m_lastMode);
}

void ScreenManager::reset()
{
    if (m_display) {
        delete m_display;
        m_display = nullptr;
    }

    m_display = new DBusDisplay(this);
    init();
}
