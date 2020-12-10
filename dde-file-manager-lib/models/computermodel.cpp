/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
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

//fixed:CD display size error
#include "views/dfmopticalmediawidget.h"
#include "gvfs/gvfsmountmanager.h"

#include "singleton.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "controllers/dfmrootcontroller.h"
#include "controllers/vaultcontroller.h"
#include "controllers/pathmanager.h"
#include "dfmrootfileinfo.h"
#include "app/define.h"
#include "dfileservices.h"
#include "dabstractfileinfo.h"
#include "deviceinfoparser.h"
#include "drootfilemanager.h"

#include "views/computerview.h"
#include "shutil/fileutils.h"
#include "vault/vaulthelper.h"
#include "computermodel.h"

#include <QtConcurrent>

bool ComputerModel::m_isQueryRootFileFinshed = false;
ComputerModel::ComputerModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_diskm(new DDiskManager(this))
{
    m_diskm->setWatchChanges(true);
    par = qobject_cast<ComputerView*>(parent);
    m_nitems = 0;

    // 光驱事件
    connect(this, &ComputerModel::opticalChanged, this, &ComputerModel::onOpticalChanged, Qt::QueuedConnection);

#ifdef ENABLE_ASYNCINIT
    m_initThread.first = false;
    m_initThread.second = QtConcurrent::run([=](){
#endif
        addItem(makeSplitterUrl(tr("My Directories")));
        auto rootInit = [=](const QList<DAbstractFileInfoPointer> &ch){
            qDebug() << "show root file" << ch.size();
            bool splt = false;
            bool opticalchanged = false;
            for (auto chi : ch) {

#ifdef ENABLE_ASYNCINIT
                //线程退出
                if (m_initThread.first)
                    return;
#endif

                if (!Singleton<PathManager>::instance()->isVisiblePartitionPath(chi)) {
                    continue;
                }

                if (chi->suffix() != SUFFIX_USRDIR && !splt) {
                    addItem(makeSplitterUrl(tr("Disks")));
                    splt = true;
                }
                if (splt) {
                    auto r = std::upper_bound(m_items.begin() + findItem(makeSplitterUrl(tr("Disks"))) + 1, m_items.end(), chi,
                                              [](const DAbstractFileInfoPointer &a, const ComputerModelItemData &b) {
                                                  return DFMRootFileInfo::typeCompare(a, b.fi);
                                              });
                    if (r == m_items.end()) {
                        addItem(chi->fileUrl());
                    } else {
                        insertBefore(chi->fileUrl(), r->url);
                    }
                } else {
                    addItem(chi->fileUrl());
                }

                if (chi->fileUrl().path().contains("sr")) {
                    opticalchanged = true;
                }
            }

            if (!m_isQueryRootFileFinshed) {
                // 获取完根目录信息后刷新光驱信息，用于区分内外置光驱。
                if (opticalchanged) {
                    emit opticalChanged();
                }
                m_isQueryRootFileFinshed = true;
            }
        };

        connect(DRootFileManager::instance(),&DRootFileManager::queryRootFileFinsh,this,[this,rootInit](){
            QList<DAbstractFileInfoPointer> ch = rootFileManager->getRootFile();
            qDebug() << "DFileService::queryRootFileFinsh computer mode get " << ch.size();
            std::sort(ch.begin(), ch.end(), &DFMRootFileInfo::typeCompareByUrl);
            rootInit(ch);

            // 判断是否启用保险箱
            if ( VaultHelper::isVaultEnabled() ) {
                const DUrl vaultUrl = VaultController::makeVaultUrl();
                const DUrl vaultUrlTag = makeSplitterUrl(QObject::tr("File Vault"));
                const int vaultIndex = findItem(vaultUrl);
                //保险箱不是最后一个则移除，再添加，保证在最后一个
                if (vaultIndex != rowCount() - 1){
                    removeItem(vaultUrl);
                    removeItem(vaultUrlTag);
                }

                addItem(vaultUrlTag);
                addItem(vaultUrl);
            }
        });

        connect(DRootFileManager::instance(),&DRootFileManager::serviceHideSystemPartition,this,[this,rootInit](){
            m_items.clear();
            m_nitems = 0;
            addItem(makeSplitterUrl(tr("My Directories")));
            QList<DAbstractFileInfoPointer> ch = rootFileManager->getRootFile();
            qDebug() << "DFileService::queryRootFileFinsh computer mode get " << ch.size();
            std::sort(ch.begin(), ch.end(), &DFMRootFileInfo::typeCompareByUrl);
            rootInit(ch);

            if ( VaultHelper::isVaultEnabled() ) {
                // 保险柜
                addItem(makeSplitterUrl(QObject::tr("File Vault")));
                addItem(VaultController::makeVaultUrl());
            }
        });

        if (DRootFileManager::instance()->isRootFileInited()) {

            QList<DAbstractFileInfoPointer> ch = rootFileManager->getRootFile();
            qDebug() << "get root file now" << ch.size();
            std::sort(ch.begin(), ch.end(), &DFMRootFileInfo::typeCompareByUrl);
            rootInit(ch);

#ifdef ENABLE_ASYNCINIT
            //线程退出
            if (m_initThread.first)
                return;
#endif
            // 根据系统类型，判断是否启用保险柜
            if ( VaultHelper::isVaultEnabled() ) {
                // 保险柜
                addItem(makeSplitterUrl(QObject::tr("File Vault")));
                addItem(VaultController::makeVaultUrl());
            }
        }
        //使用分区工具，不显示磁盘问题，再刷一次
        DRootFileManager::instance()->startQuryRootFile();

        m_watcher = DRootFileManager::instance()->rootFileWather();
        connect(m_watcher, &DAbstractFileWatcher::fileDeleted, this, &ComputerModel::removeItem);
        connect(m_watcher, &DAbstractFileWatcher::subfileCreated, this, [this](const DUrl &url) {
            DAbstractFileInfoPointer fi = fileService->createFileInfo(this, url);
            if (!fi->exists()) {
                return;
            }

            if (!Singleton<PathManager>::instance()->isVisiblePartitionPath(fi)) {
                return;
            }

            int nIndex = findItem(makeSplitterUrl(QObject::tr("File Vault")));
            if(nIndex != -1){   // 有保险箱的情况
                //fix bug PPMS20200213,在没有磁盘项时，创建磁盘项
                int ndisksIndex = findItem(makeSplitterUrl(QObject::tr("Disks")));
                if (ndisksIndex == -1 ) {
                    insertBefore(makeSplitterUrl(tr("Disks")), m_items[nIndex].url);
                    nIndex++;
                }
                if(m_items.count() > nIndex){
                    insertBefore(url, m_items[nIndex].url);
                }
            }
            else {  // 没有保险箱的情况
                //fix bug PPMS20200213,在没有磁盘项时，创建磁盘项
                int ndisksIndex = findItem(makeSplitterUrl(QObject::tr("Disks")));
                if (ndisksIndex == -1 ) {
                    addItem(makeSplitterUrl(tr("Disks")));
                }
                auto r = std::upper_bound(m_items.begin() + findItem(makeSplitterUrl(tr("Disks"))) + 1, m_items.end(), fi,
                                          [](const DAbstractFileInfoPointer &a, const ComputerModelItemData &b) {
                    return DFMRootFileInfo::typeCompare(a, b.fi);
                });
                if (r == m_items.end()) {
                    addItem(url);
                } else {
                    insertBefore(url, r->url);
                }
            }

            if (url.path().contains("sr")) {
                emit opticalChanged();
            }
        });
        connect(m_watcher, &DAbstractFileWatcher::fileAttributeChanged, this,[this](const DUrl &url) {
            int p;
            for (p = 0; p < m_items.size(); ++p) {
                if (m_items[p].url == url) {
                    break;
                }
            }
            if (p >= m_items.size()) {
                return;
            }
            QModelIndex idx = index(p, 0);
            static_cast<DFMRootFileInfo*>(m_items[p].fi.data())->checkCache();
            emit dataChanged(idx, idx, {Qt::ItemDataRole::DisplayRole});
        });
#ifdef ENABLE_ASYNCINIT
    });
#endif
}

ComputerModel::~ComputerModel()
{
#ifdef ENABLE_ASYNCINIT
    m_initThread.first = true; //强制退出
    m_initThread.second.waitForFinished();
#endif
    rootFileManager->clearThread();
}

QModelIndex ComputerModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (row >= rowCount() || row < 0) {
        return QModelIndex();
    }
    return createIndex(row, column, const_cast<ComputerModelItemData*>(&m_items[row]));
}

QModelIndex ComputerModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int ComputerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.size();
}

int ComputerModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ComputerModel::data(const QModelIndex &index, int role) const
{
    if (static_cast<ComputerModelItemData*>(index.internalPointer())->cat == ComputerModelItemData::Category::cat_widget) {
        par->view()->setIndexWidget(index, static_cast<ComputerModelItemData*>(index.internalPointer())->widget);
    }

    const ComputerModelItemData *pitmdata = &m_items[index.row()];

    if (role == Qt::DisplayRole) {
        if (index.data(DataRoles::ICategoryRole) == ComputerModelItemData::Category::cat_splitter) {
            return pitmdata->sptext;
        }
        if (pitmdata->fi) {
            return pitmdata->fi->fileDisplayName();
        }
    }

    if (role == Qt::DecorationRole) {
        if (pitmdata->fi) {

            DFMRootFileInfo::ItemType itemType = static_cast<DFMRootFileInfo::ItemType>(pitmdata->fi->fileType());
            if (itemType == DFMRootFileInfo::UDisksOptical) {
                QString udisk = pitmdata->fi->extraProperties()["udisksblk"].toString();
                QStringList strList = udisk.split("/");
                QString device = strList.back();

                bool isInternal = DeviceInfoParser::Instance().isInternalDevice(device);
                if (!isInternal) {
                    return QIcon::fromTheme("media-external");
                }
            }

            return QIcon::fromTheme(pitmdata->fi->iconName());
        }
    }

    if (role == DataRoles::IconNameRole) {
        return pitmdata->fi ? pitmdata->fi->iconName() : QVariant();
    }

    if (role == DataRoles::FileSystemRole) {
        QString fs_type = "";
        if (pitmdata->fi) {
            //! 添加文件系统格式数据
            bool bMounted = pitmdata->fi->extraProperties()["mounted"].toBool();
            if (bMounted) {
                fs_type = pitmdata->fi->extraProperties()["fsType"].toString().toUpper();
            }
        }

        return fs_type;
    }

    if (role == DataRoles::SizeInUseRole) {
        if (pitmdata->fi) {
            //fixed:CD display size error
            if (static_cast<DFMRootFileInfo::ItemType>(pitmdata->fi->fileType()) == DFMRootFileInfo::ItemType::UDisksOptical) {
                DFMRootFileInfo *pFileInfo = dynamic_cast<DFMRootFileInfo *>(pitmdata->fi.data());
                QString strVolTag;
                if (pFileInfo)
                    strVolTag = pFileInfo->getVolTag();
                //fix: 探测光盘推进,弹出和挂载状态机标识
                bool bVolFlag = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].bVolFlag;
                bool bMntFlag = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].bMntFlag;
                if (!bVolFlag && !bMntFlag) { //CD/DVD
                    return 0;
                } else {
                    return DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nUsage;
                }
            } else {
                if (pitmdata->fi->fileUrl().scheme() == DFMVAULT_SCHEME) {
                    return QString::number(pitmdata->fi->size());
                }
                return pitmdata->fi->extraProperties()["fsUsed"];
            }
        }
    }

    if (role == DataRoles::SizeTotalRole) {
        if (pitmdata->fi) {
            //fixed:CD display size error
            if (static_cast<DFMRootFileInfo::ItemType>(pitmdata->fi->fileType()) == DFMRootFileInfo::ItemType::UDisksOptical) {
                DFMRootFileInfo *pFileInfo = dynamic_cast<DFMRootFileInfo *>(pitmdata->fi.data());
                QString strVolTag;
                if (pFileInfo)
                    strVolTag = pFileInfo->getVolTag();
                //fix: 探测光盘推进,弹出和挂载状态机标识
                bool bVolFlag = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].bVolFlag;
                bool bMntFlag = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].bMntFlag;
                if (!bVolFlag && !bMntFlag) { //CD/DVD
                    return 0;
                } else {
                    return DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nTotal;
                }
            } else {
                return pitmdata->fi->extraProperties()["fsSize"];
            }
        }
    }

    if (role == DataRoles::ICategoryRole) {
        return m_items.at(index.row()).cat;
    }

    if (role == DataRoles::OpenUrlRole) {
        if (pitmdata->fi) {
            // 用保险柜根目录
            if (pitmdata->fi->scheme() == DFMVAULT_SCHEME) {
                return QVariant::fromValue(pitmdata->fi->fileUrl());
            }
            return QVariant::fromValue(pitmdata->fi->redirectedFileUrl());
        }
    }

    if (role == DataRoles::MountOpenUrlRole) {
        if (pitmdata->fi) {
            QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(pitmdata->fi->extraProperties()["udisksblk"].toString()));
            if (pitmdata->fi->suffix() == SUFFIX_UDISKS && blkdev && blkdev->mountPoints().size() == 0) {
                blkdev->mount({});
            }
            return QVariant::fromValue(pitmdata->fi->redirectedFileUrl());
        }
    }

    if (role == DataRoles::ActionVectorRole) {
        if (pitmdata->fi) {
            return QVariant::fromValue(pitmdata->fi->menuActionList());
        }
    }

    if (role == DataRoles::DFMRootUrlRole) {
        if (pitmdata->fi) {
            return QVariant::fromValue(pitmdata->fi->fileUrl());
        }
    }

    if (role == DataRoles::VolumeTagRole) {
        DFMRootFileInfo *file = dynamic_cast<DFMRootFileInfo *>(pitmdata->fi.data());
        if (file)
            return file->getVolTag();
    }

    if (role == DataRoles::ProgressRole) {

        bool bProgressVisible = true;
        if (pitmdata->fi) {
            QString scheme = pitmdata->fi->fileUrl().scheme();
            if (scheme == DFMVAULT_SCHEME) {
                // vault not show progress.
                bProgressVisible = false;
            } else {
                // optical and removable device not show progress when unmounted.
                DFMRootFileInfo::ItemType itemType = static_cast<DFMRootFileInfo::ItemType>(pitmdata->fi->fileType());
                if (itemType == DFMRootFileInfo::ItemType::UDisksOptical
                        || itemType == DFMRootFileInfo::ItemType::UDisksRemovable) {

                    bProgressVisible = pitmdata->fi->extraProperties()["mounted"].toBool();
                }
            }
        }
        return QVariant::fromValue(bProgressVisible);
    }

    if (role == DataRoles::SizeRole) {

        bool bSizeVisible = true;
        if (pitmdata->fi) {
            // optical and removable device not show size when unmounted.
            DFMRootFileInfo::ItemType itemType = static_cast<DFMRootFileInfo::ItemType>(pitmdata->fi->fileType());
            if (itemType == DFMRootFileInfo::ItemType::UDisksOptical
                    || itemType == DFMRootFileInfo::ItemType::UDisksRemovable) {

                bSizeVisible = pitmdata->fi->extraProperties()["mounted"].toBool();

            } else if (pitmdata->url.isVaultFile()) {
                // not show vault size when locked or not created.
                VaultController::VaultState state = VaultController::ins()->state();
                if (state == VaultController::Encrypted
                        || state == VaultController::NotExisted) {
                    bSizeVisible = false;
                }
            }
        }

        return QVariant::fromValue(bSizeVisible);
    }

    if (role == DataRoles::SchemeRole) {
        if (pitmdata->fi) {
            return QVariant::fromValue(pitmdata->fi->fileUrl().scheme());
        }
    }
    if (role == DataRoles::DiscUUIDRole) {
        if (!pitmdata->fi)
            return QVariant();
        QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(pitmdata->fi->extraProperties()["udisksblk"].toString()));
        return blkdev->idUUID();
    }

    if (role == DataRoles::DiscOpticalRole) {
        if (!pitmdata->fi)
            return QVariant();
        QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(pitmdata->fi->extraProperties()["udisksblk"].toString()));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blkdev->drive()));
        return drv->opticalBlank();
    }

    return QVariant();
}

bool ComputerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        ComputerModelItemData *pitmdata = &m_items[index.row()];
        if (!pitmdata || !pitmdata->fi)
            return false;
        if (!pitmdata->fi->canRename()) {
            return false;
        }
        fileService->renameFile(this, pitmdata->fi->fileUrl(), DUrl(value.toString()));
        emit dataChanged(index, index, {Qt::DisplayRole});
        return true;
    }
    return false;
}

Qt::ItemFlags ComputerModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags ret = Qt::ItemFlag::ItemNeverHasChildren;
    if (index.data(DataRoles::ICategoryRole) != ComputerModelItemData::Category::cat_splitter) {
        ret |= Qt::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }

    const ComputerModelItemData *pitmdata = &m_items[index.row()];
    if (pitmdata->fi && pitmdata->fi->canRename()) {
        ret |= Qt::ItemFlag::ItemIsEditable;
    }
    return ret;
}

QModelIndex ComputerModel::findIndex(const DUrl &url) const
{
    for (int row = 0; row < m_items.size(); ++row) {
        if (m_items[row].url == url) {
            return index(row, 0);
        }
    }
    return QModelIndex();
}

int ComputerModel::itemCount() const
{
    return m_nitems;
}

void ComputerModel::addItem(const DUrl &url, QWidget* w)
{
    if (findItem(url) != -1) {
        return;
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);
    ComputerModelItemData id;
    initItemData(id, url, w);
    m_items.append(id);
    endInsertRows();
    if (url.scheme() != SPLITTER_SCHEME && url.scheme() != WIDGET_SCHEME) {
        Q_EMIT itemCountChanged(++m_nitems);
    }
}

void ComputerModel::insertAfter(const DUrl &url, const DUrl &ref, QWidget *w)
{
    if (findItem(url) != -1) {
        return;
    }

    int p = findItem(ref);
    if (p == -1) {
        return;
    }

    beginInsertRows(QModelIndex(), p + 1, p + 2);
    ComputerModelItemData id;
    initItemData(id, url, w);
    m_items.insert(p + 1, id);
    endInsertRows();
    if (url.scheme() != SPLITTER_SCHEME && url.scheme() != WIDGET_SCHEME) {
        Q_EMIT itemCountChanged(++m_nitems);
    }
}

void ComputerModel::insertBefore(const DUrl &url, const DUrl &ref, QWidget *w)
{
    if (findItem(url) != -1) {
        return;
    }

    int p = findItem(ref);
    if (p == -1) {
        return;
    }

    beginInsertRows(QModelIndex(), p, p + 1);
    ComputerModelItemData id;
    initItemData(id, url, w);
    m_items.insert(p, id);
    endInsertRows();
    if (url.scheme() != SPLITTER_SCHEME && url.scheme() != WIDGET_SCHEME) {
        Q_EMIT itemCountChanged(++m_nitems);
    }
}

void ComputerModel::removeItem(const DUrl &url)
{
    int p = findItem(url);
    if (p == -1) {
        return;
    }
    beginRemoveRows(QModelIndex(), p, p);
    m_items.removeAt(p);
    endRemoveRows();
    //fix bug PPMS20200213,在移除最后一个磁盘时，移除磁盘这个项
    int ndiskindex = findItem(makeSplitterUrl(QObject::tr("Disks")));
    int nextspliter = findNextSplitter(ndiskindex);
    //磁盘的分割线的ndiskindex和从磁盘到下一个分割线nextspliter，如果没有下一个分割线并且磁盘分割线ndiskindex是最后一项
    //或者磁盘分割线ndiskindex到从磁盘到下一个分割线nextspliter之间的item为0就移除磁盘的分割线
    if ((nextspliter == -1 && ndiskindex != -1 && m_items.size() - 1 == ndiskindex) ||
            (ndiskindex != -1 && nextspliter != -1 && (nextspliter - ndiskindex) == 1)) {
        beginRemoveRows(QModelIndex(), ndiskindex, ndiskindex);
        m_items.removeAt(ndiskindex);
        endRemoveRows();
    }

    if (url.scheme() != SPLITTER_SCHEME && url.scheme() != WIDGET_SCHEME) {
        Q_EMIT itemCountChanged(--m_nitems);
    }
}

void ComputerModel::onGetRootFile(const DAbstractFileInfoPointer &chi)
{
    if (!chi)
        return;
    bool splt = false;
    if(!chi->exists())
    {
        return;
    }
    if (chi->suffix() != SUFFIX_USRDIR && !splt) {
        addItem(makeSplitterUrl(tr("Disks")));
        splt = true;
    }
    if (splt) {
        auto r = std::upper_bound(m_items.begin() + findItem(makeSplitterUrl(tr("Disks"))) + 1, m_items.end(), chi,
                                  [](const DAbstractFileInfoPointer &a, const ComputerModelItemData &b) {
                                      return DFMRootFileInfo::typeCompare(a, b.fi);
                                  });
        if (r == m_items.end()) {
            addItem(chi->fileUrl());
        } else {
            insertBefore(chi->fileUrl(), r->url);
        }
    } else {
        addItem(chi->fileUrl());
    }
}

void ComputerModel::onOpticalChanged()
{
    std::thread thread([](){
        DeviceInfoParser::Instance().refreshDabase();
    });
    thread.detach();
}

void ComputerModel::getRootFile()
{
    QList<DAbstractFileInfoPointer> ch = rootFileManager->getRootFile();
    qDebug() << "获取 ComputerModel  getRootFile start" << ch.size() << QThread::currentThread() << qApp->thread();
    if (ch.isEmpty())
        return;

    std::sort(ch.begin(), ch.end(), &DFMRootFileInfo::typeCompareByUrl);
    bool splt = false;
    m_nitems = 0;
    for (auto chi : ch) {
        if (chi->suffix() != SUFFIX_USRDIR && !splt) {
            addItem(makeSplitterUrl(tr("Disks")));
            splt = true;
        }
        if (splt) {
            auto r = std::upper_bound(m_items.begin() + findItem(makeSplitterUrl(tr("Disks"))) + 1, m_items.end(), chi,
                                      [](const DAbstractFileInfoPointer &a, const ComputerModelItemData &b) {
                                          return DFMRootFileInfo::typeCompare(a, b.fi);
                                      });
            if (r == m_items.end()) {
                addItem(chi->fileUrl());
            } else {
                insertBefore(chi->fileUrl(), r->url);
            }
        } else {
            addItem(chi->fileUrl());
        }
    }
}

void ComputerModel::initItemData(ComputerModelItemData &data, const DUrl &url, QWidget *w)
{
    data.url = url;
    if (url.scheme() == SPLITTER_SCHEME) {
        data.cat = ComputerModelItemData::Category::cat_splitter;
        data.sptext = url.fragment();
    } else if (url.scheme() == WIDGET_SCHEME) {
        data.cat = ComputerModelItemData::Category::cat_widget;
        data.widget = w;
    } else {
        //这里不用去创建fileinfo，已经缓存了rootfileinfo //get root file info cache
        if (url.toString().endsWith(SUFFIX_GVFSMP))
        {
            const DAbstractFileInfoPointer &info = DRootFileManager::getFileInfo(url);
            if (info) {
                info->refresh();
                data.fi = info;
            }
            else {
                data.fi = fileService->createFileInfo(this, url);
            }
        }
        else {
            data.fi = fileService->createFileInfo(this, url);
        }
        if (data.fi->suffix() == SUFFIX_USRDIR) {
            data.cat = ComputerModelItemData::Category::cat_user_directory;
        } else {
            data.cat = ComputerModelItemData::Category::cat_internal_storage;
        }
    }
}

int ComputerModel::findItem(const DUrl &url)
{
    int p;
    for (p = 0; p < m_items.size(); ++p) {
        if (m_items[p].url == url) {
            break;
        }
    }
    if (p >= m_items.size()) {
        return -1;
    }
    return p;
}

DUrl ComputerModel::makeSplitterUrl(QString text)
{
    DUrl ret;
    ret.setScheme(SPLITTER_SCHEME);
    ret.setFragment(text);
    return ret;
}

int ComputerModel::findNextSplitter(const int &index)
{
    int p = index + 1;
    if (m_items.size() <= index || -1 == index) {
        return -1;
    }
    for (; p < m_items.size(); ++p) {
        if (m_items[p].url.scheme() == SPLITTER_SCHEME) {
            break;
        }
    }
    if (p >= m_items.size()) {
        return -1;
    }
    return p;
}

