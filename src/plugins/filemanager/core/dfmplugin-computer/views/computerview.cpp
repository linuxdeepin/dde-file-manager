// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerview.h"
#include "private/computerview_p.h"
#include "models/computermodel.h"
#include "delegate/computeritemdelegate.h"
#include "utils/computerutils.h"
#include "events/computereventcaller.h"
#include "controller/computercontroller.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-framework/dpf.h>

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QtConcurrent>
#include <QApplication>

namespace dfmplugin_computer {
using namespace GlobalServerDefines;

ComputerView::ComputerView(const QUrl &url, QWidget *parent)
    : DListView(parent),
      dp(new ComputerViewPrivate(this))
{
    Q_UNUSED(url);

    initView();
    initConnect();
}

ComputerView::~ComputerView()
{
}

QWidget *ComputerView::widget() const
{
    return const_cast<ComputerView *>(this);
}

QUrl ComputerView::rootUrl() const
{
    return ComputerUtils::rootUrl();
}

DFMBASE_NAMESPACE::AbstractBaseView::ViewState ComputerView::viewState() const
{
    return DFMBASE_NAMESPACE::AbstractBaseView::ViewState::kViewIdle;
}

bool ComputerView::setRootUrl(const QUrl &url)
{
    Q_UNUSED(url);
    QApplication::restoreOverrideCursor();
    return true;
}

QList<QUrl> ComputerView::selectedUrlList() const
{
    auto selectionModel = this->selectionModel();
    if (selectionModel->hasSelection()) {
        const QModelIndex &idx = selectionModel->currentIndex();
        QUrl url = idx.data(ComputerModel::DataRoles::kDeviceUrlRole).toUrl();
        return { url };
    }
    return {};
}

bool ComputerView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease && watched == this->viewport()) {
        auto me = static_cast<QMouseEvent *>(event);
        const auto &idx = this->indexAt(me->pos());
        if (me->button() == Qt::MouseButton::LeftButton && (!idx.isValid() || !(idx.flags() & Qt::ItemFlag::ItemIsEnabled))) {
            this->selectionModel()->clearSelection();
            return false;
        }

        quint64 winId = FileManagerWindowsManager::instance().findWindowId(this->viewport());
        if (me->button() == Qt::BackButton) {
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_Navigator_Backward", winId);
            return true;
        } else if (me->button() == Qt::ForwardButton) {
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_Navigator_Forward", winId);
            return true;
        }
    } else if (event->type() == QEvent::KeyPress && watched == this) {
        auto ke = static_cast<QKeyEvent *>(event);
        if (ke->modifiers() == Qt::Modifier::ALT) {
            this->event(event);
            return true;
        }
        if (ke->key() == Qt::Key::Key_Enter || ke->key() == Qt::Key::Key_Return) {
            const auto &idx = this->selectionModel()->currentIndex();
            if (idx.isValid()) {
                if (!this->model()->data(idx, ComputerModel::DataRoles::kItemIsEditingRole).toBool()) {
                    Q_EMIT enterPressed(idx);
                    return true;
                }
            }
        }
    }
    return DListView::eventFilter(watched, event);
}

void ComputerView::showEvent(QShowEvent *event)
{
    QApplication::restoreOverrideCursor();
    handlePartitionsVisiable();
    DListView::showEvent(event);
}

void ComputerView::hideEvent(QHideEvent *event)
{
    auto selectionModel = this->selectionModel();
    selectionModel->clearSelection();
    DListView::hideEvent(event);
}

ComputerModel *ComputerView::computerModel() const
{
    auto model = qobject_cast<ComputerModel *>(DListView::model());
    return model;
}

void ComputerView::initView()
{
    dp->model = new ComputerModel(this);
    this->setModel(dp->model);
    this->setItemDelegate(new ComputerItemDelegate(this));
    qobject_cast<QListView *>(this)->setWrapping(true);
    qobject_cast<QListView *>(this)->setFlow(QListView::Flow::LeftToRight);
    this->setSpacing(10);
    this->setResizeMode(QListView::ResizeMode::Adjust);
    this->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    this->setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);
    //    this->setIconSize(QSize(iconsizes[m_statusbar->scalingSlider()->value()], iconsizes[m_statusbar->scalingSlider()->value()]));
    this->setIconSize(QSize(64, 64));
    this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    this->setFrameShape(QFrame::Shape::NoFrame);
    this->viewport()->setAutoFillBackground(false);

    this->installEventFilter(this);
    this->viewport()->installEventFilter(this);
}

void ComputerView::initConnect()
{
    const int kEnterBySingleClick = 0;
    const int kEnterByDoubleClick = 1;
    const int kEnterByEnter = 2;
    auto enter = [this](const QModelIndex &idx, int type) {
        int mode = DFMBASE_NAMESPACE::Application::appAttribute(DFMBASE_NAMESPACE::Application::ApplicationAttribute::kOpenFileMode).toInt();
        if (type == mode || type == kEnterByEnter)
            this->cdTo(idx);
    };
    connect(this, &QAbstractItemView::clicked, this, std::bind(enter, std::placeholders::_1, kEnterBySingleClick));
    connect(this, &QAbstractItemView::doubleClicked, this, std::bind(enter, std::placeholders::_1, kEnterByDoubleClick));
    connect(this, &ComputerView::enterPressed, this, &ComputerView::cdTo);

    connect(this, &ComputerView::customContextMenuRequested, this, &ComputerView::onMenuRequest);
    connect(ComputerControllerInstance, &ComputerController::requestRename, this, &ComputerView::onRenameRequest);
    connect(ComputerControllerInstance, &ComputerController::updateItemAlias, this, [this](const QUrl &url) {
        int row = computerModel()->findItem(url);
        this->update(computerModel()->index(row, 0));
    });

    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::hideNativeDisks, this, &ComputerView::handlePartitionsVisiable);
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::hideLoopPartitions, this, &ComputerView::handlePartitionsVisiable);
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::hideFileSystemTag, this, [this]() { this->update(); });
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::hideDisks, this, &ComputerView::hideSpecificDisks);

    connectShortcut(QKeySequence(Qt::Key::Key_I | Qt::Modifier::CTRL), [this](DFMEntryFileInfoPointer info) {
        if (info)
            ComputerControllerInstance->actProperties(ComputerUtils::getWinId(this), info);
        else
            ComputerEventCaller::sendShowPropertyDialog({ ComputerUtils::rootUrl() });
    });
    connectShortcut(QKeySequence(Qt::Key::Key_N | Qt::Modifier::CTRL), [this](DFMEntryFileInfoPointer info) {
        if (info)
            ComputerControllerInstance->actOpenInNewWindow(ComputerUtils::getWinId(this), info);
        else
            ComputerEventCaller::sendEnterInNewWindow(ComputerUtils::rootUrl());
    });
    connectShortcut(QKeySequence(Qt::Key::Key_T | Qt::Modifier::CTRL), [this](DFMEntryFileInfoPointer info) {
        if (info)
            ComputerControllerInstance->actOpenInNewTab(ComputerUtils::getWinId(this), info);
        else
            ComputerEventCaller::sendEnterInNewTab(ComputerUtils::getWinId(this), ComputerUtils::rootUrl());
    });
}

void ComputerView::connectShortcut(QKeySequence seq, std::function<void(DFMEntryFileInfoPointer)> slot)
{
    QAction *act = new QAction(this);
    addAction(act);
    act->setShortcut(seq);
    connect(act, &QAction::triggered, this, [this, slot] {
        QList<QUrl> &&selectedUrls = selectedUrlList();
        if (!selectedUrls.isEmpty()) {
            DFMEntryFileInfoPointer info(new EntryFileInfo(selectedUrls.first()));
            slot(info);
        } else {
            slot(nullptr);
        }
    });
}

void ComputerView::onMenuRequest(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);
    if (!index.isValid())
        return;

    if (index.data(ComputerModel::DataRoles::kItemShapeTypeRole).toInt() == ComputerItemData::kSplitterItem)
        return;

    auto url = index.data(ComputerModel::DataRoles::kDeviceUrlRole).toUrl();
    ComputerControllerInstance->onMenuRequest(ComputerUtils::getWinId(this), url, false);
}

void ComputerView::onRenameRequest(quint64 winId, const QUrl &url)
{
    if (winId != ComputerUtils::getWinId(this))
        return;

    auto model = qobject_cast<ComputerModel *>(this->model());
    if (!model)
        return;

    int r = model->findItem(url);
    auto idx = model->index(r, 0);
    if (idx.isValid())
        edit(idx);
}

void ComputerView::hideSpecificDisks(const QList<QUrl> &hiddenDisks)
{
    hideSystemPartitions(ComputerUtils::shouldSystemPartitionHide());
    hideLoopPartitions(ComputerUtils::shouldLoopPartitionsHide());

    auto model = this->computerModel();
    if (!model) {
        qCritical() << "model is released somewhere! " << __FUNCTION__;
        return;
    }

    for (int i = 7; i < model->items.count(); i++) {   // 7 means where the disk group start.
        auto item = model->items.at(i);
        if (hiddenDisks.contains(item.url))
            this->setRowHidden(i, true);
    }

    handleDiskSplitterVisiable();
}

void ComputerView::hideSystemPartitions(bool hide)
{
    auto model = this->computerModel();
    if (!model) {
        qCritical() << "model is released somewhere! " << __FUNCTION__;
        return;
    }

    for (int i = 7; i < model->items.count(); i++) {   // 7 means where the disk group start.
        auto item = model->items.at(i);
        if (!item.url.path().endsWith(SuffixInfo::kBlock))
            continue;

        bool removable = item.info && item.info->extraProperty(DeviceProperty::kRemovable).toBool();
        bool isLoop = item.info && item.info->extraProperty(DeviceProperty::kIsLoopDevice).toBool();
        if (!removable && !isLoop)
            this->setRowHidden(i, hide);
    }
    handleDiskSplitterVisiable();
}

void ComputerView::hideLoopPartitions(bool hide)
{
    auto model = this->computerModel();
    if (!model) {
        qCritical() << "model is released somewhere! " << __FUNCTION__;
        return;
    }

    for (int i = 7; i < model->items.count(); i++) {   // 7 means where the disk group start.
        auto item = model->items.at(i);
        if (!item.url.path().endsWith(SuffixInfo::kBlock))
            continue;

        bool isLoop = item.info && item.info->extraProperty(DeviceProperty::kIsLoopDevice).toBool();
        if (isLoop)
            this->setRowHidden(i, hide);
    }
    handleDiskSplitterVisiable();
}

void ComputerView::handleDiskSplitterVisiable()
{
    auto model = this->computerModel();
    if (!model) {
        qCritical() << "model is released somewhere! " << __FUNCTION__;
        return;
    }

    int diskSplitterRow = -1;
    bool traversingDisks = false;
    bool splitterHide = true;
    for (int i = 0; i < model->items.count(); i++) {
        const auto &item = model->items.at(i);
        if (item.groupId != ComputerItemWatcherInstance->getGroupId(ComputerItemWatcher::diskGroup())) {
            if (traversingDisks)
                break;
            continue;
        }

        traversingDisks = true;
        if (item.shape == ComputerItemData::kSplitterItem) {
            diskSplitterRow = i;
        } else {
            if (!isRowHidden(i)) {
                splitterHide = false;
                break;
            }
        }
    }
    setRowHidden(diskSplitterRow, splitterHide);
}

void ComputerView::handlePartitionsVisiable()
{
    hideSystemPartitions(ComputerUtils::shouldSystemPartitionHide());
    hideLoopPartitions(ComputerUtils::shouldLoopPartitionsHide());

    // handle items hidden by DConfig.
    hideSpecificDisks(ComputerItemWatcher::disksHiddenByDConf());
}

void ComputerView::cdTo(const QModelIndex &index)
{
    int r = index.row();
    if (r < 0 || r >= model()->rowCount()) {
        return;
    }

    typedef ComputerItemData::ShapeType ItemType;
    ItemType type = ItemType(index.data(ComputerModel::DataRoles::kItemShapeTypeRole).toInt());
    if (type == ItemType::kSplitterItem)
        return;

    auto url = index.data(ComputerModel::DataRoles::kDeviceUrlRole).toUrl();
    ComputerControllerInstance->onOpenItem(ComputerUtils::getWinId(this), url);
}

ComputerViewPrivate::ComputerViewPrivate(ComputerView *qq)
    : q(qq)
{
}

}

void dfmplugin_computer::ComputerView::keyPressEvent(QKeyEvent *event)
{
    switch (event->modifiers()) {
    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Left:
            [[fallthrough]];
        case Qt::Key_Right:
            [[fallthrough]];
        case Qt::Key_Up:
            [[fallthrough]];
        case Qt::Key_Down:
            return QWidget::keyPressEvent(event);
        }
    }
    return DListView::keyPressEvent(event);
}
