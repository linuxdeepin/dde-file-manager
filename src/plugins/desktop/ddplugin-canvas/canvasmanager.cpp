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
#include "view/operator/fileoperatorproxy.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"
#include "menu/canvasmenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_desktop_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/framework.h>

#include <QCoreApplication>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

class CanvasManagerGlobal : public CanvasManager
{
};
Q_GLOBAL_STATIC(CanvasManagerGlobal, canvasManagerGlobal)

CanvasManager::CanvasManager(QObject *parent)
    : QObject(parent), d(new CanvasManagerPrivate(this))
{
    Q_ASSERT(thread() == qApp->thread());
}

CanvasManager::~CanvasManager()
{
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &CanvasManager::onDetachWindows);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowBuilded, &CanvasManager::onCanvasBuild);
    CanvasCoreUnsubscribe(signal_DesktopFrame_GeometryChanged, &CanvasManager::onGeometryChanged);
    CanvasCoreUnsubscribe(signal_DesktopFrame_AvailableGeometryChanged, &CanvasManager::onGeometryChanged);
    dpfSignalDispatcher->unsubscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged", this, &CanvasManager::onTrashStateChanged);
}

CanvasManager *CanvasManager::instance()
{
    return canvasManagerGlobal;
}

static QString getScreenName(QWidget *win)
{
    return win->property(DesktopFrameProperty::kPropScreenName).toString();
}

static QMap<QString, QWidget *> rootMap()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    QMap<QString, QWidget *> ret;
    for (QWidget *win : root) {
        QString name = getScreenName(win);
        if (name.isEmpty())
            continue;
        ret.insert(name, win);
    }

    return ret;
}

void CanvasManager::init()
{
    // init single object
    DispalyIns;
    GridIns;

    CanvasCoreSubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &CanvasManager::onDetachWindows);
    CanvasCoreSubscribe(signal_DesktopFrame_WindowBuilded, &CanvasManager::onCanvasBuild);
    CanvasCoreSubscribe(signal_DesktopFrame_GeometryChanged, &CanvasManager::onGeometryChanged);
    CanvasCoreSubscribe(signal_DesktopFrame_AvailableGeometryChanged, &CanvasManager::onGeometryChanged);
    dpfSignalDispatcher->subscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged", this, &CanvasManager::onTrashStateChanged);

    // register menu
    dfmplugin_menu_util::menuSceneRegisterScene(CanvasMenuCreator::name(), new CanvasMenuCreator);

    // self hook
    d->hookIfs = new CanvasManagerHook(this);

    // self borker
    d->broker = new CanvasManagerBroker(this, this);
    d->broker->init();

    d->initModel();
    d->initSetting();
}

void CanvasManager::update()
{
    for (auto itor = d->viewMap.begin(); itor != d->viewMap.end(); ++itor) {
        itor.value()->update();
    }
}

void CanvasManager::openEditor(const QUrl &url)
{
    QString path = url.toString();
    QPair<int, QPoint> pos;

    if (!GridIns->point(path, pos)) {
        bool find = false;
        for (auto view : d->viewMap.values()) {
            if (GridIns->overloadItems(view->screenNum()).contains(path)) {
                pos.first = view->screenNum();
                find = true;
                break;
            }
        }
        if (!find) {
            qDebug() << "can not editor,file does not exist:" << url;
            return;
        }
    }

    QModelIndex index = d->canvasModel->index(url);
    if (!index.isValid())
        return;

    d->selectionModel->select(index, QItemSelectionModel::Select);
    for (auto view : d->viewMap.values()) {
        view->setCurrentIndex(index);
        if (pos.first == view->screenNum()) {
            view->edit(index, QAbstractItemView::AllEditTriggers, nullptr);
            view->activateWindow();
        }
    }
}

void CanvasManager::setIconLevel(int level)
{
    auto allView = views();
    if (allView.isEmpty())
        return;

    CanvasItemDelegate *delegate = allView.first()->itemDelegate();
    if (level >= delegate->minimumIconLevel() && level <= delegate->maximumIconLevel()) {
        for (auto v : allView) {
            v->itemDelegate()->setIconLevel(level);
            v->updateGrid();
        }

        DispalyIns->setIconLevel(level);
        // notify others that icon size changed
        d->hookIfs->iconSizeChanged(level);
    }
}

FileInfoModel *CanvasManager::fileModel() const
{
    return d->sourceModel;
}

CanvasProxyModel *CanvasManager::model() const
{
    return d->canvasModel;
}

CanvasSelectionModel *CanvasManager::selectionModel() const
{
    return d->selectionModel;
}

QList<QSharedPointer<CanvasView>> CanvasManager::views() const
{
    return d->viewMap.values();
}

void CanvasManager::onCanvasBuild()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    if (root.size() == 1) {
        QWidget *primary = root.first();
        if (primary == nullptr) {
            //屏幕信息获取失败，清空对应关系
            d->viewMap.clear();
            qCritical() << "get primary screen failed return.";
            return;
        }

        // init grid
        GridIns->initSurface(1);

        const QString screeName = getScreenName(primary);
        if (screeName.isEmpty()) {
            qWarning() << "can not get screen name from root window";
            return;
        }

        CanvasViewPointer view = d->viewMap.value(screeName);
        d->viewMap.clear();
        if (view.get())
            d->updateView(view, primary, 1);
        else
            view = d->createView(primary, 1);

        d->viewMap.insert(screeName, view);
        view->show();
    } else {
        int screenNum = 0;
        // init grid
        GridIns->initSurface(root.size());

        //检查新增的屏幕
        for (QWidget *win : root) {
            ++screenNum;

            const QString screenName = getScreenName(win);
            if (screenName.isEmpty()) {
                qWarning() << "can not get screen name from root window";
                continue;
            }

            CanvasViewPointer view = d->viewMap.value(screenName);
            //新增
            if (view.get()) {
                d->updateView(view, win, screenNum);
            } else {
                view = d->createView(win, screenNum);
                d->viewMap.insert(screenName, view);
            }

            view->show();
        }

        // 检查移除的窗口
        {
            auto winMap = rootMap();
            for (const QString &sp : d->viewMap.keys()) {
                if (!winMap.contains(sp))
                    d->viewMap.take(sp);
            }
        }
    }

    // source model is ready
    if (d->sourceModel->modelState() & 0x1)
        reloadItem();
}

void CanvasManager::onDetachWindows()
{
    for (const CanvasViewPointer &view : d->viewMap.values())
        view->setParent(nullptr);
}

void CanvasManager::onGeometryChanged()
{
    auto winMap = rootMap();
    for (auto itor = d->viewMap.begin(); itor != d->viewMap.end(); ++itor) {
        CanvasViewPointer view = itor.value();
        auto *win = winMap.value(itor.key());
        if (win == nullptr) {
            qCritical() << "can not get root " << itor.key() << "num" << view->screenNum();
            continue;
        }

        // calc current geometry.
        QRect avRect = d->relativeRect(win->property(DesktopFrameProperty::kPropScreenAvailableGeometry).toRect(),
                                       win->property(DesktopFrameProperty::kPropScreenGeometry).toRect());

        // no need to update.
        if (view->geometry() == avRect) {
            qDebug() << "view geometry is equal to rect,and discard changes" << avRect;
            continue;
        }

        qDebug() << "view geometry change from" << view->geometry() << "to" << avRect;
        view->setGeometry(avRect);
    }
}

void CanvasManager::onWallperSetting(CanvasView *view)
{
    // find screen
    QString screen;
    for (auto it = d->viewMap.begin(); it != d->viewMap.end(); ++it) {
        if (it.value().get() == view) {
            screen = it.key();
            break;
        }
    };

    if (screen.isEmpty())
        return;

    d->hookIfs->requestWallpaperSetting(screen);
}

void CanvasManager::reloadItem()
{
    GridIns->setMode(CanvasGrid::Mode::Custom);
    QStringList existItems;
    const QList<QUrl> actualList = d->canvasModel->files();
    for (const QUrl &df : actualList) {
        existItems.append(df.toString());
    }

    GridIns->setItems(existItems);

    // rearrange
    if (DispalyIns->autoAlign()) {
        GridIns->setMode(CanvasGrid::Mode::Align);
        GridIns->arrange();
    }

    update();
}

CanvasManagerPrivate::CanvasManagerPrivate(CanvasManager *qq)
    : QObject(qq), q(qq)
{
}

CanvasManagerPrivate::~CanvasManagerPrivate()
{
    viewMap.clear();
}

void CanvasManagerPrivate::initModel()
{
    sourceModel = new FileInfoModel(q);
    canvasModel = new CanvasProxyModel(q);
    canvasModel->setShowHiddenFiles(Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool());
    canvasModel->setSourceModel(sourceModel);

    // use default root url
    sourceModel->setRootUrl(QUrl());

    // sort
    {
        Qt::SortOrder sortOrder;
        int role = -1;
        DispalyIns->sortMethod(role, sortOrder);
        if (role < 0)
            role = Global::ItemRoles::kItemFileMimeTypeRole;
        canvasModel->setSortRole(role, sortOrder);
    }

    selectionModel = new CanvasSelectionModel(canvasModel, q);
    // using DirectConnection to keep signals are sequential.
    connect(canvasModel, &CanvasProxyModel::rowsInserted, this, &CanvasManagerPrivate::onFileInserted, Qt::DirectConnection);
    connect(canvasModel, &CanvasProxyModel::rowsAboutToBeRemoved, this, &CanvasManagerPrivate::onFileAboutToBeRemoved, Qt::DirectConnection);
    connect(canvasModel, &CanvasProxyModel::dataReplaced, this, &CanvasManagerPrivate::onFileRenamed, Qt::DirectConnection);

    connect(canvasModel, &CanvasProxyModel::dataChanged, this, &CanvasManagerPrivate::onFileDataChanged, Qt::QueuedConnection);
    connect(canvasModel, &CanvasProxyModel::modelReset, this, &CanvasManagerPrivate::onFileModelReset, Qt::QueuedConnection);
    connect(canvasModel, &CanvasProxyModel::layoutChanged, this, &CanvasManagerPrivate::onFileSorted, Qt::QueuedConnection);

    // hook interface
    modelHook = new CanvasModelHook(q);
    canvasModel->setModelHook(modelHook);

    viewHook = new CanvasViewHook(q);

    // external interface
    sourceModelBroker = new FileInfoModelBroker(sourceModel, q);
    sourceModelBroker->init();

    modelBroker = new CanvasModelBroker(canvasModel, q);
    modelBroker->init();

    viewBroker = new CanvasViewBroker(q, q);
    viewBroker->init();

    gridBroker = new CanvasGridBroker(GridIns, q);
    gridBroker->init();
}

void CanvasManagerPrivate::initSetting()
{
    // setting changed.
    connect(Application::instance(), &Application::showedHiddenFilesChanged, this, &CanvasManagerPrivate::onHiddenFlagsChanged);
    connect(Application::instance(), &Application::previewAttributeChanged, canvasModel, &CanvasProxyModel::update);
    connect(Application::instance(), &Application::showedFileSuffixChanged, canvasModel, &CanvasProxyModel::update);
}

CanvasViewPointer CanvasManagerPrivate::createView(QWidget *root, int index)
{
    CanvasViewPointer view;
    if (Q_UNLIKELY(root == nullptr || index < 1))
        return CanvasViewPointer();

    view.reset(new CanvasView());
    view->setParent(root);
    view->setModel(canvasModel);
    view->setSelectionModel(selectionModel);
    view->setViewHook(viewHook);
    view->setAttribute(Qt::WA_NativeWindow, false);
    view->initUI();

    view->setScreenNum(index);
    auto avRect = relativeRect(root->property(DesktopFrameProperty::kPropScreenAvailableGeometry).toRect(),
                               root->property(DesktopFrameProperty::kPropScreenGeometry).toRect());
    view->setProperty(DesktopFrameProperty::kPropScreenName, getScreenName(root));
    view->setProperty(DesktopFrameProperty::kPropWidgetName, "canvas");
    view->setProperty(DesktopFrameProperty::kPropWidgetLevel, 10.0);
    view->setGeometry(avRect);
    return view;
}

void CanvasManagerPrivate::updateView(const CanvasViewPointer &view, QWidget *root, int index)
{
    if (Q_UNLIKELY(root == nullptr || view == nullptr || index < 1))
        return;

    view->clearSelection();
    view->setScreenNum(index);
    view->setParent(root);

    view->setProperty(DesktopFrameProperty::kPropScreenName, getScreenName(root));
    auto avRect = relativeRect(root->property(DesktopFrameProperty::kPropScreenAvailableGeometry).toRect(),
                               root->property(DesktopFrameProperty::kPropScreenGeometry).toRect());
    view->setGeometry(avRect);
}

void CanvasManagerPrivate::onHiddenFlagsChanged(bool show)
{
    if (show != canvasModel->showHiddenFiles()) {
        canvasModel->setShowHiddenFiles(show);
        canvasModel->refresh(canvasModel->rootIndex());

        hookIfs->hiddenFlagChanged(!show);
    }
}

void CanvasManagerPrivate::onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (GridIns->replace(oldUrl.toString(), newUrl.toString())) {
        QModelIndex index = canvasModel->index(newUrl);
        if (!index.isValid())
            return;
        const auto &renameFileData = FileOperatorProxyIns->renameFileData();
        if (renameFileData.contains(oldUrl) && renameFileData.value(oldUrl) == newUrl) {
            FileOperatorProxyIns->removeRenameFileData(oldUrl);
            selectionModel->select(index, QItemSelectionModel::Select);
            for (auto view : viewMap.values()) {
                view->setCurrentIndex(index);
                view->activateWindow();
            }
        }

        q->update();
    }
}

void CanvasManagerPrivate::onFileInserted(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex index = canvasModel->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        QUrl url = canvasModel->fileUrl(index);

        const QString &&path = url.toString();
        const auto &touchFileData= FileOperatorProxyIns->touchFileData();
        if (path == touchFileData.first) {
            if (CanvasGrid::Mode::Custom == GridIns->mode()) {
                GridIns->tryAppendAfter({ path }, touchFileData.second.first, touchFileData.second.second);
            } else {
                GridIns->append(path);
            }
            FileOperatorProxyIns->clearTouchFileData();

            // need open editor,only by menu create file
            q->openEditor(url);
        } else {
            QPair<int, QPoint> pos;
            if (GridIns->point(path, pos)) {
                // already exists
                continue;
            }
            GridIns->append(path);
        }
    }

    q->update();
}

void CanvasManagerPrivate::onFileAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex index = canvasModel->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        QUrl url = canvasModel->fileUrl(index);

        QString path = url.toString();
        QPair<int, QPoint> pos;
        if (GridIns->point(path, pos)) {
            GridIns->remove(pos.first, path);
            if (CanvasGrid::Mode::Align == GridIns->mode()) {
                GridIns->arrange();
            } else {
                GridIns->popOverload();
            }
        } else {
            int viewCount = viewMap.keys().count();
            for (int i = 1; i <= viewCount; i++) {
                if (GridIns->overloadItems(i).contains(path)) {
                    GridIns->remove(i, path);
                }
            }
        }
    }
    q->update();
}

void CanvasManagerPrivate::onFileDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(roles)
    q->update();
}

void CanvasManagerPrivate::onFileModelReset()
{
    q->reloadItem();
}

void CanvasManagerPrivate::onFileSorted()
{
    auto oldMode = GridIns->mode();
    GridIns->setMode(CanvasGrid::Mode::Align);
    QStringList existItems;
    const QList<QUrl> &actualList = canvasModel->files();
    for (const QUrl &df : actualList)
        existItems.append(df.toString());

    GridIns->setItems(existItems);
    GridIns->setMode(oldMode);
    q->update();
}

void CanvasManager::onChangeIconLevel(bool increase)
{
    if (d->viewMap.isEmpty())
        return;
    auto view = d->viewMap.values().first();
    Q_ASSERT(view);
    auto delegate = view->itemDelegate();
    Q_ASSERT(delegate);

    int currentLevel = delegate->iconLevel();
    currentLevel = increase ? currentLevel + 1 : currentLevel - 1;
    setIconLevel(currentLevel);
}

void CanvasManager::onTrashStateChanged()
{
    d->sourceModel->update();
}
