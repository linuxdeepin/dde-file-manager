/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "gridmanager.h"

#include <QPoint>
#include <QRect>
#include <QDebug>

#include "apppresenter.h"
#include "dfileservices.h"
#include "../config/config.h"

class GridManagerPrivate
{
public:
    GridManagerPrivate()
    {
        auto settings = Config::instance()->settings();
        settings->beginGroup(Config::groupGeneral);
        positionProfile = settings->value(Config::keyProfile).toString();
        autoArrang = settings->value(Config::keyAutoAlign).toBool();
        settings->endGroup();

        coordWidth = 0;
        coordHeight = 0;
    }

    inline int cellCount() const
    {
        return  coordHeight * coordWidth;
    }

    inline void clear()
    {
        m_itemGrids.clear();
        m_gridItems.clear();
        m_overlapItems.clear();

        for (int i = 0; i < m_cellStatus.length(); ++i) {
            m_cellStatus[i] = false;
        }
    }

    QStringList rangeItems()
    {
        QStringList sortItems;

        auto inUsePos = m_gridItems.keys();
        qSort(inUsePos.begin(), inUsePos.end(), qQPointLessThanKey);

        for (int index = 0; index < inUsePos.length(); ++index) {
            auto gridPos = inUsePos.value(index);
            auto item = m_gridItems.value(gridPos);
            sortItems << item;
        }

        sortItems << m_overlapItems;
        return sortItems;
    }

    void arrange()
    {
        QStringList sortItems;

        auto inUsePos = m_gridItems.keys();
        qSort(inUsePos.begin(), inUsePos.end(), qQPointLessThanKey);

        for (int index = 0; index < inUsePos.length(); ++index) {
            auto gridPos = inUsePos.value(index);
            auto item = m_gridItems.value(gridPos);
            sortItems << item;
        }

        auto overlapItems = m_overlapItems;

        auto emptyCellCount = cellCount() - sortItems.length();
        for (int i = 0; i < emptyCellCount; ++i) {
            if (!overlapItems.isEmpty()) {
                sortItems << overlapItems.takeFirst();
            }
        }

        clear();

        for (auto item : sortItems) {
            QPoint empty_pos{ takeEmptyPos() };
            add(empty_pos, item);
        }

        m_overlapItems = overlapItems;
    }

    void createProfile()
    {
        m_cellStatus.resize(coordWidth * coordHeight);
        clear();
    }

    void loadProfile(const QList<DAbstractFileInfoPointer> &fileInfoList)
    {
        QMap<QString, int> existItems;

        for (const DAbstractFileInfoPointer &info : fileInfoList) {
            existItems.insert(info->fileUrl().toString(), 0);
        }

        auto settings = Config::instance()->settings();
        settings->beginGroup(positionProfile);
        for (auto &key : settings->allKeys()) {
            auto coords = key.split("_");
            auto x = coords.value(0).toInt();
            auto y = coords.value(1).toInt();
            auto item = settings->value(key).toString();
            if (existItems.contains(item)) {
                QPoint pos{x, y};
                add(pos, item);
                existItems.remove(item);
            }
        }
        settings->endGroup();

        if (autoArrang) {
            arrange();
        }

        for (auto &item : existItems.keys()) {
            QPoint empty_pos{ takeEmptyPos() };
            add(empty_pos, item);
        }
    }

    inline bool isValid(QPoint pos) const
    {
        QRect rect(0, 0, coordWidth, coordHeight);
        return rect.contains(pos);
    }

    inline QPoint overlapPos() const
    {
        return QPoint(coordWidth - 1, coordHeight - 1);
    }

    inline QPoint gridPosAt(int index) const
    {
        auto x = index / coordHeight;
        auto y = index % coordHeight;
        return QPoint(x, y);
    }

    inline int indexOfGridPos(const QPoint &pos) const
    {
        return pos.x() * coordHeight + pos.y();
    }

    inline QPoint emptyPos() const
    {
        for (int i = 0; i < m_cellStatus.size(); ++i) {
            if (!m_cellStatus[i]) {
                return gridPosAt(i);
            }
        }
        return overlapPos();
    }

    inline QPoint takeEmptyPos()
    {
        for (int i = 0; i < m_cellStatus.size(); ++i) {
            if (!m_cellStatus[i]) {
                m_cellStatus[i] = true;
                return gridPosAt(i);
            }
        }
        return overlapPos();
    }

    inline bool add(QPoint pos, const QString &itemId)
    {
        if (m_gridItems.contains(pos)) {
            if (pos != overlapPos()) {
                qCritical() << "add" << itemId  << "failed."
                            << pos << "grid exist item" << m_gridItems.value(pos);
                return false;
            } else {
                m_overlapItems << itemId;
                return false;
            }
        }


        m_gridItems.insert(pos, itemId);
        m_itemGrids.insert(itemId, pos);
        m_cellStatus[indexOfGridPos(pos)] = true;

        return true;
    }

    inline void syncProfile()
    {
        QStringList keyList;
        QVariantList valueList;
        for (auto pos : m_gridItems.keys()) {
            keyList << positionKey(pos);
            valueList << m_gridItems.value(pos);
        }

        if (m_gridItems.size() != m_itemGrids.size()) {
            qCritical() << "data sync failed";
            qCritical() << "-----------------------------";
            qCritical() << m_gridItems << m_itemGrids << keyList;
            qCritical() << "-----------------------------";
        }

        if (keyList.size() != m_gridItems.size()) {
            qCritical() << "data sync failed";
            qCritical() << "-----------------------------";
            qCritical() << m_gridItems << m_itemGrids << keyList;
            qCritical() << "-----------------------------";
        }

        emit Presenter::instance()->removeConfig(positionProfile, "");
        emit Presenter::instance()->setConfigList(positionProfile, keyList, valueList);
    }

    inline bool remove(QPoint pos, const QString &id)
    {
        m_overlapItems.removeAll(id);
        if (!m_itemGrids.contains(id)) {
            qDebug() << "can not remove" << pos << id;
            return false;
        }

        m_gridItems.remove(pos);
        m_itemGrids.remove(id);

        auto usageIndex = indexOfGridPos(pos);
        m_cellStatus[usageIndex] = false;

        if (!m_overlapItems.isEmpty()
                && (pos == overlapPos())) {
            auto itemId = m_overlapItems.takeFirst();
            add(pos, itemId);
        }
        return true;
    }

    inline void resetGridSize(int w, int h)
    {
        Q_ASSERT(!(coordHeight == h && coordWidth == w));
        coordWidth = w;
        coordHeight = h;

        createProfile();

        auto profile = QString("Position_%1x%2").arg(w).arg(h);
        if (profile != positionProfile) {
            positionProfile = profile;
        }
    }

    inline void changeGridSizeScared(int w, int h)
    {
        Q_ASSERT(!(coordHeight == h && coordWidth == w));

        qDebug() << "old grid size" << coordHeight << coordWidth;
        qDebug() << "new grid size" << w << h;

        auto oldCellCount = coordHeight * coordWidth;
        auto newCellCount = w * h;

        auto allItems = m_itemGrids;
        QVector<int> preferNewIndex;
        QVector<QString> itemIds;

        // record old pos index
        for (int i = 0; i < m_cellStatus.length(); ++i) {
            if (m_cellStatus.value(i)) {
                auto newIndex = i * newCellCount / oldCellCount;
                preferNewIndex.push_back(newIndex);
                itemIds.push_back(m_gridItems.value(gridPosAt(i)));
            }
        }

        coordHeight = h;
        coordWidth = w;

        createProfile();

        auto profile = QString("Position_%1x%2").arg(w).arg(h);
        if (profile != positionProfile) {
            positionProfile = profile;
        }

        for (int i = 0; i < preferNewIndex.length(); ++i) {
            auto index = preferNewIndex.value(i);
            if (m_cellStatus.size() > index && !m_cellStatus.value(index)) {
                QPoint pos{ gridPosAt(index) };
                add(pos, itemIds.value(i));
            } else {
                auto freePos = takeEmptyPos();
                add(freePos, itemIds.value(i));
            }
            allItems.remove(itemIds.value(i));
        }

        // move forward
        for (auto id : allItems.keys()) {
            auto freePos = takeEmptyPos();
            add(freePos, id);
        }
    }

    inline void changeGridSize(int w, int h)
    {
        Q_ASSERT(!(coordHeight == h && coordWidth == w));

//        qDebug() << "old grid size" << coordHeight << coordWidth;
//        qDebug() << "new grid size" << w << h;
        auto oldCellCount = coordHeight * coordWidth;
        auto newCellCount = w * h;

        auto allItems = m_itemGrids;

        auto outCellCount = 0;
        for (int i = 0; i < m_cellStatus.length(); ++i) {
            if (i >= newCellCount && m_cellStatus.value(i)) {
                outCellCount++;
            }
        }

        auto oldCellStatus = this->m_cellStatus;

        // find empty cell count
        auto indexEnd = qMin(oldCellCount, newCellCount);
        auto emptyCellCount = 0;
        for (int i = 0; i < indexEnd; ++i) {
            if (!oldCellStatus.value(i)) {
                emptyCellCount++;
            }
        }

        if (newCellCount > oldCellCount) {
            emptyCellCount += (newCellCount - oldCellCount);
        }

        if (emptyCellCount <= outCellCount + m_overlapItems.length()) {
//            qDebug() << "arrange";
            auto sortItems = rangeItems();
            resetGridSize(w, h);
            for (int i = 0; i < newCellCount; ++i) {
                add(takeEmptyPos(), sortItems.takeFirst());
            }
            m_overlapItems = sortItems;
        } else {
            // find start pos
            auto newEmptyCellCount = emptyCellCount - outCellCount + m_overlapItems.length();
            QVector<int> keepPosIndex;
            QVector<QString> keepItems;

            auto lastEmptyPosIndex = newCellCount;
            for (int i = 0; i < oldCellStatus.length(); ++i) {
                if (oldCellStatus.value(i)) {
                    keepPosIndex.push_back(i);
                    keepItems.push_back(m_gridItems.value(gridPosAt(i)));
                } else {
                    if (newEmptyCellCount <= 0) {
                        lastEmptyPosIndex = i;
                        break;
                    }
                    --newEmptyCellCount;
                }
            }

            QVector<int> nokeepPosIndex;
            QVector<QString> nokeepItems;
            for (int i = lastEmptyPosIndex; i < oldCellStatus.length(); ++i) {
                if (oldCellStatus.value(i)) {
                    nokeepPosIndex.push_back(i);
                    nokeepItems.push_back(m_gridItems.value(gridPosAt(i)));
                }
            }

            auto overlapItems = m_overlapItems;

            resetGridSize(w, h);

            for (int i = 0; i < keepPosIndex.length(); ++i) {
                auto index = keepPosIndex.value(i);
                if (m_cellStatus.size() > index && !m_cellStatus.value(index)) {
                    QPoint pos{ gridPosAt(index) };
                    add(pos, keepItems.value(i));
                }
            }

            for (int i = 0; i < nokeepItems.length(); ++i) {
                QPoint pos{ gridPosAt(lastEmptyPosIndex) };
                add(pos, nokeepItems.value(i));
                lastEmptyPosIndex++;
            }

            // TODO
            for (auto &item : overlapItems) {
                QPoint pos{ takeEmptyPos() };
                add(pos, item);
            }
        }
    }

    bool updateGridProfile(int w, int h)
    {
        if (coordHeight == h && coordWidth == w) {
            qWarning() << "profile not changed";
            return false;
        }

        if (0 == coordWidth && 0 == coordHeight) {
            resetGridSize(w, h);
            return false;
        } else {
            qDebug() << "change grid from" << coordWidth << coordHeight
                     << "to" << w << h;
            changeGridSize(w, h);

            QStringList keyList;
            QVariantList valueList;
            for (auto pos : m_gridItems.keys()) {
                keyList << positionKey(pos);
                valueList << m_gridItems.value(pos);
            }

            qDebug() << "updateGridProfile:" << keyList.size()
                     << m_gridItems.size() << m_itemGrids.size();

            emit Presenter::instance()->removeConfig(positionProfile, "");
            emit Presenter::instance()->setConfigList(positionProfile, keyList, valueList);

            return this->autoArrang;
        }
    }


    inline void setWhetherShowHiddenFiles(bool value)noexcept
    {
        m_whetherShowHiddenFiles.store(value, std::memory_order_release);
    }

    inline bool getWhetherShowHiddenFiles()noexcept
    {
        return m_whetherShowHiddenFiles.load(std::memory_order_consume);
    }

public:
    QStringList             m_overlapItems;
    QMap<QPoint, QString>   m_gridItems;
    QMap<QString, QPoint>   m_itemGrids;
    QVector<bool>           m_cellStatus;

    QString                 positionProfile;
    int                     coordWidth;
    int                     coordHeight;

    bool                    autoArrang;
    bool                    hasInited = false;

    std::atomic<bool>       m_whetherShowHiddenFiles{ false };
};

GridManager::GridManager(): d(new GridManagerPrivate)
{
}

GridManager::~GridManager()
{

}

bool GridManager::isInited() const
{
    return d->hasInited;
}

void GridManager::initProfile(const QList<DAbstractFileInfoPointer> &items)
{
    d->createProfile();
    d->loadProfile(items);
    d->hasInited = true;
}

bool GridManager::add(const QString &id)
{
#ifdef QT_DEBUG
    qDebug() << "show hidden files: " << d->getWhetherShowHiddenFiles();
#endif //QT_DEBUG

    DUrl url(id);

    if (!d->getWhetherShowHiddenFiles()) {
        const DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);

        if (info->isHidden()) {
            return false;
        }
    }

    if (d->m_itemGrids.contains(id)) {
//        qDebug() << "item exist item" << d->itemGrids.value(id) << id;
        return false;
    }

    QPoint pos{ d->takeEmptyPos() };
    return add(pos, id);
}

bool GridManager::add(QPoint pos, const QString &id)
{
//    qDebug() << "add" << pos << id;
    auto ret = d->add(pos, id);
    if (ret) {
        d->syncProfile();
    }

    return ret;
}

bool GridManager::move(const QStringList &selecteds, const QString &current, int x, int y)
{
    auto currentPos = d->m_itemGrids.value(current);
    auto destPos = QPoint(x, y);
    auto offset = destPos - currentPos;

    QList<QPoint> originPosList;
    QList<QPoint> destPosList;
    // check dest is empty;
    auto destPosMap = d->m_gridItems;
    auto destUsedGrids = d->m_cellStatus;
    for (auto &id : selecteds) {
        auto oldPos = d->m_itemGrids.value(id);
        originPosList << oldPos;
        destPosMap.remove(oldPos);
        destUsedGrids[d->indexOfGridPos(oldPos)] = false;
        auto destPos = oldPos + offset;
        destPosList << destPos;
    }

    bool conflict = false;
    for (auto pos : destPosList) {
        if (destPosMap.contains(pos) || !d->isValid(pos)) {
            conflict = true;
            break;
        }
    }

    // no need to resize
    if (conflict) {
        auto selectedHeadCount = selecteds.indexOf(current);
        // find free grid before destPos
        auto destIndex = d->indexOfGridPos(destPos);

        QList<int> emptyIndexList;

        for (int  i = 0; i < d->cellCount(); ++i) {
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
        for (int i = startIndex; i < d->cellCount(); ++i) {
            if (false == destUsedGrids.value(i)) {
                destPosList << d->gridPosAt(i);
            }
        }
    }

    for (int i = 0; i < selecteds.length(); ++i) {
        if (contains(selecteds.value(i))) {
            remove(selecteds.value(i));
        }
    }
    for (int i = 0; i < selecteds.length(); ++i) {
        QPoint point{ destPosList.value(i) };
        add(point, selecteds.value(i));
    }

    if (d->autoArrang) {
        reAlign();
    }

    return true;
}

bool GridManager::remove(const QString &id)
{
    auto pos = d->m_itemGrids.value(id);
    return remove(pos, id);
}

bool GridManager::remove(int x, int y, const QString &id)
{
    auto pos = QPoint(x, y);
    return remove(pos, id);
}

bool GridManager::remove(QPoint pos, const QString &id)
{
    auto ret = d->remove(pos, id);
    if (ret) {
        d->syncProfile();
    }
    return ret;
}

bool GridManager::clear()
{
    d->createProfile();

    emit Presenter::instance()->removeConfig(d->positionProfile, "");

    return true;
}

QString GridManager::firstItemId()
{
    for (int i = 0; i < d->m_cellStatus.length(); ++i) {
        if (d->m_cellStatus.value(i)) {
            auto pos = d->gridPosAt(i);
            return  itemId(pos);
        }
    }
    return "";
}

QString GridManager::lastItemId()
{
    auto len = d->m_cellStatus.length();
    for (int i = 0; i < len; ++i) {
        if (d->m_cellStatus.value(len - 1 - i)) {
            auto pos = d->gridPosAt(len - 1 - i);
            return  itemId(pos);
        }
    }
    return "";
}

QStringList GridManager::itemIds()
{
    QStringList ids;
    for (int i = 0; i < d->m_cellStatus.length(); ++i) {
        if (d->m_cellStatus.value(i)) {
            auto pos = d->gridPosAt(i);
            ids.append(itemId(pos));
        }
    }
    ids << d->m_overlapItems;
    return ids;
}

bool GridManager::contains(const QString &id)
{
    return d->m_itemGrids.contains(id) || d->m_overlapItems.contains(id);
}

QPoint GridManager::position(const QString &id)
{
    if (!d->m_itemGrids.contains(id)) {
        return d->overlapPos();
    }

    return d->m_itemGrids.value(id);
}

QString GridManager::itemId(int x, int y)
{
    return d->m_gridItems.value(QPoint(x, y));
}

QString GridManager::itemId(QPoint pos)
{
    return d->m_gridItems.value(pos);
}

bool GridManager::isEmpty(int x, int y)
{
    return !d->m_cellStatus.value(d->indexOfGridPos(QPoint(x, y)));
}

const QStringList &GridManager::overlapItems() const
{
    return d->m_overlapItems;
}

bool GridManager::autoAlign()
{
    return d->autoArrang;
}

void GridManager::toggleAlign()
{
    d->autoArrang = !d->autoArrang;

    if (!d->autoArrang) {
        return;
    }

    reAlign();
}


void GridManager::reAlign()
{
    d->arrange();

    QStringList keyList;
    QVariantList valueList;
    for (auto pos : d->m_gridItems.keys()) {
        keyList << positionKey(pos);
        valueList << d->m_gridItems.value(pos);
    }

    emit Presenter::instance()->removeConfig(d->positionProfile, "");
    emit Presenter::instance()->setConfigList(d->positionProfile, keyList, valueList);
}

QPoint GridManager::forwardFindEmpty(QPoint start) const
{
    auto size = gridSize();
    for (int j = start.y(); j < size.height(); ++j) {
        if (GridManager::instance()->isEmpty(start.x(), j)) {
            return QPoint(start.x(), j);
        }
    }

    for (int i = start.x() + 1; i < size.width(); ++i) {
        for (int j = 0; j < size.height(); ++j) {
            if (GridManager::instance()->isEmpty(i, j)) {
                return QPoint(i, j);
            }
        }
    }
    return d->emptyPos();
}

QSize GridManager::gridSize() const
{
    return QSize(d->coordWidth, d->coordHeight);
}

void GridManager::updateGridSize(int w, int h)
{
    if (d->updateGridProfile(w, h)) {
        d->arrange();
    }

    emit Presenter::instance()->setConfig(Config::groupGeneral,
                                          Config::keyProfile,
                                          d->positionProfile);
}

GridCore *GridManager::core()
{
    auto core = new GridCore;
    core->overlapItems = d->m_overlapItems;
    core->gridItems = d->m_gridItems;
    core->itemGrids = d->m_itemGrids;
    core->gridStatus = d->m_cellStatus;
    core->coordWidth = d->coordWidth;
    core->coordHeight = d->coordHeight;
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

void GridManager::dump()
{
    for (auto key : d->m_gridItems.keys()) {
        qDebug() << key << d->m_gridItems.value(key);
    }

    for (auto key : d->m_itemGrids.keys()) {
        qDebug() << key << d->m_itemGrids.value(key);
    }
}

