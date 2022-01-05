/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "filetreater.h"
#include "displayconfig.h"
#include "private/canvasgridmanager_p.h"

#include "dfm-base/base/schemefactory.h"

#include <QMutexLocker>
#include <QDebug>
#include <QPoint>
#include <QHash>

DSB_D_BEGIN_NAMESPACE

class CanvasGridManagerGlobal : public CanvasGridManager{};
Q_GLOBAL_STATIC(CanvasGridManagerGlobal, canvasGridManager)

static const char *const kScreenProfilePrefix = "Screen_";
static const char *const kSingleScreen = "SingleScreen";

CanvasGridManagerPrivate::CanvasGridManagerPrivate(CanvasGridManager *q_ptr)
    : q(q_ptr)
{

}

static inline QString screenProfile(int index)
{
    return QString(kScreenProfilePrefix) + QString::number(index);
}

static int profileIndex(QString screenKey)
{
    auto strIdx = screenKey.remove(kScreenProfilePrefix);
    bool ok = false;
    int idx = strIdx.toInt(&ok);

    if (ok)
        return idx;
    else
        return -1;
}

void CanvasGridManagerPrivate::saveProfile()
{
    QList<QString> profile;
    for (int index : screenCode())
        profile.append(screenProfile(index));

    DispalyIns->setProfile(profile);
}

QPair<int, QPoint> CanvasGridManagerPrivate::takeEmptyPos(const int screenNum)
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

bool CanvasGridManagerPrivate::setCellStatus(const int screenNum, const int index, bool state)
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

/*!
 * \brief GridManagerPrivate::loadProfile，加载配置文件数据，加载时会根据真实文件信息对配置文件进行过滤，
 * 过配置文件中存在但真实文件中已被删除的文件，新增配置文件中不存在的文件，这里的设计是：配置文件中是1357,若新增2468则根据
 * 排序规则追加，而不是插入。即：最终结果是135792468而不是123456789
 * \param orderedItems 需要处理的的文件列表，用于解析文件后，配置文件中不存在的项去查找新位置（该项可能比existItems少）
 * \param existItems 实际存在的文件哈西表，用于解析文件时，判断该文件实际是否存在（使用QHash而非QList是为了提升性能）
 */
void CanvasGridManagerPrivate::loadProfile(const QList<DFMLocalFileInfoPointer> &orderedItems, QHash<DFMLocalFileInfoPointer, bool> existItems)
{
    if (screenCode().isEmpty()) {
        qWarning() << "not found screen,give up load profile!!!";
        return;
    }

    // signle screen and multi-screen use different key
    QList<QString> screenProfiles;
    if (singleScreen)
        screenProfiles.append(kSingleScreen);
    else
        screenProfiles = DispalyIns->profile();

    // read config
    QHash<int, QHash<QString, QPoint>> screenFilePos;
    for (const QString &screenKey : screenProfiles) {
       int idx = profileIndex(screenKey);
       if (idx < 1)
           continue;
       QHash<QString, QPoint> filePos = DispalyIns->coordinates(screenKey);
       if (filePos.isEmpty())
           continue;

       screenFilePos.insert(idx, filePos);
    }

    QMap<int, QList<DFMLocalFileInfoPointer>> other;

    // restore current screen's files into coordinates as it configured
    for (int screenIdx : screenCode()) {
        const QHash<QString, QPoint> &filePos = screenFilePos.value(screenIdx);
        for (auto iter = filePos.cbegin(); iter != filePos.end(); ++iter) {
            QString path = dfmbase::UrlRoute::urlToPath(iter.key());
            QUrl url = dfmbase::UrlRoute::pathToReal(path);
            QString errString;
            auto itemInfo = dfmbase::InfoFactory::create<dfmbase::LocalFileInfo>(url, &errString);
            if (!itemInfo)
                qInfo() << "loadProfile error: " << errString;
            if (existItems.contains(itemInfo)) {
                const QPoint &pos = iter.value();
                if (!isValid(screenIdx, pos)) {
                    // 读取的文件实际存在，但是配置的位置不合法，则后面重新查找空位
                    other[screenIdx].append(itemInfo);
                } else {
                    // 将文件添加到指定位置
                    if (!add(screenIdx, pos, itemInfo)) {
                        continue;
                    }
                }
                existItems.remove(itemInfo);
            }
        }
    }


    // 优先处理文件位置有记录但是不合法的项
    for (int key : other.keys()) {
        foreach (auto item, other.value(key)) {
            // 优先在原记录的屏幕上添加,失败则自动查找空位
            add(key, item);
        }
    }

    // 再处理没有记录文件位置的项
    int begin = screenCode().first();
    for (auto &item : orderedItems) {
        // 已经处理过的项跳过
        if (!existItems.contains(item))
            continue;

        // 优先依据屏幕顺序查找空位
        add(begin, item);
    }
}

void CanvasGridManagerPrivate::syncAllProfile()
{
}

void CanvasGridManagerPrivate::syncProfile(const int screenNum)
{
    Q_UNUSED(screenNum)
}

/*!
 * \brief GridManagerPrivate::autoArrange,自动排列，自动排列功能仅排列收拢已排序数据
 * \param sortedItems，所有排序数据
 */
void CanvasGridManagerPrivate::autoArrange(QList<DFMLocalFileInfoPointer> sortedItems)
{
    // todo：过滤gsetting中配置的文件
    auto screenOrder = screenCode();
    for (int screenNum : screenOrder) {
        qDebug() << "arrange Num" << screenNum << sortedItems.size();
        QHash<QPoint, DFMLocalFileInfoPointer> tempGridItems;
        QHash<DFMLocalFileInfoPointer, QPoint> tempItemGrids;
        if (!sortedItems.empty()) {
            auto tempCellStatus = cellStatus.value(screenNum);
            int coordHeight = gridSize.value(screenNum).x();
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

void CanvasGridManagerPrivate::clear()
{
    itemGrids.clear();
    gridItems.clear();
    overlapItems.clear();
    cellStatus.clear();

    for (int i : screenCode()) {
        QHash<QPoint, DFMLocalFileInfoPointer> gridItem;
        gridItems.insert(i, gridItem);
        QHash<DFMLocalFileInfoPointer, QPoint> itemGrid;
        itemGrids.insert(i, itemGrid);
    }

    auto coordkeys = gridSize.keys();
    for (int key : coordkeys) {
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

bool CanvasGridManagerPrivate::add(const int screenNum, const QPoint &pos, const DFMLocalFileInfoPointer &info)
{
    if (!info) {
        qCritical() << "add empty item";
        return false;
    } else if (!gridSize.contains(screenNum)) {
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

bool CanvasGridManagerPrivate::add(const int screenNum, const DFMLocalFileInfoPointer &info)
{
    if (!info) {
        qCritical() << "add empty item";
        return false;
    }

    QPair<int, QPoint> posPair { takeEmptyPos(screenNum) };
    return add(posPair.first, posPair.second, info);
}

bool CanvasGridManagerPrivate::add(const DFMLocalFileInfoPointer &info)
{
    if (!info) {
        qCritical() << "add empty item";
        return false;
    }

    QPair<int, QPoint> posPair { takeEmptyPos() };
    return add(posPair.first, posPair.second, info);
}

bool CanvasGridManagerPrivate::remove(const int screenNum, const DFMLocalFileInfoPointer &info)
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

bool CanvasGridManagerPrivate::remove(const DFMLocalFileInfoPointer &info)
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

void CanvasGridManagerPrivate::updateGridSize(const int screenNum, const int width, const int height)
{
    if (!gridSize.contains(screenNum)) {
        return;
    }
    auto coordInfo = gridSize.find(screenNum);
    coordInfo->setX(width);
    coordInfo->setY(height);

    cellStatus[screenNum] = QVector<bool>(width * height, false);
}

/*!
 * \brief 清除栅格相关信息（需要重新解析配置文件）（初始化、模式改变）
 */
void CanvasGridManagerPrivate::clearCoord()
{
}

/*!
 * \brief 清除与记录的文件位置相关的数据（分辨率改变）
 */
void CanvasGridManagerPrivate::clearPositionData()
{
}

CanvasGridManager *CanvasGridManager::instance()
{
    return canvasGridManager;
}

void CanvasGridManager::initCoord(const int screenCount)
{
    if (screenCount <= 0)
        return;

    d->singleScreen = screenCount == 1;
    d->clearCoord();
    for (int i = 1; i <= screenCount; ++i) {
        d->gridSize[i] = QPoint(0, 0);
    }
}

void CanvasGridManager::initGridItemsInfo()
{
    QList<DFMLocalFileInfoPointer> list;
    QHash<DFMLocalFileInfoPointer, bool> existItems;
    QList<DFMLocalFileInfoPointer> &actualList = FileTreaterCt->getFiles();
    for (const DFMLocalFileInfoPointer &df : actualList) {
        list.append(df);
        qDebug() << df->fileName() << df;
        existItems.insert(df, false);
    }
    d->loadProfile(list, existItems);
}

void CanvasGridManager::initArrage(const QList<DFMLocalFileInfoPointer> &items)
{
    d->clear();
    d->autoArrange(items);
}

void CanvasGridManager::updateGridSize(const int screenNum, const int width, const int height)
{
    auto coordInfo = d->gridSize.value(screenNum);
    if (width == coordInfo.x() && height == coordInfo.y()) {
        return;
    }

    if (0 == coordInfo.x() || 0 == coordInfo.y()) {
        d->updateGridSize(screenNum, width, height);
        return;
    }
    d->clearPositionData();
    d->updateGridSize(screenNum, width, height);
}

QPoint CanvasGridManager::filePos(const int screenNum, const DFMLocalFileInfoPointer &info)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(info)
    return QPoint(0, 0);
}

QPoint CanvasGridManager::filePos(const int screenNum, const QModelIndex &info)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(info)
    return QPoint(0, 0);
}

int CanvasGridManager::emptyPostionCount(int screenNum) const
{
    Q_UNUSED(screenNum)
    return 0;
}

bool CanvasGridManager::find(const QString &itemId, QPair<int, QPoint> &pos)
{
    Q_UNUSED(itemId)
    Q_UNUSED(pos)
    return false;
}

bool CanvasGridManager::isEmpty(int screenNum, int x, int y)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(x)
    Q_UNUSED(y)
    return false;
}

bool CanvasGridManager::contains(int screebNum, const QString &id)
{
    Q_UNUSED(screebNum)
    Q_UNUSED(id)
    return false;
}

DFMLocalFileInfoPointer CanvasGridManager::firstItemId(int screenNum)
{
    Q_UNUSED(screenNum)
    return nullptr;
}

DFMLocalFileInfoPointer CanvasGridManager::lastItemId(int screenNum)
{
    Q_UNUSED(screenNum)
    return nullptr;
}

DFMLocalFileInfoPointer CanvasGridManager::itemId(int screenNum, QPoint pos)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(pos)
    return nullptr;
}

DFMLocalFileInfoPointer CanvasGridManager::itemTop(int screenNum, int x, int y)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(x)
    Q_UNUSED(y)
    return nullptr;
}

DFMLocalFileInfoPointer CanvasGridManager::itemTop(int screenNum, QPoint pos)
{
    Q_UNUSED(screenNum)
    Q_UNUSED(pos)
    return nullptr;
}

QList<DFMLocalFileInfoPointer> CanvasGridManager::overlapItems() const
{
    return {};
}

int CanvasGridManager::overlapScreen()
{
    if (d->overlapItems.isEmpty())
        return -1;
    auto screenCount = d->gridItems.size();
    while (screenCount > 0) {
        screenCount--;
        auto itemCount = d->gridSize[screenCount].x() * d->gridSize[screenCount].y();
        if (itemCount > d->gridItems[screenCount].size())
            continue;
        break;
    }
    return ++screenCount;
}

void CanvasGridManager::allItems(QList<DFMLocalFileInfoPointer> &list) const {
    Q_UNUSED(list)
}

QHash<QPoint, DFMLocalFileInfoPointer> CanvasGridManager::items(const int screenNum) const
{
    return d->gridItems.value(screenNum);
}

void CanvasGridManager::setShowHiddenFiles(bool value) noexcept
{
    d->setWhetherShowHiddenFiles(value);
}

bool CanvasGridManager::getShowHiddenFiles() noexcept
{
    return d->getWhetherShowHiddenFiles();
}

CanvasGridManager::CanvasGridManager(QObject *parent)
    : QObject(parent)
    , d(new CanvasGridManagerPrivate(this))
{
}

CanvasGridManager::~CanvasGridManager()
{
    delete d;
    d = nullptr;
}

DSB_D_END_NAMESPACE
