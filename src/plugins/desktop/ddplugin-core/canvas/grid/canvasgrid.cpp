/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "grid/canvasgrid_p.h"
#include "displayconfig.h"

#include <QApplication>
#include <QUrl>
#include <QDebug>

DSB_D_USE_NAMESPACE

class CanvasGridGlobal : public CanvasGrid{};
Q_GLOBAL_STATIC(CanvasGridGlobal, canvasGridGlobal)

CanvasGrid *CanvasGrid::instance()
{
    return canvasGridGlobal;
}

CanvasGrid::CanvasGrid(QObject *parent)
    : QObject(parent)
    , d(new CanvasGridPrivate(this))
{
    Q_ASSERT(qApp->thread() == thread());
}

CanvasGrid::~CanvasGrid()
{
}

void CanvasGrid::initSurface(int count)
{
    //todo(zy) clear all data
    d->clean();

    d->surfaces.clear();
    for (int i = 1; i <= count; ++i)
        d->surfaces.insert(i, QSize(0, 0));
}

void CanvasGrid::updateSize(int index, const QSize &size)
{
    auto itor = d->surfaces.find(index);
    if (Q_UNLIKELY(itor == d->surfaces.end()))
        return;

    if (Q_UNLIKELY(size.width() < 0 || size.height() < 0)) {
        qWarning() << "index" << index << "invaild size" << size;
        return;
    }

    if (itor.value().width() == size.width() &&
            itor.value().height() == size.height())
        return;

    // need to rearrange items if surface /a index isn't empty.
    bool rearrange = !d->itemPos.value(index).isEmpty();
    if (rearrange) {
        // get current items to restore
        auto allItems = items();

        //update surface size
        itor.value() = size;

        // rearrange all items
        setItems(allItems);
    } else {
        // just update surface size
        itor.value() = size;
    }
}

QSize CanvasGrid::surfaceSize(int index) const
{
    return d->surfaceSize(index);
}

int CanvasGrid::gridCount(int index) const
{
    int count = 0;
    if (index < 0) {
        for (auto itor = d->surfaces.begin(); itor != d->surfaces.end(); ++itor)
            count += itor.value().width() * itor.value().height();
    } else {
        auto size = d->surfaceSize(index);
        count = size.width() * size.height();
    }

    return count;
}

void CanvasGrid::setMode(CanvasGrid::Mode mode)
{
    d->mode = mode;
}

CanvasGrid::Mode CanvasGrid::mode() const
{
    return d->mode;
}

void CanvasGrid::setItems(const QStringList &items)
{
    switch (d->mode) {
    case Mode::Custom :
        d->restore(items);
        break;
    case Mode::Align :
        d->sequence(items);
        break;
    default:
        break;
    }

    return;
}

QStringList CanvasGrid::items(int index) const
{
    QStringList ret;

    // get all items
    if (index < 0) {
        for (const int &idx : d->surfaceIndex())
            ret << items(idx);
    } else { // get items which is in surface \a index
        auto item = d->itemPos.value(index);
        ret  << CanvasGridSpecialist::sortItemInGrid(item) << overloadItems(index);
    }
    return ret;
}

QHash<QString, QPoint> CanvasGrid::points(int index) const
{
    return d->itemPos.value(index);
}

QStringList CanvasGrid::overloadItems(int index) const
{
    auto idxs = d->surfaceIndex();

    // overloaded items show on the last screen.
    if (!idxs.isEmpty() && index == idxs.last())
        return d->overload;

    return QStringList();
}

bool CanvasGrid::drop(int index, const QPoint &pos, const QString &item)
{
    // check item and pos whether or not vaild
    if (!item.isEmpty() && CanvasGridSpecialist::isValid(pos, d->surfaceSize(index))) {
        // pos is enable to drop
        if (d->isVoid(index, pos)) {
            d->insert(index, pos, item);
            d->requestSync();
            return true;
        }
    }

    return false;
}

bool CanvasGrid::move(int toIndex, const QPoint &toPos, const QString &focus, const QStringList &items)
{
    // first check whether focus pos is valid.
    if (!CanvasGridSpecialist::isValid(toPos, d->surfaceSize(toIndex)))
        return false;

    // it's unmovable if target pos is not void and the item on the pos is not in \a items.
    if (!d->isVoid(toIndex, toPos) && !items.contains(d->item(GridPos(toIndex, toPos))));
        return false;

    GridPos centerPos;
    if (!d->position(focus, centerPos))
        return false;

    MoveGridOper oper(d);
    if (oper.move(GridPos(toIndex, toPos), centerPos, items)) {
        d->applay(&oper);
        return true;
    }

    return false;
}

bool CanvasGrid::remove(int index, const QString &item)
{
    if (Q_UNLIKELY(item.isEmpty()))
        return false;

    if (d->itemPos.value(index).contains(item)) {
        d->remove(index, item);
        d->requestSync();
        return true;
    }

    if (d->overload.contains(item)) {
        d->overload.removeOne(item);
        return true;
    }

    return false;
}

void CanvasGrid::append(const QString &item)
{
    if (Q_UNLIKELY(item.isEmpty()))
        return;

    GridPos pos;
    if (d->findVoidPos(pos)) {
        // insert to valid void pos.
        d->insert(pos.first, pos.second, item);
    } else {
        // push in overload
        d->pushOverload({item});
    }

    d->requestSync();
    return;
}

void CanvasGrid::append(QStringList items)
{
    if (Q_UNLIKELY(items.isEmpty()))
        return;

    AppendOper oper(d);
    oper.append(items);
    d->applay(&oper);

    d->requestSync();
}

void CanvasGrid::popOverload()
{
    if (!d->overload.isEmpty()) {
        GridPos pos;
        if (d->findVoidPos(pos)) {
            auto item = d->overload.takeFirst();
            d->insert(pos.first, pos.second, item);
            d->requestSync();
        }
    }
}

CanvasGridPrivate::CanvasGridPrivate(CanvasGrid *qq)
    : QObject(qq)
    , q(qq)
{
    syncTimer.setInterval(100);
    syncTimer.setSingleShot(true);
    connect(&syncTimer, &QTimer::timeout, this, &CanvasGridPrivate::sync);
}

void CanvasGridPrivate::clean()
{
    //todo(zy) clear all data and create clean grid
    posItem.clear();
    itemPos.clear();
    overload.clear();
}

void CanvasGridPrivate::sequence(QStringList sortedItems)
{
    clean();

    for (int idx : surfaceIndex()) {
        qDebug() << "surface id:" << idx << "left item " << sortedItems.size();
        QHash<QPoint, QString> allPos;
        QHash<QString, QPoint> allItem;
        if (!sortedItems.isEmpty()) {
            int max = q->gridCount(idx);
            const int height = surfaces.value(idx).height();
            int cur = 0;
            for (; cur < max && !sortedItems.isEmpty(); ++cur) {
                QString &&item = sortedItems.takeFirst();
                QPoint pos(cur / height, cur % height);
                allPos.insert(pos, item);
                allItem.insert(item, pos);
            }
            qDebug() << "surface" << idx << "drop items count:" << cur << "max" << max;
        }

        itemPos.insert(idx, allItem);
        posItem.insert(idx, allPos);
    }
    qDebug() << "overload items " << sortedItems.size();
    overload = sortedItems;

    requestSync();
}

void CanvasGridPrivate::restore(QStringList currentItems)
{
    clean();

    auto idxs = surfaceIndex();
    if (idxs.isEmpty()) {
        qWarning() << "no surface to drop items.";
        return;
    }

    // get item pos from record.
    QHash<int, QHash<QString, QPoint> > profile = profiles();

    // the item's pos in record is invalid to current grid.
    QStringList invalidPos;

    // restore each surface.
    for (int idx : idxs) {
        const QHash<QString, QPoint> &oldPos = profile.value(idx);
        const QSize surfaceSize = surfaces.value(idx);

        // restore item always existed to recored pos.
        for (auto itor = oldPos.begin(); itor != oldPos.end(); ++itor) {
            // for compatibility. record use 'file://' like.
            QString item = CanvasGridSpecialist::covertFileUrlToDesktop(itor.key()).toString();

            if (!currentItems.contains(item))
                continue; // item was removed.

            const QPoint &pos = itor.value();
            // pos is valid.
            if (CanvasGridSpecialist::isValid(pos, surfaceSize))
                insert(idx, pos, item);
             else
                invalidPos.append(item);

            // remove item restored
            currentItems.removeOne(item);
        }
    }

    // append invalid-pos and rest items to empty pos
    {
        QStringList overloadItems;
        overloadItems << invalidPos << currentItems;
        if (!overloadItems.isEmpty())
            q->append(overloadItems);
    }

    requestSync();
}

void CanvasGridPrivate::requestSync()
{
    syncTimer.stop();
    syncTimer.start();
}

QHash<int, QHash<QString, QPoint> > CanvasGridPrivate::profiles() const
{
    auto idxs = surfaceIndex();

    // signle screen and multi-screen use different key
    QSet<QString> keys;
    if (idxs.size() == 1)
        keys.insert(CanvasGridSpecialist::singleIndex);
    else
        keys = DispalyIns->profile();

    // read config
    QHash<int, QHash<QString, QPoint>> profile;
    for (const QString &key : keys) {
       int idx = CanvasGridSpecialist::profileIndex(key);
       if (idx < 1)
           continue;
       QHash<QString, QPoint> filePos = DispalyIns->coordinates(key);
       if (filePos.isEmpty())
           continue;

       profile.insert(idx, filePos);
    }

    return profile;
}

void CanvasGridPrivate::sync()
{
    const int count = surfaces.count();
    if (q->items().isEmpty() || count < 1)
        return;

    auto idxs = surfaceIndex();

    // single mode
    if (count == 1) {
        // update group SingleScreen
        DispalyIns->setCoordinates(CanvasGridSpecialist::singleIndex,
                                   CanvasGridSpecialist::covertDesktopUrlToFiles(itemPos.value(idxs.first())));
    } else {
        QSet<QString> profile;
        for (const int &idx : idxs) {
            auto key = CanvasGridSpecialist::profileKey(idx);
            profile.insert(key);

            // update group Screen_xx
            // for compatibility. covert "ddecesktop:/" used by code to "file://" used record file.
            DispalyIns->setCoordinates(key, CanvasGridSpecialist::covertDesktopUrlToFiles(itemPos.value(idx)));
        }

        // update group ProFile
        DispalyIns->setProfile(profile);
    }
}

const char *const CanvasGridSpecialist::profilePrefix = "Screen_";
const char *const CanvasGridSpecialist::singleIndex = "SingleScreen";

QString CanvasGridSpecialist::profileKey(int index)
{
    return QString(profilePrefix) + QString::number(index);
}

int CanvasGridSpecialist::profileIndex(QString screenKey)
{
    auto strIdx = screenKey.remove(profilePrefix);
    bool ok = false;
    int idx = strIdx.toInt(&ok);

    if (ok)
        return idx;
    else
        return -1;
}

QStringList CanvasGridSpecialist::sortItemInGrid(const QHash<QString, QPoint> &items)
{
    QList<QPair<QPoint, QString>> ordered;
    for (auto itor = items.begin(); itor != items.end(); ++ itor)
        ordered.append(qMakePair(itor.value(), itor.key()));

    // sorted as show in grid
    // 0x0 < 0x1 < 1x0 < 1x1
    qSort(ordered.begin(), ordered.end(),[](const QPair<QPoint, QString> &t1, QPair<QPoint, QString> &t2){
        if (t1.first.x() < t2.first.x())
            return true;
        else if (t1.first.x() == t2.first.x()){
            return t1.first.y() < t2.first.y();
        } else {
            return false;
        }
    });

    QStringList ret;
    for (auto itor = ordered.begin(); itor != ordered.end(); ++ itor)
        ret.append(itor->second);

    return ret;
}

// covert item from desktop url(ddedesktop:/) to file path with scheme "file://"
QHash<QString, QPoint> CanvasGridSpecialist::covertDesktopUrlToFiles(const QHash<QString, QPoint> &urls)
{
    QHash<QString, QPoint> ret;
    for (auto itor = urls.begin(); itor != urls.end(); ++itor) {
        ret.insert(covertDesktopUrlToFile(itor.key()).toString(), itor.value());
    }
    return ret;
}


