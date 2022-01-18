/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "canvas/view/operator/fileoperaterproxy.h"
#include "dfm-framework/framework.h"
#include "dfm-base/widgets/screenglobal.h"

DFMBASE_USE_NAMESPACE

DSB_D_USE_NAMESPACE
class CanvasManagerGlobal : public CanvasManager{};
Q_GLOBAL_STATIC(CanvasManagerGlobal, canvasManagerGlobal)

CanvasManager::CanvasManager(QObject *parent)
    : QObject(parent)
    , d(new CanvasManagerPrivate(this))
{
    Q_ASSERT(thread() == qApp->thread());
}

CanvasManager *CanvasManager::instance()
{
    return canvasManagerGlobal;
}

void CanvasManager::init()
{
    auto &ctx = dpfInstance.serviceContext();
    // 获取屏幕服务
    d->screenScevice = ctx.service<ScreenService>(ScreenService::name());

    // 获取背景服务
    d->backgroundService = ctx.service<BackgroundService>(BackgroundService::name());

    d->canvasModel = new CanvasModel(this);
    d->selectionModel = new CanvasSelectionModel(d->canvasModel, this);

    // 绑定背景信号
    // todo
    initConnect();

    // create views
    onCanvasBuild();
}

void CanvasManager::update()
{
    for (auto itor = d->viewMap.begin(); itor != d->viewMap.end(); ++itor) {
        itor.value()->repaint();
    }
}

CanvasModel *CanvasManager::model() const
{
    return d->canvasModel;
}

CanvasSelectionModel *CanvasManager::selectionModel() const
{
    return d->selectionModel;
}

QList<QSharedPointer<CanvasView> > CanvasManager::views() const
{
    return d->viewMap.values();
}

void CanvasManager::initConnect()
{
    // 屏幕增删，模式改变
    connect(d->backgroundService, &BackgroundService::sigBackgroundBuilded, this, &CanvasManager::onCanvasBuild);

    // 屏幕大小改变
    connect(d->screenScevice, &ScreenService::screenGeometryChanged, this, &CanvasManager::onGeometryChanged);

    // 可用区改变
    connect(d->screenScevice, &ScreenService::screenAvailableGeometryChanged, this, &CanvasManager::onGeometryChanged);

    connect(d->canvasModel, &CanvasModel::fileCreated, d, &CanvasManagerPrivate::onFileCreated, Qt::QueuedConnection);
    connect(d->canvasModel, &CanvasModel::fileDeleted, d, &CanvasManagerPrivate::onFileDeleted, Qt::QueuedConnection);
    connect(d->canvasModel, &CanvasModel::fileRenamed, d, &CanvasManagerPrivate::onFileRenamed, Qt::QueuedConnection);
    connect(d->canvasModel, &CanvasModel::fileSorted, d, &CanvasManagerPrivate::onFileSorted, Qt::QueuedConnection);
    connect(d->canvasModel, &CanvasModel::fileRefreshed, d, &CanvasManagerPrivate::onFileRefreshed, Qt::QueuedConnection);

    // todo(wangcl):callback
    connect(FileOperaterProxyIns, &FileOperaterProxy::createFileByMenu, d, &CanvasManagerPrivate::recordMenuLocation, Qt::DirectConnection);
}

void CanvasManager::onCanvasBuild()
{
    DisplayMode displayMode = d->screenScevice->lastChangedMode();
    auto screenSize = d->screenScevice->screens().size();

    if ((displayMode == DisplayMode::Showonly)
        || (displayMode == DisplayMode::Duplicate)
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
        for (const ScreenPointer &sp : currentScreens) {
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
                auto view = d->viewMap.take(sp);
                qDebug() << "mode " << displayMode << "remove " << view->screenNum();
            }
        }
    }

    // todo(zy) 优化首次加载与屏幕改变的加载重复问题，现在在初始化时有冗余
    if (d->canvasModel->isRefreshed())
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
    const QList<QUrl> &actualList = d->canvasModel->getFiles();
    for (const QUrl &df : actualList) {
        existItems.append(df.toString());
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
    view->setModel(canvasModel);
    view->setSelectionModel(selectionModel);
    view->setAttribute(Qt::WA_NativeWindow, false);
    view->initUI();

    view->setScreenNum(index);

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

void CanvasManagerPrivate::onFileCreated(const QUrl &url)
{
    QString path = url.toString();
    {
        QMutexLocker l(&createFileMutex);
        if (createFileByMenu) {
            // todo(wangcl) 优化为回调函数处理(在canvasview的右键菜单调用时传入回调函数)，此处直接刷新返回
            createFileByMenu = false;
            // todo(wangcl):自动排序的情况下，直接append
            GridIns->tryAppendAfter({path}, createFileScreenNum, createFileGridPos);

            // todo open editor for rename file
        } else {
            GridIns->append(path);
        }
    }

    q->update();
}

void CanvasManagerPrivate::onFileDeleted(const QUrl &url)
{
    QString path = url.toString();
    QPair<int, QPoint> pos;
    if (GridIns->point(path, pos)) {
        GridIns->remove(pos.first, path);
        q->update();
    }
}

void CanvasManagerPrivate::onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (GridIns->replace(oldUrl.toString(), newUrl.toString()))
        q->update();
}

void CanvasManagerPrivate::onFileRefreshed()
{
    q->reloadItem();
}

void CanvasManagerPrivate::onFileSorted()
{
    GridIns->setMode(CanvasGrid::Mode::Align);
    QStringList existItems;
    const QList<QUrl> &actualList = canvasModel->getFiles();
    for (const QUrl &df : actualList) {
        existItems.append(df.toString());
    }

    GridIns->setItems(existItems);

    q->update();
}

void CanvasManagerPrivate::recordMenuLocation(const int screenNum, const QPoint &pos)
{
    QMutexLocker l(&createFileMutex);
    createFileByMenu = true;
    createFileScreenNum = screenNum;
    createFileGridPos = pos;
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
