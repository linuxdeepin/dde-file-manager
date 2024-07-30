// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWANIMATIONHELPER_H
#define VIEWANIMATIONHELPER_H

#include <QObject>
#include <QMap>

QT_BEGIN_NAMESPACE
class QTimer;
class QLabel;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class FileView;
class ViewAnimationHelper : public QObject
{
    Q_OBJECT
public:
    explicit ViewAnimationHelper(FileView *parent);

    QRect getCurrentRectByIndex(const QModelIndex &index) const;
    void syncItemRect(const QModelIndex &index, const QRect &rect);
    void setNewItemRect(const QModelIndex &index, const QRect &rect);
    void isRectChanged(const QModelIndex &index, const QRect &newRect);
    bool checkColumnChanged(int newCount);
    void playViewAnimation();
    bool isInAnimationProccess() const;

public Q_SLOTS:
    void onProposeTimerFinish();
    void onAnimationTimerFinish();

private:
    void stopAnimation();
    void clearData();
    void clearLabels();
    void paintPixmaps();
    void createItemLabels();
    qreal easeOutExpo(qreal value) const;

    QMap<QModelIndex, QRect> currentIndexRectMap {};
    QMap<QModelIndex, QRect> newIndexRectMap {};
    QMap<QModelIndex, QRect> oldIndexRectMap {};
    QMap<QModelIndex, QPixmap> indexPixmaps {};
    QMap<QModelIndex, QLabel *> indexLabels {};

    int currentColumnCount { -1 };
    bool proposeToPlay { false };

    int animFrame { 0 };

    QTimer *proposeTimer { nullptr };
    QTimer *animationTimer { nullptr };

    FileView *view { nullptr };
    QLabel *blankBackground { nullptr };
};

}

#endif   // VIEWANIMATIONHELPER_H
