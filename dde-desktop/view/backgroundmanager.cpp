#include "backgroundmanager.h"
#include "screen/screenhelper.h"
#include "util/xcb/xcb.h"

BackgroundManager::BackgroundManager(bool preview, QObject *parent)
    : QObject(parent)
    ,windowManagerHelper(DWindowManagerHelper::instance())
    ,m_preview(preview)
{
    init();
}

BackgroundManager::~BackgroundManager()
{
    if (gsettings){
        gsettings->deleteLater();
        gsettings = nullptr;
    }

    if (wmInter){
        wmInter->deleteLater();
        wmInter = nullptr;
    }

    windowManagerHelper = nullptr;

    m_backgroundMap.clear();
}

void BackgroundManager::onRestBackgroundManager()
{
    //判断是否需要绘制背景，如果是窗口绘制背景则桌面isEnable返回false，即桌面不绘制背景
    if (m_preview || isEnabled()) {
        if (wmInter) {
            return;
        }

        wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this);
        gsettings = new DGioSettings("com.deepin.dde.appearance", "", this);

        if (!m_preview) {
            connect(wmInter, &WMInter::WorkspaceSwitched, this, [this] (int, int to) {
                currentWorkspaceIndex = to;
                //updateBackground(); //todo
            });

            connect(gsettings, &DGioSettings::valueChanged, this, [this] (const QString & key, const QVariant & value) {
                Q_UNUSED(value);
                if (key == "background-uris") {
                    //updateBackground(); //todo
                }
            });
        }

        //屏幕改变
        connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenChanged,
                this, &BackgroundManager::onBackgroundBuild);

        //显示模式改变
        connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigDisplayModeChanged,
                this, &BackgroundManager::onBackgroundBuild);

        //创建背景
        onBackgroundBuild();
    } else {
        if (!wmInter) {
            return;
        }

        // 清理数据
        gsettings->deleteLater();
        gsettings = nullptr;

        wmInter->deleteLater();
        wmInter = nullptr;

        //currentWallpaper.clear();
        currentWorkspaceIndex = 0;
        //backgroundPixmap = QPixmap();

        disconnect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenChanged,
                this, &BackgroundManager::onBackgroundBuild);

        //销毁窗口
        m_backgroundMap.clear();
    }

    emit sigBackgroundEnableChanged();
}

void BackgroundManager::init()
{
    if (!m_preview) {
        connect(windowManagerHelper, &DWindowManagerHelper::windowManagerChanged,
                this, &BackgroundManager::onRestBackgroundManager);
        connect(windowManagerHelper, &DWindowManagerHelper::hasCompositeChanged,
                this, &BackgroundManager::onRestBackgroundManager);
    }

    onRestBackgroundManager();
}

BackgroundWidgetPointer BackgroundManager::createBackgroundWidget(ScreenPointer screen)
{
    BackgroundWidgetPointer bwp(new BackgroundWidget);
    bwp->setProperty("isPreview", m_preview);
    bwp->setProperty("myScreen", screen->name()); // assert screen->name is unique
    bwp->createWinId();
    bwp->setGeometry(screen->geometry()); //经过缩放的区域

    if (m_preview) {
        bwp->setWindowFlags(bwp->windowFlags() | Qt::BypassWindowManagerHint | Qt::WindowDoesNotAcceptFocus);
    } else {
        Xcb::XcbMisc::instance().set_window_type(bwp->winId(), Xcb::XcbMisc::Desktop);
    }

    if (m_visible)
        bwp->show();
    else
        qDebug() << "Disable show the background widget, of screen:" << screen << screen->geometry();
    return bwp;
}

bool BackgroundManager::isEnabled() const
{
    // 只支持kwin，或未开启混成的桌面环境
    return windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM || !windowManagerHelper->hasComposite();
}

void BackgroundManager::onBackgroundBuild()
{
    //屏幕模式判断
    AbstractScreenManager::DisplayMode mode = ScreenMrg->displayMode();
    qDebug() << "screen mode" << mode;

    //实际是单屏
    if (AbstractScreenManager::Showonly == mode || AbstractScreenManager::Duplicate == mode){
        //移除多余的屏幕
        m_backgroundMap.clear();

        ScreenPointer primary = ScreenMrg->primaryScreen();
        BackgroundWidgetPointer bwp = createBackgroundWidget(primary);
        m_backgroundMap.insert(primary,bwp);
        //todo 设置壁纸
        bwp->setPixmap(QPixmap("/home/uos/Pictures/Wallpapers/abc-123.jpg"));

    }
    else {  //多屏

    }


    //删除没有的屏幕
}
