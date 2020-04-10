#include "canvasviewmanager.h"
#include "screen/screenhelper.h"
#include "util/xcb/xcb.h"


//static QRect relativeRect(QRect avRect,const QRect &geometry)
//{
//    return;
//}

CanvasViewManager::CanvasViewManager(BackgroundManager *bmrg, QObject *parent)
    : QObject(parent)
    , m_background(bmrg)
{
    init();
}

CanvasViewManager::~CanvasViewManager()
{
    m_background = nullptr;
    m_canvasMap.clear();
}

void CanvasViewManager::onCanvasViewBuild(int imode)
{
    //屏幕模式判断
    AbstractScreenManager::DisplayMode mode = (AbstractScreenManager::DisplayMode)imode;

    //实际是单屏
    if ((AbstractScreenManager::Showonly == mode) || (AbstractScreenManager::Duplicate == mode) //仅显示和复制
            || (ScreenMrg->screens().count() == 1)){

        ScreenPointer primary = ScreenMrg->primaryScreen();
        CanvasViewPointer mView = m_canvasMap.value(primary);

        //删除其他
        m_canvasMap.clear();

        if (mView.get() == nullptr){
            mView = CanvasViewPointer(new CanvasGridView());
            mView->show();
            mView->initRootUrl();
        }
        mView->setGeometry(primary->availableGeometry());
        m_canvasMap.insert(primary, mView);
    }
    else {

        auto currentScreens = ScreenMrg->screens();
        //检查新增的屏幕
        for (const ScreenPointer &sp : currentScreens){
            CanvasViewPointer mView = m_canvasMap.value(sp);

            //新增
            if (mView.get() == nullptr){
                mView = CanvasViewPointer(new CanvasGridView());
                mView->show();
                mView->initRootUrl();
                m_canvasMap.insert(sp, mView);
            }

            qDebug() << "Sssssss mode" << mode << mView->geometry() <<sp->availableGeometry()<< sp->geometry()
                     << sp->name();
            mView->setGeometry(sp == ScreenMrg->primaryScreen() ? sp->availableGeometry() : sp->geometry());
        }

        //检查移除的屏幕
        for (const ScreenPointer &sp : m_canvasMap.keys()){
            if (!currentScreens.contains(sp)){
                m_canvasMap.remove(sp);
            }
        }
    }

    onBackgroundEnableChanged();
}

void CanvasViewManager::onBackgroundEnableChanged()
{
    if (m_background->isEnabled()) {
        for (const ScreenPointer &sp : m_canvasMap.keys()){

            CanvasViewPointer mView = m_canvasMap.value(sp);
            BackgroundWidgetPointer bw = m_background->backgroundWidget(sp);
            mView->setAttribute(Qt::WA_NativeWindow, false);
            bw->setView(mView);
            mView->setGeometry(sp == ScreenMrg->primaryScreen() ? sp->availableGeometry() : sp->geometry());
            mView->show();
        }
    }
    else {
        for (const ScreenPointer &sp : m_canvasMap.keys()){

            CanvasViewPointer mView = m_canvasMap.value(sp);
            mView->setParent(nullptr);
            mView->setWindowFlag(Qt::FramelessWindowHint, true);
            Xcb::XcbMisc::instance().set_window_type(mView->winId(), Xcb::XcbMisc::Desktop);
            mView->setGeometry(sp == ScreenMrg->primaryScreen() ? sp->availableGeometry() : sp->geometry());
            mView->show();
        }
    }
}

void CanvasViewManager::onScreenGeometryChanged(ScreenPointer sp)
{
    CanvasViewPointer mView = m_canvasMap.value(sp);
    if (mView.get() != nullptr){
        mView->setGeometry(sp == ScreenMrg->primaryScreen() ? sp->availableGeometry() : sp->geometry());
    }
}

void CanvasViewManager::init()
{
    //屏幕增删，模式改变
    connect(m_background,&BackgroundManager::sigBackgroundBuilded
            , this,&CanvasViewManager::onCanvasViewBuild);

    //屏幕大小改变
    connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenGeometryChanged,
            this, &CanvasViewManager::onScreenGeometryChanged);

    //可用区改变
    connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenAvailableGeometryChanged,
            this, &CanvasViewManager::onScreenGeometryChanged);

    onCanvasViewBuild(ScreenMrg->displayMode());
}
