// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerview.h"
#include "computerstatusbar.h"
#include "private/computerview_p.h"
#include "models/computermodel.h"
#include "delegate/computeritemdelegate.h"
#include "utils/computerutils.h"
#include "events/computereventcaller.h"
#include "controller/computercontroller.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-framework/dpf.h>

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QtConcurrent>
#include <QApplication>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DGuiApplicationHelper>
#    include <DSizeMode>
#endif

namespace dfmplugin_computer {
static QSharedPointer<ComputerModel> kCptModelIns { nullptr };

using namespace GlobalServerDefines;

ComputerView::ComputerView(const QUrl &url, QWidget *parent)
    : DListView(parent),
      dp(new ComputerViewPrivate(this))
{
    Q_UNUSED(url);

    if (!kCptModelIns)
        kCptModelIns.reset(new ComputerModel);

    initView();
    initConnect();
}

ComputerView::~ComputerView()
{
    disconnect();

    if (model()) {
        disconnect(model(), nullptr, this, nullptr);
    }
    if (selectionModel()) {
        disconnect(selectionModel(), nullptr, this, nullptr);
        selectionModel()->clear();
    }

    DListView::setModel(nullptr);
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
    if (selectionModel && selectionModel->hasSelection()) {
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
                    this->cdTo(idx);
                    return true;
                } else {
                    this->setCurrentIndex(idx);
                }
            }
        }
    }
    return DListView::eventFilter(watched, event);
}

void ComputerView::setStatusBarHandler(ComputerStatusBar *sb)
{
    dp->statusBar = sb;
}

void ComputerView::setRowHidden(int row, bool hide)
{
    // 更新item的可见性状态，避免view隐藏了但绘制时仍然绘制该item的问题
    // 在某些性能较差的机器上，通过view隐藏item后，若在delegate中进行了
    // 设值操作（model()->data()），可能会绘制本该隐藏的item，造成显示问题
    model()->setData(model()->index(row, 0), !hide, ComputerModel::DataRoles::kItemVisibleRole);
    DListView::setRowHidden(row, hide);
}

void ComputerView::showEvent(QShowEvent *event)
{
    QApplication::restoreOverrideCursor();
    // items in computer are hidden in view, they still exist in model.
    fmInfo() << "start update item visible in computerview.";
    handleComputerItemVisible();
    fmInfo() << "end update item visible in computerview.";

    // 订阅容量监控（引用计数 +1）
    QTimer::singleShot(0, []() {
        fmDebug() << "Computer view shown, subscribing to device usage monitoring";
        DevProxyMng->refreshUsage();
        DevProxyMng->subscribeUsageMonitoring();
    });

    DListView::showEvent(event);
}

void ComputerView::hideEvent(QHideEvent *event)
{
    auto selectionModel = this->selectionModel();
    selectionModel->clearSelection();

    // 取消订阅容量监控（引用计数 -1）
    QTimer::singleShot(0, []() {
        fmDebug() << "Computer view hidden, unsubscribing from device usage monitoring";
        DevProxyMng->unsubscribeUsageMonitoring();
    });

    DListView::hideEvent(event);
}

ComputerModel *ComputerView::computerModel() const
{
    return kCptModelIns.data();
}

void ComputerView::initView()
{
    Q_ASSERT(kCptModelIns.data());
    this->setModel(kCptModelIns.data());
    this->setItemDelegate(new ComputerItemDelegate(this));
    qobject_cast<QListView *>(this)->setWrapping(true);
    qobject_cast<QListView *>(this)->setFlow(QListView::Flow::LeftToRight);
#ifdef DTKWIDGET_CLASS_DSizeMode
    this->setSpacing(DSizeModeHelper::element(5, 10));
#else
    this->setSpacing(10);
#endif
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
    connect(this, &ComputerView::clicked, this, std::bind(enter, std::placeholders::_1, kEnterBySingleClick));
    connect(this, &ComputerView::doubleClicked, this, std::bind(enter, std::placeholders::_1, kEnterByDoubleClick));
    connect(this, &ComputerView::customContextMenuRequested, this, &ComputerView::onMenuRequest);

    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ComputerView::onSelectionChanged);

    connect(ComputerControllerInstance, &ComputerController::requestRename, this, &ComputerView::onRenameRequest);
    connect(ComputerControllerInstance, &ComputerController::updateItemAlias, this, &ComputerView::onUpdateItemAlias);

    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::updatePartitionsVisiable, this, &ComputerView::handleComputerItemVisible);
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::hideFileSystemTag, this, [this]() { this->update(); });

    connect(kCptModelIns.data(), &ComputerModel::requestHandleItemVisible, this, &ComputerView::handleComputerItemVisible);
    connect(kCptModelIns.data(), &ComputerModel::requestUpdateIndex, this, [this](const QModelIndex &index) {
        update(index);
    });
    connect(kCptModelIns.data(), &ComputerModel::requestClearSelection, this, [this](const QUrl &url) {
        if (selectedUrlList().contains(url))
            //        view->clearSelection(); // NOTE: this do not work, might be a bug in QT
            setCurrentIndex(QModelIndex());   // NOTE: and this works good.
    });
    ;

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
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        this->setSpacing(DSizeModeHelper::element(5, 10));
    });
#endif
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
    if (!index.isValid()) {
        fmDebug() << "Menu request at invalid index position:" << pos;
        return;
    }

    if (index.data(ComputerModel::DataRoles::kItemShapeTypeRole).toInt() == ComputerItemData::kSplitterItem) {
        fmDebug() << "Menu request on splitter item, ignoring";
        return;
    }

    auto url = index.data(ComputerModel::DataRoles::kDeviceUrlRole).toUrl();
    ComputerControllerInstance->onMenuRequest(ComputerUtils::getWinId(this), url, false);
}

void ComputerView::onRenameRequest(quint64 winId, const QUrl &url)
{
    if (winId != ComputerUtils::getWinId(this)) {
        fmDebug() << "Rename request for different window, ignoring";
        return;
    }

    auto model = qobject_cast<ComputerModel *>(this->model());
    if (!model) {
        fmWarning() << "Failed to get ComputerModel for rename operation";
        return;
    }

    int r = model->findItem(url);
    auto idx = model->index(r, 0);
    if (idx.isValid())
        edit(idx);
}

void ComputerView::onUpdateItemAlias(const QUrl &url, const QString &alias, bool isProtocol)
{
    if (isProtocol) {
        DFMEntryFileInfoPointer info(new EntryFileInfo(url));
        const auto &itemList = computerModel()->itemList();
        for (const auto &item : itemList) {
            if (item.shape == ComputerItemData::kSplitterItem || !item.info)
                continue;

            if (info->targetUrl().scheme() != item.info->targetUrl().scheme()
                || info->targetUrl().host() != item.info->targetUrl().host())
                continue;

            QVariantMap map {
                { "Property_Key_DisplayName", item.info->displayName() },
                { "Property_Key_EditDisplayText", item.info->editDisplayText() },
                { "Property_Key_Editable", true }
            };

            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", item.info->urlOf(UrlInfoType::kUrl), map);
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_Crumb_Update", item.info->urlOf(UrlInfoType::kUrl));
            ComputerEventCaller::sendItemRenamed(item.info->urlOf(UrlInfoType::kUrl), alias);

            int row = computerModel()->findItem(item.info->urlOf(UrlInfoType::kUrl));
            this->update(computerModel()->index(row, 0));
        }
    } else {
        const auto &item = computerModel()->findItemData(url);
        if (!item.info)
            return;

        // update sidebar
        QString sidebarName = alias.isEmpty() ? item.info->displayName() : alias;
        QVariantMap map {
            { "Property_Key_DisplayName", sidebarName },
            { "Property_Key_Editable", true }
        };
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", item.info->urlOf(UrlInfoType::kUrl), map);
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_SetAlias", item.info->targetUrl(), alias);

        int row = computerModel()->findItem(url);
        this->update(computerModel()->index(row, 0));
    }
}

void ComputerView::handleDisksVisible()
{
    /* NOTE(xust): disks hidden by dconfig is treat as disks hidden by HintIgnore.
     * devices should be hidden both in sidebar and computer.
     * hidden in sidebar is handled in ComputerItemWatcher when dconfig changed.
     * this part only handle items hidden in computer.
     *
     * HintIgnore > DConfig > SettingPanel
     *
     * hidden by HintIgnore is handled in BlockEntryFileEntity::exist() function.
     * if it's true, treat the device as not exists.
     */

    auto model = this->computerModel();
    if (!model) {
        fmCritical() << "model is released somewhere!";
        return;
    }

    const auto &&hiddenPartitions = ComputerItemWatcher::hiddenPartitions();

    fmInfo() << "ignored/hidden disks:" << hiddenPartitions;
    for (int i = 7; i < model->items.count(); i++) {   // 7 means where the disk group start.
        QString currSuffix = model->data(model->index(i, 0), ComputerModel::kSuffixRole).toString();
        if (currSuffix != SuffixInfo::kBlock)
            continue;
        auto item = model->items.at(i);
        this->setRowHidden(i, hiddenPartitions.contains(item.url));
    }
    handleDiskSplitterVisible();
}

void ComputerView::handleUserDirVisible()
{
    bool hideUserDir = ComputerItemWatcher::hideUserDir();
    const int kDiskSplitterIndex = 7;
    for (int i = 0; i < kDiskSplitterIndex && i < model()->rowCount(); ++i)
        setRowHidden(i, hideUserDir);
}

void ComputerView::handle3rdEntriesVisible()
{
    bool hide3rdEntries = ComputerItemWatcher::hide3rdEntries();

    for (int i = 0; i < model()->rowCount(); ++i) {
        QString currSuffix = model()->data(model()->index(i, 0), ComputerModel::kSuffixRole).toString();
        if (ComputerUtils::isNativeDevice(currSuffix))
            continue;

        int shape = model()->data(model()->index(i, 0), ComputerModel::kItemShapeTypeRole).toInt();
        if (shape == ComputerItemData::kSplitterItem)
            continue;

        setRowHidden(i, hide3rdEntries);
    }
}

void ComputerView::handleDiskSplitterVisible()
{
    auto model = this->computerModel();
    if (!model) {
        fmCritical() << "model is released somewhere!";
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

void ComputerView::onSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    const auto &selects = selected.indexes();
    if (selects.isEmpty()) {
        dp->statusBar->itemCounted(dp->visibleItemCount());
        return;
    }

    const QModelIndex &current = selects.first();
    if (model()->data(current, ComputerModel::kSuffixRole).toString() == SuffixInfo::kUserDir) {
        const QUrl &url = model()->data(current, ComputerModel::kRealUrlRole).toUrl();
        auto info = InfoFactory::create<FileInfo>(url);
        dp->statusBar->itemSelected(QList<FileInfo *> { info.data() });
        return;
    }

    dp->statusBar->showSingleSelectionMessage();
}

void ComputerView::handleComputerItemVisible()
{
    handleUserDirVisible();
    handle3rdEntriesVisible();
    handleDisksVisible();

    dp->statusBar->itemCounted(dp->visibleItemCount());
}

void ComputerView::cdTo(const QModelIndex &index)
{
    int r = index.row();
    if (r < 0 || r >= model()->rowCount()) {
        fmWarning() << "Invalid row index for navigation:" << r;
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

int ComputerViewPrivate::visibleItemCount()
{
    if (!kCptModelIns)
        return 0;

    const int &total = kCptModelIns->rowCount();
    int visibleCount = total;

    for (int i = 0; i < total; i++) {
        if (q->isRowHidden(i)) {
            visibleCount--;
            continue;
        }

        const int &rowShape = kCptModelIns->data(kCptModelIns->index(i, 0), ComputerModel::kItemShapeTypeRole).toInt();
        if (rowShape == ComputerItemData::kSplitterItem) {
            visibleCount--;
            continue;
        }
    }

    return visibleCount;
}

void ComputerView::keyPressEvent(QKeyEvent *event)
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

}
