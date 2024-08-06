// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWANIMATIONHELPER_H
#define VIEWANIMATIONHELPER_H

#include <QObject>
#include <QMap>
#include <QRect>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class FileView;
class ViewAnimationHelper : public QObject
{
    Q_OBJECT
public:
    explicit ViewAnimationHelper(FileView *parent);

    void initAnimationHelper();
    void reset();
    void syncVisiableRect();
    void aboutToPlay();

    QRect getCurrentRectByIndex(const QModelIndex &index) const;
    void playViewAnimation();
    bool isAnimationPlaying() const;
    bool isWaitingToPlaying() const;
    bool hasInitialized() const;

    void paintItems() const;

public Q_SLOTS:
    void onDelayTimerFinish();
    void onAnimationTimerFinish();

private:
    QMap<QModelIndex, QRect> calcIndexRects(const QRect &rect) const;
    void paintPixmaps(const QMap<QModelIndex, QRect> &indexRects);
    void createPixmapsForVisiableRect();
    qreal easeOutExpo(qreal value) const;

private:
    bool initialized { false };
    QRect currentVisiableRect;
    QRect oldVisiableRect;

    QMap<QModelIndex, QRect> currentIndexRectMap {};
    QMap<QModelIndex, QRect> newIndexRectMap {};
    QMap<QModelIndex, QRect> oldIndexRectMap {};
    QMap<QModelIndex, QPixmap> indexPixmaps {};

    int animFrame { 0 };

    QTimer *delayTimer { nullptr };
    QTimer *animationTimer { nullptr };

    FileView *view { nullptr };
};

}

#endif   // VIEWANIMATIONHELPER_H
