// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsidebar.h"

#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dabstractfilewatcher.h"
#include "dfilemanagerwindow.h"
#include "dfileservices.h"
#include "singleton.h"
#include "app/define.h"
#include "drootfilemanager.h"
#include "dtoolbar.h"
#include "utils.h"
#include "dfmsidebarmanager.h"
#include "dfmsidebaritemdelegate.h"
#include "dfmsidebaritem.h"
#include "dfmopticalmediawidget.h"
#include "gvfs/secretmanager.h"
#include "dfmeventdispatcher.h"
#include "gvfs/networkmanager.h"

#include "interfaces/dfmsidebariteminterface.h"
#include "views/dfmsidebarview.h"
#include "models/dfmsidebarmodel.h"
#include "models/dfmrootfileinfo.h"
#include "controllers/dfmsidebardefaultitemhandler.h"
#include "controllers/dfmsidebarbookmarkitemhandler.h"
#include "controllers/dfmsidebardeviceitemhandler.h"
#include "controllers/dfmsidebartagitemhandler.h"
#include "controllers/dfmsidebarvaultitemhandler.h" // 保险柜
#include "controllers/vaultcontroller.h"
#include "controllers/pathmanager.h"
#include "app/filesignalmanager.h"
#include "vault/vaulthelper.h"
#include "interfaces/dfilemenu.h"
#include "accessibility/ac-lib-file-manager.h"
#include "deviceinfo/udisklistener.h"
#include "shutil/fileutils.h"
#include "shutil/smbintegrationswitcher.h"
#include "dtoolbar.h"
#include "utils.h"
#include "controllers/bookmarkmanager.h"
#include "grouppolicy.h"
#include "gvfs/networkmanager.h"


#include <DApplicationHelper>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QDebug>

#include <ddiskmanager.h>
#include <ddiskdevice.h>
#include <dblockdevice.h>
#include <QMenu>
#include <QtConcurrent>

#include <algorithm>

#include "plugins/schemepluginmanager.h"

#define SIDEBAR_ITEMORDER_KEY "SideBar/ItemOrder"
#define RECENT_HIDDEN "dfm.recent.hidden"

DFM_BEGIN_NAMESPACE

DFMSideBar::DFMSideBar(QWidget *parent)
    : QWidget(parent),
      m_sidebarView(new DFMSideBarView(this)),
      m_sidebarModel(new DFMSideBarModel(this))
{
    AC_SET_OBJECT_NAME(this, AC_DM_SIDE_BAR);
    AC_SET_ACCESSIBLE_NAME(this, AC_DM_SIDE_BAR);

    // init view.
    m_sidebarView->setModel(m_sidebarModel);
    m_sidebarView->setItemDelegate(new DFMSideBarItemDelegate(m_sidebarView));
    // #bug133653 侧边栏左右间距10个像素
    m_sidebarView->setViewportMargins(10, 0, 10, 0);
    m_sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_sidebarView->setFrameShape(QFrame::Shape::NoFrame);
    m_sidebarView->setAutoFillBackground(true);

    initUI();
    initModelData();
    initConnection();
    initUserShareItem();
    initRecentItem();

    //   DFMSideBarManager::instance();
    //NOTE [XIAO] 从Plugin中导入SideBarItem
    initItemFromPlugin();
}

DFMSideBar::~DFMSideBar()
{
#ifdef ENABLE_ASYNCINIT
    m_initDevThread.first = true;
    m_initDevThread.second.waitForFinished();
#endif
}

QWidget *DFMSideBar::sidebarView()
{
    return m_sidebarView;
}

QRect DFMSideBar::groupGeometry(const QString &groupName)
{
    return  m_sidebarView->visualRect(groupModelIndex(groupName));
}

void DFMSideBar::scrollToGroup(const QString &groupName)
{
    m_sidebarView->scrollTo(groupModelIndex(groupName));
}

void DFMSideBar::setCurrentUrl(const DUrl &url, bool changeUrl)
{
    QMutexLocker lk(&m_currentUrlMutex);
    m_currentUrl = changeUrl ? url : m_currentUrl;
    int index = findItem(m_currentUrl, true);
    if (index != -1) {
        m_sidebarView->setCurrentIndex(m_sidebarModel->index(index, 0));
        m_sidebarView->updateItemUniqueKey(m_sidebarView->currentIndex());
    } else {
        m_sidebarView->clearSelection();
    }
}

int DFMSideBar::addItem(DFMSideBarItem *item, const QString &group)
{
    if (!item) {
        return 0;
    }
    int lastAtGroup = findLastItem(group, false);
    lastAtGroup++; // append after the last item
    this->insertItem(lastAtGroup, item, group);

    return lastAtGroup;
}

bool DFMSideBar::removeItem(const DUrl &url, const QString &group)
{
    int index = findItem(url, group);
    bool succ = false;
    if (index >= 0) {
        succ = m_sidebarModel->removeRow(index);
    }

    return succ;
}

int DFMSideBar::findItem(const DFMSideBarItem *item) const
{
    return m_sidebarModel->indexFromItem(item).row();
}

int DFMSideBar::findItem(const DUrl &url, const QString &group) const
{
    for (int i = 0, nEnd = m_sidebarModel->rowCount(); i < nEnd; ++i) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        const auto &itemType = item->itemType();
        const auto &itemGroupName = item->groupName();
        if (itemType == DFMSideBarItem::SidebarItem && itemGroupName == group) {
            const auto &itemUrl = item->url();
            if (itemUrl == url) {
                return i;
            }
        }
    }

    return -1;
}

/*!
 * \brief Find the index of the first item match the given \a url
 *
 * \return the index of the item we can found, or -1 if not found.
 */
int DFMSideBar::findItem(const DUrl &url, bool fuzzy/* = false*/) const
{
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (item->itemType() == DFMSideBarItem::SidebarItem) {
            if (item->url() == url)
                return i;

            if (!fuzzy)
                continue;

            DUrl itemUrl = item->url();
            if (itemUrl.isBookMarkFile() && DUrl(itemUrl.path()) == url) {
                return i;
            } else if (itemUrl.scheme() == DFMROOT_SCHEME) {
                DAbstractFileInfoPointer pointer = DFileService::instance()->createFileInfo(nullptr, itemUrl);
                if (!pointer)
                    continue;
                if (pointer->redirectedFileUrl() == url)
                    return i;
            }
        }
    }

    return -1;
}

int DFMSideBar::findItem(std::function<bool (const DFMSideBarItem *)> cb) const
{
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (cb(item)) {
            return i;
        }
    }

    return -1;
}

int DFMSideBar::findLastItem(const QString &group, bool sidebarItemOnly) const
{
    int index = -1;
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (item->groupName() == group && (item->itemType() == DFMSideBarItem::SidebarItem || !sidebarItemOnly)) {
            index = i;
        } else if (item->groupName() != group && index != -1) {
            // already found the group and already leaved the group
            break;
        }
    }

    return index;
}

void DFMSideBar::openItemEditor(int index) const
{
    m_sidebarView->edit(m_sidebarModel->index(index, 0));
}

QSet<QString> DFMSideBar::disableUrlSchemes() const
{
    return m_disableUrlSchemes;
}

void DFMSideBar::setContextMenuEnabled(bool enabled)
{
    m_contextMenuEnabled = enabled;
}

void DFMSideBar::setDisableUrlSchemes(const QSet<QString> &schemes)
{
    m_disableUrlSchemes += schemes;
    for (QString scheme : m_disableUrlSchemes) {
        forever {
            int index = findItem([&](const DFMSideBarItem * item) -> bool {
                return item->url().scheme() == scheme;
            });

            if (index >= 0) {
                m_sidebarModel->removeRow(index);
            } else {
                break;
            }
        }
    }

    emit disableUrlSchemesChanged();
}

DUrlList DFMSideBar::savedItemOrder(const QString &groupName) const
{
    DUrlList list;

    QStringList savedList = DFMApplication::genericSetting()->value(SIDEBAR_ITEMORDER_KEY, groupName).toStringList();
    for (const QString &item : savedList) {
        list << DUrl(item);
    }

    return list;
}

void DFMSideBar::saveItemOrder(const QString &groupName) const
{
    QVariantList list;

    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(m_sidebarModel->index(i, 0));
        if (item->itemType() == DFMSideBarItem::SidebarItem && item->groupName() == groupName) {
            list << QVariant(item->url());
        }
    }

    DFMApplication::genericSetting()->setValue(SIDEBAR_ITEMORDER_KEY, groupName, list);
}

QString DFMSideBar::groupName(DFMSideBar::GroupName group)
{
    Q_ASSERT(group != Unknow);

    switch (group) {
    case Common:
        return "common";
    case Device:
        return "device";
    case Bookmark:
        return "bookmark";
    case Network:
        return "network";
    case Tag:
        return "tag";
    case Other: // deliberate
    default:
        break;
    }

    return QString();
}

DFMSideBar::GroupName DFMSideBar::groupFromName(const QString &name)
{
    if (name.isEmpty()) {
        return Other;
    }

    switch (name.toLatin1().at(0)) {
    case 'c':
        if (name == QStringLiteral("common")) {
            return Common;
        }

        break;
    case 'd':
        if (name == QStringLiteral("device")) {
            return Device;
        }

        break;
    case 'b':
        if (name == QStringLiteral("bookmark")) {
            return Bookmark;
        }

        break;
    case 'n':
        if (name == QStringLiteral("network")) {
            return Network;
        }

        break;
    case 't':
        if (name == QStringLiteral("tag")) {
            return Tag;
        }

        break;
    default:
        break;
    }

    return Unknow;
}

void DFMSideBar::rootFileResult()
{
    QList<DAbstractFileInfoPointer> filist  = rootFileManager->getRootFile();
    qDebug() << "DFileService::instance()->getRootFile() filist:" << filist.size();
    if(smbIntegrationSwitcher->isIntegrationMode()){
        foreach (DAbstractFileInfoPointer r, filist) {
                QString smbIp;
                FileUtils::isSmbRelatedUrl(r->fileUrl(),smbIp);
                if (!smbIp.isEmpty()){
                    if (r->fileUrl().toString().endsWith( QString(".%1").arg(SUFFIX_GVFSMP)) && FileUtils::isNetworkUrlMounted( r->fileUrl() )){
                        devitems.removeOne(r->fileUrl());
                        filist.removeOne(r);
                    }

                    if (r->fileUrl().toString().endsWith( QString(".%1").arg(SUFFIX_STASHED_REMOTE))){
                        filist.removeOne(r);
                    }
                }
        }
    }

    if (filist.isEmpty())
        return;

    for (const DAbstractFileInfoPointer &fi : filist) {
#ifdef ENABLE_ASYNCINIT
        if (m_initDevThread.first){
            qDebug() << "thrad cancled" << this;
            return;
        }
#endif
        if (static_cast<DFMRootFileInfo::ItemType>(fi->fileType()) != DFMRootFileInfo::ItemType::UserDirectory) {
            if (devitems.contains(fi->fileUrl())) {
                continue;
            }
            if (Singleton<PathManager>::instance()->isVisiblePartitionPath(fi)) {
                // 这里需要根据url进行排序
                const auto &url = fi->fileUrl();
                DUrl itemUrl = url;
                if(smbIntegrationSwitcher->isIntegrationMode()){
                    QString smbIp;
                    if (FileUtils::isSmbRelatedUrl(url, smbIp) && !smbIp.isEmpty())
                        itemUrl = "smb://" + smbIp;
                }
                auto r = std::upper_bound(devitems.begin(), devitems.end(), itemUrl,
                [](const DUrl & a, const DUrl & b) {
                    DAbstractFileInfoPointer fia = fileService->createFileInfo(nullptr, a);
                    DAbstractFileInfoPointer fib = fileService->createFileInfo(nullptr, b);
                    return DFMRootFileInfo::typeCompare(fia, fib);
                });
                DFMSideBarItem *newItem = DFMSideBarDeviceItemHandler::createItem(itemUrl);
                if (r == devitems.end()) {
                    this->addItem(newItem, this->groupName(Device));
                    devitems.append(itemUrl);
                } else {
                    this->insertItem(this->findLastItem(this->groupName(Device)) - (devitems.end() - r) + 1, newItem, this->groupName(Device));
                    devitems.insert(r, itemUrl);
                }
            }
        }
    }
    //无论是否设置smb挂载项常驻，此处只依据RemoteMountsStashManager::stashedSmbDevices()中的内容进行界面显示
    if(smbIntegrationSwitcher->isIntegrationMode()){
        QStringList smbSideBarItems = RemoteMountsStashManager::stashedSmbDevices();
            foreach (const QString& smbDevice, smbSideBarItems) {
                if (isSmbItemExisted(DUrl(smbDevice)))
                    removeItem(DUrl(smbDevice), this->groupName(Device));
                DFMSideBarItem *item = DFMSideBarDeviceItemHandler::createItem(smbDevice);
                if (item){
                    this->addItem(item, this->groupName(Device));
                }
            }
    }
}

/**
 * @brief DFMSideBar::jumpToItem
 */
void DFMSideBar::jumpToItem(const DUrl& url, GroupName group)
{
    int index = findItem(url, groupName(group));
    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
    if (item) {
        QString identifierStr = item->registeredHandler(SIDEBAR_ID_INTERNAL_FALLBACK);
        QScopedPointer<DFMSideBarItemInterface> interface(DFMSideBarManager::instance()->createByIdentifier(identifierStr));
        if (interface) {
                interface->cdAction(this, item);
        }
    }
}

bool DFMSideBar::isSmbItemExisted(const DUrl &smbDevice)
{
    int index = findItem(smbDevice, this->groupName(Device));
    return index >=0 && smbIntegrationSwitcher->isIntegrationMode();
}

void DFMSideBar::onItemActivated(const QModelIndex &index)
{
    if (!DRootFileManager::instance()->isRootFileInited())
        return;
    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
    QString identifierStr = item->registeredHandler(SIDEBAR_ID_INTERNAL_FALLBACK);

    if (m_lastToggleTime.isValid() && m_lastToggleTime.addMSecs(500) > QDateTime::currentDateTime()
            && m_pLastToggleItem == item) {
        m_lastToggleTime = QDateTime::currentDateTime();
        return;
    }
    m_lastToggleTime = QDateTime::currentDateTime();
    m_pLastToggleItem = item;

    QScopedPointer<DFMSideBarItemInterface> interface(DFMSideBarManager::instance()->createByIdentifier(identifierStr));
    if (interface) {
        // searchBarTextEntered also invoke "checkGvfsMountFileBusy", forbit invoke twice
        if (item->url().path().endsWith(SUFFIX_STASHED_REMOTE)) {
            DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(this->window());
            if (window) {
                auto path = RemoteMountsStashManager::normalizeConnUrl(item->url().path());
                window->getToolBar()->searchBarTextEntered(path);
                return;
            }
        }
        //判断网络文件是否可以到达
        if (DFileService::instance()->checkGvfsMountfileBusy(item->url())) {
            return;
        }
        DFileService::instance()->setCursorBusyState(true);
        interface->cdAction(this, item);

        DFileService::instance()->setCursorBusyState(DFMOpticalMediaWidget::hasVolProcessBusy());
    }
}

void DFMSideBar::onContextMenuRequested(const QPoint &pos)
{
    // block signals to avoid function reentrant.
    const QSignalBlocker blocker(this);

    if (!m_contextMenuEnabled) return;


    QModelIndex modelIndex = m_sidebarView->indexAt(pos);
    if (!modelIndex.isValid()) {
        return;
    }

    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(modelIndex);
    if (!item || item->itemType() == DFMSideBarItem::Separator) {
        return ; // separator should not show menu
    }
    QString identifierStr = item->registeredHandler(SIDEBAR_ID_INTERNAL_FALLBACK);
    DFileService::instance()->setCursorBusyState(true);
    QScopedPointer<DFMSideBarItemInterface> interface(DFMSideBarManager::instance()->createByIdentifier(identifierStr));
    QMenu *menu = nullptr;
    if (interface) {
        menu = interface->contextMenu(this, item);

        if (menu) {
            // 如果光驱正在执行刻录/擦除操作，禁用光驱的右键菜单
            QString strVolTag = item->url().path().remove("/").remove(".localdisk"); // /sr0.localdisk 去头去尾
            if (strVolTag.startsWith("sr") && DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].bBurningOrErasing) {
                for (QAction *act : menu->actions())
                    act->setEnabled(false);
            }
            DFileMenu *fmenu = qobject_cast<DFileMenu *>(menu);
            DFileService::instance()->setCursorBusyState(false);
            if (fmenu) {
                //fix bug 33305 在用右键菜单复制大量文件时，在复制过程中，关闭窗口这时this释放了，
                //在关闭拷贝menu的exec退出，menu的deleteLater崩溃
                QPointer<DFMSideBar> me = this;
                fmenu->exec(this->mapToGlobal(pos));
                fmenu->deleteLater(me);
            } else {
                menu->exec(this->mapToGlobal(pos));
                menu->deleteLater();
            }
        }
        DFileService::instance()->setCursorBusyState(false);
    }
    DFileService::instance()->setCursorBusyState(false);
}

void DFMSideBar::onRename(const QModelIndex &index, QString newName) const
{
    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
    QString identifierStr = item->registeredHandler(SIDEBAR_ID_INTERNAL_FALLBACK);

    QScopedPointer<DFMSideBarItemInterface> interface(DFMSideBarManager::instance()->createByIdentifier(identifierStr));
    if (interface && !newName.isEmpty() && item->text() != newName) {
        interface->rename(item, newName);
    }
    if (m_sidebarView)
        m_sidebarView->update();
}

void DFMSideBar::initUI()
{
    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_sidebarView);

    layout->setMargin(0);
    layout->setSpacing(0);

    this->setMaximumWidth(200);
    this->setFocusProxy(m_sidebarView);

    applySidebarColor();
}

void DFMSideBar::initModelData()
{
    // register meta type for DUrl, since we use it in item view DnD operation.
    qRegisterMetaTypeStreamOperators<DUrl>("DUrl");

    static QList<DFMSideBar::GroupName> groups = {
        //NOTE [REN] 添加Plugin类型，实现插件组的分割线

        GroupName::Common, GroupName::Device, GroupName::Bookmark, GroupName::Network, GroupName::Tag, GroupName::Plugin
    };

    //bool hasSeparator = false;
    foreach (const DFMSideBar::GroupName &groupType, groups) {
#ifdef DISABLE_TAG_SUPPORT
        if (groupType == DFMSideBar::GroupName::Tag) continue;
#endif // DISABLE_TAG_SUPPORT

        m_sidebarModel->appendRow(DFMSideBarItem::createSeparatorItem(groupName(groupType)));

        addGroupItems(groupType);
    }

    // init done, then we should update the separator visible state.
    updateSeparatorVisibleState();
}

void DFMSideBar::initConnection()
{
    connect(DFMApplication::instance(), &DFMApplication::reloadComputerModel, this, [this]{
        bool vaultEnable = VaultHelper::isVaultEnabled();
        if (vaultEnable) {
            // if vault is enabled, check if it is exist in sidebar
            int index = findItem([&](const DFMSideBarItem *item) {return item->url().scheme() == DFMVAULT_SCHEME;});
            if (index > 0)
                return;

            // otherwise insert it just below computer item
            index = findItem([&](const DFMSideBarItem *item) {return item->url().scheme() == COMPUTER_SCHEME;});
            if (index > 0)
                this->insertItem(index+1, DFMSideBarVaultItemHandler::createItem("Vault"), groupName(GroupName::Device));
        } else {
            // check if vault item is in sidebar
            int index = findItem([&](const DFMSideBarItem *item) {return item->url().scheme() == DFMVAULT_SCHEME;});
            if (index < 0)
                return;
            this->m_sidebarModel->removeRow(index);
        }
    });

    // drag to delete bookmark or tag
    connect(m_sidebarView, &DFMSideBarView::requestRemoveItem, this, [this]() {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(m_sidebarView->currentIndex());
        if (item && item->flags().testFlag(Qt::ItemIsEnabled) && item->flags().testFlag(Qt::ItemIsDragEnabled)) {
            DFileService::instance()->deleteFiles(nullptr, DUrlList{item->url()}, false);
        }
    });
    // do `cd` work
    connect(m_sidebarView, &QListView::activated, this, &DFMSideBar::onItemActivated);

    // we need single click also trigger activated()
    connect(m_sidebarView, &QListView::clicked, this, &DFMSideBar::onItemActivated);

    // context menu
    connect(m_sidebarView, &QListView::customContextMenuRequested, this, &DFMSideBar::onContextMenuRequested);

    // so no extra separator if a group is empty.
    // since we do this, ensure we do initConnection() after initModelData().
    connect(m_sidebarModel, &QStandardItemModel::rowsInserted, this, &DFMSideBar::updateSeparatorVisibleState);
    connect(m_sidebarModel, &QStandardItemModel::rowsRemoved, this, &DFMSideBar::updateSeparatorVisibleState);
    connect(m_sidebarModel, &QStandardItemModel::rowsMoved, this, &DFMSideBar::updateSeparatorVisibleState);

    // drag to move item will emit rowsInserted and rowsMoved..
    connect(m_sidebarModel, &QStandardItemModel::rowsRemoved, this,
            [this](const QModelIndex & parent, int first, int last) {
        Q_UNUSED(parent);
        Q_UNUSED(last);

        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(first);
        if (!item)
            item = m_sidebarModel->itemFromIndex(first - 1);

        // only bookmark and tag item are DragEnabled
        if (item && item->flags().testFlag(Qt::ItemIsEnabled) && item->flags().testFlag(Qt::ItemIsDragEnabled)) {
            saveItemOrder(item->groupName());
            return;
        }

        if (item && item->text().isEmpty())
            item = m_sidebarModel->itemFromIndex(item->index().row() - 1);

        // only bookmark and tag item are DragEnabled
        if (item && item->flags().testFlag(Qt::ItemIsEnabled) && item->flags().testFlag(Qt::ItemIsDragEnabled)) {
            saveItemOrder(item->groupName());
        }
    });
    DFMSideBarItemDelegate *idelegate = dynamic_cast<DFMSideBarItemDelegate *>(m_sidebarView->itemDelegate());
    if (idelegate) {
        connect(idelegate, &DFMSideBarItemDelegate::rename, this, &DFMSideBar::onRename);
    }

    connect(fileSignalManager, &FileSignalManager::requestRename, this, [this](const DFMUrlBaseEvent & event) {
        if (event.sender() == this) {
            this->openItemEditor(this->findItem(event.url()));
        }
    });

    if(smbIntegrationSwitcher->isIntegrationMode()){
        DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(this->window());
        if (window) {
            connect(networkManager,&NetworkManager::addSmbMountIntegration,this,[&](const DUrl& url){
                if(!smbIntegrationSwitcher->isIntegrationMode())
                    return;
                if (url.scheme() != SMB_SCHEME)
                    return;
                QString smbIp = url.host();
                if (!this->isSmbItemExisted(DUrl(("smb://" + smbIp)))){
                    DFMSideBarItem *newItem = DFMSideBarDeviceItemHandler::createItem(url);
                    this->addItem(newItem, this->groupName(Device));
                    this->setCurrentUrl(DUrl(("smb://" + smbIp)),true);
                }
            });
        }
    }

    connect(SmbIntegrationSwitcher::instance(),&SmbIntegrationSwitcher::smbIntegrationModeChanged,[this](bool switchToIntegration){
        QStringList smbDevices = RemoteMountsStashManager::stashedSmbDevices();
        for (const QString& smbDevice : smbDevices) {
            QList<DAbstractFileInfoPointer> filist  = rootFileManager->getRootFile();
            foreach (DAbstractFileInfoPointer r, filist) {
                QString temIp;
                bool isSmbRelated = FileUtils::isSmbRelatedUrl(r->fileUrl(),temIp);
                QString host = smbDevice;
                if ( (!isSmbRelated) || (!r->fileUrl().toString().contains(host.remove("smb://"))))//!isSmbRelated:排除ftp挂载
                    continue;

                if (r->fileUrl().toString().endsWith(QString(".%1").arg(SUFFIX_GVFSMP)) && FileUtils::isNetworkUrlMounted(r->fileUrl())) {
                    if (switchToIntegration) // 切换到smb聚合模式，从侧边栏移除已经存在的分离项
                        this->removeItem(r->fileUrl(), groupName(Device));
                    else // 切换到smb分离模式，从缓存移除已经存在的分离挂载项(否则无法添加item)
                        devitems.removeOne(r->fileUrl());
                } else if (r->fileUrl().toString().endsWith(QString(".%1").arg(SUFFIX_STASHED_REMOTE))){
                    if (switchToIntegration)  // 切换到smb聚合模式，从侧边栏移除常驻的分离项
                        this->removeItem(r->fileUrl(), groupName(Device));
                }
            }
        }
        if (!switchToIntegration) { // 切换到smb分离模式后, 需要从侧边栏移除已经存在的smb聚合项(不需要卸载)
            QStringList currentSmbDevices = RemoteMountsStashManager::stashedSmbDevices();
            for (const QString & smbDevice : currentSmbDevices) {
                this->removeItem(DUrl(smbDevice), groupName(Device));
            }
        }
    });

    initBookmarkConnection();
#ifdef ENABLE_ASYNCINIT
    m_initDevThread.first = false;
    m_initDevThread.second = QtConcurrent::run([this](){initDeviceConnection();});
#else
    initDeviceConnection();
#endif

    initTagsConnection();
}

void DFMSideBar::initUserShareItem()
{
    int count = DFileService::instance()->getChildren(nullptr, DUrl::fromUserShareFile("/"),
                                                      QStringList(), QDir::AllEntries).count();
    if (count) {
        addItem(DFMSideBarDefaultItemHandler::createItem("UserShare"), groupName(Network));
    }

    DAbstractFileWatcher *userShareFileWatcher = DFileService::instance()->createFileWatcher(this, DUrl::fromUserShareFile("/"), this);
    Q_CHECK_PTR(userShareFileWatcher);
    userShareFileWatcher->startWatcher();

    auto deleteUserShareLambda = [ = ](const DUrl & url) {
        Q_UNUSED(url)
        int cnt = DFileService::instance()->getChildren(nullptr, DUrl::fromUserShareFile("/"),
                                                        QStringList(), QDir::AllEntries).count();
        int index = findItem(DUrl::fromUserShareFile("/"));
        m_sidebarView->setRowHidden(index, cnt == 0);
    };

    auto addUserShareLambda = [ = ](const DUrl & url) {
        Q_UNUSED(url)
        int cnt = DFileService::instance()->getChildren(nullptr, DUrl::fromUserShareFile("/"),
                                                        QStringList(), QDir::AllEntries).count();
        int index = findItem(DUrl::fromUserShareFile("/"));
        if (index == -1) {
            if (cnt > 0) {
                addItem(DFMSideBarDefaultItemHandler::createItem("UserShare"), groupName(Network));
            }
        } else {
            //            DFileService::instance()->changeRootFile(url,false);
            m_sidebarView->setRowHidden(index, false);
        }
        emit addUserShareItemFinished(url);
    };

    connect(userShareFileWatcher, &DAbstractFileWatcher::fileDeleted, this, deleteUserShareLambda);
    connect(userShareFileWatcher, &DAbstractFileWatcher::subfileCreated, this, addUserShareLambda);
}

void DFMSideBar::initRecentItem()
{
    if (DTK_POLICY_SUPPORT) {
        auto recentLambda = [ = ](bool enable) {
            int index = findItem(DUrl(RECENT_ROOT), groupName(Common));
            if (index) {
                m_sidebarView->setRowHidden(index, !enable);
                if (!enable) {
                    // jump out of recent:///
                    DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT), &DAbstractFileWatcher::fileDeleted, DUrl(RECENT_ROOT));
                }
            }
        };

        auto oldWayRecentLambda = [=](bool var) {
            recentLambda(var);
            // sync policy recent
            auto policyV = GroupPolicy::instance()->getValue(RECENT_HIDDEN);
            if (policyV.isValid() && policyV.toBool() == var)
                GroupPolicy::instance()->setValue(RECENT_HIDDEN, !var);
        };

        auto policyWayRecentLambda = [=](QVariant var){
            auto tempValue = GroupPolicy::instance()->getValue(RECENT_HIDDEN);
            auto oldV = DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowRecentFileEntry);
            if (var.isValid() && (var.toString() == RECENT_HIDDEN) && (tempValue.toBool() == oldV.toBool())) {
                recentLambda(!tempValue.toBool());

                // sync old recent
                DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_ShowRecentFileEntry, !tempValue.toBool());
            }
        };

        if (!GroupPolicy::instance()->containKey(RECENT_HIDDEN)) {
            recentLambda(DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowRecentFileEntry).toBool());
        } else {
            auto tempValue = GroupPolicy::instance()->getValue(RECENT_HIDDEN);
            if (tempValue.isValid()) {
                recentLambda(!tempValue.toBool());
                // sync old recent
                auto oldV = DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowRecentFileEntry);
                if (tempValue.isValid() && tempValue.toBool() == oldV.toBool())
                    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_ShowRecentFileEntry, !tempValue.toBool());
            } else {
                recentLambda(DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowRecentFileEntry).toBool());
            }
        }
        connect(DFMApplication::instance(), &DFMApplication::recentDisplayChanged, this, oldWayRecentLambda);
        connect(GroupPolicy::instance(), &GroupPolicy::valueChanged, this, policyWayRecentLambda);
    } else {
        auto recentLambda = [ = ](bool enable) {
            int index = findItem(DUrl(RECENT_ROOT), groupName(Common));
            if (index) {
                m_sidebarView->setRowHidden(index, !enable);
                if (!enable) {
                    // jump out of recent:///
                    DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT), &DAbstractFileWatcher::fileDeleted, DUrl(RECENT_ROOT));
                }
            }
        };

        recentLambda(DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowRecentFileEntry).toBool());
        connect(DFMApplication::instance(), &DFMApplication::recentDisplayChanged, this, recentLambda);
    }
}

void DFMSideBar::initBookmarkConnection()
{
    DAbstractFileWatcher *bookmarkWatcher = DFileService::instance()->createFileWatcher(this, DUrl(BOOKMARK_ROOT), this);
    if (!bookmarkWatcher) return;

    bookmarkWatcher->startWatcher();

    connect(bookmarkWatcher, &DAbstractFileWatcher::subfileCreated, this,
    [this](const DUrl & url) {
        //        DFileService::instance()->changeRootFile(url);
        const QString &groupNameStr = groupName(Bookmark);
        this->addItem(DFMSideBarBookmarkItemHandler::createItem(url), groupNameStr);
        this->saveItemOrder(groupNameStr);
    });

    connect(bookmarkWatcher, &DAbstractFileWatcher::fileDeleted, this,
    [this](const DUrl & url) {
        qDebug() << url;
        int index = findItem(url, groupName(Bookmark));
        if (index >= 0) {
            //            DFileService::instance()->changeRootFile(url,false);
            m_sidebarModel->removeRow(index);
            this->saveItemOrder(groupName(Bookmark));
        }
    });

    connect(bookmarkWatcher, &DAbstractFileWatcher::fileMoved, this,
    [this](const DUrl & source, const DUrl & target) {
        int index = findItem(source, groupName(Bookmark));
        if (index > 0) {
            DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
            if (item) {
                item->setText(target.bookmarkName());
                item->setUrl(target);
                this->saveItemOrder(groupName(Bookmark));
            }
        }
    });

    bookmarkManager->refreshBookmark();
}

void DFMSideBar::initDeviceConnection()
{
    // 已经初始化了就直接拿结果
    if (DRootFileManager::instance()->isRootFileInited()) {
        rootFileResult();
    }

    // 获取遍历结果进行显示
    connect(DRootFileManager::instance(),&DRootFileManager::queryRootFileFinsh,this,[this](){
        rootFileResult();
    },Qt::QueuedConnection);

    connect(DRootFileManager::instance(),&DRootFileManager::serviceHideSystemPartition,this,[this](){
        QList<DUrl> removelist;
        for (auto itemurl : devitems) {
            if (!DRootFileManager::instance()->isRootFileContain(itemurl)) {
                removelist.push_back(itemurl);
            }
        }
        for (auto removeurl : removelist) {
            devitems.removeOne(removeurl);
            removeItem(removeurl, groupName(Device));
        }
        rootFileResult();
    },Qt::QueuedConnection);

    // 开启遍历线程,刷新一次root，修复分区问题
    DRootFileManager::instance()->startQuryRootFile();

    DAbstractFileWatcher *devicesWatcher = rootFileManager->rootFileWather();
    connect(devicesWatcher, &DAbstractFileWatcher::subfileCreated, this, [this](const DUrl &temUrl) {
        DUrl url = temUrl;

        QString smbIp;
        bool isSmbUrl = FileUtils::isSmbRelatedUrl(url, smbIp);
        if (isSmbUrl && isSmbItemExisted(QString("%1://%2").arg(SMB_SCHEME).arg(smbIp))){//已经添加了
            return;
        }
        // Smb url with smb-integration mode, change `url` to format smb://x.x.x.x and show ip item in sidebar.
        if (isSmbUrl && smbIntegrationSwitcher->isIntegrationMode()){ // fix bug:#168885, fix bug:#169979
            url = DUrl(QString("%1://%2").arg(SMB_SCHEME).arg(smbIp));
        }

        auto fi = fileService->createFileInfo(nullptr, url);
        if (!fi->exists()) {
            return;
        }

        if (!Singleton<PathManager>::instance()->isVisiblePartitionPath(fi)) {
            return;
        }

        if (this->findItem(url) == -1) {
            auto r = std::upper_bound(devitems.begin(), devitems.end(), url,
            [&url](const DUrl & a, const DUrl & b) {
                if(FileUtils::isSmbHostOnly(url))
                    return false;
                DAbstractFileInfoPointer fia = fileService->createFileInfo(nullptr, a);
                DAbstractFileInfoPointer fib = fileService->createFileInfo(nullptr, b);
                return DFMRootFileInfo::typeCompare(fia, fib);
            });
            if (r == devitems.end()) {
                //DFileService::instance()->changeRootFile(url); //性能优化，注释
                DFMSideBarItem *newItem = DFMSideBarDeviceItemHandler::createItem(url);
                this->addItem(newItem, this->groupName(Device));
                devitems.append(url);
            } else {
                //DFileService::instance()->changeRootFile(url); //性能优化，注释
                DFMSideBarItem *newItem = DFMSideBarDeviceItemHandler::createItem(url);
                this->insertItem(this->findLastItem(this->groupName(Device)) - (devitems.end() - r) + 1, newItem, this->groupName(Device));
                    devitems.insert(r, url);
            }
            //还原url
            if (m_currentUrl == fi->redirectedFileUrl())
                setCurrentUrl(m_currentUrl, false);
        }
    });
    connect(devicesWatcher, &DAbstractFileWatcher::fileDeleted, this, [=/*this*/](const DUrl & url) {
        if(FileUtils::isSmbHostOnly(url)){//url like: smb://x.x.x.x
            this->removeItem(url,"device");
            jumpToItem(DUrl(COMPUTER_ROOT));
            return;
        }
        bool curUrlCanAccess = true; // 初始化为true 避免影响原有逻辑
        auto fi = fileService->createFileInfo(nullptr, m_currentUrl);
        if (fi)
            curUrlCanAccess = fi->exists();
        int index = findItem(url, groupName(Device));
        int curIndex = m_sidebarView->currentIndex().row();
        QString smbIp;
        bool isSmbRelatedPath = FileUtils::isSmbRelatedUrl(url, smbIp);
        bool switchToComputerItem = false;
        int remainMountedCount = 0;
        bool isBathUnmuntSmb = devicesWatcher->property("isBathUnmuntSmb").toBool();//批量卸载
        bool lastOneShareFolderRemved = false;
        if (isSmbRelatedPath){
            remainMountedCount = devicesWatcher->property("remainUnmuntSmb").toInt();//每卸载一个，这里的值会递减1
            if (smbIntegrationSwitcher->isIntegrationMode() && remainMountedCount <=0 ){//SMB设备已经没有挂载着的共享目录了
                //标识最后一个SMB挂载目录已被卸载，可以从侧边栏移除（具体是否可以移除，还要看配置GA_AlwaysShowOfflineRemoteConnections）
                lastOneShareFolderRemved = true;
                QString smbDeviceStr = QString("%1://%2").arg(SMB_SCHEME).arg(smbIp);
                RemoteMountsStashManager::removeStashedSmbDevice(smbDeviceStr);//当smbip下，所有挂载项都被移除后，从配置中移除

                DUrl smbDevice(smbDeviceStr);

                if (!secretManager->userCheckedRememberPassword(smbDevice)){//如果挂载共享目录鉴权时，用户没有勾选记住密码
                   secretManager->clearPassworkBySmbHost(smbDevice);//取消记住密码
                   deviceListener->clearLoginData();//清除登录数据，以便下次重新弹出鉴权对话框
                }
            }

            bool keepSmb = DFMApplication::genericAttribute(DFMApplication::GA_AlwaysShowOfflineRemoteConnections).toBool();
            //如果最后一个SMB挂载已被移除 且 配置为 （无需常驻SMB挂载 或 是批量卸载），需要跳转到计算机界面和从侧边栏移除smb聚合项
            if(smbIntegrationSwitcher->isIntegrationMode() && lastOneShareFolderRemved && (!keepSmb || isBathUnmuntSmb)){
                switchToComputerItem = true;
                deviceListener->setBatchedRemovingSmbMount(false);
                //The smb ip item data like: smb://xx.xx.xx.xx
                emit rootFileManager->rootFileWather()->fileDeleted(DUrl(QString("%1://%2").arg(SMB_SCHEME).arg(smbIp)));
            }
        }
        switchToComputerItem = switchToComputerItem && (!(isBathUnmuntSmb && remainMountedCount>0));
         if ((curIndex == index && index != -1)
                || (!curUrlCanAccess) || switchToComputerItem) {
            DUrl urlSkip;
            const QString &absFilePath = url.toAbsolutePathUrl().path();
            QString localFilePath = QUrl::fromPercentEncoding(url.path().toLocal8Bit());
            localFilePath = localFilePath.startsWith("//") ? localFilePath.mid(1) : localFilePath;
            bool blockDevice = false;
            const auto &allDevice = deviceListener->getAllDeviceInfos();
            for (const auto &dev : allDevice.keys()) {
                if (dev.contains(absFilePath.left(absFilePath.indexOf(".localdisk")))) {
                    blockDevice = true;
                    break;
                }
            }

            // 判断删除的路径是否是外设路径，外设路径需要跳转到computer页面
            bool turnToComputer = false;
            if (switchToComputerItem || (deviceListener->isInDeviceFolder(absFilePath)
                    || localFilePath.startsWith("/run/user")
                    || localFilePath.startsWith("/media/")
                    || blockDevice // like u disk
                    || FileUtils::isGvfsMountFile(localFilePath) ))
                    {
                urlSkip = DUrl(COMPUTER_ROOT);
                turnToComputer = true;
            } else {
                urlSkip = DUrl::fromLocalFile(QDir::homePath());
            }
            DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(this->window());

            //后面会根据switchToComputerItem为true去removeItem(), 因此此前不能改变switchToComputerItem的状态
            if(!smbIntegrationSwitcher->isIntegrationMode() || (switchToComputerItem && window && window->isActiveWindow()))
                this->jumpToItem(urlSkip);
        }
        //DFileService::instance()->changeRootFile(url,false); //性能优化，注释
        if (!smbIntegrationSwitcher->isIntegrationMode() || (!isSmbRelatedPath || switchToComputerItem))
            this->removeItem(url, this->groupName(Device));
        devitems.removeAll(url);

    });
    connect(devicesWatcher, &DAbstractFileWatcher::fileAttributeChanged, this, [this](const DUrl & url) {
        int index = findItem(url, groupName(Device));
        DAbstractFileInfoPointer fi = DFileService::instance()->createFileInfo(nullptr, url);

        if (!~index || !fi) {
            return;
        }

        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
        DViewItemActionList actionList = item->actionList(Qt::RightEdge);
        auto list = fi->menuActionList();
        if (!actionList.isEmpty()) {
            actionList.front()->setVisible(list.contains(MenuAction::Eject) ||
                                           list.contains(MenuAction::Unmount) ||
                                           list.contains(MenuAction::SafelyRemoveDrive));
        }

        item->setText(fi->fileDisplayName());

        Qt::ItemFlags flags = item->flags() & (~Qt::ItemFlag::ItemIsEditable);
        if (fi->menuActionList().contains(MenuAction::Rename)) {
            flags |= Qt::ItemFlag::ItemIsEditable;
        }
        item->setFlags(flags);
    });
}

void DFMSideBar::initTagsConnection()
{
#ifdef DISABLE_TAG_SUPPORT
    return;
#endif

    DAbstractFileWatcher *tagsWatcher = DFileService::instance()->createFileWatcher(this, DUrl(TAG_ROOT), this);
    Q_CHECK_PTR(tagsWatcher);
    tagsWatcher->startWatcher();

    QString groupNameStr(groupName(Tag));

    // New tag added.
    connect(tagsWatcher, &DAbstractFileWatcher::subfileCreated, this, [this, groupNameStr](const DUrl & url) {
        //        DFileService::instance()->changeRootFile(url);
        this->addItem(DFMSideBarTagItemHandler::createItem(url), groupNameStr);
        this->saveItemOrder(groupNameStr);
    });

    // Tag get removed.
    connect(tagsWatcher, &DAbstractFileWatcher::fileDeleted, this, [this, groupNameStr](const DUrl & url) {
        //        DFileService::instance()->changeRootFile(url,false);
        this->removeItem(url, groupNameStr);
        this->saveItemOrder(groupNameStr);
    });

    // Tag got rename
    connect(tagsWatcher, &DAbstractFileWatcher::fileMoved, this,
    [this, groupNameStr](const DUrl & source, const DUrl & target) {
        int index = findItem(source, groupNameStr);
        if (index >= 0) {
            DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
            item->setText(target.tagName());
            item->setUrl(target);
            this->saveItemOrder(groupNameStr);
        }
    });

    //    // Tag changed color
    //    q->connect(tagsWatcher, &DAbstractFileWatcher::fileAttributeChanged, group, [group](const DUrl & url) {
    //        DFMSideBarItem *item = group->findItem(url);
    //        item->setIconFromThemeConfig("BookmarkItem." + TagManager::instance()->getTagColorName(url.tagName()));
    //    });
}

//NOTE [XIAO] 从Plugin中导入SideBarItem
void DFMSideBar::initItemFromPlugin()
{
    if (!m_disableUrlSchemes.contains(PLUGIN_SCHEME)) {
        // tab标签页不会显示“我的手机”，因为路径初始化的时候还没有开始加载插件
        // 在这里插件已经加载过所以调用initPaths再加载一下插件的路径 add by CL
        Singleton<PathManager>::instance()->initPaths();
        qInfo() << "[PLUGIN]" << "try to load plugin of sidebar item";
        auto plugins = SchemePluginManager::instance()->schemePlugins();
        for (auto plugin : plugins) {
            qInfo() << "[PLUGIN]" << "load sidebar item from plugin:" << plugin.first;
            DFMSideBarItem *item = plugin.second->createSideBarItem();
            // NOTE [XIAO] 插件中的GroupName与文管版本中一致。
            //this->addItem(item, item->groupName());
            this->appendItem(item, item->groupName());
        }
    }
}

void DFMSideBar::applySidebarColor()
{
    m_sidebarView->setBackgroundType(DStyledItemDelegate::BackgroundType(DStyledItemDelegate::RoundedBackground | DStyledItemDelegate::NoNormalState));
    m_sidebarView->setItemSpacing(0);
}

void DFMSideBar::updateSeparatorVisibleState()
{
    QString lastGroupName = "__not_existed_group";
    int lastGroupItemCount = 0;
    int lastSeparatorIndex = -1;

    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (item->groupName() != lastGroupName) {
            if (item->itemType() == DFMSideBarItem::Separator) {
                m_sidebarView->setRowHidden(i, lastGroupItemCount == 0);
                lastSeparatorIndex = i;
                lastGroupItemCount = 0;
                lastGroupName = item->groupName();
            }
        } else {
            if (item->itemType() == DFMSideBarItem::SidebarItem) {
                lastGroupItemCount++;
            }
        }
    }

    // hide the last one if last group is empty
    if (lastGroupItemCount == 0) {
        m_sidebarView->setRowHidden(lastSeparatorIndex, true);
    }
}

void DFMSideBar::addGroupItems(DFMSideBar::GroupName groupType)
{
    const QString &groupNameStr = groupName(groupType);
    switch (groupType) {
    case GroupName::Common:
        if (!m_disableUrlSchemes.contains(RECENT_SCHEME)) {
            appendItem(DFMSideBarDefaultItemHandler::createItem("Recent"), groupNameStr);
        }
        appendItem(DFMSideBarDefaultItemHandler::createItem("Home"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Desktop"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Videos"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Music"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Pictures"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Documents"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Downloads"), groupNameStr);
        if (!m_disableUrlSchemes.contains(TRASH_SCHEME)) {
            appendItem(DFMSideBarDefaultItemHandler::createItem("Trash"), groupNameStr);
        }
        break;
    case GroupName::Device: {
        if (!m_disableUrlSchemes.contains(COMPUTER_SCHEME)) {
            appendItem(DFMSideBarDefaultItemHandler::createItem("Computer"), groupNameStr);
        }
        // 判断系统类型，决定是否启用保险箱
        if (VaultHelper::isVaultEnabled()) {
            if (!m_disableUrlSchemes.contains(DFMVAULT_SCHEME)) {
                appendItem(DFMSideBarVaultItemHandler::createItem("Vault"), groupNameStr);
            }
        }
        break;
    }
    case GroupName::Bookmark: {
        if (m_disableUrlSchemes.contains(BOOKMARK_SCHEME))  {
            break;
        }
        bookmarkManager->initData();
        const DUrlList urlList = bookmarkManager->getBookmarkUrls();
        QList<DFMSideBarItem *> unsortedList;
        for (const DUrl &url : urlList) {
            unsortedList << DFMSideBarBookmarkItemHandler::createItem(url);
        }
        appendItemWithOrder(unsortedList, savedItemOrder(groupNameStr), groupNameStr);
        break;
    }
    case GroupName::Network:
        if (m_disableUrlSchemes.contains(NETWORK_SCHEME))  {
            break;
        }
        appendItem(DFMSideBarDefaultItemHandler::createItem("Network"), groupNameStr);
        break;
    case GroupName::Tag: {
        if (m_disableUrlSchemes.contains(TAG_SCHEME))  {
            break;
        }

        auto tag_infos = DFileService::instance()->getChildren(this, DUrl(TAG_ROOT),
                                                               QStringList(), QDir::AllEntries);
        QList<DFMSideBarItem *> unsortedList;
        for (const DAbstractFileInfoPointer &info : tag_infos) {
            unsortedList << DFMSideBarTagItemHandler::createItem(info->fileUrl());
        }
        appendItemWithOrder(unsortedList, savedItemOrder(groupNameStr), groupNameStr);
        break;
    }
    default:
        break;
    }
}

void DFMSideBar::insertItem(int index, DFMSideBarItem *item, const QString &groupName)
{
    if (!item) {
        return;
    }
    item->setGroupName(groupName);
    m_sidebarModel->insertRow(index, item);
}

/*!
 * \brief append an \a item to the sidebar item model, with the given \a groupName
 *
 * Warning! Item is directly append to the model, will NOT try to find the group
 * location by the given group name. For that (find group location and append item)
 * purpose, use addItem() instead.
 */
void DFMSideBar::appendItem(DFMSideBarItem *item, const QString &groupName)
{
    qInfo()<<"22 item url = "<<item->url();
    item->setGroupName(groupName);
    m_sidebarModel->appendRow(item);
}

void DFMSideBar::appendItemWithOrder(QList<DFMSideBarItem *> &list, const DUrlList &order, const QString &groupName)
{
    DUrlList urlList;

    for (const DFMSideBarItem *item : list) {
        urlList << item->url();
    }

    for (const DUrl &url : order) {
        int idx = urlList.indexOf(url);
        if (idx >= 0) {
            urlList.removeAt(idx);
            this->appendItem(list.takeAt(idx), groupName);
        }
    }

    for (DFMSideBarItem *item : list) {
        this->appendItem(item, groupName);
    }
}

/*!
 * \brief Find the model-index of the groupNname
 *
 * \return the the model-index of the groupNname we can found, or invalid-model-index if not found.
 */
QModelIndex DFMSideBar::groupModelIndex(const QString &groupName)
{
    return m_sidebarModel->index(findLastItem(groupName), 0, m_sidebarView->rootIndex());
}

void DFMSideBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        applySidebarColor();
    }

    return QWidget::changeEvent(event);
}

DFM_END_NAMESPACE
