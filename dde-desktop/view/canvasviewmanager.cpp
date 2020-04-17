#include "canvasviewmanager.h"
#include "screen/screenhelper.h"
#include "util/xcb/xcb.h"
#include "presenter/gridmanager.h"


inline QRect relativeRect(const QRect &avRect,const QRect &geometry)
{
    QPoint relativePos = avRect.topLeft() - geometry.topLeft();

    return QRect(relativePos,avRect.size());
}

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

    GridManager::instance()->restCoord();
    //实际是单屏
    if ((AbstractScreenManager::Showonly == mode) || (AbstractScreenManager::Duplicate == mode) //仅显示和复制
            || (ScreenMrg->screens().count() == 1)){

        ScreenPointer primary = ScreenMrg->primaryScreen();
        CanvasViewPointer mView = m_canvasMap.value(primary);

        //删除其他
        m_canvasMap.clear();

        if (mView.get() == nullptr){
            mView = CanvasViewPointer(new CanvasGridView(primary->name()));
            mView->setScreenNum(1);
            GridManager::instance()->addCoord(1, {0,0});
            mView->show();
        }
        else {
            mView->setScreenNum(1);
            GridManager::instance()->addCoord(1, {0,0});
        }

        GridManager::instance()->setDisplayMode(true);

        mView->initRootUrl();
        m_canvasMap.insert(primary, mView);
    }
    else {
        auto currentScreens = ScreenMrg->logicScreens();
        int screenNum = 0;
        //检查新增的屏幕
        for (const ScreenPointer &sp : currentScreens){
            ++screenNum;
            CanvasViewPointer mView = m_canvasMap.value(sp);

            //新增
            if (mView.get() == nullptr){
                mView = CanvasViewPointer(new CanvasGridView(sp->name()));
                mView->setScreenNum(screenNum);
                GridManager::instance()->addCoord(screenNum, {0,0});
                mView->show();
                mView->initRootUrl();
                m_canvasMap.insert(sp, mView);
            }
            else {
                mView->setScreenNum(screenNum);
            }

            qDebug() << "Sssssss mode" << mode << mView->geometry() <<sp->availableGeometry()<< sp->geometry()
                     << sp->name() << "num" << screenNum;
        }

        //检查移除的屏幕
        for (const ScreenPointer &sp : m_canvasMap.keys()){
            if (!currentScreens.contains(sp)){
                m_canvasMap.remove(sp);
            }
        }
        GridManager::instance()->setDisplayMode(false);
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
            QRect avRect;
            if (sp == ScreenMrg->primaryScreen()){
                avRect = relativeRect(sp->availableGeometry(),sp->geometry());
            }
            else {
                avRect = relativeRect(sp->geometry(),sp->geometry());
            }
            //qDebug() << "xxxxxxxxxxx" << avRect << sp->geometry() << sp->availableGeometry();
            mView->show();
            mView->setGeometry(avRect);
        }
    }
    else {
        for (const ScreenPointer &sp : m_canvasMap.keys()){

            CanvasViewPointer mView = m_canvasMap.value(sp);
            mView->setParent(nullptr);
            mView->setWindowFlag(Qt::FramelessWindowHint, true);
            Xcb::XcbMisc::instance().set_window_type(mView->winId(), Xcb::XcbMisc::Desktop);
            mView->show();
            mView->setGeometry(sp == ScreenMrg->primaryScreen() ? sp->availableGeometry() : sp->geometry());
            qDebug() << "rrrrrrrrrrrrrrr" << ScreenMrg->primaryScreen()->name() << mView->geometry() << sp->name() << sp->geometry() << sp->availableGeometry();
        }
    }
    GridManager::instance()->initGridItemsInfos();
}

void CanvasViewManager::onScreenGeometryChanged(ScreenPointer sp)
{
    CanvasViewPointer mView = m_canvasMap.value(sp);
    if (mView.get() != nullptr){
        QRect avRect;
        if (m_background->isEnabled()) {
            if (sp == ScreenMrg->primaryScreen()){
                avRect = relativeRect(sp->availableGeometry(), sp->geometry());
            }
            else {
                avRect = relativeRect(sp->geometry(), sp->geometry());
            }
        }
        else {
            avRect = sp == ScreenMrg->primaryScreen() ? sp->availableGeometry() : sp->geometry();
        }

        qDebug() << "dddddddddddddd" << ScreenMrg->primaryScreen()->name()
                 << mView->geometry() << sp->name()
                 << sp->geometry() << sp->availableGeometry() << avRect;
        mView->setGeometry(avRect);
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
