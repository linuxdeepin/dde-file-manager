/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef WALLPAPERLIST_H
#define WALLPAPERLIST_H

#include "ddplugin_wallpapersetting_global.h"
#include "wallpaperitem.h"

#include <DAnchors>
#include <DImageButton>

#include <QScrollArea>
#include <QPropertyAnimation>
#include <QTimer>

class QHBoxLayout;

DDP_WALLPAERSETTING_BEGIN_NAMESPACE

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
    inline QWidget *itemAt(const QPoint &pos) const {
        return itemAt(pos.x(), pos.y());
    }
    inline int count() const {
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
    DTK_WIDGET_NAMESPACE::DAnchors<DTK_WIDGET_NAMESPACE::DImageButton> prevButton = nullptr;
    DTK_WIDGET_NAMESPACE::DAnchors<DTK_WIDGET_NAMESPACE::DImageButton> nextButton = nullptr;
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

DDP_WALLPAERSETTING_END_NAMESPACE

#endif // WALLPAPERLIST_H
