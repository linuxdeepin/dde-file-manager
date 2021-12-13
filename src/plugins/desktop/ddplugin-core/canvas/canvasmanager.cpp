/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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

#include "private/canvasmanager_p.h"
#include "view/canvasview_p.h"
#include "grid/canvasgrid.h"
#include "displayconfig.h"

#include "filetreater.h"
#include "private/canvasmanager_p.h"

#include "dfm-framework/framework.h"

DSB_D_USE_NAMESPACE

CanvasManager::CanvasManager(QObject *parent)
    : QObject(parent)
    , d(new CanvasManagerPrivate(this))
{

}

void CanvasManager::init()
{
    auto &ctx = dpfInstance.serviceContext();
    // 获取屏幕服务
    d->screenScevice = ctx.service<ScreenService>(ScreenService::name());

    // 获取背景服务
    d->backgroundService = ctx.service<BackgroundService>(BackgroundService::name());

    // 绑定背景信号
    // todo
    initConnect();

    d->canvasModel = new CanvasModel(this);
    //todo create selection model

    // create views
    onCanvasBuild();
}

void CanvasManager::update()
{
    for (auto itor = d->viewMap.begin(); itor != d->viewMap.end(); ++itor) {
        itor.value()->repaint();
    }
}

void CanvasManager::initConnect()
{
    // 遍历数据完成后通知更新栅格位置信息
    connect(FileTreaterCt, &FileTreater::fileFinished, this, &CanvasManager::reloadItem, Qt::QueuedConnection);

    // 屏幕增删，模式改变
    connect(d->backgroundService, &BackgroundService::sigBackgroundBuilded, this, &CanvasManager::onCanvasBuild);

    //屏幕大小改变
    connect(d->screenScevice, &ScreenService::screenGeometryChanged, this, &CanvasManager::onGeometryChanged);

    //可用区改变
    connect(d->screenScevice, &ScreenService::screenAvailableGeometryChanged, this, &CanvasManager::onGeometryChanged);
}

void CanvasManager::onCanvasBuild()
{
    dfmbase::DisplayMode displayMode = d->screenScevice->lastChangedMode();
    auto screenSize = d->screenScevice->screens().size();

    if ((displayMode == dfmbase::DisplayMode::Showonly)
        || (displayMode == dfmbase::DisplayMode::Duplicate)
        || (1 == screenSize)) {

        // 仅显示模式(Showonly)、复制模式(Duplicate)、单屏
        auto primary = d->screenScevice->primaryScreen();
        if (primary == nullptr) {
            //屏幕信息获取失败，清空对应关系
            d->viewMap.clear();
            qCritical() << "get primary screen failed return";
            return;
        }

        // init grid
        GridIns->initSurface(1);

        const QString screeName = primary->name();
        CanvasViewPointer view = d->viewMap.value(screeName);

        // 清空非主屏的画布信息
        d->viewMap.clear();

        if (view.get())
            d->updateView(view, primary, 1);
        else
            view = d->createView(primary, 1);

        d->viewMap.insert(screeName, view);

        view->show();
    } else {
        //扩展模式、自定义模式
        auto currentScreens = d->screenScevice->logicScreens();
        int screenNum = 0;

        // init grid
        GridIns->initSurface(currentScreens.size());

        //检查新增的屏幕
        for (const dfmbase::ScreenPointer &sp : currentScreens) {
            ++screenNum;

            const QString screenName = sp->name();
            CanvasViewPointer view = d->viewMap.value(screenName);
            //新增
            if (view.get()) {
                d->updateView(view, sp, screenNum);
            } else {
                view = d->createView(sp, screenNum);
                d->viewMap.insert(screenName, view);
            }

            view->show();
        }

        //检查移除的屏幕
        for (const QString &sp : d->viewMap.keys()) {
            if (!d->screenScevice->screen(sp)) {
                d->viewMap.remove(sp);
                qDebug() << "mode " << displayMode << "remove " << sp;
            }
        }
    }

    // todo(zy) 优化首次加载与屏幕改变的加载重复问题，现在在初始化时有冗余
    if (FileTreaterCt->isDone())
        reloadItem();
}

void CanvasManager::onGeometryChanged()
{
    for (auto itor = d->viewMap.begin(); itor != d->viewMap.end(); ++itor) {
        CanvasViewPointer view = itor.value();
        ScreenPointer sp = d->screenScevice->screen(itor.key());
        if (sp.get() == nullptr) {
            qCritical() << "can not get screen " << itor.key() << "num" << view->screenNum();
            return;
        }

        // calc current geometry.
        QRect avRect = d->relativeRect(sp->availableGeometry(), sp->geometry());

        // no need to update.
        if (view->geometry() == avRect) {
            qDebug() << "view geometry is equal to rect,and discard changes" << avRect;
            continue;
        }

        qDebug() << "view geometry change from" << view->geometry() << "to" << avRect
                 << "view screen" << sp->name() << sp->geometry() << sp->availableGeometry();
        view->setGeometry(avRect);
    }
}

void CanvasManager::reloadItem()
{
    // 初始化栅布局信息

    //todo 默认初始化（按类型排序）以及按配置还原
    GridIns->setMode(CanvasGrid::Mode::Custom);
    QStringList existItems;
    QList<DFMDesktopFileInfoPointer> &actualList = FileTreaterCt->getFiles();
    for (const DFMDesktopFileInfoPointer &df : actualList) {
        existItems.append(df->url().toString());
    }

    GridIns->setItems(existItems);

    // rearrange
    if (DispalyIns->autoAlign()) {
        GridIns->setMode(CanvasGrid::Mode::Align);
        GridIns->setItems(GridIns->items());
    }

    update();
}

CanvasManagerPrivate::CanvasManagerPrivate(CanvasManager *qq)
    : QObject(qq)
    , q(qq)
{
}

CanvasManagerPrivate::~CanvasManagerPrivate()
{
    viewMap.clear();
}

CanvasViewPointer CanvasManagerPrivate::createView(const ScreenPointer &sp, int index)
{
    CanvasViewPointer view;
    if ( Q_UNLIKELY(sp == nullptr || index < 1))
        return CanvasViewPointer();

    view.reset(new CanvasView());
    view->setAttribute(Qt::WA_NativeWindow, false);
    view->initUI();

    view->setScreenNum(index);
    view->setModel(canvasModel);

    auto background = backgroundService->background(sp->name());
    view->setParent(background.get());
    connect(background.get(), &AbstractBackground::destroyed, this, &CanvasManagerPrivate::backgroundDeleted, Qt::UniqueConnection);

    auto avRect = relativeRect(sp->availableGeometry(), sp->geometry());
    view->setGeometry(avRect);

    return view;
}

void CanvasManagerPrivate::updateView(const CanvasViewPointer &view, const ScreenPointer &sp, int index)
{
    if (Q_UNLIKELY(sp == nullptr || view == nullptr || index < 1))
        return;

    view->clearSelection();
    view->setScreenNum(index);

    auto background = backgroundService->background(sp->name());
    view->setParent(background.get());
    connect(background.get(), &AbstractBackground::destroyed, this, &CanvasManagerPrivate::backgroundDeleted, Qt::UniqueConnection);

    auto avRect = relativeRect(sp->availableGeometry(), sp->geometry());
    view->setGeometry(avRect);
}

void CanvasManagerPrivate::backgroundDeleted()
{
    QWidget *bk = qobject_cast<QWidget *>(sender());
    if (!bk)
        return;

    for (const CanvasViewPointer &view : viewMap.values()) {
        if (view->parent() == bk)
            view->setParent(nullptr);
    }
}
