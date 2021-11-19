/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "defaultfiletreater.h"
#include "config.h"
#include "private/defaultcanvasgridmanager_p.h"

#include "dfm-base/base/schemefactory.h"

#include <QMutexLocker>
#include <QDebug>
#include <QPoint>
#include <QHash>

uint qHash(const QPoint &key, uint seed)
{
    QString val = QString("%1x%2").arg(key.x()).arg(key.y());
    return qHash<QString>(val, seed);
}

DSB_D_BEGIN_NAMESPACE

DefaultCanvasGridManagerPrivate::DefaultCanvasGridManagerPrivate()
{
}

void DefaultCanvasGridManagerPrivate::readProfileGroup()
{
    screenProfiles.clear();
    QMutexLocker lk(Config::instance()->mutex());
    auto settings = Config::instance()->settings();
    settings->beginGroup(Config::kKeyProfile);
    for (QString &key : settings->allKeys()) {
        screenProfiles.insert(key.toInt(), settings->value(key).toString());
    }
    settings->endGroup();
}

void DefaultCanvasGridManagerPrivate::writeProfileGroup()
{
    screenProfiles.clear();
    QStringList keys;
    QVariantList values;

    auto screens = screenCode();
    foreach (int index, screens) {
        screenProfiles.insert(index, QString("Screen_%1").arg(index));
        keys.append(QString::fromStdString(std::to_string(index)));
        values.append(QString("Screen_%1").arg(index));
    }

    Config::instance()->removeConfig(Config::kKeyProfile, "");
    Config::instance()->setConfigList(Config::kKeyProfile, keys, values);
}

QPair<int, QPoint> DefaultCanvasGridManagerPrivate::takeEmptyPos(const int screenNum)
{
    QPair<int, QPoint> posPair;
    if (cellStatus.contains(screenNum)) {
        auto tempCellStatus = cellStatus.value(screenNum);
        int index = tempCellStatus.indexOf(false);
        if (-1 != index) {
            posPair.first = screenNum;
            posPair.second = gridPosAt(screenNum, index);
            return posPair;
        }
    }

    for (int emptyScreenNum : screenCode()) {
        auto tempCellStatus = cellStatus.value(emptyScreenNum);
        int index = tempCellStatus.indexOf(false);
        if (-1 != index) {
            posPair.first = emptyScreenNum;
            posPair.second = gridPosAt(emptyScreenNum, index);
            return posPair;
        }
    }

    posPair.first = screenCode().last();
    posPair.second = overlapPos();
    return posPair;
}

bool DefaultCanvasGridManagerPrivate::setCellStatus(const int screenNum, const int index, bool state)
{
    if (!cellStatus.contains(screenNum)) {
        return false;
    }

    if (cellStatus.value(screenNum).count() <= index || index < 0) {
        return false;
    }

    cellStatus[screenNum][index] = state;
    return true;
}

DefaultCanvasGridManagerPrivate::~DefaultCanvasGridManagerPrivate()
{
}

/*!
 * \brief GridManagerPrivate::loadProfile，加载配置文件数据，加载时会根据真实文件信息对配置文件进行过滤，
 * 过配置文件中存在但真实文件中已被删除的文件，新增配置文件中不存在的文件，这里的设计是：配置文件中是1357,若新增2468则根据
 * 排序规则追加，而不是插入。即：最终结果是135792468而不是123456789
 * \param orderedItems 需要处理的的文件列表，用于解析文件后，配置文件中不存在的项去查找新位置（该项可能比existItems少）
 * \param existItems 实际存在的文件哈西表，用于解析文件时，判断该文件实际是否存在（使用QHash而非QList是为了提升性能）
 */
void DefaultCanvasGridManagerPrivate::loadProfile(const QList<DFMDesktopFileInfoPointer> &orderedItems, QHash<DFMDesktopFileInfoPointer, bool> existItems)
{
    if (screenCode().isEmpty()) {
        qWarning() << "not found screen,give up load profile!!!";
        return;
    }

    QHash<int, QString> screenNumProfiles;
    if (singleScreen) {
        screenNumProfiles.insert(1, QString("SingleScreen"));
    } else {
        readProfileGroup();
        screenNumProfiles = screenProfiles;
    }

    // 获取每个屏幕分组信息对应的图标信息
    QMap<int, QList<DFMDesktopFileInfoPointer>> moreIcon;
    QMutexLocker lk(Config::instance()->mutex());
    auto settings = Config::instance()->settings();
    for (int &screenKey : screenNumProfiles.keys()) {
        settings->beginGroup(screenNumProfiles.value(screenKey));
        auto ttt = settings->allKeys();
        for (auto &key : settings->allKeys()) {
            auto coords = key.split("_");
            auto x = coords.value(0).toInt();
            auto y = coords.value(1).toInt();
            QString item = settings->value(key).toString();
            item = UrlRoute::urlToPath(item);
            auto routeU = UrlRoute::pathToReal(item);
            QString errString;
            auto itemInfo = InfoFactory::create<DefaultDesktopFileInfo>(routeU, &errString);
            if (!itemInfo)
                qInfo() << "loadProfile error: " << errString;
            if (existItems.contains(itemInfo)) {
                QPoint pos { x, y };
                if (!screenCode().contains(screenKey) || !isValid(screenKey, pos)) {
                    // 读取的文件实际存在，但是配置的位置不合法，则后面重新查找空位
                    moreIcon[screenKey].append(itemInfo);
                } else {
                    // 将文件添加到指定位置
                    if (!add(screenKey, pos, itemInfo)) {
                        continue;
                    }
                }
                existItems.remove(itemInfo);
            }
        }
        settings->endGroup();
    }
    lk.unlock();

    // 优先处理文件位置有记录但是不合法的项
    for (int key : moreIcon.keys()) {
        foreach (auto item, moreIcon.value(key)) {
            // 优先在原记录的屏幕上添加,失败则自动查找空位
            add(key, item);
        }
    }

    // 再处理没有记录文件位置的项
    for (auto &item : orderedItems) {
        // 已经处理过的项跳过
        if (!existItems.contains(item))
            continue;

        // 优先依据屏幕顺序查找空位
        add(screenCode().first(), item);
    }
}

void DefaultCanvasGridManagerPrivate::syncAllProfile()
{
}

void DefaultCanvasGridManagerPrivate::syncProfile(const int screenNum)
{
    Q_UNUSED(screenNum)
}

/*!
 * \brief GridManagerPrivate::autoArrange,自动排列，自动排列功能仅排列收拢已排序数据
 * \param sortedItems，所有排序数据
 */
void DefaultCanvasGridManagerPrivate::autoArrange(QList<DFMDesktopFileInfoPointer> sortedItems)
{
    // todo：过滤gsetting中配置的文件
    auto screenOrder = screenCode();
    for (int screenNum : screenOrder) {
        qDebug() << "arrange Num" << screenNum << sortedItems.size();
        QHash<QPoint, DFMDesktopFileInfoPointer> tempGridItems;
        QHash<DFMDesktopFileInfoPointer, QPoint> tempItemGrids;
        if (!sortedItems.empty()) {
            auto tempCellStatus = cellStatus.value(screenNum);
            int coordHeight = screensCoordInfo.value(screenNum).second;
            for (int i = 0; i < tempCellStatus.size() && !sortedItems.empty(); ++i) {
                auto item = sortedItems.takeFirst();
                QPoint pos(i / coordHeight, i % coordHeight);
                tempGridItems.insert(pos, item);
                tempItemGrids.insert(item, pos);
                setCellStatus(screenNum, i, true);
            }
        }
        gridItems.insert(screenNum, tempGridItems);
        itemGrids.insert(screenNum, tempItemGrids);
    }
    overlapItems = sortedItems;
}

void DefaultCanvasGridManagerPrivate::clear()
{
    itemGrids.clear();
    gridItems.clear();
    overlapItems.clear();
    cellStatus.clear();

    for (int i : screenCode()) {
        QHash<QPoint, DFMDesktopFileInfoPointer> gridItem;
        gridItems.insert(i, gridItem);
        QHash<DFMDesktopFileInfoPointer, QPoint> itemGrid;
        itemGrids.insert(i, itemGrid);
    }

    auto coordkeys = screensCoordInfo.keys();
    for (auto key : coordkeys) {
        QVector<bool> tempVector;
        tempVector.resize(cellCount(key));
        cellStatus.insert(key, tempVector);
    }
    auto statusValues = cellStatus.values();
    for (auto &oneValue : statusValues) {
        for (int i = 0; i < oneValue.length(); ++i) {
            oneValue[i] = false;
        }
    }
}

bool DefaultCanvasGridManagerPrivate::add(const int screenNum, const QPoint &pos, const DFMDesktopFileInfoPointer &info)
{
    if (!info) {
        qCritical() << "add empty item";
        return false;
    } else if (!screensCoordInfo.contains(screenNum)) {
        qCritical() << "add" << info->filePath() << "failed. screen:" << screenNum << " invalid.";
        return false;
    } else if (!isValid(screenNum, pos)) {
        qCritical() << "add" << info->filePath() << "failed. Pos:" << pos << " invalid in screen:" << screenNum;
        return false;
    } else if (itemGrids.value(screenNum).contains(info)) {
        qCritical() << "add" << info->filePath() << "failed. It already exists and pos is:" << itemGrids.value(screenNum).value(info);
        return false;
    }

    if (gridItems.value(screenNum).contains(pos)) {
        if (pos != overlapPos()) {
            qCritical() << "add" << info->filePath() << "failed. Pos:" << pos << " have other file:" << gridItems.value(screenNum).value(pos)->filePath();
            return false;
        } else {
            if (!overlapItems.contains(info)) {
                overlapItems.append(info);
                addIntoOverlap = true;
                return true;
            }
            return false;
        }
    }

    gridItems[screenNum].insert(pos, info);
    itemGrids[screenNum].insert(info, pos);
    int index = indexOfGridPos(screenNum, pos);
    bool ret = setCellStatus(screenNum, index, true);

    return ret;
}

bool DefaultCanvasGridManagerPrivate::add(const int screenNum, const DFMDesktopFileInfoPointer &info)
{
    if (!info) {
        qCritical() << "add empty item";
        return false;
    }

    QPair<int, QPoint> posPair { takeEmptyPos(screenNum) };
    return add(posPair.first, posPair.second, info);
}

bool DefaultCanvasGridManagerPrivate::add(const DFMDesktopFileInfoPointer &info)
{
    if (!info) {
        qCritical() << "add empty item";
        return false;
    }

    QPair<int, QPoint> posPair { takeEmptyPos() };
    return add(posPair.first, posPair.second, info);
}

bool DefaultCanvasGridManagerPrivate::remove(const int screenNum, const DFMDesktopFileInfoPointer &info)
{
    if (!itemGrids.value(screenNum).contains(info)) {
        qDebug() << "can not remove:" << info->filePath() << " from screen:" << screenNum;
        return false;
    }

    auto posKey = gridItems[screenNum].key(info);
    gridItems[screenNum].remove(posKey);
    itemGrids[screenNum].remove(info);

    auto usageIndex = indexOfGridPos(screenNum, posKey);

    if (!cellStatus.contains(screenNum))
        return false;

    return setCellStatus(screenNum, usageIndex, false);
}

bool DefaultCanvasGridManagerPrivate::remove(const DFMDesktopFileInfoPointer &info)
{
    if (overlapItems.contains(info)) {
        overlapItems.removeAll(info);
        return true;
    }

    for (const auto s : screenCode()) {
        auto temp = itemGrids.value(s);
        if (temp.contains(info)) {
            return remove(s, info);
        }
    }
    return false;
}

void DefaultCanvasGridManagerPrivate::updateGridSize(const int screenNum, const int width, const int height)
{
    if (!screensCoordInfo.contains(screenNum)) {
        return;
    }
    auto coordInfo = screensCoordInfo.find(screenNum);
    coordInfo.value().first = width;
    coordInfo.value().second = height;

    cellStatus[screenNum] = QVector<bool>(width * height, false);
}

/*!
 * \brief 清除栅格相关信息（需要重新解析配置文件）（初始化、模式改变）
 */
void DefaultCanvasGridManagerPrivate::clearCoord()
{
}

/*!
 * \brief 清除与记录的文件位置相关的数据（分辨率改变）
 */
void DefaultCanvasGridManagerPrivate::clearPositionData()
{
}

DefaultCanvasGridManager *DefaultCanvasGridManager::instance()
{
    static DefaultCanvasGridManager gridMgr;
    return &gridMgr;
}

void DefaultCanvasGridManager::initCoord(const int screenCount)
{
    if (screenCount <= 0)
        return;
    if (screenCount == 1)
        d->singleScreen = true;
    else
        d->singleScreen = false;

    d->clearCoord();
    for (int i = 1; i <= screenCount; ++i) {
        d->screensCoordInfo[i] = qMakePair(0, 0);
    }
}

void DefaultCanvasGridManager::initGridItemsInfo()
{
    QList<DFMDesktopFileInfoPointer> list;
    QHash<DFMDesktopFileInfoPointer, bool> existItems;
    QList<DFMDesktopFileInfoPointer> &actualList = DefaultFileTreaterCt->getFiles();
    for (const DFMDesktopFileInfoPointer &df : actualList) {
        list.append(df);
        qDebug() << df->fileName() << df;
        existItems.insert(df, false);
    }
    d->loadProfile(list, existItems);
}

void DefaultCanvasGridManager::initArrage(const QList<DFMDesktopFileInfoPointer> &items)
{
    d->clear();
    d->autoArrange(items);
}

void DefaultCanvasGridManager::updateGridSize(const int screenNum, const int width, const int height)
{
    auto coordInfo = d->screensCoordInfo.value(screenNum);
    if (width == coordInfo.first && height == coordInfo.second) {
        return;
    }

    if (0 == coordInfo.first || 0 == coordInfo.second) {
        d->updateGridSize(screenNum, width, height);
        return;
    }
    d->clearPositionData();
    d->updateGridSize(screenNum, width, height);
}

QPoint DefaultCanvasGridManager::filePos(const int screenNum, const DFMDesktopFileInfoPointer &info)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(info)
    return QPoint(0, 0);
}

QPoint DefaultCanvasGridManager::filePos(const int screenNum, const QModelIndex &info)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(info)
    return QPoint(0, 0);
}

int DefaultCanvasGridManager::emptyPostionCount(int screenNum) const
{
    Q_UNUSED(screenNum)
    return 0;
}

bool DefaultCanvasGridManager::find(const QString &itemId, QPair<int, QPoint> &pos)
{
    Q_UNUSED(itemId)
    Q_UNUSED(pos)
    return false;
}

bool DefaultCanvasGridManager::isEmpty(int screenNum, int x, int y)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(x)
    Q_UNUSED(y)
    return false;
}

bool DefaultCanvasGridManager::contains(int screebNum, const QString &id)
{
    Q_UNUSED(screebNum)
    Q_UNUSED(id)
    return false;
}

DFMDesktopFileInfoPointer DefaultCanvasGridManager::firstItemId(int screenNum)
{
    Q_UNUSED(screenNum)
    return nullptr;
}

DFMDesktopFileInfoPointer DefaultCanvasGridManager::lastItemId(int screenNum)
{
    Q_UNUSED(screenNum)
    return nullptr;
}

DFMDesktopFileInfoPointer DefaultCanvasGridManager::itemId(int screenNum, QPoint pos)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(pos)
    return nullptr;
}

DFMDesktopFileInfoPointer DefaultCanvasGridManager::itemTop(int screenNum, int x, int y)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(x)
    Q_UNUSED(y)
    return nullptr;
}

DFMDesktopFileInfoPointer DefaultCanvasGridManager::itemTop(int screenNum, QPoint pos)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(pos)
    return nullptr;
}

QList<DFMDesktopFileInfoPointer> DefaultCanvasGridManager::overlapItems() const
{
    return {};
}

int DefaultCanvasGridManager::overLapScreen()
{
    if (d->overlapItems.isEmpty())
        return -1;
    auto screenCount = d->gridItems.size();
    while (screenCount > 0) {
        screenCount--;
        auto itemCount = d->screensCoordInfo[screenCount].first * d->screensCoordInfo[screenCount].second;
        if (itemCount > d->gridItems[screenCount].size())
            continue;
        break;
    }
    return ++screenCount;
}

void DefaultCanvasGridManager::allItems(QList<DFMDesktopFileInfoPointer> &list) const {
    Q_UNUSED(list)
}

QHash<QPoint, DFMDesktopFileInfoPointer> DefaultCanvasGridManager::items(const int screenNum) const
{
    return d->gridItems.value(screenNum);
}

void DefaultCanvasGridManager::setShowHiddenFiles(bool value) noexcept
{
    d->setWhetherShowHiddenFiles(value);
}

bool DefaultCanvasGridManager::getShowHiddenFiles() noexcept
{
    return d->getWhetherShowHiddenFiles();
}

DefaultCanvasGridManager::DefaultCanvasGridManager(QObject *parent)
    : dfmbase::AbstractCanvasGridManager(parent), d(new DefaultCanvasGridManagerPrivate())
{
}

DSB_D_END_NAMESPACE
