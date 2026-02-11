// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grid/canvasgrid_p.h"
#include "displayconfig.h"

#include <QApplication>
#include <QUrl>
#include <QDebug>

using namespace ddplugin_canvas;

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
    fmInfo() << "CanvasGrid destroyed";
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
    if (Q_UNLIKELY(itor == d->surfaces.end())) {
        fmWarning() << "Failed to update size: surface index" << index << "not found";
        return;
    }

    if (Q_UNLIKELY(size.width() < 0 || size.height() < 0)) {
        fmWarning() << "Invalid size for surface index" << index << ":" << size;
        return;
    }

    if (itor.value().width() == size.width() &&
            itor.value().height() == size.height()) {
        fmDebug() << "Surface" << index << "size unchanged:" << size;
        return;
    }

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
        auto itemInfo = d->itemPos.value(index);
        ret  << CanvasGridSpecialist::sortItemInGrid(itemInfo) << overloadItems(index);
    }
    return ret;
}

QString CanvasGrid::item(int index, const QPoint &pos) const
{
    return d->posItem[index].value(pos);
}

QHash<QString, QPoint> CanvasGrid::points(int index) const
{
    return d->itemPos.value(index);
}

bool CanvasGrid::point(const QString &item, QPair<int, QPoint> &pos) const
{
    if (item.isEmpty()) {
        fmWarning() << "Empty item provided for point lookup";
        return false;
    }

    for (auto itor = d->itemPos.begin(); itor != d->itemPos.end(); ++itor) {
        if (itor->contains(item)) {
            pos.first = itor.key();
            pos.second = itor->value(item);
            return true;
        }
    }

    return false;
}

QStringList CanvasGrid::overloadItems(int index) const
{
    auto idxs = d->surfaceIndex();

    // overloaded items show on the last screen.
    if (!idxs.isEmpty() && (index == idxs.last() || index < 0))
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
            requestSync();
            return true;
        }
    }

    return false;
}

bool CanvasGrid::move(int toIndex, const QPoint &toPos, const QString &focus, const QStringList &items)
{
    // first check whether focus pos is valid.
    if (!CanvasGridSpecialist::isValid(toPos, d->surfaceSize(toIndex))) {
        fmWarning() << "Invalid target position" << toPos << "for surface" << toIndex;
        return false;
    }

    // it's unmovable if target pos is not void and the item on the pos is not in \a items.
    if (!d->isVoid(toIndex, toPos) && !items.contains(d->item(GridPos(toIndex, toPos)))) {
        fmWarning() << "Target position" << toPos << "is occupied by item not in move list";
        return false;
    }

    GridPos centerPos;
    if (!d->position(focus, centerPos)) {
        fmWarning() << "Cannot find position for focus item:" << focus;
        return false;
    }

    MoveGridOper oper(d);
    if (oper.move(GridPos(toIndex, toPos), centerPos, items)) {
        d->applay(&oper);
        requestSync();
        return true;
    }

    return false;
}

bool CanvasGrid::remove(int index, const QString &item)
{
    if (Q_UNLIKELY(item.isEmpty())) {
        fmWarning() << "Empty item provided for remove operation";
        return false;
    }

    if (d->itemPos.value(index).contains(item)) {
        d->remove(index, item);
        requestSync();
        return true;
    }

    if (d->overload.contains(item)) {
        d->overload.removeOne(item);
        return true;
    }

    fmWarning() << "Item" << item << "not found for removal";
    return false;
}

bool CanvasGrid::replace(const QString &oldItem, const QString &newItem)
{
    QPair<int, QPoint> pos;
    if (point(oldItem, pos)) {
        d->remove(pos.first, pos.second);
        d->insert(pos.first, pos.second, newItem);
        requestSync();
        return true;
    }

    int idx = d->overload.indexOf(oldItem);
    if (idx > -1) {
        fmInfo() << "Replacing overload item" << oldItem << "with" << newItem;
        d->overload.replace(idx, newItem);
        return true;
    }

    fmWarning() << "Item" << oldItem << "not found for replacement";
    return false;
}

void CanvasGrid::append(const QString &item)
{
    if (Q_UNLIKELY(item.isEmpty())) {
        fmWarning() << "Empty item provided for append operation";
        return;
    }

    GridPos pos;
    if (d->findVoidPos(pos)) {
        // insert to valid void pos.
        d->insert(pos.first, pos.second, item);
    } else {
        // push in overload
        d->pushOverload({item});
    }

    requestSync();
    return;
}

void CanvasGrid::append(const QStringList &items)
{
    if (Q_UNLIKELY(items.isEmpty())) {
        fmWarning() << "Empty items list provided for append operation";
        return;
    }

    AppendOper oper(d);
    oper.append(items);
    d->applay(&oper);

    requestSync();
}

void CanvasGrid::tryAppendAfter(const QStringList &items, int index, const QPoint &begin)
{
    if (Q_UNLIKELY(items.isEmpty())) {
        fmWarning() << "Empty items list provided for tryAppendAfter operation";
        return;
    }

    AppendOper oper(d);
    oper.tryAppendAfter(items, index, begin);
    d->applay(&oper);

    requestSync();
}

void CanvasGrid::popOverload()
{
    if (!d->overload.isEmpty()) {
        GridPos pos;
        if (d->findVoidPos(pos)) {
            const QString &it = d->overload.takeFirst();
            d->insert(pos.first, pos.second, it);
            requestSync();
        }
    }
}

void CanvasGrid::arrange()
{
    d->sequence(items());
}

GridCore &CanvasGrid::core() const
{
    return *static_cast<GridCore*>(d);
}

void CanvasGrid::requestSync(int ms)
{
    d->syncTimer.stop();
    d->syncTimer.start(ms);
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
        fmDebug() << "Processing surface" << idx << "with" << sortedItems.size() << "remaining items";
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
            fmInfo() << "Surface" << idx << "placed" << cur << "items out of" << max << "available positions";
        }

        itemPos.insert(idx, allItem);
        posItem.insert(idx, allPos);
    }

    fmInfo() << "Added" << sortedItems.size() << "items to overload";
    overload = sortedItems;

    q->requestSync();
}

void CanvasGridPrivate::restore(QStringList currentItems)
{
    clean();

    auto idxs = surfaceIndex();
    if (idxs.isEmpty()) {
        fmWarning() << "No surfaces available to place items";
        return;
    }

    // get item pos from record.
    QHash<int, QHash<QString, QPoint> > profile = profiles();
    fmInfo() << "Loaded profiles for surfaces:" << profile.keys();

    // the item's pos in record is invalid to current grid.
    QStringList invalidPos;

    // restore each surface.
    for (int idx : idxs) {
        const QHash<QString, QPoint> &oldPos = profile.value(idx);
        fmInfo() << "Restoring surface" << idx << "with" << oldPos.size() << "recorded items";
        const QSize surfaceSize = surfaces.value(idx);

        // restore item always existed to recored pos.
        for (auto itor = oldPos.begin(); itor != oldPos.end(); ++itor) {
            // for compatibility. record use 'file://' like.
            // if currentItems's schema is desktop://,
            // using covertFileUrlToDesktop(itor.key()).toString() to cover it to file://
            QString item = itor.key();

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

    q->requestSync();
}

QHash<int, QHash<QString, QPoint> > CanvasGridPrivate::profiles() const
{
    auto idxs = surfaceIndex();

    // signle screen and multi-screen use different key
    QList<QString> keys;
    if (idxs.size() == 1)
        keys.append(CanvasGridSpecialist::singleIndex);
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
    if (q->items().isEmpty() || count < 1) {
        fmDebug() << "Skipping sync: no items or no surfaces";
        return;
    }

    auto idxs = surfaceIndex();

    // single mode
    if (count == 1) {
        // update group SingleScreen
        DispalyIns->setCoordinates(CanvasGridSpecialist::singleIndex,itemPos.value(idxs.first()));
        // if itemPos's schema is desktop://,
        // using CanvasGridSpecialist::covertDesktopUrlToFiles(itemPos.value(idxs.first())) to cover it to file://

    } else {
        QList<QString> profile;
        for (const int &idx : idxs) {
            auto key = CanvasGridSpecialist::profileKey(idx);
            profile.append(key);

            // update group Screen_xx
            // for compatibility. covert "ddecesktop:/" used by code to "file://" used record file.
            // if itemPos's schema is desktop://,
            // using CanvasGridSpecialist::covertDesktopUrlToFiles(itemPos.value(idx)) to cover it to file://
            DispalyIns->setCoordinates(key, itemPos.value(idx));
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
    // single screen return index 1.
    if (screenKey == QString(CanvasGridSpecialist::singleIndex))
        return 1;

    // multi-screen
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
    std::sort(ordered.begin(), ordered.end(),[](const QPair<QPoint, QString> &t1, QPair<QPoint, QString> &t2){
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
//QHash<QString, QPoint> CanvasGridSpecialist::covertDesktopUrlToFiles(const QHash<QString, QPoint> &urls)
//{
//    QHash<QString, QPoint> ret;
//    for (auto itor = urls.begin(); itor != urls.end(); ++itor) {
//        ret.insert(covertDesktopUrlToFile(itor.key()).toString(), itor.value());
//    }
//    return ret;
//}


