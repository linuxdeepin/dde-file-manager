// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/canvasmanager_p.h"
#include "view/canvasview_p.h"
#include "grid/canvasgrid.h"
#include "displayconfig.h"
#include "view/operator/fileoperatorproxy.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"
#include "menu/canvasmenuscene.h"
#include "menu/canvasbasesortmenuscene.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <QCoreApplication>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

CanvasManager *CanvasManagerPrivate::global = nullptr;

CanvasManager::CanvasManager(QObject *parent)
    : QObject(parent), d(new CanvasManagerPrivate(this))
{
    Q_ASSERT_X(thread() == qApp->thread(), "CanvasManager", "object can only be created in main thread.");

    Q_ASSERT_X(!CanvasManagerPrivate::global, "CanvasManager", "there should be only one object");
    CanvasManagerPrivate::global = this;
}

CanvasManager::~CanvasManager()
{
    CanvasManagerPrivate::global = nullptr;

    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &CanvasManager::onDetachWindows);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowBuilded, &CanvasManager::onCanvasBuild);
    CanvasCoreUnsubscribe(signal_DesktopFrame_GeometryChanged, &CanvasManager::onGeometryChanged);
    CanvasCoreUnsubscribe(signal_DesktopFrame_AvailableGeometryChanged, &CanvasManager::onGeometryChanged);
    dpfSignalDispatcher->unsubscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged", this, &CanvasManager::onTrashStateChanged);
}

CanvasManager *CanvasManager::instance()
{
    return CanvasManagerPrivate::global;
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
    dfmplugin_menu_util::menuSceneRegisterScene(CanvasBaseSortMenuCreator::name(), new CanvasBaseSortMenuCreator);

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
    qInfo() << "change icon level to" << level;
    auto allView = views();
    if (allView.isEmpty()) {
        if (DispalyIns->iconLevel() != level) {
            DispalyIns->setIconLevel(level);
            d->hookIfs->iconSizeChanged(level);
        }
        return;
    }

    CanvasItemDelegate *delegate = allView.first()->itemDelegate();
    if (level != delegate->iconLevel() && level >= delegate->minimumIconLevel() && level <= delegate->maximumIconLevel()) {
        for (auto v : allView) {
            v->itemDelegate()->setIconLevel(level);
            v->updateGrid();
        }

        DispalyIns->setIconLevel(level);
        // notify others that icon size changed
        d->hookIfs->iconSizeChanged(level);
    }
}

int CanvasManager::iconLevel() const
{
    auto allView = views();
    if (allView.isEmpty())
        return DispalyIns->iconLevel();

    CanvasItemDelegate *delegate = allView.first()->itemDelegate();
    return delegate->iconLevel();
}

bool CanvasManager::autoArrange() const
{
    return DispalyIns->autoAlign();
}

void CanvasManager::setAutoArrange(bool on)
{
    qInfo() << "set auto arrange" << on;
    DispalyIns->setAutoAlign(on);
    GridIns->setMode(on ? CanvasGrid::Mode::Align : CanvasGrid::Mode::Custom);
    if (on) {
        GridIns->arrange();
        update();
    }

    d->hookIfs->autoArrangeChanged(on);
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

        const QString screenName = getScreenName(primary);
        if (screenName.isEmpty()) {
            qWarning() << "can not get screen name from root window";
            return;
        }

        CanvasViewPointer view = d->viewMap.value(screenName);
        d->viewMap.clear();
        if (view.get()) {
            d->updateView(view, primary, 1);
            qInfo() << "update primary view" << screenName;
        } else {
            view = d->createView(primary, 1);
            qInfo() << "create primary view" << screenName;
        }

        d->viewMap.insert(screenName, view);
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
                qInfo() << "update view" << screenNum << screenName;
            } else {
                view = d->createView(win, screenNum);
                qInfo() << "create view" << screenNum << screenName;
                d->viewMap.insert(screenName, view);
            }
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
            qInfo() << "view geometry is equal to rect,and discard changes" << avRect;
            continue;
        }

        qInfo() << "view geometry change from" << view->geometry() << "to" << avRect;
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

    qInfo() << "add items to grid, count:" << existItems.count() << DispalyIns->autoAlign();
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
    qInfo() << "hidden flags changed to" << show;
    if (show != canvasModel->showHiddenFiles()) {
        canvasModel->setShowHiddenFiles(show);
        canvasModel->refresh(canvasModel->rootIndex());
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
    auto fileInsertByTouch = [this ](const QUrl &url)-> bool {
        const QString path = url.toString();
        auto touchFileData = FileOperatorProxyIns->touchFileData();
        if (path == touchFileData.first) {
            if (CanvasGrid::Mode::Custom == GridIns->mode()) {
                GridIns->tryAppendAfter({ path }, touchFileData.second.first, touchFileData.second.second);
            } else {
                GridIns->append(path);
            }
            FileOperatorProxyIns->clearTouchFileData();

            // need open editor,only by menu create file
            qInfo() << "grid touch file " << path;
            q->openEditor(url);
            return true;
        }

        return false;
    };

    auto fileInsertByPasted = [this ](const QUrl &url, const QModelIndex &index)-> bool {
        auto pasteFileData = FileOperatorProxyIns->pasteFileData();
        if (pasteFileData.contains(url)) {
            FileOperatorProxyIns->removePasteFileData(url);
            selectionModel->select(index, QItemSelectionModel::Select);
        }

        return false;
    };

    auto fileInsertToGrid = [this ](const QUrl &url) {
        const QString path = url.toString();
        QPair<int, QPoint> pos;

        // file is not existed, append it.
        if (!GridIns->point(path, pos))
            GridIns->append(path);
    };

    for (int i = first; i <= last; i++) {
        QModelIndex index = canvasModel->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        QUrl url = canvasModel->fileUrl(index);

        // append to grid on specified point and open editor
        if (fileInsertByTouch(url))
            continue;

        // to select
        fileInsertByPasted(url, index);

        // append to grid.
        fileInsertToGrid(url);
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
            for (int j = 1; j <= viewCount; j++) {
                if (GridIns->overloadItems(j).contains(path)) {
                    GridIns->remove(j, path);
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

    qInfo() << "layout items to align" << existItems.size();
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
    // update trash file
    QUrl trash = d->sourceModel->rootUrl().toString() + "/dde-trash.desktop";
    auto idx = d->sourceModel->index(trash);
    if (idx.isValid()) {
        if (auto file = d->sourceModel->fileInfo(idx)) {
            file->refresh();
            emit d->sourceModel->dataChanged(idx, idx);
        }
    }
}

void CanvasManager::refresh(bool silent)
{
    for (auto it = d->viewMap.begin(); it != d->viewMap.end(); ++it) {
        it.value()->refresh(silent);
    };
}
