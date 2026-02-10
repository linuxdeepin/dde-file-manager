// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dodgeoper.h"
#include "canvasmanager.h"
#include "utils/keyutil.h"
#include "view/canvasview_p.h"
#include "grid/canvasgrid.h"
#include "model/canvasselectionmodel.h"

#include <QDragMoveEvent>
#include <QMimeData>
#include <QDebug>

using namespace ddplugin_canvas;

DodgeOper::DodgeOper(CanvasView *parent)
    : QObject(parent), view(parent)
{
    dodgeDelayTimer.setInterval(200);
    dodgeDelayTimer.setSingleShot(true);

    connect(&dodgeDelayTimer, &QTimer::timeout, this, &DodgeOper::startDodgeAnimation);
}

DodgeOper::~DodgeOper()
{
}

void DodgeOper::updatePrepareDodgeValue(QEvent *event)
{
    if (event) {
        if (QEvent::DragEnter == event->type()) {
            if (QDragEnterEvent *dragEnterEvent = dynamic_cast<QDragEnterEvent *>(event)) {
                CanvasView *fromView = qobject_cast<CanvasView *>(dragEnterEvent->source());
                if (fromView
                    && dragEnterEvent->mimeData()
                    && !isCtrlPressed()
                    && CanvasGrid::Mode::Custom == GridIns->mode()) {
                    prepareDodge = true;
                    fmInfo() << "Dodge preparation enabled - from view:" << fromView->screenNum() << "to view:" << view->screenNum();
                    return;
                }
            }
        }
    }

    prepareDodge = false;
}

void DodgeOper::tryDodge(QDragMoveEvent *event)
{
    // check dodging
    if (dodgeAnimationing) {
        fmDebug() << "Dodge operation skipped - animation already in progress";
        return;
    }

    // check data
    if (!event->mimeData()) {
        fmWarning() << "No mime data in drag move event";
        return;
    }

    // check copy file
    if (isCtrlPressed()) {
        fmDebug() << "Dodge operation skipped - Ctrl key pressed (copy mode)";
        return;
    }

    // check from view
    CanvasView *fromView = qobject_cast<CanvasView *>(event->source());
    if (!fromView) {
        fmDebug() << "Dodge operation skipped - invalid source view";
        return;
    }

    // check origin file
    auto urls = event->mimeData()->urls();
    QPair<int, QPoint> originPos;
    if (urls.isEmpty() || !GridIns->point(urls.first().toString(), originPos)) {
        fmDebug() << "Dodge operation skipped - no valid origin file position";
        return;
    }

    // check target has file
    auto gridPos = view->d->gridAt(event->pos());
    auto targetItem = GridIns->item(view->screenNum(), gridPos);
    if (targetItem.isEmpty())
        return;

    dragTargetGridPos = gridPos;

#if 1
    if (originPos.first == view->screenNum()) {
        startDelayDodge();
    } else {
        int emptyPosCount = GridIns->gridCount(view->screenNum()) - GridIns->items(view->screenNum()).count();
        if (emptyPosCount >= urls.size()) {
            startDelayDodge();
        }
    }

#else
    // support cross screen
#endif
}

bool DodgeOper::getDodgeItemGridPos(const QString &item, GridPos &gridPos)
{
    if (!oper.get()) {
        fmDebug() << "No dodge operation available for item position query:" << item;
        return false;
    }

    return oper->position(item, gridPos);
}

void DodgeOper::setDodgeDuration(double duration)
{
    if (qFuzzyCompare(dodgeDuration, duration))
        return;

    dodgeDuration = duration;
    emit dodgeDurationChanged(dodgeDuration);
}

void DodgeOper::startDodgeAnimation()
{
    dodgeAnimationing = true;
    if (!calcDodgeTargetGrid()) {
        fmWarning() << "Failed to calculate dodge target grid - animation cancelled";
        dodgeAnimationing = false;
        return;
    }

    if (animation.get()) {
        fmDebug() << "Disconnecting existing animation";
        animation->disconnect();
    }

    animation.reset(new QPropertyAnimation(this, "dodgeDuration"));
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);

    connect(animation.get(), &QPropertyAnimation::valueChanged, this, &DodgeOper::dodgeAnimationUpdate);
    connect(animation.get(), &QPropertyAnimation::finished, this, &DodgeOper::dodgeAnimationFinished);

    animation->start();
}

void DodgeOper::dodgeAnimationUpdate()
{
    CanvasIns->update();
}

void DodgeOper::dodgeAnimationFinished()
{
    fmInfo() << "Dodge animation finished";
    dodgeAnimationing = false;
    CanvasIns->update();

    if (!oper.get()) {
        fmWarning() << "No dodge operation to apply after animation";
        return;
    }

    GridIns->core().applay(oper.get());
    GridIns->requestSync();
}

bool DodgeOper::calcDodgeTargetGrid()
{
    auto selUrls = view->selectionModel()->selectedUrls();

    QStringList orgItems;
    for (auto url : selUrls)
        orgItems << url.toString();

    GridPos gridPos { view->screenNum(), dragTargetGridPos };

    oper.reset(new DodgeItemsOper(&GridIns->core()));

    dodgeItems.clear();
    return oper->tryDodge(orgItems, gridPos, dodgeItems);
}

void DodgeOper::startDelayDodge()
{
    dodgeDelayTimer.start();
}

void DodgeOper::stopDelayDodge()
{
    dodgeDelayTimer.stop();
    dragTargetGridPos = QPoint(-1, -1);
}

DodgeItemsOper::DodgeItemsOper(GridCore *core)
    : GridCore(*core)
{
}

bool DodgeItemsOper::tryDodge(const QStringList &orgItems, const GridPos &ref, QStringList &dodgeItems)
{
    // find target index
    int targetIndex = toIndex(ref.first, ref.second);

    // find all empty index
    QList<QPoint> emptyPos = voidPos(ref.first);
    QList<int> emptyindexes = toIndex(ref.first, emptyPos);

    // sort empty index by distance from the target
    std::stable_sort(emptyindexes.begin(), emptyindexes.end(), [targetIndex](const int &index1, const int &index2) {
        return qAbs(index1 - targetIndex) < qAbs(index2 - targetIndex);
    });

    // according to the principle of proximity, calculate the number of empty required before and after the target
    int targetBeforNeedEmptyCount = 0;
    int targetAfterNeedEmptyCount = 0;
    int itemIndex = 0;
    for (auto item : orgItems) {
        GridPos itemPos;
        if (position(item, itemPos)) {
            if (itemPos.first == ref.first) {
                // same screen
                itemIndex = toIndex(itemPos.first, itemPos.second);
            } else {
                // cross screen
                if (!emptyindexes.isEmpty()) {
                    itemIndex = emptyindexes.takeFirst();
                } else {
                    fmCritical() << "Insufficient empty positions for cross-screen drag operation";
                    return false;
                }
            }
            // ===========remove origin item=============
            remove(itemPos.first, itemPos.second);

            if (itemIndex < targetIndex) {
                // hope to find empty befor target
                ++targetBeforNeedEmptyCount;
            } else {
                // hope to find empty after target
                ++targetAfterNeedEmptyCount;
            }
        }
    }

    int dodgeMidPosition = targetIndex;
    if (0 == targetAfterNeedEmptyCount) {
        // msut.Thant make sure the target itself can dodge
        ++dodgeMidPosition;
    }

    dodgeItems = reloach(ref.first, dodgeMidPosition, targetBeforNeedEmptyCount, targetAfterNeedEmptyCount);
    QPoint newPos;
    for (int i = 0; i < orgItems.count(); ++i) {
        // ===========add origin item=============
        newPos = toPos(ref.first, dodgeMidPosition - targetBeforNeedEmptyCount + i);
        insert(ref.first, newPos, orgItems.at(i));
    }

    return true;
}

int DodgeItemsOper::toIndex(const int screenNumber, const QPoint &pos)
{
    return surfaceSize(screenNumber).height() * pos.x() + pos.y();
}

QList<int> DodgeItemsOper::toIndex(const int screenNumber, const QList<QPoint> &pos)
{
    int height = surfaceSize(screenNumber).height();
    QList<int> indexes;
    for (const QPoint &point : pos) {
        Q_ASSERT(point.y() < height);
        int index = point.x() * height + point.y();
        indexes << index;
    }

    return indexes;
}

QPoint DodgeItemsOper::toPos(const int screenNumber, const int index)
{
    auto surfaceInfo = surfaceSize(screenNumber);
    auto x = index / surfaceInfo.height();
    auto y = index % surfaceInfo.height();

    return QPoint(x, y);
}

QStringList DodgeItemsOper::reloach(int screenNumber, int targetIndex, int targetBeforNeedEmptyCount, int targetAfterNeedEmptyCount)
{
    QStringList dodgeItems;

    auto end = findEmptyBackward(screenNumber, targetIndex, targetAfterNeedEmptyCount);
    auto dodgeItemsBackward = reloachBackward(screenNumber, targetIndex, end);
    dodgeItems << dodgeItemsBackward;

    auto start = findEmptyForward(screenNumber, targetIndex - 1, targetBeforNeedEmptyCount);
    auto dodgeItemsForward = reloachForward(screenNumber, start, targetIndex - 1);
    dodgeItems << dodgeItemsForward;

    return dodgeItems;
}

int DodgeItemsOper::findEmptyBackward(int screenNum, int index, int targetAfterNeedEmptyCount)
{
    if (!surfaces.contains(screenNum) || 0 == targetAfterNeedEmptyCount) {
        fmDebug() << "No backward empty search needed for screen" << screenNum;
        return index;
    }

    // find all empty indexes
    auto posList = voidPos(screenNum);
    auto emptyindexes = toIndex(screenNum, posList);

    // e.g.[2, 3, 8, 10, 12],and index is 5
    for (int endIndex = index; endIndex <= emptyindexes.last();) {
        if (!emptyindexes.contains(endIndex)) {
            // find the actual first empty backwards
            ++endIndex;
            continue;
        }

        --targetAfterNeedEmptyCount;
        if (0 == targetAfterNeedEmptyCount) {
            return endIndex;
        }

        const int nextPosition = emptyindexes.indexOf(endIndex) + 1;
        if (Q_UNLIKELY(nextPosition >= emptyindexes.count())) {
            fmWarning() << "Backward empty search failed - insufficient empty positions";
            break;
        }

        // next empty index
        endIndex = emptyindexes.at(nextPosition);
    }

    auto lastPoint = QPoint(surfaces.value(screenNum).width(), surfaces.value(screenNum).height());
    return toIndex(screenNum, lastPoint);
}

QStringList DodgeItemsOper::reloachBackward(int screenNum, int start, int end)
{
    QStringList dodgeItems;
    if (Q_UNLIKELY(!surfaces.contains(screenNum))) {
        fmWarning() << "Invalid screen number for backward relocation:" << screenNum;
        return dodgeItems;
    }

    for (int index = end; index >= start; --index) {
        auto pos = toPos(screenNum, index);
        GridPos gridPos(screenNum, pos);
        auto itemFile = item(gridPos);
        if (!itemFile.isEmpty()) {
            // extract the files that need to be dodged one by one from the back to the front
            dodgeItems << itemFile;
            remove(screenNum, pos);
        }
    }

    for (int index = end; index > end - dodgeItems.count(); --index) {
        auto pos = toPos(screenNum, index);
        auto itemFile = dodgeItems.at(end - index);
        // files that need to be dodged are inserted last in the original order
        insert(screenNum, pos, itemFile);
    }

    return dodgeItems;
}

int DodgeItemsOper::findEmptyForward(int screenNum, int index, int targetBeforNeedEmptyCount)
{
    if (!surfaces.contains(screenNum) || 0 == targetBeforNeedEmptyCount) {
        fmDebug() << "No forward empty search needed for screen" << screenNum;
        return index;
    }

    // find all empty indexes
    auto posList = voidPos(screenNum);
    auto emptyindexes = toIndex(screenNum, posList);

    for (int startIndex = index; startIndex >= 0;) {
        if (!emptyindexes.contains(startIndex)) {
            // find the actual first empty forwards
            --startIndex;
            continue;
        }

        --targetBeforNeedEmptyCount;
        if (0 == targetBeforNeedEmptyCount) {
            fmDebug() << "Found forward empty position at index" << startIndex;
            return startIndex;
        }

        int arrayPosition = emptyindexes.indexOf(startIndex);
        if (Q_UNLIKELY(arrayPosition == 0)) {
            fmWarning() << "Forward empty search failed - insufficient empty positions";
            break;
        }
        startIndex = emptyindexes.at(--arrayPosition);
    }

    fmDebug() << "Using start position as fallback: 0";
    return 0;
}

QStringList DodgeItemsOper::reloachForward(int screenNum, int start, int end)
{
    QStringList dodgeItems;
    if (Q_UNLIKELY(!surfaces.contains(screenNum))) {
        fmWarning() << "Invalid screen number for forward relocation:" << screenNum;
        return dodgeItems;
    }

    for (int index = start; index <= end; ++index) {
        auto pos = toPos(screenNum, index);
        GridPos gridPos(screenNum, pos);
        auto itemFile = item(gridPos);
        if (!itemFile.isEmpty()) {
            dodgeItems << itemFile;
            // extract the files that need to be dodged one by one from the front to the back
            remove(screenNum, pos);
        }
    }

    for (int index = start; index < start + dodgeItems.count(); ++index) {
        auto pos = toPos(screenNum, index);
        auto itemFile = dodgeItems.at(index - start);
        // files that need to be dodged are inserted front in the original order
        insert(screenNum, pos, itemFile);
    }

    return dodgeItems;
}
