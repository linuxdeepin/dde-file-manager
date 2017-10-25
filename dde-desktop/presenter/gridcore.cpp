#include "gridcore.h"

GridCore::GridCore()
{

}

QStringList GridCore::reloacle(GIndex targetIndex, int emptyBefore, int emptyAfter)
{
//    qDebug() << targetIndex << emptyBefore << emptyAfter;
    QStringList dodgeItems;
    auto end = findEmptyBackward(targetIndex, emptyAfter);
    dodgeItems << reloacleBackward(targetIndex, end);
//    for (auto i = targetIndex; i <= end; ++i) {
//        qDebug() << gridItems.value(toPos(i));
//    }

    auto start = findEmptyForward(targetIndex - 1, emptyBefore);
    dodgeItems << reloacleForward(start, targetIndex - 1);
//    for (auto i = start; i < targetIndex; ++i) {
//        qDebug() << gridItems.value(toPos(i));
//    }

    return dodgeItems;
}
