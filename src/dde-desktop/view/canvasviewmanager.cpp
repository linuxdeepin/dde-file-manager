/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "canvasviewmanager.h"
#include "screen/screenhelper.h"
#include "presenter/gridmanager.h"
#include "desktopitemdelegate.h"
#include "dfilesystemmodel.h"
#include "util/util.h"
#include "util/dde/desktopinfo.h"

#include <QPair>

static const char * const PROPERTY_VIEW_INITED = "view_inited";
inline QRect relativeRect(const QRect &avRect,const QRect &geometry)
{
    QPoint relativePos = avRect.topLeft() - geometry.topLeft();

    return QRect(relativePos,avRect.size());
}

inline void initView(CanvasViewPointer view){
    if (view && !view->property(PROPERTY_VIEW_INITED).toBool()){
        view->setProperty(PROPERTY_VIEW_INITED,true);
        view->initRootUrl();
    }
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
    //在画布创建时做保护先判断背景，无背景信息，清空画布
    if (m_background->allbackgroundWidgets().isEmpty() && m_background->isEnabled()){
        m_canvasMap.clear();
        qWarning() << "not get background.....,current mode: "<< imode;
        return;
    }

    //屏幕模式判断
    AbstractScreenManager::DisplayMode mode = static_cast<AbstractScreenManager::DisplayMode>(imode);

    GridManager::instance()->restCoord();
    //实际是单屏
    if ((AbstractScreenManager::Showonly == mode) || (AbstractScreenManager::Duplicate == mode) //仅显示和复制
            || (ScreenMrg->screens().count() == 1)){

        ScreenPointer primary = ScreenMrg->primaryScreen();
        if (primary == nullptr) {
            m_canvasMap.clear();
            qCritical() << "get primary screen failed return";
            return;
        }

        CanvasViewPointer mView = m_canvasMap.value(primary);

        //删除其他
        m_canvasMap.clear();

        if (mView.get() == nullptr){
            mView = CanvasViewPointer(new CanvasGridView(primary->name()));
            mView->setScreenNum(1);
            //设置未初始化
            mView->setProperty(PROPERTY_VIEW_INITED,false);
            GridManager::instance()->addCoord(1, {0,0});
        }
        else {
            mView->setScreenNum(1);
            mView->setScreenName(primary->name());
            mView->clearSelection();
            /*!
             * 在DIconItemDelegate::paint中，通过setEditorData将expandedItem和index产生关联，
             * 只执行clearSelection无法清除QAbstractItemView的数据editorIndexHash，导致执行
             * updateEditorGeometries时，将不存在的expandedItem显示到错误的地方
             */
            mView->itemDelegate()->hideNotEditingIndexWidget();
            GridManager::instance()->addCoord(1, {0,0});
        }

        GridManager::instance()->setDisplayMode(true);
        m_canvasMap.insert(primary, mView);

        qDebug() << "mode" << mode << "inited" << mView->property(PROPERTY_VIEW_INITED).toBool()
                 << primary << primary->name() << "num" << 1
                 << "devicePixelRatio" << ScreenMrg->devicePixelRatio();
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
                //设置未初始化
                mView->setProperty(PROPERTY_VIEW_INITED,false);
                GridManager::instance()->addCoord(screenNum, {0,0});
                m_canvasMap.insert(sp, mView);
            }
            else {
                GridManager::instance()->addCoord(screenNum, {0,0});
                mView->setScreenNum(screenNum);
                mView->setScreenName(sp->name());
                mView->clearSelection();
                mView->itemDelegate()->hideNotEditingIndexWidget();
            }

            qDebug() << "mode" << mode << "inited" << mView->property(PROPERTY_VIEW_INITED).toBool()
                     << sp << sp->name() << "num" << screenNum << mView->screenName()
                     << "devicePixelRatio" << ScreenMrg->devicePixelRatio();
        }

        //检查移除的屏幕
        for (const ScreenPointer &sp : m_canvasMap.keys()){
            if (!currentScreens.contains(sp)){
                auto rmd = m_canvasMap.take(sp);
                qDebug() << "mode" << mode << "removed" << rmd->screenName();
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
            //fix bug52928 主屏数据不同步导致桌面崩溃(插上扩展屏后模式切换为复制模式，背景管理类中获取主屏为VGA，画布管理类中再次获取变成了HDMI）
            if (bw == nullptr) {
                auto datas = m_background->allbackgroundWidgets();
                for (auto bsp : datas.keys()) {
                    qWarning() << "BackgroundManager give screen:" << bsp->name();
                }
                qWarning() << "CanvasViewManager give screen:" << sp->name();
                m_canvasMap.clear();
                qCritical() << "ERROR!! The screen data obtained by CanvasViewManager is inconsistent with that obtained by BackGroundManager.";
                return;
            }

            QRect avRect;
            QRect screenAvaRect = sp->availableGeometry();
#ifndef UNUSED_SMARTDOCK
            avRect = relativeRect(screenAvaRect,sp->geometry());
#else
            if (sp == ScreenMrg->primaryScreen()){
                avRect = relativeRect(sp->availableGeometry(),sp->geometry());
            }
            else {
                avRect = relativeRect(sp->geometry(),sp->geometry());
            }
#endif
            // fix bug105508 获取到屏幕数据为空导致卡死。创建时屏幕数据错误，则直接放弃创建。
            if (avRect.size().width() < 1 || avRect.size().height() < 1) {
                m_canvasMap.clear();
                qCritical() << "ERROR!! The screen data obtained by CanvasViewManager is invalid."
                            << "Screen name:" << sp->name() << "    geometry:" << sp->geometry();
                for(auto w : m_background->allbackgroundWidgets().values()) {
                    // 清空已经设置过的view
                    w->setView(nullptr);
                }

                return;
            }

            mView->setAttribute(Qt::WA_NativeWindow, false);
            bw->setView(mView);

            mView->setGeometry(avRect);
            mView->show();

            initView(mView);
            qDebug() << "canvas" << mView << "availableGeometry" << avRect
                     << "inited" << mView->property(PROPERTY_VIEW_INITED).toBool()
                     << "screen" << sp->name() << sp->geometry() << screenAvaRect;
        }
    }
    else {
        for (const ScreenPointer &sp : m_canvasMap.keys()){

            QRect avRect;
#ifndef UNUSED_SMARTDOCK
            avRect = sp->availableGeometry();
#else
            avRect = sp == ScreenMrg->primaryScreen() ? sp->availableGeometry() : sp->geometry();
#endif
            // fix bug105508 获取到屏幕数据为空导致卡死。创建时屏幕数据错误，则直接放弃创建。
            if (avRect.size().width() < 1 || avRect.size().height() < 1) {
                m_canvasMap.clear();
                qCritical() << "ERROR!! The screen data obtained by CanvasViewManager is invalid."
                            << "Screen name:" << sp->name() << "    geometry:" << sp->geometry() << sp->availableGeometry();
                return;
            }

            CanvasViewPointer mView = m_canvasMap.value(sp);
            mView->setParent(nullptr);
            mView->setWindowFlag(Qt::FramelessWindowHint, true);

            DesktopUtil::set_desktop_window(mView.data());
            mView->setGeometry(avRect);
            mView->show();
            initView(mView);
            qDebug() << "no background. primaryScreen" << ScreenMrg->primaryScreen()->name()
                     << "canvas geo" << mView->geometry()  << "inited" << mView->property(PROPERTY_VIEW_INITED).toBool()
                     << "canvas's screen"<< sp->name() << sp->geometry() << "availableGeometry" << avRect;
        }
    }
    GridManager::instance()->initGridItemsInfos();
}

void CanvasViewManager::onScreenGeometryChanged()
{
#ifdef UNUSED_SMARTDOCK
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

        qDebug() << "primary Screen" << ScreenMrg->primaryScreen()->name()
                 << "view geometry change from" << mView->geometry() << "to" << avRect
                 << "view screen" << sp->name() << sp->geometry() << sp->availableGeometry();
        //fix bug32166 bug32205
        if (mView->geometry() == avRect) {
            qDebug() << "view geometry is equal to rect,and discard changes";
            return;
        }
        mView->setGeometry(avRect);
    }
#else
    for (const ScreenPointer &sp : m_canvasMap.keys()) {
        CanvasViewPointer mView = m_canvasMap.value(sp);
        if (mView == nullptr)
            continue;

        QRect avRect;
        if (m_background->isEnabled()) {
            avRect = relativeRect(sp->availableGeometry(), sp->geometry());
        }
        else {
            avRect = sp->availableGeometry();
        }

        //fix bug32166 bug32205
        if (mView->geometry() == avRect) {
            qDebug() << "view geometry is equal to rect,and discard changes" << avRect;
            continue;
        }

        qDebug() << "view geometry change from" << mView->geometry() << "to" << avRect
                 << "view screen" << sp->name() << sp->geometry() << sp->availableGeometry();
        mView->setGeometry(avRect);
    }
#endif
}

void CanvasViewManager::onSyncOperation(int so,QVariant var)
{
    GridManager::SyncOperation type = static_cast<GridManager::SyncOperation>(so);
    qDebug() << "sync type" << type << "data" << var;

    switch (type) {
    case GridManager::soAutoMerge:{
        bool enable = var.toBool();
        for (CanvasViewPointer view : m_canvasMap.values()){
            view->setAutoMerge(enable);
        }

        if (!enable)
            GridManager::instance()->initGridItemsInfos();
        break;
    }
    case GridManager::soRename:{ //处理自动排列时右键新建文件，编辑框显示问题
        QString file = var.toString();
        arrageEditDeal(file);
        break;
    }
    case GridManager::soIconSize:{  //处理图标大小
        int level = var.toInt();
        for (CanvasViewPointer view : m_canvasMap.values()){
            view->syncIconLevel(level);
            /*!
             * #bug102678:在DIconItemDelegate::paint中，通过setEditorData将expandedItem和index产生关联，
             * 只执行clearSelection无法清除QAbstractItemView的数据editorIndexHash，导致执行
             * updateEditorGeometries时，将不存在的expandedItem显示到错误的地方
             */
            view->itemDelegate()->hideNotEditingIndexWidget();
        }
        break;
    }
    case GridManager::soSort:{
        QPoint sort = var.toPoint();
        for (CanvasViewPointer view : m_canvasMap.values()){
            view->model()->setSortRole(sort.x(),static_cast<Qt::SortOrder>(sort.y()));
            view->update();
        }
        break;
    }
    case GridManager::soHideEditing:{   //隐藏文件移动后留下编辑框
        for (CanvasViewPointer view : m_canvasMap.values()){
            view->itemDelegate()->hideNotEditingIndexWidget();
            view->update();
        }
        break;
    }
    case GridManager::soUpdate:{
        for (CanvasViewPointer view : m_canvasMap.values()){
            view->update();
        }
        break;
    }
    case GridManager::soAutoMergeUpdate:{
        qDebug() << "update when canvas folder expand changed";
        auto mergeMap = var.value<QMap<QString,DUrl>>();
        if(mergeMap.isEmpty())
            return;
        auto one = mergeMap.begin();

        for (CanvasViewPointer view : m_canvasMap.values()){
            if(one.key() == view->canvansScreenName())
                continue;
            view->updateEntryExpandedState(one.value());
        }
        break;
    }
    case GridManager::soHidenSwitch:{
        for (CanvasViewPointer view : m_canvasMap.values()){
            view->updateHiddenItems();
        }
        break;
    }
    case GridManager::soGsettingUpdate:{
        for (CanvasViewPointer view : m_canvasMap.values()){
            view->delayModelRefresh();
        }
        break;
    }
    case GridManager::soExpandItemUpdate: {
            for (CanvasViewPointer view : m_canvasMap.values()) {
                view->updateExpandItemGeometry();
            }
            break;
        }
        //default处会报警告："warning: default label in switch which covers all enumeration values"
        //这里是全量case，因此为解决警告删除default
    }
}

void CanvasViewManager::onSyncSelection(CanvasGridView *v, DUrlList selected)
{
    disconnect(GridManager::instance(), &GridManager::sigSyncSelection,
            this,&CanvasViewManager::onSyncSelection);
    //qDebug() << "sync selection " << v->canvansScreenName() << selected.size();
    for (CanvasViewPointer view : m_canvasMap.values()) {
        if (view == v)
            continue;
        view->select(selected);
        view->update();
    }
    connect(GridManager::instance(), &GridManager::sigSyncSelection,
            this,&CanvasViewManager::onSyncSelection,Qt::DirectConnection);
}

void CanvasViewManager::arrageEditDeal(const QString &file)
{
    QPair<int, QPoint> orgPos;
    //找文件在哪个屏上
    if (GridManager::instance()->find(file,orgPos)){
        for (CanvasViewPointer view : m_canvasMap.values()) {
            //绘制屏上，打开编辑框
            if (view->screenNum() == orgPos.first){
                //已有editor，跳过
                if (view->itemDelegate()->editingIndexWidget()){
                    qDebug() << "has editor" << view->itemDelegate()->editingIndex();
                    continue;
                }

                //激活编辑框
                DUrl fileUrl(file);
                auto index = view->model()->index(fileUrl);
                view->select(QList<DUrl>() << fileUrl);
                bool bEdit = view->edit(index,QAbstractItemView::EditKeyPressed,nullptr);
                Q_UNUSED(bEdit)
                QWidget *editor = view->itemDelegate()->editingIndexWidget();
                if (editor)
                    editor->activateWindow();
            }
        }
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

    //grid改变
    connect(GridManager::instance(), &GridManager::sigSyncOperation,
            this, &CanvasViewManager::onSyncOperation,Qt::QueuedConnection);

    //同步选中状态
    connect(GridManager::instance(), &GridManager::sigSyncSelection,
            this,&CanvasViewManager::onSyncSelection,Qt::DirectConnection);

    onCanvasViewBuild(ScreenMrg->lastChangedMode());
}
