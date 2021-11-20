/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#include "gridmanager.h"

#include <QPoint>
#include <QRect>
#include <QDebug>
#include <QStandardPaths>
#include <QTimer>

#include <dgiosettings.h>
#include <dfilesystemmodel.h>
#include <dfmapplication.h>
#include "apppresenter.h"
#include "dfileservices.h"
#include "../config/config.h"
#include "dabstractfileinfo.h"
#include "screen/screenhelper.h"
#include "interfaces/private/mergeddesktop_common_p.h"


class GridManagerPrivate
{
public:
    GridManagerPrivate()
    {
        //新需求gsetting控制桌面部分文件
        m_desktopSettings = new DGioSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
        QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
        DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);
        QDir dir(desktopUrl.toString());
        m_gsettings << dir.filePath("dde-home.desktop")
                    << dir.filePath("dde-trash.desktop")
                    << dir.filePath("dde-computer.desktop");

        autoArrange = Config::instance()->getConfig(Config::groupGeneral, Config::keyAutoAlign).toBool();
#ifdef ENABLE_AUTOMERGE  //sp2需求调整，屏蔽自动整理
        autoMerge = Config::instance()->getConfig(Config::groupGeneral, Config::keyAutoMerge, false).toBool();
#else
        //取消自动整理
        autoMerge = Config::instance()->getConfig(Config::groupGeneral, Config::keyAutoMerge, false).toBool();
        //若用户开启了自动整理，则强制退出
        if (autoMerge) {
            autoMerge = false;
            Config::instance()->setConfig(Config::groupGeneral, Config::keyAutoMerge, false);
        }
#endif
//        auto settings = Config::instance()->settings();
//        settings->beginGroup(Config::groupGeneral);
//        autoArrange = settings->value(Config::keyAutoAlign).toBool();
//        autoMerge = settings->value(Config::keyAutoMerge, false).toBool();
//        settings->endGroup();
    }

    GridManagerPrivate(GridManagerPrivate &) = delete;
    GridManagerPrivate &operator=(GridManagerPrivate &) = delete;

    ~GridManagerPrivate()
    {
        if (m_desktopSettings) {
            delete m_desktopSettings;
            m_desktopSettings = nullptr;
        }
    }
    inline int cellCount(int screenNum) const
    {
        auto coordInfo = screensCoordInfo.value(screenNum);
        return  coordInfo.second * coordInfo.first;
    }

    void clear()
    {
        m_itemGrids.clear();
        m_gridItems.clear();
        m_overlapItems.clear();
        m_cellStatus.clear();

        for (int i : screenCode()) {

            //add empty m_itemGrids
            QMap<QPoint, QString> gridItem;
            m_gridItems.insert(i, gridItem);

            //add empty m_gridItems
            QMap<QString, QPoint> itemGrid;
            m_itemGrids.insert(i, itemGrid);

            m_screenFullStatus.insert(i, false);
        }

        auto coordkeys = screensCoordInfo.keys();
        for (auto key : coordkeys) {
            //add empty coordInfo
            QVector<bool> tempVector;
            tempVector.resize(cellCount(key));
            m_cellStatus.insert(key, tempVector);
        }
        auto statusValues = m_cellStatus.values();
        for (auto &oneValue : statusValues) {
            for (int i = 0; i < oneValue.length(); ++i) {
                oneValue[i] = false;
            }
        }
    }

    QStringList rangeItems(int screenNum)
    {
        QStringList sortItems;

        auto inUsePos = m_gridItems.value(screenNum).keys();
        qSort(inUsePos.begin(), inUsePos.end(), qQPointLessThanKey);

        for (int index = 0; index < inUsePos.length(); ++index) {
            auto gridPos = inUsePos.value(index);
            auto item = m_gridItems.value(screenNum).value(gridPos);
            sortItems << item;
        }

        sortItems << m_overlapItems;
        return sortItems;
    }

    QStringList rangeItems(const int screenNum, const QStringList itemList)
    {
        QStringList sortItems;
        QList<QPoint> itemPosList;
        QStringList unknownItemList;
        foreach (auto item, itemList) {
            if (m_itemGrids.contains(screenNum) && m_itemGrids.value(screenNum).contains(item)) {
                itemPosList.append(m_itemGrids.value(screenNum).value(item));
            } else {
                unknownItemList.append(item);
            }
        }

        qSort(itemPosList.begin(), itemPosList.end(), qQPointLessThanKey);

        for (int index = 0; index < itemPosList.length(); ++index) {
            auto gridPos = itemPosList.value(index);
            auto item = m_gridItems.value(screenNum).value(gridPos);
            sortItems << item;
        }
        sortItems << unknownItemList;
        return sortItems;
    }

    void autoMergeItems(QStringList items)
    {
        arrange(items);
    }

    void arrange(QStringList sortedItems)
    {
        //过滤gsetting中配置的文件
        for (auto &one : m_gsettings) {
            //todo:如果开放自动整理需要转化成真是路径做处理
            DUrl tempUrl(one);
            if (!GridManager::instance()->desktopFileShow(tempUrl, true))
                sortedItems.removeOne(one);
        }

        auto screenOrder = screenCode();
        QTime t;
        t.start();
        qDebug() << "screen count" << screenOrder.size();
        for (int screenNum : screenOrder) {
            qDebug() << "arrange Num" << screenNum << sortedItems.size();
            QMap<QPoint, QString> gridItems;
            QMap<QString, QPoint> itemGrids;
            if (!sortedItems.empty()) {
                auto cellStatus = m_cellStatus.value(screenNum);
                int coordHeight = screensCoordInfo.value(screenNum).second;
                int i = 0;
                for (; i < cellStatus.size() && !sortedItems.empty(); ++i) {
                    QString item = sortedItems.takeFirst();
                    QPoint pos(i / coordHeight, i % coordHeight);
                    //cellStatus[i] = true;
                    gridItems.insert(pos, item);
                    itemGrids.insert(item, pos);
                    setCellStatus(screenNum, i, true);
                }
                qDebug() << "screen" << screenNum << "put item:" << i << "cell" << cellStatus.size();
            }
            m_gridItems.insert(screenNum, gridItems);
            m_itemGrids.insert(screenNum, itemGrids);
        }
        qDebug() << "time " << t.elapsed() << "(ms) overlapItems " << sortedItems.size();
        m_overlapItems = sortedItems;
    }

    void createProfile()
    {
        if (0 == createProfileTime)
            clear();
        createProfileTime++;
    }

    void loadProfile(const QStringList &orderedItems, QHash<QString, bool> existItems)
    {
        QMap<int, QString> screenNumProfiles;

        if (m_bSingleMode) {
            screenNumProfiles.insert(1, QString("SingleScreen"));
        } else {
            readProfiles();
            screenNumProfiles = positionProfiles;
        }

        //获取个屏幕分组信息对应的图标信息
        QMap<int, QStringList> moreIcon;
        QMutexLocker lk(Config::instance()->mutex());
        auto settings = Config::instance()->settings();
        for (int &screenKey : screenNumProfiles.keys()) {
            settings->beginGroup(screenNumProfiles.value(screenKey));
            for (auto &key : settings->allKeys()) {
                auto coords = key.split("_");
                auto x = coords.value(0).toInt();
                auto y = coords.value(1).toInt();
                QString item = settings->value(key).toString();
                if (existItems.contains(item)) {
                    QPoint pos{x, y};
                    if (!screenCode().contains(screenKey) || m_screenFullStatus.value(screenKey) || !isValid(screenKey, pos)) {
                        if (moreIcon.contains(screenKey)) {
                            moreIcon.find(screenKey)->append(item);
                        } else {
                            moreIcon.insert(screenKey, QStringList() << item);
                        }
                    } else {
                        if (!add(screenKey, pos, item)) {
                            continue;
                        }
                    }
                    existItems.remove(item);
                }
            }
            settings->endGroup();
        }
        lk.unlock();

        for (int key : moreIcon.keys()) {
            foreach (QString item, moreIcon.value(key)) {
                QPair<int, QPoint> emptyPos = getEmptyPos(key);
                add(emptyPos.first, emptyPos.second, item);
            }
        }

        for (const QString &item : orderedItems) {
            //已经有位子的，跳过
            if (!existItems.contains(item))
                continue;

            QPair<int, QPoint> empty_pos{ takeEmptyPos() };
            add(empty_pos.first, empty_pos.second, item);
        }
    }

    void syncAllProfile()
    {
        if (allItems().isEmpty())
            return;

        if (m_bSingleMode) {
            syncProfile(1);
        } else {
            foreach (int screenNum, positionProfiles.keys()) {
                syncProfile(screenNum);
            }

            updateProfiles();
        }
    }

    void readProfiles()
    {
        positionProfiles.clear();
        QMutexLocker lk(Config::instance()->mutex());
        auto settings = Config::instance()->settings();
        settings->beginGroup(Config::keyProfile);
        for (QString &key : settings->allKeys()) {
            positionProfiles.insert(key.toInt(), settings->value(key).toString());
        }
        settings->endGroup();
    }

    void updateProfiles()
    {
        positionProfiles.clear();
        QStringList keys;
        QVariantList values;

        QList<int> screens = screenCode();
        foreach (int index, screens) {
            positionProfiles.insert(index, QString("Screen_%1").arg(index));
            keys.append(QString::fromStdString(std::to_string(index)));
            values.append(QString("Screen_%1").arg(index));
        }

        Config::instance()->removeConfig(Config::keyProfile, "");
        Config::instance()->setConfigList(Config::keyProfile, keys, values);
    }

    inline bool isValid(int screenNum, QPoint pos) const
    {
        auto coordInfo = screensCoordInfo.value(screenNum);
        return  coordInfo.first > pos.x() && coordInfo.second > pos.y()
                && pos.x() >= 0 && pos.y() >= 0;
    }

    inline QPoint overlapPos(int screenNum) const
    {
        auto coordInfo = screensCoordInfo.value(screenNum);
        return QPoint(coordInfo.first - 1, coordInfo.second - 1);
    }

    inline QPoint gridPosAt(int screenNum, int index) const
    {
        auto coordHeight = screensCoordInfo.value(screenNum).second;
        auto x = index / coordHeight;
        auto y = index % coordHeight;
        return QPoint(x, y);
    }

    inline int indexOfGridPos(int screenNum, const QPoint &pos) const
    {
        //return pos.x() * coordHeight + pos.y();
        auto oneScreenCoord = screensCoordInfo.value(screenNum);
        if (0 == oneScreenCoord.first)
            return 0;
        else {
            return pos.x() * oneScreenCoord.second + pos.y();
        }
    }

    QPair<int, QPoint> takeEmptyPos()
    {
        //返回空位屏以及编号
        QPair<int, QPoint> posPair;
        //if(-1 != emptyScreenNum){
        for (int emptyScreenNum : screenCode()) { //todo 优化效率
            auto cellStatus = m_cellStatus.value(emptyScreenNum);
            for (int i = 0; i < cellStatus.size(); ++i) {
                if (!cellStatus[i]) {
                    //cellStatus[i] = true;
                    posPair.first = emptyScreenNum;
                    posPair.second = gridPosAt(emptyScreenNum, i);
                    return posPair;
                }
            }
        }

        if (!m_cellStatus.isEmpty()) {
            posPair.first = screenCode().last();
            posPair.second = overlapPos(posPair.first);
        }
        return  posPair;
    }

    QPair<int, QPoint> getEmptyPos(int screenNum)
    {
        QPair<int, QPoint> emptyPosPair;
        if (screenNum <= 0) {
            return emptyPosPair;
        }

        if (getEmptyPos(screenNum, false, emptyPosPair.second)) {
            emptyPosPair.first = screenNum;
            return emptyPosPair;
        }

        for (int screenIndex = screenNum - 1; screenIndex > 0; --screenIndex) {
            if (getEmptyPos(screenIndex, true, emptyPosPair.second)) {
                emptyPosPair.first = screenIndex;
                return emptyPosPair;
            }
        }

        for (int screenIndex = screenNum + 1; screenIndex <= screenCode().size(); ++screenIndex) {
            if (getEmptyPos(screenIndex, false, emptyPosPair.second)) {
                emptyPosPair.first = screenIndex;
                return emptyPosPair;
            }
        }

        if (!m_cellStatus.isEmpty()) {
            emptyPosPair.first = screenCode().last();
            emptyPosPair.second = overlapPos(screenCode().last());
        }
        return  emptyPosPair;
    }

    bool getEmptyPos(int screenNum, bool isRightTop, QPoint &resultPos)
    {
        if (!m_screenFullStatus.contains(screenNum) || m_screenFullStatus.value(screenNum)
                || !m_cellStatus.contains(screenNum) || !screensCoordInfo.contains(screenNum)) {
            return  false;
        }

        auto cellStatus = m_cellStatus.value(screenNum);

        if (isRightTop) {
            QPair<int, int> screenSize = screensCoordInfo.value(screenNum);
            for (int xIndex = screenSize.first - 1; xIndex >= 0; --xIndex) {
                for (int yIndex = 0; yIndex < screenSize.second; ++yIndex) {
                    int rightTopIndex = xIndex * screenSize.second + yIndex;
                    if (rightTopIndex < cellStatus.size() && !cellStatus[rightTopIndex]) {
                        resultPos = QPoint(xIndex, yIndex);
                        return  true;
                    }
                }
            }
        } else {
            for (int index = 0; index < cellStatus.size(); ++index) {
                if (!cellStatus[index]) {
                    resultPos = gridPosAt(screenNum, index);
                    return true;
                }
            }
        }

        return  false;
    }

    bool setCellStatus(int screenNum, int index, bool state)
    {
        if (!m_cellStatus.contains(screenNum))
            return false;

        if (m_cellStatus.value(screenNum).length() <= index || index < 0) {
            return false;
        }

        auto cellStatusItor = m_cellStatus.find(screenNum);
        cellStatusItor.value()[index] = state;

        updateScreenFullStatus(screenNum);
        return true;
    }

    void updateScreenFullStatus(int screenNum)
    {
        auto cellStatus = m_cellStatus.value(screenNum);
        for (int index = 0; index < cellStatus.size(); ++index) {
            if (!cellStatus[index]) {
                m_screenFullStatus.find(screenNum).value() = false;
                return;
            }
        }

        m_screenFullStatus.find(screenNum).value() = true;
    }

    bool add(int screenNum, QPoint pos, const QString &itemId)
    {
        if (itemId.isEmpty()) {
            qCritical() << "add empty item"; // QVector<QString>.value() may retruen an empty QString
            return false;
        } else if (m_itemGrids.value(screenNum).contains(itemId)) {
            qCritical() << "add" << itemId  << "failed."
                        << m_itemGrids.value(screenNum).value(itemId) << "grid exist item";
            return false;
        }

        if (m_gridItems.value(screenNum).contains(pos)) {
            if (pos != overlapPos(screenNum)) {
                qCritical() << "add" << itemId  << "failed."
                            << pos << "grid exist item in screenNun " << screenNum << "-" << m_gridItems.value(screenNum).value(pos);
                return false;
            } else {
                if (!m_overlapItems.contains(itemId)) {
                    m_overlapItems << itemId;
                    return true;
                }
                return false;
            }
        }

        if (!isValid(screenNum, pos)) {
            return false;
        }

        if (m_gridItems.end() != m_gridItems.find(screenNum)) {
            m_gridItems.find(screenNum)->insert(pos, itemId);
        }
        if (m_itemGrids.end() != m_itemGrids.find(screenNum)) {
            m_itemGrids.find(screenNum)->insert(itemId, pos);
        }

        int index = indexOfGridPos(screenNum, pos);
        return setCellStatus(screenNum, index, true);
    }

    QPair<QStringList, QVariantList> generateProfileConfigVariable(int screenNum)
    {
        //根据屏幕编号获取对应屏幕图标信息
        QStringList keyList;
        QVariantList valueList;
        QMap<QPoint, QString> onceScreenGridItems = m_gridItems.value(screenNum);
        for (auto pos : onceScreenGridItems.keys()) {
            keyList << positionKey(pos);
            valueList << onceScreenGridItems.value(pos);
        }

        return QPair<QStringList, QVariantList>(keyList, valueList);
    }

    void syncProfile(int screenNum)
    {
        QPair<QStringList, QVariantList> kvList = generateProfileConfigVariable(screenNum);
        //positionProfile需要调整一下，添加上屏幕编号 :Position_%1_%2x%3
        QString screenPositionProfile;
        if (m_bSingleMode) {
            screenPositionProfile = QString("SingleScreen");//单一桌面模式和扩展桌面模式需要独立保存桌面项目位置配置文件，以便两种模式互相切换时仍然完好如初
        } else {
            screenPositionProfile = QString("Screen_%1").arg(screenNum);
        }

        Config::instance()->removeConfig(screenPositionProfile, "");
        Config::instance()->setConfigList(screenPositionProfile, kvList.first, kvList.second);
    }

    bool remove(int screenNum, QPoint pos, const QString &id)
    {
        //m_overlapItems 重叠items
        m_overlapItems.removeAll(id);
        if (!m_itemGrids.value(screenNum).contains(id)) {
            qDebug() << "can not remove" << pos << id;
            return false;
        }

        auto tempGridItemsItor = m_gridItems.find(screenNum);
        auto tempItemGridsItor = m_itemGrids.find(screenNum);
        tempGridItemsItor->remove(pos);
        tempItemGridsItor->remove(id);

        auto usageIndex = indexOfGridPos(screenNum, pos);

        if (!m_cellStatus.contains(screenNum))
            return false;

        setCellStatus(screenNum, usageIndex, false);
        return true;
    }

    void resetGridSize(int screenNum, int w, int h)
    {
        if (!screensCoordInfo.contains(screenNum))
            return;

        auto coordInfo = screensCoordInfo.find(screenNum);
        Q_ASSERT(!(coordInfo.value().second == h && coordInfo.value().first == w));
        coordInfo.value().first = w;
        coordInfo.value().second = h;

        createProfile();
    }

    inline void setWhetherShowHiddenFiles(bool value)noexcept
    {
        m_whetherShowHiddenFiles.store(value, std::memory_order_release);
    }

    inline bool getWhetherShowHiddenFiles()noexcept
    {
        return m_whetherShowHiddenFiles.load(std::memory_order_consume);
    }

    inline QList<int> screenCode() const
    {
        QList<int> screenOrder = screensCoordInfo.keys();
        qSort(screenOrder.begin(), screenOrder.end());
        return screenOrder;
    }

    //必须在screensCoordInfo改变前获取，即resetGridSize前获取
    QStringList allItems() const
    {
        QStringList items;
        auto screens = screenCode();
        for (int num : screens) {
            auto cells = m_cellStatus.value(num);
            auto gridItems = m_gridItems.value(num);
            for (int i = 0; i < cells.size(); ++i) {
                if (cells.value(i)) {
                    QPoint pos = gridPosAt(num, i);
                    const QString &item = gridItems.value(pos);
                    items << item;
                }
            }
        }
        items << m_overlapItems;
        return items;
    }

    void reAutoArrage()
    {
        QStringList items = allItems();
        clear();
        arrange(items);
    }
public:
    QStringList                                         m_overlapItems;
    QList<DAbstractFileInfoPointer>                     m_allItems;
    QMap<int, QMap<QPoint, QString>>                    m_gridItems;
    QMap<int, QMap<QString, QPoint>>                    m_itemGrids;
    //newer
    QMap<int, bool>                                     m_screenFullStatus;//屏幕图标状态
    QMap<int, QVector<bool>>                            m_cellStatus;//<screenNum, QVector<bool>>
    QMap<int, QString>           positionProfiles;
    QMap<int, QPair<int, int>>                           screensCoordInfo; //<screenNum,<coordWidth,coordHeight>>
    bool                                                autoArrange;
    bool                                                autoMerge = false;
    int                                                 createProfileTime{0};
    std::atomic<bool>                                   m_whetherShowHiddenFiles{ false };
    bool                                                m_bSingleMode = true;
    bool                                                m_doneInit{false};
    DUrl                                                m_currentVirtualUrl{""};//保留当前屏幕上的扩展情况，用于插拔屏和分辨率变化等重新刷新图标位置
    DGioSettings                                        *m_desktopSettings{nullptr};
    QStringList                                         m_gsettings;
};

GridManager::GridManager(): d(new GridManagerPrivate)
{
    bool showHidden = DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedHiddenFiles).toBool();
    setWhetherShowHiddenFiles(showHidden);
    //gsetting配置发生变化
    connect(d->m_desktopSettings, &DGioSettings::valueChanged, this, [this](const QString & key, const QVariant & value) {
        Q_UNUSED(value)
        QStringList desktopIconKey{"desktop-computer", "desktop-trash", "desktop-home-directory"};
        if (desktopIconKey.contains(key))
            emit this->sigSyncOperation(soGsettingUpdate);
    });
}

GridManager::~GridManager()
{

}

DUrl GridManager::getInitRootUrl()
{
    setAutoMerge(d->autoMerge);
    if (d->autoMerge) {
        //刷新虚拟路径时是先查看是否已有展开状态,即使是初始化也得先检查
        DUrl virtualExpandUrl = GridManager::instance()->getCurrentVirtualExpandUrl();
        if (virtualExpandUrl.fragment().isEmpty()) {
            return DUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER);
        } else {
            return virtualExpandUrl;
        }

    } else {
        // sa
        QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
        DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

        if (!QDir(desktopPath).exists()) {
            QDir::home().mkpath(desktopPath);
        }

        return desktopUrl;
    }
}

void GridManager::initGridItemsInfos()
{
    DUrl fileUrl = getInitRootUrl();
    QScopedPointer<DFileSystemModel> tempModel(new DFileSystemModel(nullptr));

    //设置是否显示隐藏文件
    auto filters = tempModel->filters();
    filters = GridManager::instance()->getWhetherShowHiddenFiles() ?
              filters | QDir::Hidden : filters & ~QDir::Hidden;
    tempModel->setFilters(filters);
    qDebug() << "desktop init filters " << filters;

    QList<DAbstractFileInfoPointer> infoList = DFileService::instance()->getChildren(this, fileUrl, QStringList(), tempModel->filters());
    d->clear();

    //设置排序
    DFileSystemModel::Roles sortRole = static_cast<DFileSystemModel::Roles>(Config::instance()->getConfig(Config::groupGeneral, Config::keySortBy, DFileSystemModel::FileMimeTypeRole).toInt());
    Qt::SortOrder sortOrder = Config::instance()->getConfig(Config::groupGeneral, Config::keySortOrder).toInt() == Qt::AscendingOrder ?
                              Qt::AscendingOrder : Qt::DescendingOrder;

    qDebug() << "sort rules" << sortRole << sortOrder;
    {
        QPoint sort(sortRole, sortOrder);
        emit sigSyncOperation(soSort, sort);
    }

    if (!d->m_doneInit) {
        d->m_doneInit = true;
        //todo：考虑用此变量做刷新时的自动整理优化的，不太理想，后续优化看能否有更好的方式
        //d->m_allItems = infoList;
    }
    if (GridManager::instance()->autoMerge()) {
        GridManager::instance()->initAutoMerge(infoList);
    }
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    else if (GridManager::instance()->autoArrange()) {
        QModelIndex index = tempModel->setRootUrl(fileUrl);
        DAbstractFileInfoPointer root = tempModel->fileInfo(index);
        QTime t;
        if (root != nullptr) {
            DAbstractFileInfo::CompareFunction sortFun = root->compareFunByColumn(sortRole);
            qDebug() << "DAbstractFileInfo::CompareFunction " << (sortFun != nullptr);
            t.start();
            if (sortFun) {
                qSort(infoList.begin(), infoList.end(), [sortFun, sortOrder](const DAbstractFileInfoPointer & node1, const DAbstractFileInfoPointer & node2) {
                    return sortFun(node1, node2, sortOrder);
                });
            }
            qDebug() << "sort complete time " << t.elapsed();
        }
        QStringList list;
        for (const DAbstractFileInfoPointer &df : infoList) {
            list << df->fileUrl().toString();
        }
        qDebug() << "sorted desktop items num" << list.size() << " time " << t.elapsed();
        initArrage(list);
    }
#endif
    else {
        QModelIndex index = tempModel->setRootUrl(fileUrl);
        DAbstractFileInfoPointer root = tempModel->fileInfo(index);
        QTime t;
        if (root != nullptr) {
            DAbstractFileInfo::CompareFunction sortFun = root->compareFunByColumn(sortRole);
            qDebug() << "DAbstractFileInfo::CompareFunction " << (sortFun != nullptr);
            t.start();
            if (sortFun) {
                qSort(infoList.begin(), infoList.end(), [sortFun, sortOrder](const DAbstractFileInfoPointer & node1, const DAbstractFileInfoPointer & node2) {
                    return sortFun(node1, node2, sortOrder);
                });
            }
            qDebug() << "sort complete time " << t.elapsed();
        }

        //顺序
        QStringList list;
        //加载配置文件位置信息，此加载应当加载所有，通过add来将不同屏幕图标信息加载到m_gridItems和m_itemGrids
        QHash<QString, bool> indexHash;

        for (const DAbstractFileInfoPointer &df : infoList) {
            auto path = df->fileUrl();
            if (!GridManager::instance()->desktopFileShow(path, true))
                continue;
            list << path.toString();
            indexHash.insert(path.toString(), false);
        }
        sortMainDesktopFile(list, sortRole, sortOrder); //按类型排序的特殊处理
        qDebug() << "sorted desktop items num" << list.size() << " time " << t.elapsed();

        //初始化Profile,用实际地址的文件去匹配图标位置（自动整理则图标顺延展开，自定义则按照配置文件对应顺序）
        d->createProfile();
        d->loadProfile(list, indexHash);
#ifndef USE_SP2_AUTOARRAGE
        if (GridManager::instance()->autoArrange()) {
            d->arrange(d->allItems());
        }
#endif
        delaySyncAllProfile();
    }
    //fixbug81490 栅格数据更新完成后，需要更新扩展显示图标的区域
    emit sigSyncOperation(soExpandItemUpdate);
}

void GridManager::initAutoMerge(const QList<DAbstractFileInfoPointer> &items)
{
    d->clear();
    QStringList list;
    for (const DAbstractFileInfoPointer &df : items) {
        list << df->fileUrl().toString();
    }
    d->autoMergeItems(list);
    emit sigSyncOperation(soUpdate);
}


void GridManager::initArrage(const QStringList &items)
{
    d->clear();
    d->arrange(items);
    if (m_needRenameItem.isEmpty()) //经过排序后，需要打开重命名
        emit sigSyncOperation(soUpdate);
    else {
        emit sigSyncOperation(soUpdate);
        emit sigSyncOperation(soRename, m_needRenameItem);
        m_needRenameItem.clear(); //处理后清除
    }
}

void GridManager::initCustom(const QStringList &orderedItems, const QHash<QString, bool> &indexHash)
{
    clear();
    d->loadProfile(orderedItems, indexHash);
    delaySyncAllProfile();
}

void GridManager::initCustom(QStringList &items)
{
    for (auto &one : d->m_gsettings) {
        DUrl tempUrl(one);
        if (!GridManager::instance()->desktopFileShow(tempUrl, true))
            items.removeOne(one);
    }

    QHash<QString, bool> indexHash;
    for (const QString &item : items) {
        indexHash.insert(item, false);
    }

    initCustom(items, indexHash);
}

bool GridManager::add(int screenNum, const QString &id)
{
    Q_UNUSED(screenNum)
    DUrl tempUrl(id);
    if (!GridManager::instance()->desktopFileShow(tempUrl, true))
        return true;
    for (int screenNum : d->screenCode()) {
        if (d->m_itemGrids.value(screenNum).contains(id)) {
            qWarning() << "item exist item" << screenNum << id;
            return false;
        }
    }

    QPair<int, QPoint> posPair{ d->takeEmptyPos() };
    return add(posPair.first, posPair.second, id);
}

bool GridManager::add(int screenNum, QPoint pos, const QString &id)
{
    qDebug() << "add" << pos << id;
    auto ret = d->add(screenNum, pos, id);
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    if (ret && !autoMerge() && !autoArrange()) {
#else
    if (ret && !autoMerge()) {
#endif
        d->syncProfile(screenNum);
    }

    return ret;
}
bool GridManager::move(int screenNum, const QStringList &selecteds, const QString &current, int x, int y)
{
    auto currentPos = d->m_itemGrids.value(screenNum).value(current);
    auto destPos = QPoint(x, y);
    auto offset = destPos - currentPos;

    QList<QPoint> originPosList;
    QList<QPoint> destPosList;
    // check dest is empty;
    auto destPosMap = d->m_gridItems[screenNum];
    auto destUsedGrids = d->m_cellStatus[screenNum];
    for (auto &id : selecteds) {
        auto oldPos = d->m_itemGrids.value(screenNum).value(id);
        originPosList << oldPos;
        destPosMap.remove(oldPos);
        destUsedGrids[d->indexOfGridPos(screenNum, oldPos)] = false;
        auto tempDestPos = oldPos + offset;
        destPosList << tempDestPos;
    }

    bool conflict = false;
    for (auto pos : destPosList) {
        if (destPosMap.contains(pos) || !d->isValid(screenNum, pos)) {
            conflict = true;
            break;
        }
    }

    // no need to resize
    if (conflict) {
        auto selectedHeadCount = selecteds.indexOf(current);
        // find free grid before destPos
        auto destIndex = d->indexOfGridPos(screenNum, destPos);

        QList<int> emptyIndexList;

        for (int  i = 0; i < d->cellCount(screenNum); ++i) {
            if (false == destUsedGrids.value(i)) {
                emptyIndexList << i;
            }
        }
        auto destGridHeadCount = emptyIndexList.indexOf(destIndex);

        Q_ASSERT(emptyIndexList.length() >= selecteds.length());

        auto startIndex = emptyIndexList.indexOf(destIndex) - selectedHeadCount;
        if (destGridHeadCount < selectedHeadCount) {
            startIndex = 0;
        }
        auto destTailCount = emptyIndexList.length() - destGridHeadCount;
        auto selectedTailCount = selecteds.length() - selectedHeadCount;
        if (destTailCount <= selectedTailCount) {
            startIndex = emptyIndexList.length() - selecteds.length();
        }

        destPosList.clear();

        startIndex = emptyIndexList.value(startIndex);
        for (int i = startIndex; i < d->cellCount(screenNum); ++i) {
            if (false == destUsedGrids.value(i)) {
                destPosList << d->gridPosAt(screenNum, i);
            }
        }
    }

    for (int i = 0; i < selecteds.length(); ++i) {
        if (contains(screenNum, selecteds.value(i))) {
            remove(screenNum, selecteds.value(i));
        }
    }
    for (int i = 0; i < selecteds.length(); ++i) {
        QPoint point{ destPosList.value(i) };
        bool ret = add(screenNum, point, selecteds.value(i));
        if (!ret) {
            d->m_overlapItems << selecteds.value(i);
//            auto fPos = forwardFindEmpty(screenNum,point);
//            add(fPos.first, fPos.second, selecteds.value(i));
        }
    }

    return true;
}

bool GridManager::move(int fromScreen, int toScreen, const QStringList &selectedIds, const QString &itemId, int x, int y)
{
    QPoint currentPos = d->m_itemGrids.value(fromScreen).value(itemId);
    QPoint destPos = QPoint(x, y);
    QPoint offset = destPos - currentPos;

    QList<QPoint> originPosList;
    QList<QPoint> destPosList;
    // check dest is empty;
    QMap<QPoint, QString> &destPosMap = d->m_gridItems[toScreen];

    //源
    QMap<QPoint, QString> &orgPosMap = d->m_gridItems[fromScreen];
    QMap<QString, QPoint> &orgItemMap = d->m_itemGrids[fromScreen];
    QVector<bool> &orgUsedGrids = d->m_cellStatus[fromScreen];

    QStringList overflowItemList;
    QStringList sortItems = d->rangeItems(fromScreen, selectedIds);
    //移除源
    for (const QString &id : sortItems) {
        QPoint oldPos = orgItemMap.value(id);
        originPosList << oldPos;
        orgPosMap.remove(oldPos);
        orgItemMap.remove(id);
        orgUsedGrids[d->indexOfGridPos(fromScreen, oldPos)] = false;

        auto tempDestPos = oldPos + offset;
        destPosList << tempDestPos;
    }

    bool conflict = false;
    for (auto pos : destPosList) {
        if (destPosMap.contains(pos) || !d->isValid(toScreen, pos)) {
            conflict = true;
            break;
        }
    }

    // no need to resize
    if (conflict) {
        QVector<bool> &destUsedGrids = d->m_cellStatus[toScreen];
        auto selectedHeadCount = sortItems.indexOf(itemId);
        // find free grid before destPos
        auto destIndex = d->indexOfGridPos(toScreen, destPos);

        QList<int> emptyIndexList;

        for (int  i = 0; i < d->cellCount(toScreen); ++i) {
            if (false == destUsedGrids.value(i)) {
                emptyIndexList << i;
            }
        }
        auto destGridHeadCount = emptyIndexList.indexOf(destIndex);

        if (emptyIndexList.length() < sortItems.length()) {
            int overflowCnt = sortItems.length() - emptyIndexList.length();
            for (int index = 0; index < overflowCnt; ++index) {
                overflowItemList.append(sortItems.takeLast());
            }
        }

        Q_ASSERT(emptyIndexList.length() >= sortItems.length());
        auto startIndex = emptyIndexList.indexOf(destIndex) - selectedHeadCount;
        if (destGridHeadCount < selectedHeadCount) {
            startIndex = 0;
        }
        auto destTailCount = emptyIndexList.length() - destGridHeadCount;
        auto selectedTailCount = sortItems.length() - selectedHeadCount;
        if (destTailCount <= selectedTailCount) {
            startIndex = emptyIndexList.length() - sortItems.length();
        }

        destPosList.clear();

        startIndex = emptyIndexList.value(startIndex);
        for (int i = startIndex; i < d->cellCount(toScreen); ++i) {
            if (false == destUsedGrids.value(i)) {
                destPosList << d->gridPosAt(toScreen, i);
            }
        }
    }

    for (int i = 0; i < sortItems.length(); ++i) {
        if (contains(toScreen, sortItems.value(i))) {
            remove(toScreen, sortItems.value(i));
        }
    }

    for (int i = 0; i < sortItems.length(); ++i) {
        QPoint point{ destPosList.value(i) };
        add(toScreen, point, sortItems.value(i));
    }

    for (int i = 0; i < overflowItemList.length(); ++i) {
        add(toScreen, overflowItemList.value(i));
    }


    //保存源屏配置
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    if (!autoMerge() && !autoArrange()) {
#else
    if (!autoMerge()) {
#endif
        d->syncProfile(fromScreen);
    }

    emit sigSyncOperation(soHideEditing);
    return true;
}


bool GridManager::remove(int screenNum, const QString &id)
{
    if (d->m_itemGrids.value(screenNum).contains(id)) {
        auto pos = d->m_itemGrids.value(screenNum).value(id);
        bool ret = remove(screenNum, pos, id);
        qDebug() << screenNum << id  << pos << ret;
        return ret;
    } else if (d->m_overlapItems.contains(id)) {
        d->m_overlapItems.removeAll(id);
        qDebug() << screenNum << id  << "overlapItems" << true;
        return true;
    }
    return false;
}

void GridManager::popOverlap()
{
    if (!d->m_overlapItems.isEmpty()) {
        auto itemId = d->m_overlapItems.takeFirst();
        auto pos = d->takeEmptyPos();
        add(pos.first, pos.second, itemId);
    }
}

int GridManager::addToOverlap(const QString &itemId)
{
    if (d->m_overlapItems.contains(itemId))
        return 1;

    for (auto coor : d->m_itemGrids.values()) {
        if (coor.contains(itemId))
            return -1;
    }

    d->m_overlapItems << itemId;
    return 0;
}

int GridManager::emptyPostionCount(int screenNum) const
{
    return d->m_cellStatus.value(screenNum).size() - d->m_gridItems.value(screenNum).size();
}

bool GridManager::remove(int screenNum, QPoint pos, const QString &id)
{
    auto ret = d->remove(screenNum, pos, id);
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    if (ret && !(autoMerge() || autoArrange())) {
#else
    if (ret && !autoMerge()) {
#endif
        d->syncProfile(screenNum);
    }
    return ret;
}

bool GridManager::clear()
{
    d->clear();
    return true;
}

void GridManager::restCoord()
{
    d->screensCoordInfo.clear();
    d->m_gridItems.clear();
    d->m_itemGrids.clear();
    d->m_cellStatus.clear();
    d->m_overlapItems.clear();
    d->m_screenFullStatus.clear();
}

void GridManager::addCoord(int screenNum, QPair<int, int> coordInfo)
{
    qDebug() << "add screen" << screenNum << coordInfo;
    //todo：若在add的时候存在了对应的num，考虑要不要直接更新value，而不是return
    if (d->screensCoordInfo.contains(screenNum))
        return;
    //初始化栅格
    d->screensCoordInfo.insert(screenNum, coordInfo);
    if (!d->m_gridItems.contains(screenNum)) {
        QMap<QPoint, QString> gridItem;
        d->m_gridItems.insert(screenNum, gridItem);
    }
    if (!d->m_itemGrids.contains(screenNum)) {
        QMap<QString, QPoint> itemGrid;
        d->m_itemGrids.insert(screenNum, itemGrid);
    }
//    if(!d->m_cellStatus.contains(screenNum)){
//        QVector<bool> cellStatus;
//        cellStatus.resize(coordInfo.first * coordInfo.second);
//        d->m_cellStatus.insert(screenNum, cellStatus);
//    }
}

QString GridManager::firstItemId(int screenNum)
{
    for (int i = 0; i < d->m_cellStatus.value(screenNum).length(); ++i) {
        if (d->m_cellStatus.value(screenNum).value(i)) {
            auto pos = d->gridPosAt(screenNum, i);
            return  itemId(screenNum, pos);
        }
    }
    return "";
}

QString GridManager::lastItemId(int screenNum)
{
    auto len = d->m_cellStatus.value(screenNum).length();
    for (int i = 0; i < len; ++i) {
        if (d->m_cellStatus.value(screenNum).value(len - 1 - i)) {
            auto pos = d->gridPosAt(screenNum, len - 1 - i);
            return itemId(screenNum, pos);
        }
    }
    return "";
}

QString GridManager::lastItemTop(int screenNum)
{
    auto len = d->m_cellStatus.value(screenNum).length();
    for (int i = 0; i < len; ++i) {
        if (d->m_cellStatus.value(screenNum).value(len - 1 - i)) {
            auto pos = d->gridPosAt(screenNum, len - 1 - i);
            return itemTop(screenNum, pos);
        }
    }
    return "";
}

QStringList GridManager::itemIds(int screenNum)
{
    QStringList ids;
    for (int i = 0; i < d->m_cellStatus.value(screenNum).length(); ++i) {
        if (d->m_cellStatus.value(screenNum).value(i)) {
            auto pos = d->gridPosAt(screenNum, i);
            ids.append(itemId(screenNum, pos));
        }
    }
    if (screenNum == d->screenCode().last())
        ids << d->m_overlapItems;
    return ids;
}

QStringList GridManager::allItems() const
{
    return d->allItems();
}

bool GridManager::contains(int screebNum, const QString &id)
{
    return d->m_itemGrids.value(screebNum).contains(id) ||
           (d->screenCode().last() == screebNum && d->m_overlapItems.contains(id));
}

QPoint GridManager::position(int screenNum, const QString &id)
{
    if (!d->m_itemGrids.value(screenNum).contains(id)) {
        return d->overlapPos(screenNum);
    }

    return d->m_itemGrids.value(screenNum).value(id);
}

bool GridManager::find(const QString &itemId, QPair<int, QPoint> &pos)
{
    bool ret = false;
    auto screenOrder = d->screenCode();
    auto iter = std::find_if(screenOrder.begin(), screenOrder.end(), [this, itemId](const int &screen) {
        return contains(screen, itemId);
    });

    if (iter != screenOrder.end()) {
        pos.first = *iter;
        pos.second = position(*iter, itemId);
        ret = true;
    }

    return ret;
}

QString GridManager::itemId(int screenNum, int x, int y)
{
    return d->m_gridItems.value(screenNum).value(QPoint(x, y));
}

QString GridManager::itemId(int screenNum, QPoint pos)
{
    return d->m_gridItems.value(screenNum).value(pos);
}

QString GridManager::itemTop(int screenNum, int x, int y)
{
    return itemTop(screenNum, QPoint(x, y));
}

QString GridManager::itemTop(int screenNum, QPoint pos)
{
    if (screenNum == d->screenCode().last()
            && pos == d->overlapPos(screenNum)
            && !d->m_overlapItems.isEmpty()) {
        return d->m_overlapItems.last();
    }

    return itemId(screenNum, pos);
}

bool GridManager::isEmpty(int screenNum, int x, int y)
{
    auto cellStatus = d->m_cellStatus.value(screenNum);
    int pos = d->indexOfGridPos(screenNum, QPoint(x, y));
    if (pos >= cellStatus.size() || pos < 0)
        return false;

    return !cellStatus.value(pos);
}

QStringList GridManager::overlapItems(int screen) const
{
    if (!d->screenCode().empty() && screen == d->screenCode().last())
        return d->m_overlapItems;

    return QStringList();
}

bool GridManager::shouldArrange() const
{
    return d->autoArrange || d->autoMerge;
}

bool GridManager::autoArrange() const
{
    return d->autoArrange;
}

bool GridManager::autoMerge() const
{
    return d->autoMerge;
}

bool GridManager::doneInit() const
{
    return d->m_doneInit;
}

void GridManager::toggleArrange()
{
    d->autoArrange = !d->autoArrange;
}

void GridManager::setAutoMerge(bool enable)
{
    if (d->autoMerge == enable) return;
    d->autoMerge = enable;
}

void GridManager::toggleAutoMerge()
{
    setAutoMerge(!d->autoMerge);
}

void GridManager::reArrange()
{
    if (shouldArrange()) {
        d->reAutoArrage();
        emit sigSyncOperation(soUpdate);
        return;
    }
}

int GridManager::gridCount() const
{
    auto screens = d->screensCoordInfo.values();
    int totalCount = std::accumulate(screens.begin(), screens.end(), 0, [](int total, const QPair<int, int> &coordInfo) {
        return total += coordInfo.first * coordInfo.second;
    });

    return  totalCount;
}

int GridManager::gridCount(int screenNum) const
{
    int totalCount = 0;
    if (d->screensCoordInfo.contains(screenNum)) {
        auto coordInfo = d->screensCoordInfo.value(screenNum);
        totalCount = coordInfo.first * coordInfo.second;
    }
    return totalCount;
}

QPair<int, QPoint> GridManager::forwardFindEmpty(int screenNum, QPoint start) const
{
    QPair<int, QPoint> emptyPos;
    auto size = gridSize(screenNum);
    for (int j = start.y(); j < size.height(); ++j) {
        if (GridManager::instance()->isEmpty(screenNum, start.x(), j)) {
            emptyPos.first = screenNum;
            emptyPos.second = QPoint(start.x(), j);
            return emptyPos;
        }
    }

    for (int i = start.x() + 1; i < size.width(); ++i) {
        for (int j = 0; j < size.height(); ++j) {
            if (GridManager::instance()->isEmpty(screenNum, i, j)) {
                emptyPos.first = screenNum;
                emptyPos.second = QPoint(i, j);
                return emptyPos;
            }
        }
    }
    return d->takeEmptyPos();
}

QList<int> GridManager::allScreenNum()
{
    return d->screensCoordInfo.keys();
}

QSize GridManager::gridSize(int screenNum) const
{
    auto coordInfo = d->screensCoordInfo.value(screenNum);
    return QSize(coordInfo.first, coordInfo.second);
}

void GridManager::updateGridSize(int screenNum, int w, int h)
{
    auto coordInfo = d->screensCoordInfo.value(screenNum);
    if (coordInfo.second == h && coordInfo.first == w) {
        return;
    }

    if (0 == coordInfo.first && 0 == coordInfo.second) {
        d->resetGridSize(screenNum, w, h);
        return;
    }

    QStringList items = d->allItems(); //必须在resetGridSize前获取
    d->resetGridSize(screenNum, w, h);

    if (shouldArrange()) {
        d->clear();
        d->arrange(items);
#ifndef USE_SP2_AUTOARRAGE   //sp3需求改动
        if (autoArrange()) {
            d->syncAllProfile();
        }
#endif
    } else {
        initCustom(items);
    }
    emit sigSyncOperation(soUpdate);
}

GridCore *GridManager::core()
{
    auto core = new GridCore;
    core->overlapItems = d->m_overlapItems;
    core->gridItems = d->m_gridItems;
    core->itemGrids = d->m_itemGrids;
    core->m_cellStatus = d->m_cellStatus;
    core->screensCoordInfo = d->screensCoordInfo;
    return core;
}

void GridManager::setWhetherShowHiddenFiles(bool value) noexcept
{
    d->setWhetherShowHiddenFiles(value);
}

bool GridManager::getWhetherShowHiddenFiles() noexcept
{
    return d->getWhetherShowHiddenFiles();
}

bool GridManager::getCanvasFullStatus(int screenId)
{
    return d->m_screenFullStatus.value(screenId);
}

void GridManager::dump()
{
    for (auto key : d->m_gridItems.keys()) {
        qDebug() << key << d->m_gridItems.value(key);
    }

    for (auto key : d->m_itemGrids.keys()) {
        qDebug() << key << d->m_itemGrids.value(key);
    }
}

void GridManager::sortMainDesktopFile(QStringList &list, int role, Qt::SortOrder order)
{
    if (role != DFileSystemModel::FileMimeTypeRole)
        return;
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);
    QDir dir(desktopUrl.toString());

    QList<QPair<QString, bool>> mainDesktop = {{dir.filePath("dde-home.desktop"), false},
        {dir.filePath("dde-trash.desktop"), false},
        {dir.filePath("dde-computer.desktop"), false}
    };
    for (auto it = mainDesktop.begin(); it != mainDesktop.end(); ++it) {
        if (list.removeOne(it->first)) {
            it->second = true;
        }
    }

    for (auto it = mainDesktop.begin(); it != mainDesktop.end(); ++it) {
        if (it->second) {
            //升序
            if (order == Qt::AscendingOrder) {
                list.push_front(it->first);
            }//降序
            else {
                list.push_back(it->first);
            }
        }
    }
}

void GridManager::setDisplayMode(bool single)
{
    d->m_bSingleMode = single;
}

void GridManager::delaySyncAllProfile(int ms)
{
    static QTimer *syncTimer = nullptr;
    qDebug() << "delaySyncAllProfile" << QThread::currentThread() << qApp->thread();
    if (syncTimer != nullptr) {
        qDebug() << "reset timer" << syncTimer;
        syncTimer->stop();
        delete syncTimer;
        syncTimer = nullptr;
    }
    if (ms < 1) {
        d->syncAllProfile();
    }

    syncTimer = new QTimer;
    syncTimer->setSingleShot(true);
    connect(syncTimer, &QTimer::timeout, this, [ = ]() {
        syncTimer->stop();
        d->syncAllProfile();
    });
    syncTimer->start(ms);
}

void GridManager::setCurrentVirtualExpandUrl(const DUrl url)
{
    d->m_currentVirtualUrl = url;
}

DUrl GridManager::getCurrentVirtualExpandUrl()
{
    return d->m_currentVirtualUrl;
}

void GridManager::setCurrentAllItems(const QList<DAbstractFileInfoPointer> &infoList)
{
    d->m_allItems = infoList;
}

QVariant GridManager::isGsettingShow(const QString &targetkey, const QVariant defaultValue)
{
    if (!d->m_desktopSettings->keys().contains(targetkey))
        return defaultValue;
    QVariant tempValue = d->m_desktopSettings->value(targetkey);
    if (!tempValue.isValid())
        return defaultValue;
    return tempValue.toBool();
}

bool GridManager::desktopFileShow(const DUrl &url, const bool defaultValue)
{
    if (!url.isValid())
        return defaultValue;
    if (0 == url.fileName().localeAwareCompare("dde-computer.desktop")) {
        auto tempComputer = isGsettingShow("desktop-computer", QVariant());
        if (!tempComputer.isValid())
            return defaultValue;
        return tempComputer.toBool();
    }
    if (0 == url.fileName().localeAwareCompare("dde-trash.desktop")) {
        auto tempTrash = isGsettingShow("desktop-trash", QVariant());
        if (!tempTrash.isValid())
            return defaultValue;
        return tempTrash.toBool();
    }
    if (0 == url.fileName().localeAwareCompare("dde-home.desktop")) {
        auto tempHome = isGsettingShow("desktop-home-directory", QVariant());
        if (!tempHome.isValid())
            return defaultValue;
        return tempHome.toBool();
    }
    return defaultValue;
}

