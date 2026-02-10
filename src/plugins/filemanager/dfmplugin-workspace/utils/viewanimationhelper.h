// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWANIMATIONHELPER_H
#define VIEWANIMATIONHELPER_H

#include <QObject>
#include <QMap>
#include <QRect>
#include <QPixmap>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
class QTimer;
class QPropertyAnimation;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class FileView;
class ViewAnimationHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double animProcess READ getAnimProcess WRITE setAnimProcess)
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

    void playAnimationWithWidthChange(int deltaWidth);

    void paintItems() const;

    void setAnimProcess(double value);
    inline double getAnimProcess() const { return animProcess; };

public Q_SLOTS:
    void onDelayTimerFinish();
    void onAnimationValueChanged();
    void onAnimationTimerFinish();

private:
    QMap<QModelIndex, QRect> calcIndexRects(const QRect &rect) const;
    void paintPixmaps(const QMap<QModelIndex, QRect> &indexRects);
    void createPixmapsForVisiableRect();
    void resetAnimation();
    void resetExpandItem();

private:
    bool initialized { false };
    bool playingAnim { false };
    QRect currentVisiableRect;
    QRect oldVisiableRect;

    QMap<QModelIndex, QRect> currentIndexRectMap {};
    QMap<QModelIndex, QRect> newIndexRectMap {};
    QMap<QModelIndex, QRect> oldIndexRectMap {};
    QMap<QModelIndex, QPixmap> indexPixmaps {};
    QModelIndex expandItemIndex {};
    QPixmap expandItemPixmap {};
    QPoint expandItemOffset {};

    QTimer *delayTimer { nullptr };

    QPropertyAnimation *animPtr { nullptr };

    double animProcess { 0.0 };

    FileView *view { nullptr };
};

}

#endif   // VIEWANIMATIONHELPER_H
