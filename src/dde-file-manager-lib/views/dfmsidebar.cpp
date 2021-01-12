/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmsidebar.h"

#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dabstractfilewatcher.h"
#include "dfilemanagerwindow.h"
#include "dfileservices.h"
#include "singleton.h"
#include "app/define.h"
#include "drootfilemanager.h"

#include "dfmsidebarmanager.h"
#include "interfaces/dfmsidebariteminterface.h"
#include "views/dfmsidebarview.h"
#include "models/dfmsidebarmodel.h"
#include "dfmsidebaritemdelegate.h"
#include "dfmsidebaritem.h"
#include "models/dfmrootfileinfo.h"
#include "controllers/dfmsidebardefaultitemhandler.h"
#include "controllers/dfmsidebarbookmarkitemhandler.h"
#include "controllers/dfmsidebardeviceitemhandler.h"
#include "controllers/dfmsidebartagitemhandler.h"
#include "controllers/dfmsidebarvaultitemhandler.h" // 保险柜
#include "controllers/vaultcontroller.h"
#include "controllers/pathmanager.h"
#include "app/filesignalmanager.h"
#include "interfaces/dfilemenu.h"
#include "dfmopticalmediawidget.h"
#include "vault/vaulthelper.h"
#include "accessibility/ac-lib-file-manager.h"

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

#define SIDEBAR_ITEMORDER_KEY "SideBar/ItemOrder"

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
    m_sidebarView->setViewportMargins(10, 0, m_sidebarView->verticalScrollBar()->sizeHint().width(), 0);
    m_sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_sidebarView->setFrameShape(QFrame::Shape::NoFrame);
    m_sidebarView->setAutoFillBackground(true);

    initUI();
    initModelData();
    initConnection();
    initUserShareItem();
    initRecentItem();

    //   DFMSideBarManager::instance();
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

void DFMSideBar::setCurrentUrl(const DUrl &url)
{
    int index = findItem(url, true);
    if (index != -1) {
        m_sidebarView->setCurrentIndex(m_sidebarModel->index(index, 0));
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
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (item->itemType() == DFMSideBarItem::SidebarItem && item->groupName() == group) {
            if (item->url() == url) {
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
    if (filist.isEmpty())
        return;
    std::sort(filist.begin(), filist.end(), &DFMRootFileInfo::typeCompare);

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
                addItem(DFMSideBarDeviceItemHandler::createItem(fi->fileUrl()), groupName(Device));
                devitems.push_back(fi->fileUrl());
            }
        }
    }
}

void DFMSideBar::onItemActivated(const QModelIndex &index)
{
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
    return;
}

void DFMSideBar::onRename(const QModelIndex &index, QString newName) const
{
    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
    QString identifierStr = item->registeredHandler(SIDEBAR_ID_INTERNAL_FALLBACK);

    QScopedPointer<DFMSideBarItemInterface> interface(DFMSideBarManager::instance()->createByIdentifier(identifierStr));
    if (interface && !newName.isEmpty() && item->text() != newName) {
        interface->rename(item, newName);
    }
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
        GroupName::Common, GroupName::Device, GroupName::Bookmark, GroupName::Network, GroupName::Tag
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
        if (!item) {
            item = m_sidebarModel->itemFromIndex(first - 1);
        }

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
}

void DFMSideBar::initDeviceConnection()
{
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

    // 已经初始化了就直接拿结果
   if (DRootFileManager::instance()->isRootFileInited()) {
        rootFileResult();
    }
    // 开启遍历线程,刷新一次root，修复分区问题
    DRootFileManager::instance()->startQuryRootFile();

    DAbstractFileWatcher *devicesWatcher = rootFileManager->rootFileWather();
    connect(devicesWatcher, &DAbstractFileWatcher::subfileCreated, this, [this](const DUrl &url) {
        auto fi = fileService->createFileInfo(nullptr, url);
        if (!fi->exists()) {
            return;
        }

        if (!Singleton<PathManager>::instance()->isVisiblePartitionPath(fi)) {
            return;
        }

        if (this->findItem(url) == -1) {
            auto r = std::upper_bound(devitems.begin(), devitems.end(), url,
            [](const DUrl & a, const DUrl & b) {
                DAbstractFileInfoPointer fia = fileService->createFileInfo(nullptr, a);
                DAbstractFileInfoPointer fib = fileService->createFileInfo(nullptr, b);
                return DFMRootFileInfo::typeCompare(fia, fib);
            });
            if (r == devitems.end()) {
                //DFileService::instance()->changeRootFile(url); //性能优化，注释
                this->addItem(DFMSideBarDeviceItemHandler::createItem(url), this->groupName(Device));
                devitems.append(url);
            } else {
                //DFileService::instance()->changeRootFile(url); //性能优化，注释
                this->insertItem(this->findLastItem(this->groupName(Device)) - (devitems.end() - r) + 1, DFMSideBarDeviceItemHandler::createItem(url), this->groupName(Device));
                devitems.insert(r, url);
            }
        }
    });
    connect(devicesWatcher, &DAbstractFileWatcher::fileDeleted, this, [this](const DUrl & url) {
        int index = findItem(url, groupName(Device));
        if (m_sidebarView->currentIndex().row() == index && index != -1) {
            index = findItem(DUrl::fromLocalFile(QDir::homePath()), groupName(Common));
            DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
            if (item) {
                QString identifierStr = item->registeredHandler(SIDEBAR_ID_INTERNAL_FALLBACK);
                QScopedPointer<DFMSideBarItemInterface> interface(DFMSideBarManager::instance()->createByIdentifier(identifierStr));
                if (interface) {
                    interface->cdAction(this, item);
                }
            }
        }
        //DFileService::instance()->changeRootFile(url,false); //性能优化，注释
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

        QList<DAbstractFileInfoPointer> bookmarkInfos = DFileService::instance()->getChildren(this, DUrl(BOOKMARK_ROOT),                                                         QStringList(), QDir::AllEntries);
        QList<DFMSideBarItem *> unsortedList;
        for (const DAbstractFileInfoPointer &info : bookmarkInfos) {
            unsortedList << DFMSideBarBookmarkItemHandler::createItem(info->fileUrl());
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
