// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarwidget.h"
#include "treemodels/sidebarmodel.h"
#include "sidebarview.h"
#include "sidebaritem.h"
#include "sidebaritemdelegate.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarmanager.h"
#include "utils/sidebarinfocachemananger.h"

#include <dfm-framework/event/event.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/dialogmanager.h>

#include <QApplication>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QThread>
#include <QDebug>
#include <QCloseEvent>
#include <QTimer>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

QSharedPointer<SideBarModel> SideBarWidget::kSidebarModelIns { nullptr };
constexpr int kWinOptimalPerformance = 4;
constexpr char kWindowEffectTypeKey[] = "user_type";

SideBarWidget::SideBarWidget(QFrame *parent)
    : AbstractFrame(parent),
      sidebarViewContainer(new DBlurEffectWidget(this))
{
    compositingConfig = DConfig::create("org.kde.kwin", "org.kde.kwin.compositing", QString(), this);
    sidebarView = new SideBarView(sidebarViewContainer);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(sidebarView), AcName::kAcDmSideBarView);

#endif
    if (!kSidebarModelIns) {
        kSidebarModelIns.reset(new SideBarModel);
        initDefaultModel();
    }
    initializeUi();
    initConnect();
    sidebarView->updateSeparatorVisibleState();
}

void SideBarWidget::setCurrentUrl(const QUrl &url)
{
    sidebarView->setCurrentUrl(url);
}

QUrl SideBarWidget::currentUrl() const
{
    return sidebarView->currentUrl();
}

void SideBarWidget::changeEvent(QEvent *event)
{
    return QWidget::changeEvent(event);
}

void SideBarWidget::resetSettingPanel()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    clearSettingPanel();
    initSettingPannel();
}

QAbstractItemView *SideBarWidget::view()
{
    return sidebarView;
}

int SideBarWidget::addItem(SideBarItem *item, bool direct)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    int r { kSidebarModelIns->appendRow(item, direct) };
    bool hidden { !SideBarHelper::hiddenRules().value(item->itemInfo().visiableControlKey, true).toBool() };
    if (r >= 0 && hidden)
        setItemVisiable(item->url(), false);

    return r;
}

bool SideBarWidget::insertItem(const int index, SideBarItem *item)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    bool r { kSidebarModelIns->insertRow(index, item) };
    bool hidden { !SideBarHelper::hiddenRules().value(item->itemInfo().visiableControlKey, true).toBool() };

    if (r && hidden)
        setItemVisiable(item->url(), false);

    return r;
}

bool SideBarWidget::removeItem(const QUrl &url)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    return kSidebarModelIns->removeRow(url);
}

void SideBarWidget::updateItem(const QUrl &url, const ItemInfo &newInfo)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    kSidebarModelIns->updateRow(url, newInfo);
    bool hidden { !SideBarHelper::hiddenRules().value(newInfo.visiableControlKey, true).toBool() };
    if (hidden)
        setItemVisiable(newInfo.url, false);
}

/*!
 * \brief Find the index of the first item match the given \a url
 * \param url
 * \return the index of the item we can found, or -1 if not found.
 */
int SideBarWidget::findItem(const QUrl &url) const
{
    // TODO(zhuangshu): In sidebar tree-model mode, this function is deprecated and be instead of findItemIndex()
    for (int i = 0; i < kSidebarModelIns->rowCount(); i++) {
        SideBarItem *item = kSidebarModelIns->itemFromIndex(i);
        if (!dynamic_cast<SideBarItemSeparator *>(item)) {
            bool foundByCb = item->itemInfo().findMeCb && item->itemInfo().findMeCb(item->url(), url);
            if (foundByCb || (item->url().scheme() == url.scheme() && item->url().path() == url.path()))
                return i;
        }
    }

    fmDebug() << "Item not found in sidebar, URL:" << url;
    return -1;
}
// zhuangshu: Currently, function findItemIndex can only support to find out the sub item of group,
// so it can not find out the top item (group item),
// but do not effect our function.
QModelIndex SideBarWidget::findItemIndex(const QUrl &url) const
{
    return sidebarView->findItemIndex(url);
}

void SideBarWidget::editItem(const QUrl &url)
{
    QModelIndex ret = findItemIndex(url);
    int pos = ret.row();
    if (pos < 0) {
        fmWarning() << "Cannot edit item, not found in sidebar, URL:" << url;
        return;
    }

    auto idx = kSidebarModelIns->index(pos, 0, ret.parent());
    if (idx.isValid()) {
        sidebarView->edit(idx);
    } else {
        fmWarning() << "Cannot edit item, invalid index, URL:" << url;
    }
}

void SideBarWidget::setItemVisiable(const QUrl &url, bool visible)
{
    fmDebug() << "url = " << url << ",visible = " << visible;

    Q_ASSERT(qApp->thread() == QThread::currentThread());
    // find out the item index by url
    const QModelIndex index = this->findItemIndex(url);   // ps: currently,findItemIndex can only find the sub item
    if (!index.isValid()) {
        fmWarning() << "setItemVisiable index is invalid:" << url;
        return;
    }
    SideBarItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
    if (item && item->parent()) {
        //  item->setHiiden(visible);
        sidebarView->setRowHidden(item->row(), item->parent()->index(), !visible);
    } else {
        fmWarning() << "Cannot change item visibility, invalid item or parent, URL:" << url;
    }

    sidebarView->updateSeparatorVisibleState();
}

void SideBarWidget::updateItemVisiable(const QVariantMap &states)
{
    for (auto iter = states.cbegin(); iter != states.cend(); ++iter) {
        auto urls = findItemUrlsByVisibleControlKey(iter.key());
        bool visiable = iter.value().toBool();
        std::for_each(urls.cbegin(), urls.cend(), [visiable, this](const QUrl &url) { setItemVisiable(url, visiable); });
    }

    sidebarView->updateSeparatorVisibleState();
}

QList<QUrl> SideBarWidget::findItemUrlsByVisibleControlKey(const QString &key) const
{
    QList<QUrl> ret;
    for (const SideBarItem *item : kSidebarModelIns->subItems()) {
        if (item->itemInfo().visiableControlKey == key)
            ret.append(item->url());
    }

    return ret;
}

void SideBarWidget::updateSelection()
{
    // after sort the hightlight may lose, re-select the highlight item.
    quint64 winId = SideBarHelper::windowId(this);
    auto window = FMWindowsIns.findWindowById(winId);
    if (window)
        setCurrentUrl(window->currentUrl());
}

void SideBarWidget::saveStateWhenClose()
{
    sidebarView->saveStateWhenClose();
}

void SideBarWidget::onItemActived(const QModelIndex &index)
{
    SideBarItem *item = kSidebarModelIns->itemFromIndex(index);
    if (!item || dynamic_cast<SideBarItemSeparator *>(item)) {
        fmDebug() << "Item activation ignored for separator or null item";
        return;
    }

    DViewItemActionList list = item->actionList(Qt::RightEdge);
    if (list.count() > 0 && !list.first()->isEnabled()) {
        list.first()->setDisabled(false);
        setCurrentUrl(list.first()->property("currentItem").toUrl());   // for keeping the selected item.
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QUrl url { qvariant_cast<QUrl>(item->data(SideBarItem::Roles::kItemUrlRole)) };
    if (NetworkUtils::instance()->checkFtpOrSmbBusy(url)) {
        DialogManager::instance()->showUnableToVistDir(url.path());
        QApplication::restoreOverrideCursor();
        auto preIndex = sidebarView->previousIndex();
        if (!preIndex.isValid()) {
            sidebarView->setPreviousIndex(preIndex);
            fmWarning() << "Network resource busy, unable to visit:" << url;
            return;
        }
        SideBarItem *preItem = kSidebarModelIns->itemFromIndex(preIndex);
        if (!preItem || dynamic_cast<SideBarItemSeparator *>(preItem))
            return;
        setCurrentUrl(qvariant_cast<QUrl>(preItem->data(SideBarItem::Roles::kItemUrlRole)));
        sidebarView->setPreviousIndex(preIndex);
        fmWarning() << "Reverted to previous item due to network busy:" << url;
        return;
    }

    QApplication::restoreOverrideCursor();
    auto flag = !DConfigManager::instance()->value(kViewDConfName,
                                                   kOpenFolderWindowsInASeparateProcess, true)
                         .toBool();

    auto target = item->targetUrl();
    if (flag && FileManagerWindowsManager::instance().containsCurrentUrl(target, window())) {

        // run Open folder windows in a separate process
        SideBarManager::instance()->openFolderInASeparateProcess(target);
        auto preIndex = sidebarView->previousIndex();
        if (!preIndex.isValid()) {
            sidebarView->setPreviousIndex(preIndex);
            fmDebug() << "Opened folder in separate process:" << target;
            return;
        }
        SideBarItem *preItem = kSidebarModelIns->itemFromIndex(preIndex);
        if (!preItem || dynamic_cast<SideBarItemSeparator *>(preItem))
            return;
        auto win = qobject_cast<FileManagerWindow *>(window());
        QUrl cur;
        if (win)
            cur = win->currentUrl();
        auto preUrl = preItem->data(SideBarItem::Roles::kItemUrlRole).toUrl();
        if (cur.isValid() && cur != preUrl) {
            setCurrentUrl(cur);
            return;
        }
        setCurrentUrl(qvariant_cast<QUrl>(preItem->data(SideBarItem::Roles::kItemUrlRole)));
        sidebarView->setPreviousIndex(preIndex);
        fmDebug() << "Reverted to previous item after separate process launch";
        return;
    }
    SideBarManager::instance()->runCd(item, SideBarHelper::windowId(this));
    sidebarView->update(sidebarView->previousIndex());
    sidebarView->update(sidebarView->currentIndex());

    fmInfo() << "Item activation completed, URL:" << url;
}

void SideBarWidget::customContextMenuCall(const QPoint &pos)
{
    SideBarItem *item = sidebarView->itemAt(pos);
    if (!item) {
        fmDebug() << "Context menu request ignored, no item at position";
        return;
    }

    // 相对坐标转全局坐标
    const QPoint &globalPos = sidebarView->mapToGlobal(pos);

    SideBarManager::instance()->runContextMenu(item, SideBarHelper::windowId(this), globalPos);
}

void SideBarWidget::onItemRenamed(const QModelIndex &index, const QString &newName)
{
    SideBarItem *item = kSidebarModelIns->itemFromIndex(index);
    if (!item) {
        fmWarning() << "Item rename failed, item not found";
        return;
    }

    QUrl url { qvariant_cast<QUrl>(item->data(SideBarItem::Roles::kItemUrlRole)) };
    fmInfo() << "Renaming item from" << item->text() << "to" << newName << "URL:" << url;
    SideBarManager::instance()->runRename(item, SideBarHelper::windowId(this), newName);
}

void SideBarWidget::updateWindowEffect()
{
    if (!compositingConfig) {
        fmWarning() << "Compositing config is null, cannot update window effect";
        return;
    }

    auto winEffect = compositingConfig->value(kWindowEffectTypeKey).toInt() != kWinOptimalPerformance;
    if (winEffect == isWindowEffect)
        return;

    isWindowEffect = winEffect;
    sidebarViewContainer->setBlurEnabled(winEffect);
    setAutoFillBackground(!winEffect);
}

void SideBarWidget::initializeUi()
{
    sidebarViewContainer->setMode(DBlurEffectWidget::GaussianBlur);
    sidebarViewContainer->setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    sidebarViewContainer->setBlurRectXRadius(-1);
    sidebarViewContainer->setMaskAlpha(204);
    updateBackgroundColor();

    QHBoxLayout *frameLayout = new QHBoxLayout();
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);

    QWidget *leftSpacer = new QWidget();
    leftSpacer->setFixedWidth(2);
    leftSpacer->setBackgroundRole(QPalette::Base);

    QWidget *topSpacer = new QWidget();
    topSpacer->setFixedHeight(50);
    topSpacer->setBackgroundRole(QPalette::Base);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addWidget(topSpacer);
    vlayout->addWidget(sidebarView);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);

    QHBoxLayout *leftLayout = new QHBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    leftLayout->addWidget(leftSpacer);
    leftLayout->addLayout(vlayout);
    sidebarViewContainer->setLayout(leftLayout);

    frameLayout->addWidget(sidebarViewContainer);
    setLayout(frameLayout);

    sidebarView->setModel(kSidebarModelIns.data());
    kSidebarModelIns->addEmptyItem();
    sidebarView->setItemDelegate(new SideBarItemDelegate(sidebarView));

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "SplitterState").toMap();
    int splitState = state.value("sidebar", 200).toInt();
    QSize size = this->size();
    size.setWidth(splitState);
    resize(size);

    setFocusProxy(sidebarView);
    updateWindowEffect();
}

void SideBarWidget::initDefaultModel()
{
    currentGroups << DefaultGroup::kCommon
                  << DefaultGroup::kDevice
                  << DefaultGroup::kNetwork
                  << DefaultGroup::kTag
                  << DefaultGroup::kOther
                  << DefaultGroup::kNotExistedGroup;

    groupDisplayName.insert(DefaultGroup::kCommon, tr("Quick access"));
    groupDisplayName.insert(DefaultGroup::kDevice, tr("Partitions"));
    groupDisplayName.insert(DefaultGroup::kNetwork, tr("Network"));
    groupDisplayName.insert(DefaultGroup::kTag, tr("Tag"));
    groupDisplayName.insert(DefaultGroup::kOther, tr("Other"));
    groupDisplayName.insert(DefaultGroup::kNotExistedGroup, tr("Unknown Group"));

    // create defualt separator item.
    for (const QString &group : currentGroups) {
        auto item = SideBarHelper::createSeparatorItem(group);
        item->setData(groupDisplayName.value(group), Qt::DisplayRole);
        addItem(item);
    }

    sidebarView->updateSeparatorVisibleState();
}

void SideBarWidget::initConnect()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this](DGuiApplicationHelper::ColorType themeType) {
                Q_UNUSED(themeType);
                updateBackgroundColor();
            });

    // do `cd` work
    connect(sidebarView, &SideBarView::activated,
            this, &SideBarWidget::onItemActived);

    // we need single click also trigger activated()
    connect(sidebarView, &SideBarView::clicked,
            this, &SideBarWidget::onItemActived);

    // context menu
    connect(sidebarView, &SideBarView::customContextMenuRequested,
            this, &SideBarWidget::customContextMenuCall);

    auto delegate = qobject_cast<SideBarItemDelegate *>(sidebarView->itemDelegate());
    if (delegate) {
        connect(delegate, &SideBarItemDelegate::rename, this, &SideBarWidget::onItemRenamed);
        connect(delegate, &SideBarItemDelegate::changeExpandState, sidebarView, &SideBarView::onChangeExpandState);
    }

    // so no extra separator if a group is empty.
    // since we do this, ensure we do initConnection() after initModelData().
    connect(kSidebarModelIns.data(), &SideBarModel::rowsInserted, sidebarView, &SideBarView::updateSeparatorVisibleState);
    connect(kSidebarModelIns.data(), &SideBarModel::rowsRemoved, sidebarView, &SideBarView::updateSeparatorVisibleState);
    connect(kSidebarModelIns.data(), &SideBarModel::rowsMoved, sidebarView, &SideBarView::updateSeparatorVisibleState);

    // 监听窗口特效模式切换
    connect(compositingConfig, &DConfig::valueChanged, this, [this](const QString &key) {
        if (key == kWindowEffectTypeKey)
            updateWindowEffect();
    });
}

void SideBarWidget::clearSettingPanel()
{
    const auto &bindings { SideBarInfoCacheMananger::instance()->getLastSettingBindingKeys() };
    const auto &settings { SideBarInfoCacheMananger::instance()->getLastSettingKeys() };
    // clear binding
    std::for_each(bindings.begin(), bindings.end(), [](const QString &key) {
        SideBarHelper::removebindingSetting(key);
    });
    // clear config
    std::for_each(settings.begin(), settings.end(), [](const QString &key) {
        SideBarHelper::removeItemFromSetting(key);
    });

    // clear cache
    SideBarInfoCacheMananger::instance()->clearLastSettingKey();
    SideBarInfoCacheMananger::instance()->clearLastSettingBindingKey();
}

void SideBarWidget::initSettingPannel()
{
    // travel model and add group settings to panel
    QMap<QString, QStringList> itemKeysMap { { DefaultGroup::kCommon, {} },
                                             { DefaultGroup::kDevice, {} },
                                             { DefaultGroup::kNetwork, {} },
                                             { DefaultGroup::kTag, {} } };
    QMap<QString, int> levelMap { { DefaultGroup::kCommon, 0 },
                                  { DefaultGroup::kDevice, 0 },
                                  { DefaultGroup::kNetwork, 0 },
                                  { DefaultGroup::kTag, 0 } };
    const auto &groups { SideBarInfoCacheMananger::instance()->groups() };
    std::for_each(groups.begin(), groups.end(), [&itemKeysMap, &levelMap](const QString &group) {
        auto items { kSidebarModelIns->subItems(group) };
        for (auto item : items) {
            const QString &key { item->itemInfo().visiableControlKey };
            const QString &name { item->itemInfo().visiableDisplayName };
            Q_ASSERT(!key.isEmpty() && !name.isEmpty());
            if (itemKeysMap[group].contains(key) || key == "hidden_me") {
                fmDebug() << "reject key:" << key << group;
                continue;
            }
            itemKeysMap[group].push_back(key);
            SideBarHelper::addItemToSettingPannel(group, key, name, &levelMap);
        }
    });
}

void SideBarWidget::updateBackgroundColor()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        sidebarViewContainer->setMaskColor(QColor(16, 16, 16));
    } else {
        sidebarViewContainer->setMaskColor(QColor(255, 255, 255));
    }
}
