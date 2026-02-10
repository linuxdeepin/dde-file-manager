// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERLIST_H
#define WALLPAPERLIST_H

#include "ddplugin_wallpapersetting_global.h"
#include "wallpaperitem.h"

#include <DAnchors>
#include <DIconButton>

#include <QScrollArea>
#include <QPropertyAnimation>
#include <QTimer>

class QHBoxLayout;

namespace ddplugin_wallpapersetting {

class WallpaperList : public QScrollArea
{
    Q_OBJECT
public:
    explicit WallpaperList(QWidget *parent = 0);
    ~WallpaperList();
    void setMaskWidget(QWidget *);
    QWidget *removeMaskWidget();
    WallpaperItem *addItem(const QString &itemData);
    void removeItem(const QString &itemData);
    QSize gridSize() const;
    void setGridSize(const QSize &size);
    QWidget *itemAt(int x, int y) const;
    QWidget *itemAt(int idx) const;
    void clear();
    void setCurrentIndex(int index);
    WallpaperItem *currentItem() const;

public:
    inline QWidget *itemAt(const QPoint &pos) const
    {
        return itemAt(pos.x(), pos.y());
    }
    inline int count() const
    {
        return items.count();
    }
signals:
    void itemPressed(const QString &data) const;
    void mouseOverItemChanged(const QString &data, QPoint pos) const;
public slots:
    void prevPage();
    void nextPage();
    void updateItemThumb();
private slots:
    void onItemPressed(WallpaperItem *);
    void onItemHoverIn(WallpaperItem *);
    void onItemHoverOut(WallpaperItem *);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void updateBothEndsItem();
    void showDeleteButtonForItem(const WallpaperItem *item) const;
    void scrollList(int step, int duration = 100);

private:
    void init();

public:
    static const int kItemWidth;
    static const int kItemHeight;

private:
    DTK_WIDGET_NAMESPACE::DAnchors<DTK_WIDGET_NAMESPACE::DIconButton> prevButton = nullptr;
    DTK_WIDGET_NAMESPACE::DAnchors<DTK_WIDGET_NAMESPACE::DIconButton> nextButton = nullptr;

    QTimer *updateTimer = nullptr;
    QPropertyAnimation scrollAnimation;

    QWidget *contentWidget = nullptr;
    QHBoxLayout *contentLayout = nullptr;
    QList<WallpaperItem *> items;

    WallpaperItem *prevItem = nullptr;
    WallpaperItem *nextItem = nullptr;

    QSize grid;
    int currentIndex = 0;
};

}

#endif   // WALLPAPERLIST_H
